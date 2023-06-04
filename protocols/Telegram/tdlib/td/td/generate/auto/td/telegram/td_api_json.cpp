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
    {"backgroundTypeFill", 993008684}
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
    {"chatAvailableReactionsAll", -537887666},
    {"chatAvailableReactionsSome", -640810821}
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
    {"chatMemberStatusMember", 844723285},
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

Result<int32> tl_constructor_from_string(td_api::ChatReportReason *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"chatReportReasonSpam", -510848863},
    {"chatReportReasonViolence", -1330235395},
    {"chatReportReasonPornography", 722614385},
    {"chatReportReasonChildAbuse", -1070686531},
    {"chatReportReasonCopyright", 986898080},
    {"chatReportReasonUnrelatedLocation", 2632403},
    {"chatReportReasonFake", -1713230446},
    {"chatReportReasonIllegalDrugs", -844539307},
    {"chatReportReasonPersonalDetails", 1121159029},
    {"chatReportReasonCustom", 1288925974}
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
    {"fileTypeProfilePhoto", 1795089315},
    {"fileTypeSecret", -1871899401},
    {"fileTypeSecretThumbnail", -1401326026},
    {"fileTypeSecure", -1419133146},
    {"fileTypeSticker", 475233385},
    {"fileTypeThumbnail", -12443298},
    {"fileTypeUnknown", -2011566768},
    {"fileTypeVideo", 1430816539},
    {"fileTypeVideoNote", -518412385},
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
    {"inputInvoiceName", -1312155917}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InputMessageContent *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputMessageText", 247050392},
    {"inputMessageAnimation", 850750601},
    {"inputMessageAudio", -626786126},
    {"inputMessageDocument", 1633383097},
    {"inputMessagePhoto", 1658132071},
    {"inputMessageSticker", 1072805625},
    {"inputMessageVideo", -1699308024},
    {"inputMessageVideoNote", 279108859},
    {"inputMessageVoiceNote", 2136519657},
    {"inputMessageLocation", 648735088},
    {"inputMessageVenue", 1447926269},
    {"inputMessageContact", -982446849},
    {"inputMessageDice", 841574313},
    {"inputMessageGame", 1252944610},
    {"inputMessageInvoice", 885857632},
    {"inputMessagePoll", 2054629900},
    {"inputMessageForwarded", 1696232440}
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

Result<int32> tl_constructor_from_string(td_api::InternalLinkType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"internalLinkTypeActiveSessions", 1886108589},
    {"internalLinkTypeAttachmentMenuBot", 1682719269},
    {"internalLinkTypeAuthenticationCode", -209235982},
    {"internalLinkTypeBackground", 185411848},
    {"internalLinkTypeBotAddToChannel", 1401602752},
    {"internalLinkTypeBotStart", 1066950637},
    {"internalLinkTypeBotStartInGroup", -905081650},
    {"internalLinkTypeChangePhoneNumber", -265856255},
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
    {"internalLinkTypeMessage", 978541650},
    {"internalLinkTypeMessageDraft", 661633749},
    {"internalLinkTypePassportDataRequest", -988819839},
    {"internalLinkTypePhoneNumberConfirmation", 1757375254},
    {"internalLinkTypePremiumFeatures", 1216892745},
    {"internalLinkTypePrivacyAndSecuritySettings", -1386255665},
    {"internalLinkTypeProxy", -1313788694},
    {"internalLinkTypePublicChat", -1485547717},
    {"internalLinkTypeQrCodeAuthentication", -1089332956},
    {"internalLinkTypeRestorePurchases", 606090371},
    {"internalLinkTypeSettings", 393561524},
    {"internalLinkTypeStickerSet", -1589227614},
    {"internalLinkTypeTheme", -200935417},
    {"internalLinkTypeThemeSettings", -1051903722},
    {"internalLinkTypeUnknownDeepLink", 625596379},
    {"internalLinkTypeUnsupportedProxy", -566649079},
    {"internalLinkTypeUserPhoneNumber", -1955751319},
    {"internalLinkTypeUserToken", -1462248615},
    {"internalLinkTypeVideoChat", -2020149068},
    {"internalLinkTypeWebApp", -57094065}
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
    {"keyboardButtonTypeRequestUser", -1996508112},
    {"keyboardButtonTypeRequestChat", 272541602},
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
    {"premiumFeatureRealTimeChatTranslation", -1143471488}
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
    {"premiumLimitTypeCaptionLength", 293984314},
    {"premiumLimitTypeBioLength", -1146976765},
    {"premiumLimitTypeChatFolderInviteLinkCount", -128702950},
    {"premiumLimitTypeShareableChatFolderCount", 1612625095}
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
    {"premiumSourceLink", 2135071132},
    {"premiumSourceSettings", -285702859}
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

Result<int32> tl_constructor_from_string(td_api::ReactionType *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"reactionTypeEmoji", -1942084920},
    {"reactionTypeCustomEmoji", -989117709}
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
    {"storePaymentPurposeGiftedPremium", 1916846289}
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
    {"suggestedActionSubscribeToAnnualPremium", 373913787}
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
    {"autoDownloadSettings", -813805749},
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
    {"botCommand", -1032140601},
    {"botCommandScopeDefault", 795652779},
    {"botCommandScopeAllPrivateChats", -344889543},
    {"botCommandScopeAllGroupChats", -981088162},
    {"botCommandScopeAllChatAdministrators", 1998329169},
    {"botCommandScopeChat", -430234971},
    {"botCommandScopeChatAdministrators", 1119682126},
    {"botCommandScopeChatMember", -211380494},
    {"botMenuButton", -944407322},
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
    {"chatAdministratorRights", -1384650041},
    {"chatAvailableReactionsAll", -537887666},
    {"chatAvailableReactionsSome", -640810821},
    {"chatEventLogFilters", 1648261677},
    {"chatFolder", -459081051},
    {"chatFolderIcon", -146104090},
    {"chatInviteLinkMember", 29156795},
    {"chatJoinRequest", 59341416},
    {"chatListMain", -400991316},
    {"chatListArchive", 362770115},
    {"chatListFolder", 385760856},
    {"chatLocation", -1566863583},
    {"chatMemberStatusCreator", -160019714},
    {"chatMemberStatusAdministrator", -70024163},
    {"chatMemberStatusMember", 844723285},
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
    {"chatNotificationSettings", 944322400},
    {"chatPermissions", -1355062837},
    {"chatPhotoSticker", -1459387485},
    {"chatPhotoStickerTypeRegularOrMask", -415147620},
    {"chatPhotoStickerTypeCustomEmoji", -266224943},
    {"chatReportReasonSpam", -510848863},
    {"chatReportReasonViolence", -1330235395},
    {"chatReportReasonPornography", 722614385},
    {"chatReportReasonChildAbuse", -1070686531},
    {"chatReportReasonCopyright", 986898080},
    {"chatReportReasonUnrelatedLocation", 2632403},
    {"chatReportReasonFake", -1713230446},
    {"chatReportReasonIllegalDrugs", -844539307},
    {"chatReportReasonPersonalDetails", 1121159029},
    {"chatReportReasonCustom", 1288925974},
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
    {"draftMessage", 1373050112},
    {"emailAddressAuthenticationCode", -993257022},
    {"emailAddressAuthenticationAppleId", 633948265},
    {"emailAddressAuthenticationGoogleId", -19142846},
    {"emojiCategoryTypeDefault", 1188782699},
    {"emojiCategoryTypeEmojiStatus", 1381282631},
    {"emojiCategoryTypeChatPhoto", 1059063081},
    {"emojiStatus", 1092133478},
    {"error", -1679978726},
    {"fileTypeNone", 2003009189},
    {"fileTypeAnimation", -290816582},
    {"fileTypeAudio", -709112160},
    {"fileTypeDocument", -564722929},
    {"fileTypeNotificationSound", -1020289271},
    {"fileTypePhoto", -1718914651},
    {"fileTypeProfilePhoto", 1795089315},
    {"fileTypeSecret", -1871899401},
    {"fileTypeSecretThumbnail", -1401326026},
    {"fileTypeSecure", -1419133146},
    {"fileTypeSticker", 475233385},
    {"fileTypeThumbnail", -12443298},
    {"fileTypeUnknown", -2011566768},
    {"fileTypeVideo", 1430816539},
    {"fileTypeVideoNote", -518412385},
    {"fileTypeVoiceNote", -588681661},
    {"fileTypeWallpaper", 1854930076},
    {"firebaseAuthenticationSettingsAndroid", -1771112932},
    {"firebaseAuthenticationSettingsIos", 222930116},
    {"formattedText", -252624564},
    {"forumTopicIcon", -818765421},
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
    {"inputIdentityDocument", -381776063},
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
    {"inputMessageText", 247050392},
    {"inputMessageAnimation", 850750601},
    {"inputMessageAudio", -626786126},
    {"inputMessageDocument", 1633383097},
    {"inputMessagePhoto", 1658132071},
    {"inputMessageSticker", 1072805625},
    {"inputMessageVideo", -1699308024},
    {"inputMessageVideoNote", 279108859},
    {"inputMessageVoiceNote", 2136519657},
    {"inputMessageLocation", 648735088},
    {"inputMessageVenue", 1447926269},
    {"inputMessageContact", -982446849},
    {"inputMessageDice", 841574313},
    {"inputMessageGame", 1252944610},
    {"inputMessageInvoice", 885857632},
    {"inputMessagePoll", 2054629900},
    {"inputMessageForwarded", 1696232440},
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
    {"inputSticker", 735226185},
    {"inputThumbnail", 1582387236},
    {"internalLinkTypeActiveSessions", 1886108589},
    {"internalLinkTypeAttachmentMenuBot", 1682719269},
    {"internalLinkTypeAuthenticationCode", -209235982},
    {"internalLinkTypeBackground", 185411848},
    {"internalLinkTypeBotAddToChannel", 1401602752},
    {"internalLinkTypeBotStart", 1066950637},
    {"internalLinkTypeBotStartInGroup", -905081650},
    {"internalLinkTypeChangePhoneNumber", -265856255},
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
    {"internalLinkTypeMessage", 978541650},
    {"internalLinkTypeMessageDraft", 661633749},
    {"internalLinkTypePassportDataRequest", -988819839},
    {"internalLinkTypePhoneNumberConfirmation", 1757375254},
    {"internalLinkTypePremiumFeatures", 1216892745},
    {"internalLinkTypePrivacyAndSecuritySettings", -1386255665},
    {"internalLinkTypeProxy", -1313788694},
    {"internalLinkTypePublicChat", -1485547717},
    {"internalLinkTypeQrCodeAuthentication", -1089332956},
    {"internalLinkTypeRestorePurchases", 606090371},
    {"internalLinkTypeSettings", 393561524},
    {"internalLinkTypeStickerSet", -1589227614},
    {"internalLinkTypeTheme", -200935417},
    {"internalLinkTypeThemeSettings", -1051903722},
    {"internalLinkTypeUnknownDeepLink", 625596379},
    {"internalLinkTypeUnsupportedProxy", -566649079},
    {"internalLinkTypeUserPhoneNumber", -1955751319},
    {"internalLinkTypeUserToken", -1462248615},
    {"internalLinkTypeVideoChat", -2020149068},
    {"internalLinkTypeWebApp", -57094065},
    {"invoice", 1977205639},
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
    {"keyboardButtonTypeRequestUser", -1996508112},
    {"keyboardButtonTypeRequestChat", 272541602},
    {"keyboardButtonTypeWebApp", 1892220770},
    {"labeledPricePart", 552789798},
    {"languagePackInfo", 542199642},
    {"languagePackString", 1307632736},
    {"languagePackStringValueOrdinary", -249256352},
    {"languagePackStringValuePluralized", 1906840261},
    {"languagePackStringValueDeleted", 1834792698},
    {"location", -443392141},
    {"logStreamDefault", 1390581436},
    {"logStreamFile", 1532136933},
    {"logStreamEmpty", -499912244},
    {"maskPointForehead", 1027512005},
    {"maskPointEyes", 1748310861},
    {"maskPointMouth", 411773406},
    {"maskPointChin", 534995335},
    {"maskPosition", -2097433026},
    {"messageAutoDeleteTime", 1972045589},
    {"messageCopyOptions", 1208442937},
    {"messageSchedulingStateSendAtDate", -1485570073},
    {"messageSchedulingStateSendWhenOnline", 2092947464},
    {"messageSendOptions", 32902046},
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
    {"messageSourceOther", 901818114},
    {"networkStatisticsEntryFile", 188452706},
    {"networkStatisticsEntryCall", 737000365},
    {"networkTypeNone", -1971691759},
    {"networkTypeMobile", 819228239},
    {"networkTypeMobileRoaming", -1435199760},
    {"networkTypeWiFi", -633872070},
    {"networkTypeOther", 1942128539},
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
    {"phoneNumberAuthenticationSettings", -493266762},
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
    {"premiumLimitTypeSupergroupCount", -247467131},
    {"premiumLimitTypePinnedChatCount", -998947871},
    {"premiumLimitTypeCreatedPublicChatCount", 446086841},
    {"premiumLimitTypeSavedAnimationCount", -19759735},
    {"premiumLimitTypeFavoriteStickerCount", 639754787},
    {"premiumLimitTypeChatFolderCount", 377489774},
    {"premiumLimitTypeChatFolderChosenChatCount", 1691435861},
    {"premiumLimitTypePinnedArchivedChatCount", 1485515276},
    {"premiumLimitTypeCaptionLength", 293984314},
    {"premiumLimitTypeBioLength", -1146976765},
    {"premiumLimitTypeChatFolderInviteLinkCount", -128702950},
    {"premiumLimitTypeShareableChatFolderCount", 1612625095},
    {"premiumSourceLimitExceeded", -2052159742},
    {"premiumSourceFeature", 445813541},
    {"premiumSourceLink", 2135071132},
    {"premiumSourceSettings", -285702859},
    {"proxyTypeSocks5", -890027341},
    {"proxyTypeHttp", -1547188361},
    {"proxyTypeMtproto", -1964826627},
    {"publicChatTypeHasUsername", 350789758},
    {"publicChatTypeIsLocationBased", 1183735952},
    {"reactionTypeEmoji", -1942084920},
    {"reactionTypeCustomEmoji", -989117709},
    {"replyMarkupRemoveKeyboard", -691252879},
    {"replyMarkupForceReply", 1101461919},
    {"replyMarkupShowKeyboard", -791495984},
    {"replyMarkupInlineKeyboard", -619317658},
    {"scopeAutosaveSettings", 1546821427},
    {"scopeNotificationSettings", -1383458661},
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
    {"stickerFormatWebp", -2123043040},
    {"stickerFormatTgs", 1614588662},
    {"stickerFormatWebm", -2070162097},
    {"stickerTypeRegular", 56345973},
    {"stickerTypeMask", -1765394796},
    {"stickerTypeCustomEmoji", -120752249},
    {"storePaymentPurposePremiumSubscription", 1263894804},
    {"storePaymentPurposeGiftedPremium", 1916846289},
    {"suggestedActionEnableArchiveAndMuteNewChats", 2017586255},
    {"suggestedActionCheckPassword", 1910534839},
    {"suggestedActionCheckPhoneNumber", 648771563},
    {"suggestedActionViewChecksHint", 891303239},
    {"suggestedActionConvertToBroadcastGroup", -965071304},
    {"suggestedActionSetPassword", 1863613848},
    {"suggestedActionUpgradePremium", 1890220539},
    {"suggestedActionSubscribeToAnnualPremium", 373913787},
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
    {"textEntityTypeTextUrl", 445719651},
    {"textEntityTypeMentionName", -1570974289},
    {"textEntityTypeCustomEmoji", 1724820677},
    {"textEntityTypeMediaTimestamp", -1841898992},
    {"textParseModeMarkdown", 360073407},
    {"textParseModeHTML", 1660208627},
    {"themeParameters", -644809106},
    {"topChatCategoryUsers", 1026706816},
    {"topChatCategoryBots", -1577129195},
    {"topChatCategoryGroups", 1530056846},
    {"topChatCategoryChannels", -500825885},
    {"topChatCategoryInlineBots", 377023356},
    {"topChatCategoryCalls", 356208861},
    {"topChatCategoryForwardChats", 1695922133},
    {"userPrivacySettingShowStatus", 1862829310},
    {"userPrivacySettingShowProfilePhoto", 1408485877},
    {"userPrivacySettingShowLinkInForwardedMessages", 592688870},
    {"userPrivacySettingShowPhoneNumber", -791567831},
    {"userPrivacySettingAllowChatInvites", 1271668007},
    {"userPrivacySettingAllowCalls", -906967291},
    {"userPrivacySettingAllowPeerToPeerCalls", 352500032},
    {"userPrivacySettingAllowFindingByPhoneNumber", -1846645423},
    {"userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages", 338112060},
    {"userPrivacySettingRuleAllowAll", -1967186881},
    {"userPrivacySettingRuleAllowContacts", -1892733680},
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
    {"addApplicationChangelog", -1946976311},
    {"addChatFolderByInviteLink", -858593816},
    {"addChatMember", 499426810},
    {"addChatMembers", -519951226},
    {"addChatToList", -80523595},
    {"addContact", 1869640000},
    {"addCustomServerLanguagePack", 4492771},
    {"addFavoriteSticker", 324504799},
    {"addFileToDownloads", 867533751},
    {"addLocalMessage", -1998878725},
    {"addLogMessage", 1597427692},
    {"addMessageReaction", 1419269613},
    {"addNetworkStatistics", 1264825305},
    {"addProxy", 331529432},
    {"addRecentSticker", -1478109026},
    {"addRecentlyFoundChat", -1746396787},
    {"addSavedAnimation", -1538525088},
    {"addSavedNotificationSound", 1043956975},
    {"addStickerToSet", 1457266235},
    {"answerCallbackQuery", -1153028490},
    {"answerCustomQuery", -1293603521},
    {"answerInlineQuery", 1343853844},
    {"answerPreCheckoutQuery", -1486789653},
    {"answerShippingQuery", -434601324},
    {"answerWebAppQuery", -1598776079},
    {"assignAppStoreTransaction", -2030892112},
    {"assignGooglePlayTransaction", -1992704860},
    {"banChatMember", -888111748},
    {"blockMessageSenderFromReplies", -1214384757},
    {"canPurchasePremium", -371319616},
    {"canTransferOwnership", 634602508},
    {"cancelDownloadFile", -1954524450},
    {"cancelPasswordReset", 940733538},
    {"cancelPreliminaryUploadFile", 823412414},
    {"changeImportedContacts", 1968207955},
    {"changePhoneNumber", -124666973},
    {"changeStickerSet", 449357293},
    {"checkAuthenticationBotToken", 639321206},
    {"checkAuthenticationCode", -302103382},
    {"checkAuthenticationEmailCode", -582827361},
    {"checkAuthenticationPassword", -2025698400},
    {"checkAuthenticationPasswordRecoveryCode", -603309083},
    {"checkChangePhoneNumberCode", -1720278429},
    {"checkChatFolderInviteLink", 522557851},
    {"checkChatInviteLink", -496940997},
    {"checkChatUsername", -119119344},
    {"checkCreatedPublicChatsLimit", -445546591},
    {"checkEmailAddressVerificationCode", -426386685},
    {"checkLoginEmailAddressCode", -1454244766},
    {"checkPasswordRecoveryCode", -200794600},
    {"checkPhoneNumberConfirmationCode", -1348060966},
    {"checkPhoneNumberVerificationCode", 1497462718},
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
    {"clickAnimatedEmojiMessage", 196179554},
    {"clickPremiumSubscriptionButton", -369319162},
    {"close", -1187782273},
    {"closeChat", 39749353},
    {"closeSecretChat", -471006133},
    {"closeWebApp", 1755391174},
    {"confirmQrCodeAuthentication", -376199379},
    {"createBasicGroupChat", 1972024548},
    {"createCall", -1104663024},
    {"createChatFolder", 1015399680},
    {"createChatFolderInviteLink", -2037911099},
    {"createChatInviteLink", 287744833},
    {"createForumTopic", -1040570140},
    {"createInvoiceLink", 216787233},
    {"createNewBasicGroupChat", 624404360},
    {"createNewSecretChat", -620682651},
    {"createNewStickerSet", 1057318406},
    {"createNewSupergroupChat", 804058822},
    {"createPrivateChat", -947758327},
    {"createSecretChat", 1930285615},
    {"createSupergroupChat", 1187475691},
    {"createTemporaryPassword", -1626509434},
    {"createVideoChat", 2124715405},
    {"deleteAccount", 1395816134},
    {"deleteAllCallMessages", -1466445325},
    {"deleteAllRevokedChatInviteLinks", 1112020698},
    {"deleteChat", -171253666},
    {"deleteChatFolder", -1956364551},
    {"deleteChatFolderInviteLink", -930057858},
    {"deleteChatHistory", -1472081761},
    {"deleteChatMessagesByDate", -1639653185},
    {"deleteChatMessagesBySender", -1164235161},
    {"deleteChatReplyMarkup", 100637531},
    {"deleteCommands", 1002732586},
    {"deleteFile", 1807653676},
    {"deleteForumTopic", 1864916152},
    {"deleteLanguagePack", -2108761026},
    {"deleteMessages", 1130090173},
    {"deletePassportElement", -1719555468},
    {"deleteProfilePhoto", 1319794625},
    {"deleteRevokedChatInviteLink", -1859711873},
    {"deleteSavedCredentials", 826300114},
    {"deleteSavedOrderInfo", 1629058164},
    {"deleteStickerSet", 1577745325},
    {"destroy", 685331274},
    {"disableAllSupergroupUsernames", 843511216},
    {"disableProxy", -2100095102},
    {"discardCall", -1784044162},
    {"disconnectAllWebsites", -1082985981},
    {"disconnectWebsite", -778767395},
    {"downloadFile", 1059402292},
    {"editChatFolder", 53672754},
    {"editChatFolderInviteLink", -2141872095},
    {"editChatInviteLink", 1320303996},
    {"editCustomLanguagePackInfo", 1320751257},
    {"editForumTopic", -1485402016},
    {"editInlineMessageCaption", -760985929},
    {"editInlineMessageLiveLocation", -156902912},
    {"editInlineMessageMedia", 23553921},
    {"editInlineMessageReplyMarkup", -67565858},
    {"editInlineMessageText", -855457307},
    {"editMessageCaption", 1154677038},
    {"editMessageLiveLocation", -14047982},
    {"editMessageMedia", -1152678125},
    {"editMessageReplyMarkup", 332127881},
    {"editMessageSchedulingState", -1372976192},
    {"editMessageText", 196272567},
    {"editProxy", -1605883821},
    {"enableProxy", 1494450838},
    {"endGroupCall", 573131959},
    {"endGroupCallRecording", -75799927},
    {"endGroupCallScreenSharing", -2047599540},
    {"finishFileGeneration", -1055060835},
    {"forwardMessages", -1054441908},
    {"getAccountTtl", -443905161},
    {"getActiveLiveLocationMessages", -1425459567},
    {"getActiveSessions", 1119710526},
    {"getAllPassportElements", -2038945045},
    {"getAnimatedEmoji", 1065635702},
    {"getApplicationConfig", -1823144318},
    {"getApplicationDownloadLink", 112013252},
    {"getArchivedStickerSets", 1001931341},
    {"getAttachedStickerSets", 1302172429},
    {"getAttachmentMenuBot", 1034248699},
    {"getAuthorizationState", 1949154877},
    {"getAutoDownloadSettingsPresets", -1721088201},
    {"getAutosaveSettings", 2136207914},
    {"getBackgroundUrl", 733769682},
    {"getBackgrounds", 249072633},
    {"getBankCardInfo", -1310515792},
    {"getBasicGroup", -1635174828},
    {"getBasicGroupFullInfo", -1822039253},
    {"getBlockedMessageSenders", 1947079776},
    {"getBotInfoDescription", -762841035},
    {"getBotInfoShortDescription", 1243358740},
    {"getBotName", -1707118036},
    {"getCallbackQueryAnswer", 116357727},
    {"getCallbackQueryMessage", -1121939086},
    {"getChat", 1866601536},
    {"getChatAdministrators", 1544468155},
    {"getChatAvailableMessageSenders", 1158670635},
    {"getChatEventLog", -1281344669},
    {"getChatFolder", 92809880},
    {"getChatFolderChatsToLeave", -1916672337},
    {"getChatFolderDefaultIconName", 754425959},
    {"getChatFolderInviteLinks", 329079776},
    {"getChatFolderNewChats", 2123181260},
    {"getChatHistory", -799960451},
    {"getChatInviteLink", -479575555},
    {"getChatInviteLinkCounts", 890299025},
    {"getChatInviteLinkMembers", -977921638},
    {"getChatInviteLinks", 883252396},
    {"getChatJoinRequests", -388428126},
    {"getChatListsToAddChat", 654956193},
    {"getChatMember", -792636814},
    {"getChatMessageByDate", 1062564150},
    {"getChatMessageCalendar", -326164204},
    {"getChatMessageCount", 205435308},
    {"getChatMessagePosition", -559474920},
    {"getChatNotificationSettingsExceptions", 201199121},
    {"getChatPinnedMessage", 359865008},
    {"getChatScheduledMessages", -549638149},
    {"getChatSparseMessagePositions", -462227580},
    {"getChatSponsoredMessages", 1353203864},
    {"getChatStatistics", 327057816},
    {"getChats", -972768574},
    {"getChatsForChatFolderInviteLink", 1873561929},
    {"getCommands", 1488621559},
    {"getConnectedWebsites", -170536110},
    {"getContacts", -1417722768},
    {"getCountries", -51902050},
    {"getCountryCode", 1540593906},
    {"getCreatedPublicChats", 710354415},
    {"getCurrentState", -1191417719},
    {"getCustomEmojiReactionAnimations", 1232375250},
    {"getCustomEmojiStickers", -2127427955},
    {"getDatabaseStatistics", -1942760263},
    {"getDeepLinkInfo", 680673150},
    {"getDefaultChatPhotoCustomEmojiStickers", -376342683},
    {"getDefaultEmojiStatuses", 618946243},
    {"getDefaultMessageAutoDeleteTime", -450857574},
    {"getDefaultProfilePhotoCustomEmojiStickers", 1280041655},
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
    {"getGroupCall", 1468491406},
    {"getGroupCallInviteLink", 719407396},
    {"getGroupCallStreamSegment", -2077959515},
    {"getGroupCallStreams", -1619226268},
    {"getGroupsInCommon", 381539178},
    {"getImportedContactCount", -656336346},
    {"getInactiveSupergroupChats", -657720907},
    {"getInlineGameHighScores", -533107798},
    {"getInlineQueryResults", 2044524652},
    {"getInstalledStickerSets", 1630467830},
    {"getInternalLink", 962654640},
    {"getInternalLinkType", -1948428535},
    {"getJsonString", 663458849},
    {"getJsonValue", -1829086715},
    {"getLanguagePackInfo", 2077809320},
    {"getLanguagePackString", 150789747},
    {"getLanguagePackStrings", 1246259088},
    {"getLocalizationTargetInfo", 1849499526},
    {"getLogStream", 1167608667},
    {"getLogTagVerbosityLevel", 951004547},
    {"getLogTags", -254449190},
    {"getLogVerbosityLevel", 594057956},
    {"getLoginUrl", 791844305},
    {"getLoginUrlInfo", -859202125},
    {"getMapThumbnailFile", -152660070},
    {"getMarkdownText", 164524584},
    {"getMe", -191516033},
    {"getMenuButton", -437324736},
    {"getMessage", -1821196160},
    {"getMessageAddedReactions", 2110172754},
    {"getMessageAvailableReactions", 1994098354},
    {"getMessageEmbeddingCode", 1654967561},
    {"getMessageFileType", -490270764},
    {"getMessageImportConfirmationText", 390627752},
    {"getMessageLink", -984158342},
    {"getMessageLinkInfo", -700533672},
    {"getMessageLocally", -603575444},
    {"getMessagePublicForwards", 1611049289},
    {"getMessageStatistics", 1270194648},
    {"getMessageThread", 2062695998},
    {"getMessageThreadHistory", -1808411608},
    {"getMessageViewers", -1584457010},
    {"getMessages", 425299338},
    {"getNetworkStatistics", -986228706},
    {"getOption", -1572495746},
    {"getPassportAuthorizationForm", 1636107398},
    {"getPassportAuthorizationFormAvailableElements", 1068700924},
    {"getPassportElement", -1882398342},
    {"getPasswordState", -174752904},
    {"getPaymentForm", -1924172076},
    {"getPaymentReceipt", 1013758294},
    {"getPhoneNumberInfo", -1608344583},
    {"getPhoneNumberInfoSync", 547061048},
    {"getPollVoters", 2075288734},
    {"getPreferredCountryLanguage", -933049386},
    {"getPremiumFeatures", -1260640695},
    {"getPremiumLimit", 1075313898},
    {"getPremiumState", 663632610},
    {"getPremiumStickerExamples", 1399442328},
    {"getPremiumStickers", -280950192},
    {"getProxies", -95026381},
    {"getProxyLink", -1054495112},
    {"getPushReceiverId", -286505294},
    {"getRecentEmojiStatuses", -1371914967},
    {"getRecentInlineBots", 1437823548},
    {"getRecentStickers", -579622241},
    {"getRecentlyOpenedChats", -1924156893},
    {"getRecentlyVisitedTMeUrls", 806754961},
    {"getRecommendedChatFolders", -145540217},
    {"getRecoveryEmailAddress", -1594770947},
    {"getRemoteFile", 2137204530},
    {"getRepliedMessage", -641918531},
    {"getSavedAnimations", 7051032},
    {"getSavedNotificationSound", 459569431},
    {"getSavedNotificationSounds", -1070305368},
    {"getSavedOrderInfo", -1152016675},
    {"getScopeNotificationSettings", -995613361},
    {"getSecretChat", 40599169},
    {"getStatisticalGraph", 1100975515},
    {"getStickerEmojis", -1895508665},
    {"getStickerSet", 1052318659},
    {"getStickers", 1158058819},
    {"getStorageStatistics", -853193929},
    {"getStorageStatisticsFast", 61368066},
    {"getSuggestedFileName", -2049399674},
    {"getSuggestedStickerSetName", -1340995520},
    {"getSuitableDiscussionChats", 49044982},
    {"getSupergroup", 989663458},
    {"getSupergroupFullInfo", 1099776056},
    {"getSupergroupMembers", -570940984},
    {"getSupportName", 1302205794},
    {"getSupportUser", -1733497700},
    {"getTemporaryPasswordState", -12670830},
    {"getTextEntities", -341490693},
    {"getThemeParametersJsonString", -1850145288},
    {"getThemedEmojiStatuses", 1791346882},
    {"getTopChats", -388410847},
    {"getTrendingStickerSets", -531085986},
    {"getUser", 1117363211},
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
    {"getWebPagePreview", 573441580},
    {"hideSuggestedAction", -1561384065},
    {"importContacts", -215132767},
    {"importMessages", -1864116784},
    {"inviteGroupCallParticipants", 1867097679},
    {"joinChat", 326769313},
    {"joinChatByInviteLink", -1049973882},
    {"joinGroupCall", -1043773467},
    {"leaveChat", -1825080735},
    {"leaveGroupCall", 980152233},
    {"loadChats", -1885635205},
    {"loadGroupCallParticipants", 938720974},
    {"logOut", -1581923301},
    {"openChat", -323371509},
    {"openMessageContent", -739088005},
    {"openWebApp", -763742925},
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
    {"recognizeSpeech", 1741947577},
    {"recoverAuthenticationPassword", -131001053},
    {"recoverPassword", -1524262541},
    {"registerDevice", 366088823},
    {"registerUser", -109994467},
    {"removeAllFilesFromDownloads", -1186433402},
    {"removeBackground", -1484545642},
    {"removeChatActionBar", -1650968070},
    {"removeContacts", 1943858054},
    {"removeFavoriteSticker", 1152945264},
    {"removeFileFromDownloads", 1460060142},
    {"removeMessageReaction", -1756934789},
    {"removeNotification", 862630734},
    {"removeNotificationGroup", 1713005454},
    {"removeProxy", 1369219847},
    {"removeRecentHashtag", -1013735260},
    {"removeRecentSticker", 1246577677},
    {"removeRecentlyFoundChat", 717340444},
    {"removeSavedAnimation", -495605479},
    {"removeSavedNotificationSound", -480032946},
    {"removeStickerFromSet", 1642196644},
    {"removeTopChat", -1907876267},
    {"reorderActiveBotUsernames", 1801166592},
    {"reorderActiveUsernames", -455399375},
    {"reorderChatFolders", 1665299546},
    {"reorderInstalledStickerSets", 1074928158},
    {"reorderSupergroupActiveUsernames", -1962466095},
    {"replacePrimaryChatInviteLink", 1067350941},
    {"replaceVideoChatRtmpUrl", 558862304},
    {"reportChat", -964543587},
    {"reportChatPhoto", 734652708},
    {"reportMessageReactions", 919111719},
    {"reportSupergroupAntiSpamFalsePositive", -516050872},
    {"reportSupergroupSpam", -94825000},
    {"requestAuthenticationPasswordRecovery", 1393896118},
    {"requestPasswordRecovery", -13777582},
    {"requestQrCodeAuthentication", 1363496527},
    {"resendAuthenticationCode", -814377191},
    {"resendChangePhoneNumberCode", -786772060},
    {"resendEmailAddressVerificationCode", -1872416732},
    {"resendLoginEmailAddressCode", 292966933},
    {"resendMessages", -940655817},
    {"resendPhoneNumberConfirmationCode", 2069068522},
    {"resendPhoneNumberVerificationCode", 1367629820},
    {"resendRecoveryEmailAddressCode", 433483548},
    {"resetAllNotificationSettings", -174020359},
    {"resetAuthenticationEmailAddress", -415075796},
    {"resetBackgrounds", 204852088},
    {"resetNetworkStatistics", 1646452102},
    {"resetPassword", -593589091},
    {"revokeChatInviteLink", -776514135},
    {"revokeGroupCallInviteLink", 501589140},
    {"saveApplicationLogEvent", -811154930},
    {"searchBackground", -2130996959},
    {"searchCallMessages", -1942229221},
    {"searchChatMembers", -445823291},
    {"searchChatMessages", 1879195132},
    {"searchChatRecentLocationMessages", 950238950},
    {"searchChats", -1879787060},
    {"searchChatsNearby", -196753377},
    {"searchChatsOnServer", -1158402188},
    {"searchContacts", -1794690715},
    {"searchEmojis", 398837927},
    {"searchFileDownloads", 706611286},
    {"searchHashtags", 1043637617},
    {"searchInstalledStickerSets", 2120122276},
    {"searchMessages", 714016935},
    {"searchOutgoingDocumentMessages", -1071397762},
    {"searchPublicChat", 857135533},
    {"searchPublicChats", 970385337},
    {"searchSecretMessages", -852865892},
    {"searchStickerSet", 1157930222},
    {"searchStickerSets", -1082314629},
    {"searchStickers", -1709577973},
    {"searchUserByPhoneNumber", -1562236142},
    {"searchUserByToken", -666766282},
    {"searchWebApp", -1241740747},
    {"sendAuthenticationFirebaseSms", 364994111},
    {"sendBotStartMessage", -1435877650},
    {"sendCallDebugInformation", 2019243839},
    {"sendCallLog", 1057638353},
    {"sendCallRating", -1402719502},
    {"sendCallSignalingData", 1412280732},
    {"sendChatAction", 2096947540},
    {"sendChatScreenshotTakenNotification", 448399457},
    {"sendCustomRequest", 285045153},
    {"sendEmailAddressVerificationCode", -221621379},
    {"sendInlineQueryResultMessage", -948639588},
    {"sendMessage", 960453021},
    {"sendMessageAlbum", -1639797862},
    {"sendPassportAuthorizationForm", 652160701},
    {"sendPaymentForm", -965855094},
    {"sendPhoneNumberConfirmationCode", -1901171495},
    {"sendPhoneNumberVerificationCode", 2081689035},
    {"sendWebAppData", -1423978996},
    {"setAccountTtl", 701389032},
    {"setAlarm", -873497067},
    {"setAuthenticationEmailAddress", 1773323522},
    {"setAuthenticationPhoneNumber", 868276259},
    {"setAutoDownloadSettings", -353671948},
    {"setAutosaveSettings", 6846656},
    {"setBackground", -1035439225},
    {"setBio", -1619582124},
    {"setBotInfoDescription", 693574984},
    {"setBotInfoShortDescription", 982956771},
    {"setBotName", -761922959},
    {"setBotProfilePhoto", -1115272346},
    {"setBotUpdatesStatus", -1154926191},
    {"setChatAvailableReactions", 267075078},
    {"setChatBackground", 1027545896},
    {"setChatClientData", -827119811},
    {"setChatDescription", 1957213277},
    {"setChatDiscussionGroup", -918801736},
    {"setChatDraftMessage", 1683889946},
    {"setChatLocation", -767091286},
    {"setChatMemberStatus", 81794847},
    {"setChatMessageAutoDeleteTime", -1505643265},
    {"setChatMessageSender", -1421513858},
    {"setChatNotificationSettings", 777199614},
    {"setChatPermissions", 2138507006},
    {"setChatPhoto", -377778941},
    {"setChatSlowModeDelay", -540350914},
    {"setChatTheme", -1895234925},
    {"setChatTitle", 164282047},
    {"setCommands", -907165606},
    {"setCustomEmojiStickerSetThumbnail", -1122836246},
    {"setCustomLanguagePack", -296742819},
    {"setCustomLanguagePackString", 1316365592},
    {"setDatabaseEncryptionKey", -1204599371},
    {"setDefaultChannelAdministratorRights", -234004967},
    {"setDefaultGroupAdministratorRights", 1700231016},
    {"setDefaultMessageAutoDeleteTime", -1772301460},
    {"setDefaultReactionType", 1694730813},
    {"setEmojiStatus", -1310680569},
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
    {"setName", 1711693584},
    {"setNetworkType", -701635234},
    {"setOption", 2114670322},
    {"setPassportElement", 2068173212},
    {"setPassportElementErrors", -2056754881},
    {"setPassword", -1193589027},
    {"setPinnedChats", -695640000},
    {"setPinnedForumTopics", -475084011},
    {"setPollAnswer", -1399388792},
    {"setProfilePhoto", -2048260627},
    {"setRecoveryEmailAddress", -1981836385},
    {"setScopeNotificationSettings", -2049984966},
    {"setStickerEmojis", -638843855},
    {"setStickerKeywords", 137223565},
    {"setStickerMaskPosition", 1202280912},
    {"setStickerPositionInSet", 2075281185},
    {"setStickerSetThumbnail", 1870737953},
    {"setStickerSetTitle", 1693004706},
    {"setSupergroupStickerSet", -2056344215},
    {"setSupergroupUsername", 1346325252},
    {"setTdlibParameters", 1384743481},
    {"setUserPersonalProfilePhoto", 464136438},
    {"setUserPrivacySettingRules", -473812741},
    {"setUserSupportInfo", -2088986621},
    {"setUsername", 439901214},
    {"setVideoChatDefaultParticipant", -240749901},
    {"shareChatWithBot", -1504507166},
    {"sharePhoneNumber", 1097130069},
    {"shareUserWithBot", -248315940},
    {"startGroupCallRecording", 1757774971},
    {"startGroupCallScreenSharing", -884068051},
    {"startScheduledGroupCall", 1519938838},
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
    {"toggleChatDefaultDisableNotification", 314794002},
    {"toggleChatHasProtectedContent", 975231309},
    {"toggleChatIsMarkedAsUnread", -986129697},
    {"toggleChatIsPinned", -1485429186},
    {"toggleChatIsTranslatable", -1812345889},
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
    {"toggleMessageSenderIsBlocked", -2008084779},
    {"toggleSessionCanAcceptCalls", 1819027208},
    {"toggleSessionCanAcceptSecretChats", 1000843390},
    {"toggleSupergroupHasAggressiveAntiSpamEnabled", 1748956943},
    {"toggleSupergroupHasHiddenMembers", -1537892918},
    {"toggleSupergroupIsAllHistoryAvailable", 1155110478},
    {"toggleSupergroupIsBroadcastGroup", 884089365},
    {"toggleSupergroupIsForum", -1771071990},
    {"toggleSupergroupJoinByRequest", 2111807454},
    {"toggleSupergroupJoinToSendMessages", -182022642},
    {"toggleSupergroupSignMessages", 1156568356},
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
  TRY_STATUS(from_json(to.days_, get_json_object_field_force(from, "days")));
  return Status::OK();
}

Status from_json(td_api::address &to, JsonObject &from) {
  TRY_STATUS(from_json(to.country_code_, get_json_object_field_force(from, "country_code")));
  TRY_STATUS(from_json(to.state_, get_json_object_field_force(from, "state")));
  TRY_STATUS(from_json(to.city_, get_json_object_field_force(from, "city")));
  TRY_STATUS(from_json(to.street_line1_, get_json_object_field_force(from, "street_line1")));
  TRY_STATUS(from_json(to.street_line2_, get_json_object_field_force(from, "street_line2")));
  TRY_STATUS(from_json(to.postal_code_, get_json_object_field_force(from, "postal_code")));
  return Status::OK();
}

Status from_json(td_api::autoDownloadSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_auto_download_enabled_, get_json_object_field_force(from, "is_auto_download_enabled")));
  TRY_STATUS(from_json(to.max_photo_file_size_, get_json_object_field_force(from, "max_photo_file_size")));
  TRY_STATUS(from_json(to.max_video_file_size_, get_json_object_field_force(from, "max_video_file_size")));
  TRY_STATUS(from_json(to.max_other_file_size_, get_json_object_field_force(from, "max_other_file_size")));
  TRY_STATUS(from_json(to.video_upload_bitrate_, get_json_object_field_force(from, "video_upload_bitrate")));
  TRY_STATUS(from_json(to.preload_large_videos_, get_json_object_field_force(from, "preload_large_videos")));
  TRY_STATUS(from_json(to.preload_next_audio_, get_json_object_field_force(from, "preload_next_audio")));
  TRY_STATUS(from_json(to.use_less_data_for_calls_, get_json_object_field_force(from, "use_less_data_for_calls")));
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
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::backgroundFillSolid &to, JsonObject &from) {
  TRY_STATUS(from_json(to.color_, get_json_object_field_force(from, "color")));
  return Status::OK();
}

Status from_json(td_api::backgroundFillGradient &to, JsonObject &from) {
  TRY_STATUS(from_json(to.top_color_, get_json_object_field_force(from, "top_color")));
  TRY_STATUS(from_json(to.bottom_color_, get_json_object_field_force(from, "bottom_color")));
  TRY_STATUS(from_json(to.rotation_angle_, get_json_object_field_force(from, "rotation_angle")));
  return Status::OK();
}

Status from_json(td_api::backgroundFillFreeformGradient &to, JsonObject &from) {
  TRY_STATUS(from_json(to.colors_, get_json_object_field_force(from, "colors")));
  return Status::OK();
}

Status from_json(td_api::backgroundTypeWallpaper &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_blurred_, get_json_object_field_force(from, "is_blurred")));
  TRY_STATUS(from_json(to.is_moving_, get_json_object_field_force(from, "is_moving")));
  return Status::OK();
}

Status from_json(td_api::backgroundTypePattern &to, JsonObject &from) {
  TRY_STATUS(from_json(to.fill_, get_json_object_field_force(from, "fill")));
  TRY_STATUS(from_json(to.intensity_, get_json_object_field_force(from, "intensity")));
  TRY_STATUS(from_json(to.is_inverted_, get_json_object_field_force(from, "is_inverted")));
  TRY_STATUS(from_json(to.is_moving_, get_json_object_field_force(from, "is_moving")));
  return Status::OK();
}

Status from_json(td_api::backgroundTypeFill &to, JsonObject &from) {
  TRY_STATUS(from_json(to.fill_, get_json_object_field_force(from, "fill")));
  return Status::OK();
}

Status from_json(td_api::botCommand &to, JsonObject &from) {
  TRY_STATUS(from_json(to.command_, get_json_object_field_force(from, "command")));
  TRY_STATUS(from_json(to.description_, get_json_object_field_force(from, "description")));
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
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::botCommandScopeChatAdministrators &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::botCommandScopeChatMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  return Status::OK();
}

Status from_json(td_api::botMenuButton &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, get_json_object_field_force(from, "text")));
  TRY_STATUS(from_json(to.url_, get_json_object_field_force(from, "url")));
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
  TRY_STATUS(from_json(to.udp_p2p_, get_json_object_field_force(from, "udp_p2p")));
  TRY_STATUS(from_json(to.udp_reflector_, get_json_object_field_force(from, "udp_reflector")));
  TRY_STATUS(from_json(to.min_layer_, get_json_object_field_force(from, "min_layer")));
  TRY_STATUS(from_json(to.max_layer_, get_json_object_field_force(from, "max_layer")));
  TRY_STATUS(from_json(to.library_versions_, get_json_object_field_force(from, "library_versions")));
  return Status::OK();
}

Status from_json(td_api::callbackQueryPayloadData &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.data_, get_json_object_field_force(from, "data")));
  return Status::OK();
}

Status from_json(td_api::callbackQueryPayloadDataWithPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.password_, get_json_object_field_force(from, "password")));
  TRY_STATUS(from_json_bytes(to.data_, get_json_object_field_force(from, "data")));
  return Status::OK();
}

Status from_json(td_api::callbackQueryPayloadGame &to, JsonObject &from) {
  TRY_STATUS(from_json(to.game_short_name_, get_json_object_field_force(from, "game_short_name")));
  return Status::OK();
}

Status from_json(td_api::chatActionTyping &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatActionRecordingVideo &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatActionUploadingVideo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.progress_, get_json_object_field_force(from, "progress")));
  return Status::OK();
}

Status from_json(td_api::chatActionRecordingVoiceNote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatActionUploadingVoiceNote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.progress_, get_json_object_field_force(from, "progress")));
  return Status::OK();
}

Status from_json(td_api::chatActionUploadingPhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.progress_, get_json_object_field_force(from, "progress")));
  return Status::OK();
}

Status from_json(td_api::chatActionUploadingDocument &to, JsonObject &from) {
  TRY_STATUS(from_json(to.progress_, get_json_object_field_force(from, "progress")));
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
  TRY_STATUS(from_json(to.progress_, get_json_object_field_force(from, "progress")));
  return Status::OK();
}

Status from_json(td_api::chatActionWatchingAnimations &to, JsonObject &from) {
  TRY_STATUS(from_json(to.emoji_, get_json_object_field_force(from, "emoji")));
  return Status::OK();
}

Status from_json(td_api::chatActionCancel &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatAdministratorRights &to, JsonObject &from) {
  TRY_STATUS(from_json(to.can_manage_chat_, get_json_object_field_force(from, "can_manage_chat")));
  TRY_STATUS(from_json(to.can_change_info_, get_json_object_field_force(from, "can_change_info")));
  TRY_STATUS(from_json(to.can_post_messages_, get_json_object_field_force(from, "can_post_messages")));
  TRY_STATUS(from_json(to.can_edit_messages_, get_json_object_field_force(from, "can_edit_messages")));
  TRY_STATUS(from_json(to.can_delete_messages_, get_json_object_field_force(from, "can_delete_messages")));
  TRY_STATUS(from_json(to.can_invite_users_, get_json_object_field_force(from, "can_invite_users")));
  TRY_STATUS(from_json(to.can_restrict_members_, get_json_object_field_force(from, "can_restrict_members")));
  TRY_STATUS(from_json(to.can_pin_messages_, get_json_object_field_force(from, "can_pin_messages")));
  TRY_STATUS(from_json(to.can_manage_topics_, get_json_object_field_force(from, "can_manage_topics")));
  TRY_STATUS(from_json(to.can_promote_members_, get_json_object_field_force(from, "can_promote_members")));
  TRY_STATUS(from_json(to.can_manage_video_chats_, get_json_object_field_force(from, "can_manage_video_chats")));
  TRY_STATUS(from_json(to.is_anonymous_, get_json_object_field_force(from, "is_anonymous")));
  return Status::OK();
}

Status from_json(td_api::chatAvailableReactionsAll &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatAvailableReactionsSome &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reactions_, get_json_object_field_force(from, "reactions")));
  return Status::OK();
}

Status from_json(td_api::chatEventLogFilters &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_edits_, get_json_object_field_force(from, "message_edits")));
  TRY_STATUS(from_json(to.message_deletions_, get_json_object_field_force(from, "message_deletions")));
  TRY_STATUS(from_json(to.message_pins_, get_json_object_field_force(from, "message_pins")));
  TRY_STATUS(from_json(to.member_joins_, get_json_object_field_force(from, "member_joins")));
  TRY_STATUS(from_json(to.member_leaves_, get_json_object_field_force(from, "member_leaves")));
  TRY_STATUS(from_json(to.member_invites_, get_json_object_field_force(from, "member_invites")));
  TRY_STATUS(from_json(to.member_promotions_, get_json_object_field_force(from, "member_promotions")));
  TRY_STATUS(from_json(to.member_restrictions_, get_json_object_field_force(from, "member_restrictions")));
  TRY_STATUS(from_json(to.info_changes_, get_json_object_field_force(from, "info_changes")));
  TRY_STATUS(from_json(to.setting_changes_, get_json_object_field_force(from, "setting_changes")));
  TRY_STATUS(from_json(to.invite_link_changes_, get_json_object_field_force(from, "invite_link_changes")));
  TRY_STATUS(from_json(to.video_chat_changes_, get_json_object_field_force(from, "video_chat_changes")));
  TRY_STATUS(from_json(to.forum_changes_, get_json_object_field_force(from, "forum_changes")));
  return Status::OK();
}

Status from_json(td_api::chatFolder &to, JsonObject &from) {
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  TRY_STATUS(from_json(to.icon_, get_json_object_field_force(from, "icon")));
  TRY_STATUS(from_json(to.is_shareable_, get_json_object_field_force(from, "is_shareable")));
  TRY_STATUS(from_json(to.pinned_chat_ids_, get_json_object_field_force(from, "pinned_chat_ids")));
  TRY_STATUS(from_json(to.included_chat_ids_, get_json_object_field_force(from, "included_chat_ids")));
  TRY_STATUS(from_json(to.excluded_chat_ids_, get_json_object_field_force(from, "excluded_chat_ids")));
  TRY_STATUS(from_json(to.exclude_muted_, get_json_object_field_force(from, "exclude_muted")));
  TRY_STATUS(from_json(to.exclude_read_, get_json_object_field_force(from, "exclude_read")));
  TRY_STATUS(from_json(to.exclude_archived_, get_json_object_field_force(from, "exclude_archived")));
  TRY_STATUS(from_json(to.include_contacts_, get_json_object_field_force(from, "include_contacts")));
  TRY_STATUS(from_json(to.include_non_contacts_, get_json_object_field_force(from, "include_non_contacts")));
  TRY_STATUS(from_json(to.include_bots_, get_json_object_field_force(from, "include_bots")));
  TRY_STATUS(from_json(to.include_groups_, get_json_object_field_force(from, "include_groups")));
  TRY_STATUS(from_json(to.include_channels_, get_json_object_field_force(from, "include_channels")));
  return Status::OK();
}

Status from_json(td_api::chatFolderIcon &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  return Status::OK();
}

Status from_json(td_api::chatInviteLinkMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.joined_chat_date_, get_json_object_field_force(from, "joined_chat_date")));
  TRY_STATUS(from_json(to.via_chat_folder_invite_link_, get_json_object_field_force(from, "via_chat_folder_invite_link")));
  TRY_STATUS(from_json(to.approver_user_id_, get_json_object_field_force(from, "approver_user_id")));
  return Status::OK();
}

Status from_json(td_api::chatJoinRequest &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.date_, get_json_object_field_force(from, "date")));
  TRY_STATUS(from_json(to.bio_, get_json_object_field_force(from, "bio")));
  return Status::OK();
}

Status from_json(td_api::chatListMain &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatListArchive &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatListFolder &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, get_json_object_field_force(from, "chat_folder_id")));
  return Status::OK();
}

Status from_json(td_api::chatLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, get_json_object_field_force(from, "location")));
  TRY_STATUS(from_json(to.address_, get_json_object_field_force(from, "address")));
  return Status::OK();
}

Status from_json(td_api::chatMemberStatusCreator &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_title_, get_json_object_field_force(from, "custom_title")));
  TRY_STATUS(from_json(to.is_anonymous_, get_json_object_field_force(from, "is_anonymous")));
  TRY_STATUS(from_json(to.is_member_, get_json_object_field_force(from, "is_member")));
  return Status::OK();
}

Status from_json(td_api::chatMemberStatusAdministrator &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_title_, get_json_object_field_force(from, "custom_title")));
  TRY_STATUS(from_json(to.can_be_edited_, get_json_object_field_force(from, "can_be_edited")));
  TRY_STATUS(from_json(to.rights_, get_json_object_field_force(from, "rights")));
  return Status::OK();
}

Status from_json(td_api::chatMemberStatusMember &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatMemberStatusRestricted &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_member_, get_json_object_field_force(from, "is_member")));
  TRY_STATUS(from_json(to.restricted_until_date_, get_json_object_field_force(from, "restricted_until_date")));
  TRY_STATUS(from_json(to.permissions_, get_json_object_field_force(from, "permissions")));
  return Status::OK();
}

Status from_json(td_api::chatMemberStatusLeft &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatMemberStatusBanned &to, JsonObject &from) {
  TRY_STATUS(from_json(to.banned_until_date_, get_json_object_field_force(from, "banned_until_date")));
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
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
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
  TRY_STATUS(from_json(to.use_default_mute_for_, get_json_object_field_force(from, "use_default_mute_for")));
  TRY_STATUS(from_json(to.mute_for_, get_json_object_field_force(from, "mute_for")));
  TRY_STATUS(from_json(to.use_default_sound_, get_json_object_field_force(from, "use_default_sound")));
  TRY_STATUS(from_json(to.sound_id_, get_json_object_field_force(from, "sound_id")));
  TRY_STATUS(from_json(to.use_default_show_preview_, get_json_object_field_force(from, "use_default_show_preview")));
  TRY_STATUS(from_json(to.show_preview_, get_json_object_field_force(from, "show_preview")));
  TRY_STATUS(from_json(to.use_default_disable_pinned_message_notifications_, get_json_object_field_force(from, "use_default_disable_pinned_message_notifications")));
  TRY_STATUS(from_json(to.disable_pinned_message_notifications_, get_json_object_field_force(from, "disable_pinned_message_notifications")));
  TRY_STATUS(from_json(to.use_default_disable_mention_notifications_, get_json_object_field_force(from, "use_default_disable_mention_notifications")));
  TRY_STATUS(from_json(to.disable_mention_notifications_, get_json_object_field_force(from, "disable_mention_notifications")));
  return Status::OK();
}

Status from_json(td_api::chatPermissions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.can_send_basic_messages_, get_json_object_field_force(from, "can_send_basic_messages")));
  TRY_STATUS(from_json(to.can_send_audios_, get_json_object_field_force(from, "can_send_audios")));
  TRY_STATUS(from_json(to.can_send_documents_, get_json_object_field_force(from, "can_send_documents")));
  TRY_STATUS(from_json(to.can_send_photos_, get_json_object_field_force(from, "can_send_photos")));
  TRY_STATUS(from_json(to.can_send_videos_, get_json_object_field_force(from, "can_send_videos")));
  TRY_STATUS(from_json(to.can_send_video_notes_, get_json_object_field_force(from, "can_send_video_notes")));
  TRY_STATUS(from_json(to.can_send_voice_notes_, get_json_object_field_force(from, "can_send_voice_notes")));
  TRY_STATUS(from_json(to.can_send_polls_, get_json_object_field_force(from, "can_send_polls")));
  TRY_STATUS(from_json(to.can_send_other_messages_, get_json_object_field_force(from, "can_send_other_messages")));
  TRY_STATUS(from_json(to.can_add_web_page_previews_, get_json_object_field_force(from, "can_add_web_page_previews")));
  TRY_STATUS(from_json(to.can_change_info_, get_json_object_field_force(from, "can_change_info")));
  TRY_STATUS(from_json(to.can_invite_users_, get_json_object_field_force(from, "can_invite_users")));
  TRY_STATUS(from_json(to.can_pin_messages_, get_json_object_field_force(from, "can_pin_messages")));
  TRY_STATUS(from_json(to.can_manage_topics_, get_json_object_field_force(from, "can_manage_topics")));
  return Status::OK();
}

Status from_json(td_api::chatPhotoSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  TRY_STATUS(from_json(to.background_fill_, get_json_object_field_force(from, "background_fill")));
  return Status::OK();
}

Status from_json(td_api::chatPhotoStickerTypeRegularOrMask &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_set_id_, get_json_object_field_force(from, "sticker_set_id")));
  TRY_STATUS(from_json(to.sticker_id_, get_json_object_field_force(from, "sticker_id")));
  return Status::OK();
}

Status from_json(td_api::chatPhotoStickerTypeCustomEmoji &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_emoji_id_, get_json_object_field_force(from, "custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::chatReportReasonSpam &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatReportReasonViolence &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatReportReasonPornography &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatReportReasonChildAbuse &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatReportReasonCopyright &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatReportReasonUnrelatedLocation &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatReportReasonFake &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatReportReasonIllegalDrugs &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatReportReasonPersonalDetails &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatReportReasonCustom &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::contact &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, get_json_object_field_force(from, "phone_number")));
  TRY_STATUS(from_json(to.first_name_, get_json_object_field_force(from, "first_name")));
  TRY_STATUS(from_json(to.last_name_, get_json_object_field_force(from, "last_name")));
  TRY_STATUS(from_json(to.vcard_, get_json_object_field_force(from, "vcard")));
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  return Status::OK();
}

Status from_json(td_api::date &to, JsonObject &from) {
  TRY_STATUS(from_json(to.day_, get_json_object_field_force(from, "day")));
  TRY_STATUS(from_json(to.month_, get_json_object_field_force(from, "month")));
  TRY_STATUS(from_json(to.year_, get_json_object_field_force(from, "year")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenFirebaseCloudMessaging &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, get_json_object_field_force(from, "token")));
  TRY_STATUS(from_json(to.encrypt_, get_json_object_field_force(from, "encrypt")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenApplePush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.device_token_, get_json_object_field_force(from, "device_token")));
  TRY_STATUS(from_json(to.is_app_sandbox_, get_json_object_field_force(from, "is_app_sandbox")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenApplePushVoIP &to, JsonObject &from) {
  TRY_STATUS(from_json(to.device_token_, get_json_object_field_force(from, "device_token")));
  TRY_STATUS(from_json(to.is_app_sandbox_, get_json_object_field_force(from, "is_app_sandbox")));
  TRY_STATUS(from_json(to.encrypt_, get_json_object_field_force(from, "encrypt")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenWindowsPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.access_token_, get_json_object_field_force(from, "access_token")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenMicrosoftPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.channel_uri_, get_json_object_field_force(from, "channel_uri")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenMicrosoftPushVoIP &to, JsonObject &from) {
  TRY_STATUS(from_json(to.channel_uri_, get_json_object_field_force(from, "channel_uri")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenWebPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.endpoint_, get_json_object_field_force(from, "endpoint")));
  TRY_STATUS(from_json(to.p256dh_base64url_, get_json_object_field_force(from, "p256dh_base64url")));
  TRY_STATUS(from_json(to.auth_base64url_, get_json_object_field_force(from, "auth_base64url")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenSimplePush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.endpoint_, get_json_object_field_force(from, "endpoint")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenUbuntuPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, get_json_object_field_force(from, "token")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenBlackBerryPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, get_json_object_field_force(from, "token")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenTizenPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reg_id_, get_json_object_field_force(from, "reg_id")));
  return Status::OK();
}

Status from_json(td_api::deviceTokenHuaweiPush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, get_json_object_field_force(from, "token")));
  TRY_STATUS(from_json(to.encrypt_, get_json_object_field_force(from, "encrypt")));
  return Status::OK();
}

Status from_json(td_api::draftMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reply_to_message_id_, get_json_object_field_force(from, "reply_to_message_id")));
  TRY_STATUS(from_json(to.date_, get_json_object_field_force(from, "date")));
  TRY_STATUS(from_json(to.input_message_text_, get_json_object_field_force(from, "input_message_text")));
  return Status::OK();
}

Status from_json(td_api::emailAddressAuthenticationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, get_json_object_field_force(from, "code")));
  return Status::OK();
}

Status from_json(td_api::emailAddressAuthenticationAppleId &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, get_json_object_field_force(from, "token")));
  return Status::OK();
}

Status from_json(td_api::emailAddressAuthenticationGoogleId &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, get_json_object_field_force(from, "token")));
  return Status::OK();
}

Status from_json(td_api::emojiCategoryTypeDefault &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::emojiCategoryTypeEmojiStatus &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::emojiCategoryTypeChatPhoto &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::emojiStatus &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_emoji_id_, get_json_object_field_force(from, "custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::error &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, get_json_object_field_force(from, "code")));
  TRY_STATUS(from_json(to.message_, get_json_object_field_force(from, "message")));
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
  TRY_STATUS(from_json(to.device_token_, get_json_object_field_force(from, "device_token")));
  TRY_STATUS(from_json(to.is_app_sandbox_, get_json_object_field_force(from, "is_app_sandbox")));
  return Status::OK();
}

Status from_json(td_api::formattedText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, get_json_object_field_force(from, "text")));
  TRY_STATUS(from_json(to.entities_, get_json_object_field_force(from, "entities")));
  return Status::OK();
}

Status from_json(td_api::forumTopicIcon &to, JsonObject &from) {
  TRY_STATUS(from_json(to.color_, get_json_object_field_force(from, "color")));
  TRY_STATUS(from_json(to.custom_emoji_id_, get_json_object_field_force(from, "custom_emoji_id")));
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
  TRY_STATUS(from_json(to.text_, get_json_object_field_force(from, "text")));
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, get_json_object_field_force(from, "url")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeLoginUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, get_json_object_field_force(from, "url")));
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  TRY_STATUS(from_json(to.forward_text_, get_json_object_field_force(from, "forward_text")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, get_json_object_field_force(from, "url")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeCallback &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.data_, get_json_object_field_force(from, "data")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeCallbackWithPassword &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.data_, get_json_object_field_force(from, "data")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeCallbackGame &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeSwitchInline &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  TRY_STATUS(from_json(to.target_chat_, get_json_object_field_force(from, "target_chat")));
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeBuy &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeUser &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  return Status::OK();
}

Status from_json(td_api::inlineQueryResultsButton &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, get_json_object_field_force(from, "text")));
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  return Status::OK();
}

Status from_json(td_api::inlineQueryResultsButtonTypeStartBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.parameter_, get_json_object_field_force(from, "parameter")));
  return Status::OK();
}

Status from_json(td_api::inlineQueryResultsButtonTypeWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, get_json_object_field_force(from, "url")));
  return Status::OK();
}

Status from_json(td_api::inputBackgroundLocal &to, JsonObject &from) {
  TRY_STATUS(from_json(to.background_, get_json_object_field_force(from, "background")));
  return Status::OK();
}

Status from_json(td_api::inputBackgroundRemote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.background_id_, get_json_object_field_force(from, "background_id")));
  return Status::OK();
}

Status from_json(td_api::inputBackgroundPrevious &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  return Status::OK();
}

Status from_json(td_api::inputChatPhotoPrevious &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_photo_id_, get_json_object_field_force(from, "chat_photo_id")));
  return Status::OK();
}

Status from_json(td_api::inputChatPhotoStatic &to, JsonObject &from) {
  TRY_STATUS(from_json(to.photo_, get_json_object_field_force(from, "photo")));
  return Status::OK();
}

Status from_json(td_api::inputChatPhotoAnimation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.animation_, get_json_object_field_force(from, "animation")));
  TRY_STATUS(from_json(to.main_frame_timestamp_, get_json_object_field_force(from, "main_frame_timestamp")));
  return Status::OK();
}

Status from_json(td_api::inputChatPhotoSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, get_json_object_field_force(from, "sticker")));
  return Status::OK();
}

Status from_json(td_api::inputCredentialsSaved &to, JsonObject &from) {
  TRY_STATUS(from_json(to.saved_credentials_id_, get_json_object_field_force(from, "saved_credentials_id")));
  return Status::OK();
}

Status from_json(td_api::inputCredentialsNew &to, JsonObject &from) {
  TRY_STATUS(from_json(to.data_, get_json_object_field_force(from, "data")));
  TRY_STATUS(from_json(to.allow_save_, get_json_object_field_force(from, "allow_save")));
  return Status::OK();
}

Status from_json(td_api::inputCredentialsApplePay &to, JsonObject &from) {
  TRY_STATUS(from_json(to.data_, get_json_object_field_force(from, "data")));
  return Status::OK();
}

Status from_json(td_api::inputCredentialsGooglePay &to, JsonObject &from) {
  TRY_STATUS(from_json(to.data_, get_json_object_field_force(from, "data")));
  return Status::OK();
}

Status from_json(td_api::inputFileId &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  return Status::OK();
}

Status from_json(td_api::inputFileRemote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  return Status::OK();
}

Status from_json(td_api::inputFileLocal &to, JsonObject &from) {
  TRY_STATUS(from_json(to.path_, get_json_object_field_force(from, "path")));
  return Status::OK();
}

Status from_json(td_api::inputFileGenerated &to, JsonObject &from) {
  TRY_STATUS(from_json(to.original_path_, get_json_object_field_force(from, "original_path")));
  TRY_STATUS(from_json(to.conversion_, get_json_object_field_force(from, "conversion")));
  TRY_STATUS(from_json(to.expected_size_, get_json_object_field_force(from, "expected_size")));
  return Status::OK();
}

Status from_json(td_api::inputIdentityDocument &to, JsonObject &from) {
  TRY_STATUS(from_json(to.number_, get_json_object_field_force(from, "number")));
  TRY_STATUS(from_json(to.expiry_date_, get_json_object_field_force(from, "expiry_date")));
  TRY_STATUS(from_json(to.front_side_, get_json_object_field_force(from, "front_side")));
  TRY_STATUS(from_json(to.reverse_side_, get_json_object_field_force(from, "reverse_side")));
  TRY_STATUS(from_json(to.selfie_, get_json_object_field_force(from, "selfie")));
  TRY_STATUS(from_json(to.translation_, get_json_object_field_force(from, "translation")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultAnimation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  TRY_STATUS(from_json(to.thumbnail_url_, get_json_object_field_force(from, "thumbnail_url")));
  TRY_STATUS(from_json(to.thumbnail_mime_type_, get_json_object_field_force(from, "thumbnail_mime_type")));
  TRY_STATUS(from_json(to.video_url_, get_json_object_field_force(from, "video_url")));
  TRY_STATUS(from_json(to.video_mime_type_, get_json_object_field_force(from, "video_mime_type")));
  TRY_STATUS(from_json(to.video_duration_, get_json_object_field_force(from, "video_duration")));
  TRY_STATUS(from_json(to.video_width_, get_json_object_field_force(from, "video_width")));
  TRY_STATUS(from_json(to.video_height_, get_json_object_field_force(from, "video_height")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, get_json_object_field_force(from, "input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultArticle &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  TRY_STATUS(from_json(to.url_, get_json_object_field_force(from, "url")));
  TRY_STATUS(from_json(to.hide_url_, get_json_object_field_force(from, "hide_url")));
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  TRY_STATUS(from_json(to.description_, get_json_object_field_force(from, "description")));
  TRY_STATUS(from_json(to.thumbnail_url_, get_json_object_field_force(from, "thumbnail_url")));
  TRY_STATUS(from_json(to.thumbnail_width_, get_json_object_field_force(from, "thumbnail_width")));
  TRY_STATUS(from_json(to.thumbnail_height_, get_json_object_field_force(from, "thumbnail_height")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, get_json_object_field_force(from, "input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultAudio &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  TRY_STATUS(from_json(to.performer_, get_json_object_field_force(from, "performer")));
  TRY_STATUS(from_json(to.audio_url_, get_json_object_field_force(from, "audio_url")));
  TRY_STATUS(from_json(to.audio_duration_, get_json_object_field_force(from, "audio_duration")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, get_json_object_field_force(from, "input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultContact &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  TRY_STATUS(from_json(to.contact_, get_json_object_field_force(from, "contact")));
  TRY_STATUS(from_json(to.thumbnail_url_, get_json_object_field_force(from, "thumbnail_url")));
  TRY_STATUS(from_json(to.thumbnail_width_, get_json_object_field_force(from, "thumbnail_width")));
  TRY_STATUS(from_json(to.thumbnail_height_, get_json_object_field_force(from, "thumbnail_height")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, get_json_object_field_force(from, "input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultDocument &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  TRY_STATUS(from_json(to.description_, get_json_object_field_force(from, "description")));
  TRY_STATUS(from_json(to.document_url_, get_json_object_field_force(from, "document_url")));
  TRY_STATUS(from_json(to.mime_type_, get_json_object_field_force(from, "mime_type")));
  TRY_STATUS(from_json(to.thumbnail_url_, get_json_object_field_force(from, "thumbnail_url")));
  TRY_STATUS(from_json(to.thumbnail_width_, get_json_object_field_force(from, "thumbnail_width")));
  TRY_STATUS(from_json(to.thumbnail_height_, get_json_object_field_force(from, "thumbnail_height")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, get_json_object_field_force(from, "input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultGame &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  TRY_STATUS(from_json(to.game_short_name_, get_json_object_field_force(from, "game_short_name")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  TRY_STATUS(from_json(to.location_, get_json_object_field_force(from, "location")));
  TRY_STATUS(from_json(to.live_period_, get_json_object_field_force(from, "live_period")));
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  TRY_STATUS(from_json(to.thumbnail_url_, get_json_object_field_force(from, "thumbnail_url")));
  TRY_STATUS(from_json(to.thumbnail_width_, get_json_object_field_force(from, "thumbnail_width")));
  TRY_STATUS(from_json(to.thumbnail_height_, get_json_object_field_force(from, "thumbnail_height")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, get_json_object_field_force(from, "input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultPhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  TRY_STATUS(from_json(to.description_, get_json_object_field_force(from, "description")));
  TRY_STATUS(from_json(to.thumbnail_url_, get_json_object_field_force(from, "thumbnail_url")));
  TRY_STATUS(from_json(to.photo_url_, get_json_object_field_force(from, "photo_url")));
  TRY_STATUS(from_json(to.photo_width_, get_json_object_field_force(from, "photo_width")));
  TRY_STATUS(from_json(to.photo_height_, get_json_object_field_force(from, "photo_height")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, get_json_object_field_force(from, "input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  TRY_STATUS(from_json(to.thumbnail_url_, get_json_object_field_force(from, "thumbnail_url")));
  TRY_STATUS(from_json(to.sticker_url_, get_json_object_field_force(from, "sticker_url")));
  TRY_STATUS(from_json(to.sticker_width_, get_json_object_field_force(from, "sticker_width")));
  TRY_STATUS(from_json(to.sticker_height_, get_json_object_field_force(from, "sticker_height")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, get_json_object_field_force(from, "input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultVenue &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  TRY_STATUS(from_json(to.venue_, get_json_object_field_force(from, "venue")));
  TRY_STATUS(from_json(to.thumbnail_url_, get_json_object_field_force(from, "thumbnail_url")));
  TRY_STATUS(from_json(to.thumbnail_width_, get_json_object_field_force(from, "thumbnail_width")));
  TRY_STATUS(from_json(to.thumbnail_height_, get_json_object_field_force(from, "thumbnail_height")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, get_json_object_field_force(from, "input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultVideo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  TRY_STATUS(from_json(to.description_, get_json_object_field_force(from, "description")));
  TRY_STATUS(from_json(to.thumbnail_url_, get_json_object_field_force(from, "thumbnail_url")));
  TRY_STATUS(from_json(to.video_url_, get_json_object_field_force(from, "video_url")));
  TRY_STATUS(from_json(to.mime_type_, get_json_object_field_force(from, "mime_type")));
  TRY_STATUS(from_json(to.video_width_, get_json_object_field_force(from, "video_width")));
  TRY_STATUS(from_json(to.video_height_, get_json_object_field_force(from, "video_height")));
  TRY_STATUS(from_json(to.video_duration_, get_json_object_field_force(from, "video_duration")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, get_json_object_field_force(from, "input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputInlineQueryResultVoiceNote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  TRY_STATUS(from_json(to.voice_note_url_, get_json_object_field_force(from, "voice_note_url")));
  TRY_STATUS(from_json(to.voice_note_duration_, get_json_object_field_force(from, "voice_note_duration")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, get_json_object_field_force(from, "input_message_content")));
  return Status::OK();
}

Status from_json(td_api::inputInvoiceMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  return Status::OK();
}

Status from_json(td_api::inputInvoiceName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  return Status::OK();
}

Status from_json(td_api::inputMessageText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, get_json_object_field_force(from, "text")));
  TRY_STATUS(from_json(to.disable_web_page_preview_, get_json_object_field_force(from, "disable_web_page_preview")));
  TRY_STATUS(from_json(to.clear_draft_, get_json_object_field_force(from, "clear_draft")));
  return Status::OK();
}

Status from_json(td_api::inputMessageAnimation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.animation_, get_json_object_field_force(from, "animation")));
  TRY_STATUS(from_json(to.thumbnail_, get_json_object_field_force(from, "thumbnail")));
  TRY_STATUS(from_json(to.added_sticker_file_ids_, get_json_object_field_force(from, "added_sticker_file_ids")));
  TRY_STATUS(from_json(to.duration_, get_json_object_field_force(from, "duration")));
  TRY_STATUS(from_json(to.width_, get_json_object_field_force(from, "width")));
  TRY_STATUS(from_json(to.height_, get_json_object_field_force(from, "height")));
  TRY_STATUS(from_json(to.caption_, get_json_object_field_force(from, "caption")));
  TRY_STATUS(from_json(to.has_spoiler_, get_json_object_field_force(from, "has_spoiler")));
  return Status::OK();
}

Status from_json(td_api::inputMessageAudio &to, JsonObject &from) {
  TRY_STATUS(from_json(to.audio_, get_json_object_field_force(from, "audio")));
  TRY_STATUS(from_json(to.album_cover_thumbnail_, get_json_object_field_force(from, "album_cover_thumbnail")));
  TRY_STATUS(from_json(to.duration_, get_json_object_field_force(from, "duration")));
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  TRY_STATUS(from_json(to.performer_, get_json_object_field_force(from, "performer")));
  TRY_STATUS(from_json(to.caption_, get_json_object_field_force(from, "caption")));
  return Status::OK();
}

Status from_json(td_api::inputMessageDocument &to, JsonObject &from) {
  TRY_STATUS(from_json(to.document_, get_json_object_field_force(from, "document")));
  TRY_STATUS(from_json(to.thumbnail_, get_json_object_field_force(from, "thumbnail")));
  TRY_STATUS(from_json(to.disable_content_type_detection_, get_json_object_field_force(from, "disable_content_type_detection")));
  TRY_STATUS(from_json(to.caption_, get_json_object_field_force(from, "caption")));
  return Status::OK();
}

Status from_json(td_api::inputMessagePhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.photo_, get_json_object_field_force(from, "photo")));
  TRY_STATUS(from_json(to.thumbnail_, get_json_object_field_force(from, "thumbnail")));
  TRY_STATUS(from_json(to.added_sticker_file_ids_, get_json_object_field_force(from, "added_sticker_file_ids")));
  TRY_STATUS(from_json(to.width_, get_json_object_field_force(from, "width")));
  TRY_STATUS(from_json(to.height_, get_json_object_field_force(from, "height")));
  TRY_STATUS(from_json(to.caption_, get_json_object_field_force(from, "caption")));
  TRY_STATUS(from_json(to.self_destruct_time_, get_json_object_field_force(from, "self_destruct_time")));
  TRY_STATUS(from_json(to.has_spoiler_, get_json_object_field_force(from, "has_spoiler")));
  return Status::OK();
}

Status from_json(td_api::inputMessageSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, get_json_object_field_force(from, "sticker")));
  TRY_STATUS(from_json(to.thumbnail_, get_json_object_field_force(from, "thumbnail")));
  TRY_STATUS(from_json(to.width_, get_json_object_field_force(from, "width")));
  TRY_STATUS(from_json(to.height_, get_json_object_field_force(from, "height")));
  TRY_STATUS(from_json(to.emoji_, get_json_object_field_force(from, "emoji")));
  return Status::OK();
}

Status from_json(td_api::inputMessageVideo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.video_, get_json_object_field_force(from, "video")));
  TRY_STATUS(from_json(to.thumbnail_, get_json_object_field_force(from, "thumbnail")));
  TRY_STATUS(from_json(to.added_sticker_file_ids_, get_json_object_field_force(from, "added_sticker_file_ids")));
  TRY_STATUS(from_json(to.duration_, get_json_object_field_force(from, "duration")));
  TRY_STATUS(from_json(to.width_, get_json_object_field_force(from, "width")));
  TRY_STATUS(from_json(to.height_, get_json_object_field_force(from, "height")));
  TRY_STATUS(from_json(to.supports_streaming_, get_json_object_field_force(from, "supports_streaming")));
  TRY_STATUS(from_json(to.caption_, get_json_object_field_force(from, "caption")));
  TRY_STATUS(from_json(to.self_destruct_time_, get_json_object_field_force(from, "self_destruct_time")));
  TRY_STATUS(from_json(to.has_spoiler_, get_json_object_field_force(from, "has_spoiler")));
  return Status::OK();
}

Status from_json(td_api::inputMessageVideoNote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.video_note_, get_json_object_field_force(from, "video_note")));
  TRY_STATUS(from_json(to.thumbnail_, get_json_object_field_force(from, "thumbnail")));
  TRY_STATUS(from_json(to.duration_, get_json_object_field_force(from, "duration")));
  TRY_STATUS(from_json(to.length_, get_json_object_field_force(from, "length")));
  return Status::OK();
}

Status from_json(td_api::inputMessageVoiceNote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.voice_note_, get_json_object_field_force(from, "voice_note")));
  TRY_STATUS(from_json(to.duration_, get_json_object_field_force(from, "duration")));
  TRY_STATUS(from_json_bytes(to.waveform_, get_json_object_field_force(from, "waveform")));
  TRY_STATUS(from_json(to.caption_, get_json_object_field_force(from, "caption")));
  return Status::OK();
}

Status from_json(td_api::inputMessageLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, get_json_object_field_force(from, "location")));
  TRY_STATUS(from_json(to.live_period_, get_json_object_field_force(from, "live_period")));
  TRY_STATUS(from_json(to.heading_, get_json_object_field_force(from, "heading")));
  TRY_STATUS(from_json(to.proximity_alert_radius_, get_json_object_field_force(from, "proximity_alert_radius")));
  return Status::OK();
}

Status from_json(td_api::inputMessageVenue &to, JsonObject &from) {
  TRY_STATUS(from_json(to.venue_, get_json_object_field_force(from, "venue")));
  return Status::OK();
}

Status from_json(td_api::inputMessageContact &to, JsonObject &from) {
  TRY_STATUS(from_json(to.contact_, get_json_object_field_force(from, "contact")));
  return Status::OK();
}

Status from_json(td_api::inputMessageDice &to, JsonObject &from) {
  TRY_STATUS(from_json(to.emoji_, get_json_object_field_force(from, "emoji")));
  TRY_STATUS(from_json(to.clear_draft_, get_json_object_field_force(from, "clear_draft")));
  return Status::OK();
}

Status from_json(td_api::inputMessageGame &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.game_short_name_, get_json_object_field_force(from, "game_short_name")));
  return Status::OK();
}

Status from_json(td_api::inputMessageInvoice &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invoice_, get_json_object_field_force(from, "invoice")));
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  TRY_STATUS(from_json(to.description_, get_json_object_field_force(from, "description")));
  TRY_STATUS(from_json(to.photo_url_, get_json_object_field_force(from, "photo_url")));
  TRY_STATUS(from_json(to.photo_size_, get_json_object_field_force(from, "photo_size")));
  TRY_STATUS(from_json(to.photo_width_, get_json_object_field_force(from, "photo_width")));
  TRY_STATUS(from_json(to.photo_height_, get_json_object_field_force(from, "photo_height")));
  TRY_STATUS(from_json_bytes(to.payload_, get_json_object_field_force(from, "payload")));
  TRY_STATUS(from_json(to.provider_token_, get_json_object_field_force(from, "provider_token")));
  TRY_STATUS(from_json(to.provider_data_, get_json_object_field_force(from, "provider_data")));
  TRY_STATUS(from_json(to.start_parameter_, get_json_object_field_force(from, "start_parameter")));
  TRY_STATUS(from_json(to.extended_media_content_, get_json_object_field_force(from, "extended_media_content")));
  return Status::OK();
}

Status from_json(td_api::inputMessagePoll &to, JsonObject &from) {
  TRY_STATUS(from_json(to.question_, get_json_object_field_force(from, "question")));
  TRY_STATUS(from_json(to.options_, get_json_object_field_force(from, "options")));
  TRY_STATUS(from_json(to.is_anonymous_, get_json_object_field_force(from, "is_anonymous")));
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  TRY_STATUS(from_json(to.open_period_, get_json_object_field_force(from, "open_period")));
  TRY_STATUS(from_json(to.close_date_, get_json_object_field_force(from, "close_date")));
  TRY_STATUS(from_json(to.is_closed_, get_json_object_field_force(from, "is_closed")));
  return Status::OK();
}

Status from_json(td_api::inputMessageForwarded &to, JsonObject &from) {
  TRY_STATUS(from_json(to.from_chat_id_, get_json_object_field_force(from, "from_chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.in_game_share_, get_json_object_field_force(from, "in_game_share")));
  TRY_STATUS(from_json(to.copy_options_, get_json_object_field_force(from, "copy_options")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementPersonalDetails &to, JsonObject &from) {
  TRY_STATUS(from_json(to.personal_details_, get_json_object_field_force(from, "personal_details")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementPassport &to, JsonObject &from) {
  TRY_STATUS(from_json(to.passport_, get_json_object_field_force(from, "passport")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementDriverLicense &to, JsonObject &from) {
  TRY_STATUS(from_json(to.driver_license_, get_json_object_field_force(from, "driver_license")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementIdentityCard &to, JsonObject &from) {
  TRY_STATUS(from_json(to.identity_card_, get_json_object_field_force(from, "identity_card")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementInternalPassport &to, JsonObject &from) {
  TRY_STATUS(from_json(to.internal_passport_, get_json_object_field_force(from, "internal_passport")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.address_, get_json_object_field_force(from, "address")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementUtilityBill &to, JsonObject &from) {
  TRY_STATUS(from_json(to.utility_bill_, get_json_object_field_force(from, "utility_bill")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementBankStatement &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bank_statement_, get_json_object_field_force(from, "bank_statement")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementRentalAgreement &to, JsonObject &from) {
  TRY_STATUS(from_json(to.rental_agreement_, get_json_object_field_force(from, "rental_agreement")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementPassportRegistration &to, JsonObject &from) {
  TRY_STATUS(from_json(to.passport_registration_, get_json_object_field_force(from, "passport_registration")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementTemporaryRegistration &to, JsonObject &from) {
  TRY_STATUS(from_json(to.temporary_registration_, get_json_object_field_force(from, "temporary_registration")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementPhoneNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, get_json_object_field_force(from, "phone_number")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementEmailAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.email_address_, get_json_object_field_force(from, "email_address")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementError &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  TRY_STATUS(from_json(to.message_, get_json_object_field_force(from, "message")));
  TRY_STATUS(from_json(to.source_, get_json_object_field_force(from, "source")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceUnspecified &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.element_hash_, get_json_object_field_force(from, "element_hash")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceDataField &to, JsonObject &from) {
  TRY_STATUS(from_json(to.field_name_, get_json_object_field_force(from, "field_name")));
  TRY_STATUS(from_json_bytes(to.data_hash_, get_json_object_field_force(from, "data_hash")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceFrontSide &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.file_hash_, get_json_object_field_force(from, "file_hash")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceReverseSide &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.file_hash_, get_json_object_field_force(from, "file_hash")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceSelfie &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.file_hash_, get_json_object_field_force(from, "file_hash")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceTranslationFile &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.file_hash_, get_json_object_field_force(from, "file_hash")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceTranslationFiles &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_hashes_, get_json_object_field_force(from, "file_hashes")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceFile &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.file_hash_, get_json_object_field_force(from, "file_hash")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceFiles &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_hashes_, get_json_object_field_force(from, "file_hashes")));
  return Status::OK();
}

Status from_json(td_api::inputPersonalDocument &to, JsonObject &from) {
  TRY_STATUS(from_json(to.files_, get_json_object_field_force(from, "files")));
  TRY_STATUS(from_json(to.translation_, get_json_object_field_force(from, "translation")));
  return Status::OK();
}

Status from_json(td_api::inputSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, get_json_object_field_force(from, "sticker")));
  TRY_STATUS(from_json(to.emojis_, get_json_object_field_force(from, "emojis")));
  TRY_STATUS(from_json(to.mask_position_, get_json_object_field_force(from, "mask_position")));
  TRY_STATUS(from_json(to.keywords_, get_json_object_field_force(from, "keywords")));
  return Status::OK();
}

Status from_json(td_api::inputThumbnail &to, JsonObject &from) {
  TRY_STATUS(from_json(to.thumbnail_, get_json_object_field_force(from, "thumbnail")));
  TRY_STATUS(from_json(to.width_, get_json_object_field_force(from, "width")));
  TRY_STATUS(from_json(to.height_, get_json_object_field_force(from, "height")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeActiveSessions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeAttachmentMenuBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.target_chat_, get_json_object_field_force(from, "target_chat")));
  TRY_STATUS(from_json(to.bot_username_, get_json_object_field_force(from, "bot_username")));
  TRY_STATUS(from_json(to.url_, get_json_object_field_force(from, "url")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeAuthenticationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, get_json_object_field_force(from, "code")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeBackground &to, JsonObject &from) {
  TRY_STATUS(from_json(to.background_name_, get_json_object_field_force(from, "background_name")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeBotAddToChannel &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_username_, get_json_object_field_force(from, "bot_username")));
  TRY_STATUS(from_json(to.administrator_rights_, get_json_object_field_force(from, "administrator_rights")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeBotStart &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_username_, get_json_object_field_force(from, "bot_username")));
  TRY_STATUS(from_json(to.start_parameter_, get_json_object_field_force(from, "start_parameter")));
  TRY_STATUS(from_json(to.autostart_, get_json_object_field_force(from, "autostart")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeBotStartInGroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_username_, get_json_object_field_force(from, "bot_username")));
  TRY_STATUS(from_json(to.start_parameter_, get_json_object_field_force(from, "start_parameter")));
  TRY_STATUS(from_json(to.administrator_rights_, get_json_object_field_force(from, "administrator_rights")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeChangePhoneNumber &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeChatFolderInvite &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, get_json_object_field_force(from, "invite_link")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeChatFolderSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeChatInvite &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, get_json_object_field_force(from, "invite_link")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeDefaultMessageAutoDeleteTimerSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeEditProfileSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeGame &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_username_, get_json_object_field_force(from, "bot_username")));
  TRY_STATUS(from_json(to.game_short_name_, get_json_object_field_force(from, "game_short_name")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeInstantView &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, get_json_object_field_force(from, "url")));
  TRY_STATUS(from_json(to.fallback_url_, get_json_object_field_force(from, "fallback_url")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeInvoice &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invoice_name_, get_json_object_field_force(from, "invoice_name")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeLanguagePack &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, get_json_object_field_force(from, "language_pack_id")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeLanguageSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, get_json_object_field_force(from, "url")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeMessageDraft &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, get_json_object_field_force(from, "text")));
  TRY_STATUS(from_json(to.contains_link_, get_json_object_field_force(from, "contains_link")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypePassportDataRequest &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.scope_, get_json_object_field_force(from, "scope")));
  TRY_STATUS(from_json(to.public_key_, get_json_object_field_force(from, "public_key")));
  TRY_STATUS(from_json(to.nonce_, get_json_object_field_force(from, "nonce")));
  TRY_STATUS(from_json(to.callback_url_, get_json_object_field_force(from, "callback_url")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypePhoneNumberConfirmation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.hash_, get_json_object_field_force(from, "hash")));
  TRY_STATUS(from_json(to.phone_number_, get_json_object_field_force(from, "phone_number")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypePremiumFeatures &to, JsonObject &from) {
  TRY_STATUS(from_json(to.referrer_, get_json_object_field_force(from, "referrer")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypePrivacyAndSecuritySettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeProxy &to, JsonObject &from) {
  TRY_STATUS(from_json(to.server_, get_json_object_field_force(from, "server")));
  TRY_STATUS(from_json(to.port_, get_json_object_field_force(from, "port")));
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypePublicChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_username_, get_json_object_field_force(from, "chat_username")));
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
  TRY_STATUS(from_json(to.sticker_set_name_, get_json_object_field_force(from, "sticker_set_name")));
  TRY_STATUS(from_json(to.expect_custom_emoji_, get_json_object_field_force(from, "expect_custom_emoji")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeTheme &to, JsonObject &from) {
  TRY_STATUS(from_json(to.theme_name_, get_json_object_field_force(from, "theme_name")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeThemeSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeUnknownDeepLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, get_json_object_field_force(from, "link")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeUnsupportedProxy &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeUserPhoneNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, get_json_object_field_force(from, "phone_number")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeUserToken &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, get_json_object_field_force(from, "token")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeVideoChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_username_, get_json_object_field_force(from, "chat_username")));
  TRY_STATUS(from_json(to.invite_hash_, get_json_object_field_force(from, "invite_hash")));
  TRY_STATUS(from_json(to.is_live_stream_, get_json_object_field_force(from, "is_live_stream")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_username_, get_json_object_field_force(from, "bot_username")));
  TRY_STATUS(from_json(to.web_app_short_name_, get_json_object_field_force(from, "web_app_short_name")));
  TRY_STATUS(from_json(to.start_parameter_, get_json_object_field_force(from, "start_parameter")));
  return Status::OK();
}

Status from_json(td_api::invoice &to, JsonObject &from) {
  TRY_STATUS(from_json(to.currency_, get_json_object_field_force(from, "currency")));
  TRY_STATUS(from_json(to.price_parts_, get_json_object_field_force(from, "price_parts")));
  TRY_STATUS(from_json(to.max_tip_amount_, get_json_object_field_force(from, "max_tip_amount")));
  TRY_STATUS(from_json(to.suggested_tip_amounts_, get_json_object_field_force(from, "suggested_tip_amounts")));
  TRY_STATUS(from_json(to.recurring_payment_terms_of_service_url_, get_json_object_field_force(from, "recurring_payment_terms_of_service_url")));
  TRY_STATUS(from_json(to.is_test_, get_json_object_field_force(from, "is_test")));
  TRY_STATUS(from_json(to.need_name_, get_json_object_field_force(from, "need_name")));
  TRY_STATUS(from_json(to.need_phone_number_, get_json_object_field_force(from, "need_phone_number")));
  TRY_STATUS(from_json(to.need_email_address_, get_json_object_field_force(from, "need_email_address")));
  TRY_STATUS(from_json(to.need_shipping_address_, get_json_object_field_force(from, "need_shipping_address")));
  TRY_STATUS(from_json(to.send_phone_number_to_provider_, get_json_object_field_force(from, "send_phone_number_to_provider")));
  TRY_STATUS(from_json(to.send_email_address_to_provider_, get_json_object_field_force(from, "send_email_address_to_provider")));
  TRY_STATUS(from_json(to.is_flexible_, get_json_object_field_force(from, "is_flexible")));
  return Status::OK();
}

Status from_json(td_api::jsonObjectMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.key_, get_json_object_field_force(from, "key")));
  TRY_STATUS(from_json(to.value_, get_json_object_field_force(from, "value")));
  return Status::OK();
}

Status from_json(td_api::jsonValueNull &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::jsonValueBoolean &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, get_json_object_field_force(from, "value")));
  return Status::OK();
}

Status from_json(td_api::jsonValueNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, get_json_object_field_force(from, "value")));
  return Status::OK();
}

Status from_json(td_api::jsonValueString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, get_json_object_field_force(from, "value")));
  return Status::OK();
}

Status from_json(td_api::jsonValueArray &to, JsonObject &from) {
  TRY_STATUS(from_json(to.values_, get_json_object_field_force(from, "values")));
  return Status::OK();
}

Status from_json(td_api::jsonValueObject &to, JsonObject &from) {
  TRY_STATUS(from_json(to.members_, get_json_object_field_force(from, "members")));
  return Status::OK();
}

Status from_json(td_api::keyboardButton &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, get_json_object_field_force(from, "text")));
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
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
  TRY_STATUS(from_json(to.force_regular_, get_json_object_field_force(from, "force_regular")));
  TRY_STATUS(from_json(to.force_quiz_, get_json_object_field_force(from, "force_quiz")));
  return Status::OK();
}

Status from_json(td_api::keyboardButtonTypeRequestUser &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  TRY_STATUS(from_json(to.restrict_user_is_bot_, get_json_object_field_force(from, "restrict_user_is_bot")));
  TRY_STATUS(from_json(to.user_is_bot_, get_json_object_field_force(from, "user_is_bot")));
  TRY_STATUS(from_json(to.restrict_user_is_premium_, get_json_object_field_force(from, "restrict_user_is_premium")));
  TRY_STATUS(from_json(to.user_is_premium_, get_json_object_field_force(from, "user_is_premium")));
  return Status::OK();
}

Status from_json(td_api::keyboardButtonTypeRequestChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  TRY_STATUS(from_json(to.chat_is_channel_, get_json_object_field_force(from, "chat_is_channel")));
  TRY_STATUS(from_json(to.restrict_chat_is_forum_, get_json_object_field_force(from, "restrict_chat_is_forum")));
  TRY_STATUS(from_json(to.chat_is_forum_, get_json_object_field_force(from, "chat_is_forum")));
  TRY_STATUS(from_json(to.restrict_chat_has_username_, get_json_object_field_force(from, "restrict_chat_has_username")));
  TRY_STATUS(from_json(to.chat_has_username_, get_json_object_field_force(from, "chat_has_username")));
  TRY_STATUS(from_json(to.chat_is_created_, get_json_object_field_force(from, "chat_is_created")));
  TRY_STATUS(from_json(to.user_administrator_rights_, get_json_object_field_force(from, "user_administrator_rights")));
  TRY_STATUS(from_json(to.bot_administrator_rights_, get_json_object_field_force(from, "bot_administrator_rights")));
  TRY_STATUS(from_json(to.bot_is_member_, get_json_object_field_force(from, "bot_is_member")));
  return Status::OK();
}

Status from_json(td_api::keyboardButtonTypeWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, get_json_object_field_force(from, "url")));
  return Status::OK();
}

Status from_json(td_api::labeledPricePart &to, JsonObject &from) {
  TRY_STATUS(from_json(to.label_, get_json_object_field_force(from, "label")));
  TRY_STATUS(from_json(to.amount_, get_json_object_field_force(from, "amount")));
  return Status::OK();
}

Status from_json(td_api::languagePackInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  TRY_STATUS(from_json(to.base_language_pack_id_, get_json_object_field_force(from, "base_language_pack_id")));
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  TRY_STATUS(from_json(to.native_name_, get_json_object_field_force(from, "native_name")));
  TRY_STATUS(from_json(to.plural_code_, get_json_object_field_force(from, "plural_code")));
  TRY_STATUS(from_json(to.is_official_, get_json_object_field_force(from, "is_official")));
  TRY_STATUS(from_json(to.is_rtl_, get_json_object_field_force(from, "is_rtl")));
  TRY_STATUS(from_json(to.is_beta_, get_json_object_field_force(from, "is_beta")));
  TRY_STATUS(from_json(to.is_installed_, get_json_object_field_force(from, "is_installed")));
  TRY_STATUS(from_json(to.total_string_count_, get_json_object_field_force(from, "total_string_count")));
  TRY_STATUS(from_json(to.translated_string_count_, get_json_object_field_force(from, "translated_string_count")));
  TRY_STATUS(from_json(to.local_string_count_, get_json_object_field_force(from, "local_string_count")));
  TRY_STATUS(from_json(to.translation_url_, get_json_object_field_force(from, "translation_url")));
  return Status::OK();
}

Status from_json(td_api::languagePackString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.key_, get_json_object_field_force(from, "key")));
  TRY_STATUS(from_json(to.value_, get_json_object_field_force(from, "value")));
  return Status::OK();
}

Status from_json(td_api::languagePackStringValueOrdinary &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, get_json_object_field_force(from, "value")));
  return Status::OK();
}

Status from_json(td_api::languagePackStringValuePluralized &to, JsonObject &from) {
  TRY_STATUS(from_json(to.zero_value_, get_json_object_field_force(from, "zero_value")));
  TRY_STATUS(from_json(to.one_value_, get_json_object_field_force(from, "one_value")));
  TRY_STATUS(from_json(to.two_value_, get_json_object_field_force(from, "two_value")));
  TRY_STATUS(from_json(to.few_value_, get_json_object_field_force(from, "few_value")));
  TRY_STATUS(from_json(to.many_value_, get_json_object_field_force(from, "many_value")));
  TRY_STATUS(from_json(to.other_value_, get_json_object_field_force(from, "other_value")));
  return Status::OK();
}

Status from_json(td_api::languagePackStringValueDeleted &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::location &to, JsonObject &from) {
  TRY_STATUS(from_json(to.latitude_, get_json_object_field_force(from, "latitude")));
  TRY_STATUS(from_json(to.longitude_, get_json_object_field_force(from, "longitude")));
  TRY_STATUS(from_json(to.horizontal_accuracy_, get_json_object_field_force(from, "horizontal_accuracy")));
  return Status::OK();
}

Status from_json(td_api::logStreamDefault &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::logStreamFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.path_, get_json_object_field_force(from, "path")));
  TRY_STATUS(from_json(to.max_file_size_, get_json_object_field_force(from, "max_file_size")));
  TRY_STATUS(from_json(to.redirect_stderr_, get_json_object_field_force(from, "redirect_stderr")));
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
  TRY_STATUS(from_json(to.point_, get_json_object_field_force(from, "point")));
  TRY_STATUS(from_json(to.x_shift_, get_json_object_field_force(from, "x_shift")));
  TRY_STATUS(from_json(to.y_shift_, get_json_object_field_force(from, "y_shift")));
  TRY_STATUS(from_json(to.scale_, get_json_object_field_force(from, "scale")));
  return Status::OK();
}

Status from_json(td_api::messageAutoDeleteTime &to, JsonObject &from) {
  TRY_STATUS(from_json(to.time_, get_json_object_field_force(from, "time")));
  return Status::OK();
}

Status from_json(td_api::messageCopyOptions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.send_copy_, get_json_object_field_force(from, "send_copy")));
  TRY_STATUS(from_json(to.replace_caption_, get_json_object_field_force(from, "replace_caption")));
  TRY_STATUS(from_json(to.new_caption_, get_json_object_field_force(from, "new_caption")));
  return Status::OK();
}

Status from_json(td_api::messageSchedulingStateSendAtDate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.send_date_, get_json_object_field_force(from, "send_date")));
  return Status::OK();
}

Status from_json(td_api::messageSchedulingStateSendWhenOnline &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSendOptions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.disable_notification_, get_json_object_field_force(from, "disable_notification")));
  TRY_STATUS(from_json(to.from_background_, get_json_object_field_force(from, "from_background")));
  TRY_STATUS(from_json(to.protect_content_, get_json_object_field_force(from, "protect_content")));
  TRY_STATUS(from_json(to.update_order_of_installed_sticker_sets_, get_json_object_field_force(from, "update_order_of_installed_sticker_sets")));
  TRY_STATUS(from_json(to.scheduling_state_, get_json_object_field_force(from, "scheduling_state")));
  TRY_STATUS(from_json(to.sending_id_, get_json_object_field_force(from, "sending_id")));
  return Status::OK();
}

Status from_json(td_api::messageSenderUser &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  return Status::OK();
}

Status from_json(td_api::messageSenderChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
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

Status from_json(td_api::messageSourceOther &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::networkStatisticsEntryFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_type_, get_json_object_field_force(from, "file_type")));
  TRY_STATUS(from_json(to.network_type_, get_json_object_field_force(from, "network_type")));
  TRY_STATUS(from_json(to.sent_bytes_, get_json_object_field_force(from, "sent_bytes")));
  TRY_STATUS(from_json(to.received_bytes_, get_json_object_field_force(from, "received_bytes")));
  return Status::OK();
}

Status from_json(td_api::networkStatisticsEntryCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.network_type_, get_json_object_field_force(from, "network_type")));
  TRY_STATUS(from_json(to.sent_bytes_, get_json_object_field_force(from, "sent_bytes")));
  TRY_STATUS(from_json(to.received_bytes_, get_json_object_field_force(from, "received_bytes")));
  TRY_STATUS(from_json(to.duration_, get_json_object_field_force(from, "duration")));
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
  TRY_STATUS(from_json(to.value_, get_json_object_field_force(from, "value")));
  return Status::OK();
}

Status from_json(td_api::optionValueEmpty &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::optionValueInteger &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, get_json_object_field_force(from, "value")));
  return Status::OK();
}

Status from_json(td_api::optionValueString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, get_json_object_field_force(from, "value")));
  return Status::OK();
}

Status from_json(td_api::orderInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  TRY_STATUS(from_json(to.phone_number_, get_json_object_field_force(from, "phone_number")));
  TRY_STATUS(from_json(to.email_address_, get_json_object_field_force(from, "email_address")));
  TRY_STATUS(from_json(to.shipping_address_, get_json_object_field_force(from, "shipping_address")));
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
  TRY_STATUS(from_json(to.first_name_, get_json_object_field_force(from, "first_name")));
  TRY_STATUS(from_json(to.middle_name_, get_json_object_field_force(from, "middle_name")));
  TRY_STATUS(from_json(to.last_name_, get_json_object_field_force(from, "last_name")));
  TRY_STATUS(from_json(to.native_first_name_, get_json_object_field_force(from, "native_first_name")));
  TRY_STATUS(from_json(to.native_middle_name_, get_json_object_field_force(from, "native_middle_name")));
  TRY_STATUS(from_json(to.native_last_name_, get_json_object_field_force(from, "native_last_name")));
  TRY_STATUS(from_json(to.birthdate_, get_json_object_field_force(from, "birthdate")));
  TRY_STATUS(from_json(to.gender_, get_json_object_field_force(from, "gender")));
  TRY_STATUS(from_json(to.country_code_, get_json_object_field_force(from, "country_code")));
  TRY_STATUS(from_json(to.residence_country_code_, get_json_object_field_force(from, "residence_country_code")));
  return Status::OK();
}

Status from_json(td_api::phoneNumberAuthenticationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.allow_flash_call_, get_json_object_field_force(from, "allow_flash_call")));
  TRY_STATUS(from_json(to.allow_missed_call_, get_json_object_field_force(from, "allow_missed_call")));
  TRY_STATUS(from_json(to.is_current_phone_number_, get_json_object_field_force(from, "is_current_phone_number")));
  TRY_STATUS(from_json(to.allow_sms_retriever_api_, get_json_object_field_force(from, "allow_sms_retriever_api")));
  TRY_STATUS(from_json(to.firebase_authentication_settings_, get_json_object_field_force(from, "firebase_authentication_settings")));
  TRY_STATUS(from_json(to.authentication_tokens_, get_json_object_field_force(from, "authentication_tokens")));
  return Status::OK();
}

Status from_json(td_api::pollTypeRegular &to, JsonObject &from) {
  TRY_STATUS(from_json(to.allow_multiple_answers_, get_json_object_field_force(from, "allow_multiple_answers")));
  return Status::OK();
}

Status from_json(td_api::pollTypeQuiz &to, JsonObject &from) {
  TRY_STATUS(from_json(to.correct_option_id_, get_json_object_field_force(from, "correct_option_id")));
  TRY_STATUS(from_json(to.explanation_, get_json_object_field_force(from, "explanation")));
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

Status from_json(td_api::premiumSourceLimitExceeded &to, JsonObject &from) {
  TRY_STATUS(from_json(to.limit_type_, get_json_object_field_force(from, "limit_type")));
  return Status::OK();
}

Status from_json(td_api::premiumSourceFeature &to, JsonObject &from) {
  TRY_STATUS(from_json(to.feature_, get_json_object_field_force(from, "feature")));
  return Status::OK();
}

Status from_json(td_api::premiumSourceLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.referrer_, get_json_object_field_force(from, "referrer")));
  return Status::OK();
}

Status from_json(td_api::premiumSourceSettings &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::proxyTypeSocks5 &to, JsonObject &from) {
  TRY_STATUS(from_json(to.username_, get_json_object_field_force(from, "username")));
  TRY_STATUS(from_json(to.password_, get_json_object_field_force(from, "password")));
  return Status::OK();
}

Status from_json(td_api::proxyTypeHttp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.username_, get_json_object_field_force(from, "username")));
  TRY_STATUS(from_json(to.password_, get_json_object_field_force(from, "password")));
  TRY_STATUS(from_json(to.http_only_, get_json_object_field_force(from, "http_only")));
  return Status::OK();
}

Status from_json(td_api::proxyTypeMtproto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.secret_, get_json_object_field_force(from, "secret")));
  return Status::OK();
}

Status from_json(td_api::publicChatTypeHasUsername &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::publicChatTypeIsLocationBased &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::reactionTypeEmoji &to, JsonObject &from) {
  TRY_STATUS(from_json(to.emoji_, get_json_object_field_force(from, "emoji")));
  return Status::OK();
}

Status from_json(td_api::reactionTypeCustomEmoji &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_emoji_id_, get_json_object_field_force(from, "custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::replyMarkupRemoveKeyboard &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_personal_, get_json_object_field_force(from, "is_personal")));
  return Status::OK();
}

Status from_json(td_api::replyMarkupForceReply &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_personal_, get_json_object_field_force(from, "is_personal")));
  TRY_STATUS(from_json(to.input_field_placeholder_, get_json_object_field_force(from, "input_field_placeholder")));
  return Status::OK();
}

Status from_json(td_api::replyMarkupShowKeyboard &to, JsonObject &from) {
  TRY_STATUS(from_json(to.rows_, get_json_object_field_force(from, "rows")));
  TRY_STATUS(from_json(to.is_persistent_, get_json_object_field_force(from, "is_persistent")));
  TRY_STATUS(from_json(to.resize_keyboard_, get_json_object_field_force(from, "resize_keyboard")));
  TRY_STATUS(from_json(to.one_time_, get_json_object_field_force(from, "one_time")));
  TRY_STATUS(from_json(to.is_personal_, get_json_object_field_force(from, "is_personal")));
  TRY_STATUS(from_json(to.input_field_placeholder_, get_json_object_field_force(from, "input_field_placeholder")));
  return Status::OK();
}

Status from_json(td_api::replyMarkupInlineKeyboard &to, JsonObject &from) {
  TRY_STATUS(from_json(to.rows_, get_json_object_field_force(from, "rows")));
  return Status::OK();
}

Status from_json(td_api::scopeAutosaveSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.autosave_photos_, get_json_object_field_force(from, "autosave_photos")));
  TRY_STATUS(from_json(to.autosave_videos_, get_json_object_field_force(from, "autosave_videos")));
  TRY_STATUS(from_json(to.max_video_file_size_, get_json_object_field_force(from, "max_video_file_size")));
  return Status::OK();
}

Status from_json(td_api::scopeNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.mute_for_, get_json_object_field_force(from, "mute_for")));
  TRY_STATUS(from_json(to.sound_id_, get_json_object_field_force(from, "sound_id")));
  TRY_STATUS(from_json(to.show_preview_, get_json_object_field_force(from, "show_preview")));
  TRY_STATUS(from_json(to.disable_pinned_message_notifications_, get_json_object_field_force(from, "disable_pinned_message_notifications")));
  TRY_STATUS(from_json(to.disable_mention_notifications_, get_json_object_field_force(from, "disable_mention_notifications")));
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
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  TRY_STATUS(from_json(to.price_parts_, get_json_object_field_force(from, "price_parts")));
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
  TRY_STATUS(from_json(to.is_restore_, get_json_object_field_force(from, "is_restore")));
  TRY_STATUS(from_json(to.is_upgrade_, get_json_object_field_force(from, "is_upgrade")));
  return Status::OK();
}

Status from_json(td_api::storePaymentPurposeGiftedPremium &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.currency_, get_json_object_field_force(from, "currency")));
  TRY_STATUS(from_json(to.amount_, get_json_object_field_force(from, "amount")));
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
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  return Status::OK();
}

Status from_json(td_api::suggestedActionSetPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.authorization_delay_, get_json_object_field_force(from, "authorization_delay")));
  return Status::OK();
}

Status from_json(td_api::suggestedActionUpgradePremium &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::suggestedActionSubscribeToAnnualPremium &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterRecent &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterContacts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterAdministrators &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterSearch &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterRestricted &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterBanned &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterMention &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterBots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::targetChatCurrent &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::targetChatChosen &to, JsonObject &from) {
  TRY_STATUS(from_json(to.allow_user_chats_, get_json_object_field_force(from, "allow_user_chats")));
  TRY_STATUS(from_json(to.allow_bot_chats_, get_json_object_field_force(from, "allow_bot_chats")));
  TRY_STATUS(from_json(to.allow_group_chats_, get_json_object_field_force(from, "allow_group_chats")));
  TRY_STATUS(from_json(to.allow_channel_chats_, get_json_object_field_force(from, "allow_channel_chats")));
  return Status::OK();
}

Status from_json(td_api::targetChatInternalLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, get_json_object_field_force(from, "link")));
  return Status::OK();
}

Status from_json(td_api::testInt &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, get_json_object_field_force(from, "value")));
  return Status::OK();
}

Status from_json(td_api::testString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, get_json_object_field_force(from, "value")));
  return Status::OK();
}

Status from_json(td_api::textEntity &to, JsonObject &from) {
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  TRY_STATUS(from_json(to.length_, get_json_object_field_force(from, "length")));
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
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
  TRY_STATUS(from_json(to.language_, get_json_object_field_force(from, "language")));
  return Status::OK();
}

Status from_json(td_api::textEntityTypeTextUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, get_json_object_field_force(from, "url")));
  return Status::OK();
}

Status from_json(td_api::textEntityTypeMentionName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  return Status::OK();
}

Status from_json(td_api::textEntityTypeCustomEmoji &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_emoji_id_, get_json_object_field_force(from, "custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::textEntityTypeMediaTimestamp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.media_timestamp_, get_json_object_field_force(from, "media_timestamp")));
  return Status::OK();
}

Status from_json(td_api::textParseModeMarkdown &to, JsonObject &from) {
  TRY_STATUS(from_json(to.version_, get_json_object_field_force(from, "version")));
  return Status::OK();
}

Status from_json(td_api::textParseModeHTML &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::themeParameters &to, JsonObject &from) {
  TRY_STATUS(from_json(to.background_color_, get_json_object_field_force(from, "background_color")));
  TRY_STATUS(from_json(to.secondary_background_color_, get_json_object_field_force(from, "secondary_background_color")));
  TRY_STATUS(from_json(to.text_color_, get_json_object_field_force(from, "text_color")));
  TRY_STATUS(from_json(to.hint_color_, get_json_object_field_force(from, "hint_color")));
  TRY_STATUS(from_json(to.link_color_, get_json_object_field_force(from, "link_color")));
  TRY_STATUS(from_json(to.button_color_, get_json_object_field_force(from, "button_color")));
  TRY_STATUS(from_json(to.button_text_color_, get_json_object_field_force(from, "button_text_color")));
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

Status from_json(td_api::userPrivacySettingRuleAllowUsers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_ids_, get_json_object_field_force(from, "user_ids")));
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleAllowChatMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_ids_, get_json_object_field_force(from, "chat_ids")));
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleRestrictAll &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleRestrictContacts &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleRestrictUsers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_ids_, get_json_object_field_force(from, "user_ids")));
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleRestrictChatMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_ids_, get_json_object_field_force(from, "chat_ids")));
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRules &to, JsonObject &from) {
  TRY_STATUS(from_json(to.rules_, get_json_object_field_force(from, "rules")));
  return Status::OK();
}

Status from_json(td_api::venue &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, get_json_object_field_force(from, "location")));
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  TRY_STATUS(from_json(to.address_, get_json_object_field_force(from, "address")));
  TRY_STATUS(from_json(to.provider_, get_json_object_field_force(from, "provider")));
  TRY_STATUS(from_json(to.id_, get_json_object_field_force(from, "id")));
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  return Status::OK();
}

Status from_json(td_api::acceptCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.call_id_, get_json_object_field_force(from, "call_id")));
  TRY_STATUS(from_json(to.protocol_, get_json_object_field_force(from, "protocol")));
  return Status::OK();
}

Status from_json(td_api::acceptTermsOfService &to, JsonObject &from) {
  TRY_STATUS(from_json(to.terms_of_service_id_, get_json_object_field_force(from, "terms_of_service_id")));
  return Status::OK();
}

Status from_json(td_api::addApplicationChangelog &to, JsonObject &from) {
  TRY_STATUS(from_json(to.previous_application_version_, get_json_object_field_force(from, "previous_application_version")));
  return Status::OK();
}

Status from_json(td_api::addChatFolderByInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, get_json_object_field_force(from, "invite_link")));
  TRY_STATUS(from_json(to.chat_ids_, get_json_object_field_force(from, "chat_ids")));
  return Status::OK();
}

Status from_json(td_api::addChatMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.forward_limit_, get_json_object_field_force(from, "forward_limit")));
  return Status::OK();
}

Status from_json(td_api::addChatMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.user_ids_, get_json_object_field_force(from, "user_ids")));
  return Status::OK();
}

Status from_json(td_api::addChatToList &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.chat_list_, get_json_object_field_force(from, "chat_list")));
  return Status::OK();
}

Status from_json(td_api::addContact &to, JsonObject &from) {
  TRY_STATUS(from_json(to.contact_, get_json_object_field_force(from, "contact")));
  TRY_STATUS(from_json(to.share_phone_number_, get_json_object_field_force(from, "share_phone_number")));
  return Status::OK();
}

Status from_json(td_api::addCustomServerLanguagePack &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, get_json_object_field_force(from, "language_pack_id")));
  return Status::OK();
}

Status from_json(td_api::addFavoriteSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, get_json_object_field_force(from, "sticker")));
  return Status::OK();
}

Status from_json(td_api::addFileToDownloads &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, get_json_object_field_force(from, "file_id")));
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.priority_, get_json_object_field_force(from, "priority")));
  return Status::OK();
}

Status from_json(td_api::addLocalMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.sender_id_, get_json_object_field_force(from, "sender_id")));
  TRY_STATUS(from_json(to.reply_to_message_id_, get_json_object_field_force(from, "reply_to_message_id")));
  TRY_STATUS(from_json(to.disable_notification_, get_json_object_field_force(from, "disable_notification")));
  TRY_STATUS(from_json(to.input_message_content_, get_json_object_field_force(from, "input_message_content")));
  return Status::OK();
}

Status from_json(td_api::addLogMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.verbosity_level_, get_json_object_field_force(from, "verbosity_level")));
  TRY_STATUS(from_json(to.text_, get_json_object_field_force(from, "text")));
  return Status::OK();
}

Status from_json(td_api::addMessageReaction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.reaction_type_, get_json_object_field_force(from, "reaction_type")));
  TRY_STATUS(from_json(to.is_big_, get_json_object_field_force(from, "is_big")));
  TRY_STATUS(from_json(to.update_recent_reactions_, get_json_object_field_force(from, "update_recent_reactions")));
  return Status::OK();
}

Status from_json(td_api::addNetworkStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.entry_, get_json_object_field_force(from, "entry")));
  return Status::OK();
}

Status from_json(td_api::addProxy &to, JsonObject &from) {
  TRY_STATUS(from_json(to.server_, get_json_object_field_force(from, "server")));
  TRY_STATUS(from_json(to.port_, get_json_object_field_force(from, "port")));
  TRY_STATUS(from_json(to.enable_, get_json_object_field_force(from, "enable")));
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  return Status::OK();
}

Status from_json(td_api::addRecentSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_attached_, get_json_object_field_force(from, "is_attached")));
  TRY_STATUS(from_json(to.sticker_, get_json_object_field_force(from, "sticker")));
  return Status::OK();
}

Status from_json(td_api::addRecentlyFoundChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::addSavedAnimation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.animation_, get_json_object_field_force(from, "animation")));
  return Status::OK();
}

Status from_json(td_api::addSavedNotificationSound &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sound_, get_json_object_field_force(from, "sound")));
  return Status::OK();
}

Status from_json(td_api::addStickerToSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  TRY_STATUS(from_json(to.sticker_, get_json_object_field_force(from, "sticker")));
  return Status::OK();
}

Status from_json(td_api::answerCallbackQuery &to, JsonObject &from) {
  TRY_STATUS(from_json(to.callback_query_id_, get_json_object_field_force(from, "callback_query_id")));
  TRY_STATUS(from_json(to.text_, get_json_object_field_force(from, "text")));
  TRY_STATUS(from_json(to.show_alert_, get_json_object_field_force(from, "show_alert")));
  TRY_STATUS(from_json(to.url_, get_json_object_field_force(from, "url")));
  TRY_STATUS(from_json(to.cache_time_, get_json_object_field_force(from, "cache_time")));
  return Status::OK();
}

Status from_json(td_api::answerCustomQuery &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_query_id_, get_json_object_field_force(from, "custom_query_id")));
  TRY_STATUS(from_json(to.data_, get_json_object_field_force(from, "data")));
  return Status::OK();
}

Status from_json(td_api::answerInlineQuery &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_query_id_, get_json_object_field_force(from, "inline_query_id")));
  TRY_STATUS(from_json(to.is_personal_, get_json_object_field_force(from, "is_personal")));
  TRY_STATUS(from_json(to.button_, get_json_object_field_force(from, "button")));
  TRY_STATUS(from_json(to.results_, get_json_object_field_force(from, "results")));
  TRY_STATUS(from_json(to.cache_time_, get_json_object_field_force(from, "cache_time")));
  TRY_STATUS(from_json(to.next_offset_, get_json_object_field_force(from, "next_offset")));
  return Status::OK();
}

Status from_json(td_api::answerPreCheckoutQuery &to, JsonObject &from) {
  TRY_STATUS(from_json(to.pre_checkout_query_id_, get_json_object_field_force(from, "pre_checkout_query_id")));
  TRY_STATUS(from_json(to.error_message_, get_json_object_field_force(from, "error_message")));
  return Status::OK();
}

Status from_json(td_api::answerShippingQuery &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shipping_query_id_, get_json_object_field_force(from, "shipping_query_id")));
  TRY_STATUS(from_json(to.shipping_options_, get_json_object_field_force(from, "shipping_options")));
  TRY_STATUS(from_json(to.error_message_, get_json_object_field_force(from, "error_message")));
  return Status::OK();
}

Status from_json(td_api::answerWebAppQuery &to, JsonObject &from) {
  TRY_STATUS(from_json(to.web_app_query_id_, get_json_object_field_force(from, "web_app_query_id")));
  TRY_STATUS(from_json(to.result_, get_json_object_field_force(from, "result")));
  return Status::OK();
}

Status from_json(td_api::assignAppStoreTransaction &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.receipt_, get_json_object_field_force(from, "receipt")));
  TRY_STATUS(from_json(to.purpose_, get_json_object_field_force(from, "purpose")));
  return Status::OK();
}

Status from_json(td_api::assignGooglePlayTransaction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.package_name_, get_json_object_field_force(from, "package_name")));
  TRY_STATUS(from_json(to.store_product_id_, get_json_object_field_force(from, "store_product_id")));
  TRY_STATUS(from_json(to.purchase_token_, get_json_object_field_force(from, "purchase_token")));
  TRY_STATUS(from_json(to.purpose_, get_json_object_field_force(from, "purpose")));
  return Status::OK();
}

Status from_json(td_api::banChatMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.member_id_, get_json_object_field_force(from, "member_id")));
  TRY_STATUS(from_json(to.banned_until_date_, get_json_object_field_force(from, "banned_until_date")));
  TRY_STATUS(from_json(to.revoke_messages_, get_json_object_field_force(from, "revoke_messages")));
  return Status::OK();
}

Status from_json(td_api::blockMessageSenderFromReplies &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.delete_message_, get_json_object_field_force(from, "delete_message")));
  TRY_STATUS(from_json(to.delete_all_messages_, get_json_object_field_force(from, "delete_all_messages")));
  TRY_STATUS(from_json(to.report_spam_, get_json_object_field_force(from, "report_spam")));
  return Status::OK();
}

Status from_json(td_api::canPurchasePremium &to, JsonObject &from) {
  TRY_STATUS(from_json(to.purpose_, get_json_object_field_force(from, "purpose")));
  return Status::OK();
}

Status from_json(td_api::canTransferOwnership &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::cancelDownloadFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, get_json_object_field_force(from, "file_id")));
  TRY_STATUS(from_json(to.only_if_pending_, get_json_object_field_force(from, "only_if_pending")));
  return Status::OK();
}

Status from_json(td_api::cancelPasswordReset &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::cancelPreliminaryUploadFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, get_json_object_field_force(from, "file_id")));
  return Status::OK();
}

Status from_json(td_api::changeImportedContacts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.contacts_, get_json_object_field_force(from, "contacts")));
  return Status::OK();
}

Status from_json(td_api::changePhoneNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, get_json_object_field_force(from, "phone_number")));
  TRY_STATUS(from_json(to.settings_, get_json_object_field_force(from, "settings")));
  return Status::OK();
}

Status from_json(td_api::changeStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.set_id_, get_json_object_field_force(from, "set_id")));
  TRY_STATUS(from_json(to.is_installed_, get_json_object_field_force(from, "is_installed")));
  TRY_STATUS(from_json(to.is_archived_, get_json_object_field_force(from, "is_archived")));
  return Status::OK();
}

Status from_json(td_api::checkAuthenticationBotToken &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, get_json_object_field_force(from, "token")));
  return Status::OK();
}

Status from_json(td_api::checkAuthenticationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, get_json_object_field_force(from, "code")));
  return Status::OK();
}

Status from_json(td_api::checkAuthenticationEmailCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, get_json_object_field_force(from, "code")));
  return Status::OK();
}

Status from_json(td_api::checkAuthenticationPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.password_, get_json_object_field_force(from, "password")));
  return Status::OK();
}

Status from_json(td_api::checkAuthenticationPasswordRecoveryCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.recovery_code_, get_json_object_field_force(from, "recovery_code")));
  return Status::OK();
}

Status from_json(td_api::checkChangePhoneNumberCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, get_json_object_field_force(from, "code")));
  return Status::OK();
}

Status from_json(td_api::checkChatFolderInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, get_json_object_field_force(from, "invite_link")));
  return Status::OK();
}

Status from_json(td_api::checkChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, get_json_object_field_force(from, "invite_link")));
  return Status::OK();
}

Status from_json(td_api::checkChatUsername &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.username_, get_json_object_field_force(from, "username")));
  return Status::OK();
}

Status from_json(td_api::checkCreatedPublicChatsLimit &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  return Status::OK();
}

Status from_json(td_api::checkEmailAddressVerificationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, get_json_object_field_force(from, "code")));
  return Status::OK();
}

Status from_json(td_api::checkLoginEmailAddressCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, get_json_object_field_force(from, "code")));
  return Status::OK();
}

Status from_json(td_api::checkPasswordRecoveryCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.recovery_code_, get_json_object_field_force(from, "recovery_code")));
  return Status::OK();
}

Status from_json(td_api::checkPhoneNumberConfirmationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, get_json_object_field_force(from, "code")));
  return Status::OK();
}

Status from_json(td_api::checkPhoneNumberVerificationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, get_json_object_field_force(from, "code")));
  return Status::OK();
}

Status from_json(td_api::checkRecoveryEmailAddressCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, get_json_object_field_force(from, "code")));
  return Status::OK();
}

Status from_json(td_api::checkStickerSetName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  return Status::OK();
}

Status from_json(td_api::cleanFileName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_name_, get_json_object_field_force(from, "file_name")));
  return Status::OK();
}

Status from_json(td_api::clearAllDraftMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.exclude_secret_chats_, get_json_object_field_force(from, "exclude_secret_chats")));
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
  TRY_STATUS(from_json(to.is_attached_, get_json_object_field_force(from, "is_attached")));
  return Status::OK();
}

Status from_json(td_api::clearRecentlyFoundChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::clickAnimatedEmojiMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  return Status::OK();
}

Status from_json(td_api::clickPremiumSubscriptionButton &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::close &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::closeChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::closeSecretChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.secret_chat_id_, get_json_object_field_force(from, "secret_chat_id")));
  return Status::OK();
}

Status from_json(td_api::closeWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.web_app_launch_id_, get_json_object_field_force(from, "web_app_launch_id")));
  return Status::OK();
}

Status from_json(td_api::confirmQrCodeAuthentication &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, get_json_object_field_force(from, "link")));
  return Status::OK();
}

Status from_json(td_api::createBasicGroupChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.basic_group_id_, get_json_object_field_force(from, "basic_group_id")));
  TRY_STATUS(from_json(to.force_, get_json_object_field_force(from, "force")));
  return Status::OK();
}

Status from_json(td_api::createCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.protocol_, get_json_object_field_force(from, "protocol")));
  TRY_STATUS(from_json(to.is_video_, get_json_object_field_force(from, "is_video")));
  return Status::OK();
}

Status from_json(td_api::createChatFolder &to, JsonObject &from) {
  TRY_STATUS(from_json(to.folder_, get_json_object_field_force(from, "folder")));
  return Status::OK();
}

Status from_json(td_api::createChatFolderInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, get_json_object_field_force(from, "chat_folder_id")));
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  TRY_STATUS(from_json(to.chat_ids_, get_json_object_field_force(from, "chat_ids")));
  return Status::OK();
}

Status from_json(td_api::createChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  TRY_STATUS(from_json(to.expiration_date_, get_json_object_field_force(from, "expiration_date")));
  TRY_STATUS(from_json(to.member_limit_, get_json_object_field_force(from, "member_limit")));
  TRY_STATUS(from_json(to.creates_join_request_, get_json_object_field_force(from, "creates_join_request")));
  return Status::OK();
}

Status from_json(td_api::createForumTopic &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  TRY_STATUS(from_json(to.icon_, get_json_object_field_force(from, "icon")));
  return Status::OK();
}

Status from_json(td_api::createInvoiceLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invoice_, get_json_object_field_force(from, "invoice")));
  return Status::OK();
}

Status from_json(td_api::createNewBasicGroupChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_ids_, get_json_object_field_force(from, "user_ids")));
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  TRY_STATUS(from_json(to.message_auto_delete_time_, get_json_object_field_force(from, "message_auto_delete_time")));
  return Status::OK();
}

Status from_json(td_api::createNewSecretChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  return Status::OK();
}

Status from_json(td_api::createNewStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  TRY_STATUS(from_json(to.sticker_format_, get_json_object_field_force(from, "sticker_format")));
  TRY_STATUS(from_json(to.sticker_type_, get_json_object_field_force(from, "sticker_type")));
  TRY_STATUS(from_json(to.needs_repainting_, get_json_object_field_force(from, "needs_repainting")));
  TRY_STATUS(from_json(to.stickers_, get_json_object_field_force(from, "stickers")));
  TRY_STATUS(from_json(to.source_, get_json_object_field_force(from, "source")));
  return Status::OK();
}

Status from_json(td_api::createNewSupergroupChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  TRY_STATUS(from_json(to.is_forum_, get_json_object_field_force(from, "is_forum")));
  TRY_STATUS(from_json(to.is_channel_, get_json_object_field_force(from, "is_channel")));
  TRY_STATUS(from_json(to.description_, get_json_object_field_force(from, "description")));
  TRY_STATUS(from_json(to.location_, get_json_object_field_force(from, "location")));
  TRY_STATUS(from_json(to.message_auto_delete_time_, get_json_object_field_force(from, "message_auto_delete_time")));
  TRY_STATUS(from_json(to.for_import_, get_json_object_field_force(from, "for_import")));
  return Status::OK();
}

Status from_json(td_api::createPrivateChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.force_, get_json_object_field_force(from, "force")));
  return Status::OK();
}

Status from_json(td_api::createSecretChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.secret_chat_id_, get_json_object_field_force(from, "secret_chat_id")));
  return Status::OK();
}

Status from_json(td_api::createSupergroupChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  TRY_STATUS(from_json(to.force_, get_json_object_field_force(from, "force")));
  return Status::OK();
}

Status from_json(td_api::createTemporaryPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.password_, get_json_object_field_force(from, "password")));
  TRY_STATUS(from_json(to.valid_for_, get_json_object_field_force(from, "valid_for")));
  return Status::OK();
}

Status from_json(td_api::createVideoChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  TRY_STATUS(from_json(to.start_date_, get_json_object_field_force(from, "start_date")));
  TRY_STATUS(from_json(to.is_rtmp_stream_, get_json_object_field_force(from, "is_rtmp_stream")));
  return Status::OK();
}

Status from_json(td_api::deleteAccount &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reason_, get_json_object_field_force(from, "reason")));
  TRY_STATUS(from_json(to.password_, get_json_object_field_force(from, "password")));
  return Status::OK();
}

Status from_json(td_api::deleteAllCallMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.revoke_, get_json_object_field_force(from, "revoke")));
  return Status::OK();
}

Status from_json(td_api::deleteAllRevokedChatInviteLinks &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.creator_user_id_, get_json_object_field_force(from, "creator_user_id")));
  return Status::OK();
}

Status from_json(td_api::deleteChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::deleteChatFolder &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, get_json_object_field_force(from, "chat_folder_id")));
  TRY_STATUS(from_json(to.leave_chat_ids_, get_json_object_field_force(from, "leave_chat_ids")));
  return Status::OK();
}

Status from_json(td_api::deleteChatFolderInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, get_json_object_field_force(from, "chat_folder_id")));
  TRY_STATUS(from_json(to.invite_link_, get_json_object_field_force(from, "invite_link")));
  return Status::OK();
}

Status from_json(td_api::deleteChatHistory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.remove_from_chat_list_, get_json_object_field_force(from, "remove_from_chat_list")));
  TRY_STATUS(from_json(to.revoke_, get_json_object_field_force(from, "revoke")));
  return Status::OK();
}

Status from_json(td_api::deleteChatMessagesByDate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.min_date_, get_json_object_field_force(from, "min_date")));
  TRY_STATUS(from_json(to.max_date_, get_json_object_field_force(from, "max_date")));
  TRY_STATUS(from_json(to.revoke_, get_json_object_field_force(from, "revoke")));
  return Status::OK();
}

Status from_json(td_api::deleteChatMessagesBySender &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.sender_id_, get_json_object_field_force(from, "sender_id")));
  return Status::OK();
}

Status from_json(td_api::deleteChatReplyMarkup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  return Status::OK();
}

Status from_json(td_api::deleteCommands &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, get_json_object_field_force(from, "scope")));
  TRY_STATUS(from_json(to.language_code_, get_json_object_field_force(from, "language_code")));
  return Status::OK();
}

Status from_json(td_api::deleteFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, get_json_object_field_force(from, "file_id")));
  return Status::OK();
}

Status from_json(td_api::deleteForumTopic &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  return Status::OK();
}

Status from_json(td_api::deleteLanguagePack &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, get_json_object_field_force(from, "language_pack_id")));
  return Status::OK();
}

Status from_json(td_api::deleteMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_ids_, get_json_object_field_force(from, "message_ids")));
  TRY_STATUS(from_json(to.revoke_, get_json_object_field_force(from, "revoke")));
  return Status::OK();
}

Status from_json(td_api::deletePassportElement &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  return Status::OK();
}

Status from_json(td_api::deleteProfilePhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.profile_photo_id_, get_json_object_field_force(from, "profile_photo_id")));
  return Status::OK();
}

Status from_json(td_api::deleteRevokedChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.invite_link_, get_json_object_field_force(from, "invite_link")));
  return Status::OK();
}

Status from_json(td_api::deleteSavedCredentials &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::deleteSavedOrderInfo &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::deleteStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  return Status::OK();
}

Status from_json(td_api::destroy &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::disableAllSupergroupUsernames &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  return Status::OK();
}

Status from_json(td_api::disableProxy &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::discardCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.call_id_, get_json_object_field_force(from, "call_id")));
  TRY_STATUS(from_json(to.is_disconnected_, get_json_object_field_force(from, "is_disconnected")));
  TRY_STATUS(from_json(to.duration_, get_json_object_field_force(from, "duration")));
  TRY_STATUS(from_json(to.is_video_, get_json_object_field_force(from, "is_video")));
  TRY_STATUS(from_json(to.connection_id_, get_json_object_field_force(from, "connection_id")));
  return Status::OK();
}

Status from_json(td_api::disconnectAllWebsites &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::disconnectWebsite &to, JsonObject &from) {
  TRY_STATUS(from_json(to.website_id_, get_json_object_field_force(from, "website_id")));
  return Status::OK();
}

Status from_json(td_api::downloadFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, get_json_object_field_force(from, "file_id")));
  TRY_STATUS(from_json(to.priority_, get_json_object_field_force(from, "priority")));
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  TRY_STATUS(from_json(to.synchronous_, get_json_object_field_force(from, "synchronous")));
  return Status::OK();
}

Status from_json(td_api::editChatFolder &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, get_json_object_field_force(from, "chat_folder_id")));
  TRY_STATUS(from_json(to.folder_, get_json_object_field_force(from, "folder")));
  return Status::OK();
}

Status from_json(td_api::editChatFolderInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, get_json_object_field_force(from, "chat_folder_id")));
  TRY_STATUS(from_json(to.invite_link_, get_json_object_field_force(from, "invite_link")));
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  TRY_STATUS(from_json(to.chat_ids_, get_json_object_field_force(from, "chat_ids")));
  return Status::OK();
}

Status from_json(td_api::editChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.invite_link_, get_json_object_field_force(from, "invite_link")));
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  TRY_STATUS(from_json(to.expiration_date_, get_json_object_field_force(from, "expiration_date")));
  TRY_STATUS(from_json(to.member_limit_, get_json_object_field_force(from, "member_limit")));
  TRY_STATUS(from_json(to.creates_join_request_, get_json_object_field_force(from, "creates_join_request")));
  return Status::OK();
}

Status from_json(td_api::editCustomLanguagePackInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.info_, get_json_object_field_force(from, "info")));
  return Status::OK();
}

Status from_json(td_api::editForumTopic &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  TRY_STATUS(from_json(to.edit_icon_custom_emoji_, get_json_object_field_force(from, "edit_icon_custom_emoji")));
  TRY_STATUS(from_json(to.icon_custom_emoji_id_, get_json_object_field_force(from, "icon_custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::editInlineMessageCaption &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_message_id_, get_json_object_field_force(from, "inline_message_id")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.caption_, get_json_object_field_force(from, "caption")));
  return Status::OK();
}

Status from_json(td_api::editInlineMessageLiveLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_message_id_, get_json_object_field_force(from, "inline_message_id")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.location_, get_json_object_field_force(from, "location")));
  TRY_STATUS(from_json(to.heading_, get_json_object_field_force(from, "heading")));
  TRY_STATUS(from_json(to.proximity_alert_radius_, get_json_object_field_force(from, "proximity_alert_radius")));
  return Status::OK();
}

Status from_json(td_api::editInlineMessageMedia &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_message_id_, get_json_object_field_force(from, "inline_message_id")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, get_json_object_field_force(from, "input_message_content")));
  return Status::OK();
}

Status from_json(td_api::editInlineMessageReplyMarkup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_message_id_, get_json_object_field_force(from, "inline_message_id")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  return Status::OK();
}

Status from_json(td_api::editInlineMessageText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_message_id_, get_json_object_field_force(from, "inline_message_id")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, get_json_object_field_force(from, "input_message_content")));
  return Status::OK();
}

Status from_json(td_api::editMessageCaption &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.caption_, get_json_object_field_force(from, "caption")));
  return Status::OK();
}

Status from_json(td_api::editMessageLiveLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.location_, get_json_object_field_force(from, "location")));
  TRY_STATUS(from_json(to.heading_, get_json_object_field_force(from, "heading")));
  TRY_STATUS(from_json(to.proximity_alert_radius_, get_json_object_field_force(from, "proximity_alert_radius")));
  return Status::OK();
}

Status from_json(td_api::editMessageMedia &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, get_json_object_field_force(from, "input_message_content")));
  return Status::OK();
}

Status from_json(td_api::editMessageReplyMarkup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  return Status::OK();
}

Status from_json(td_api::editMessageSchedulingState &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.scheduling_state_, get_json_object_field_force(from, "scheduling_state")));
  return Status::OK();
}

Status from_json(td_api::editMessageText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, get_json_object_field_force(from, "input_message_content")));
  return Status::OK();
}

Status from_json(td_api::editProxy &to, JsonObject &from) {
  TRY_STATUS(from_json(to.proxy_id_, get_json_object_field_force(from, "proxy_id")));
  TRY_STATUS(from_json(to.server_, get_json_object_field_force(from, "server")));
  TRY_STATUS(from_json(to.port_, get_json_object_field_force(from, "port")));
  TRY_STATUS(from_json(to.enable_, get_json_object_field_force(from, "enable")));
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  return Status::OK();
}

Status from_json(td_api::enableProxy &to, JsonObject &from) {
  TRY_STATUS(from_json(to.proxy_id_, get_json_object_field_force(from, "proxy_id")));
  return Status::OK();
}

Status from_json(td_api::endGroupCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  return Status::OK();
}

Status from_json(td_api::endGroupCallRecording &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  return Status::OK();
}

Status from_json(td_api::endGroupCallScreenSharing &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  return Status::OK();
}

Status from_json(td_api::finishFileGeneration &to, JsonObject &from) {
  TRY_STATUS(from_json(to.generation_id_, get_json_object_field_force(from, "generation_id")));
  TRY_STATUS(from_json(to.error_, get_json_object_field_force(from, "error")));
  return Status::OK();
}

Status from_json(td_api::forwardMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  TRY_STATUS(from_json(to.from_chat_id_, get_json_object_field_force(from, "from_chat_id")));
  TRY_STATUS(from_json(to.message_ids_, get_json_object_field_force(from, "message_ids")));
  TRY_STATUS(from_json(to.options_, get_json_object_field_force(from, "options")));
  TRY_STATUS(from_json(to.send_copy_, get_json_object_field_force(from, "send_copy")));
  TRY_STATUS(from_json(to.remove_caption_, get_json_object_field_force(from, "remove_caption")));
  TRY_STATUS(from_json(to.only_preview_, get_json_object_field_force(from, "only_preview")));
  return Status::OK();
}

Status from_json(td_api::getAccountTtl &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getActiveLiveLocationMessages &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getActiveSessions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getAllPassportElements &to, JsonObject &from) {
  TRY_STATUS(from_json(to.password_, get_json_object_field_force(from, "password")));
  return Status::OK();
}

Status from_json(td_api::getAnimatedEmoji &to, JsonObject &from) {
  TRY_STATUS(from_json(to.emoji_, get_json_object_field_force(from, "emoji")));
  return Status::OK();
}

Status from_json(td_api::getApplicationConfig &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getApplicationDownloadLink &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getArchivedStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, get_json_object_field_force(from, "sticker_type")));
  TRY_STATUS(from_json(to.offset_sticker_set_id_, get_json_object_field_force(from, "offset_sticker_set_id")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::getAttachedStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, get_json_object_field_force(from, "file_id")));
  return Status::OK();
}

Status from_json(td_api::getAttachmentMenuBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
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

Status from_json(td_api::getBackgroundUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  return Status::OK();
}

Status from_json(td_api::getBackgrounds &to, JsonObject &from) {
  TRY_STATUS(from_json(to.for_dark_theme_, get_json_object_field_force(from, "for_dark_theme")));
  return Status::OK();
}

Status from_json(td_api::getBankCardInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bank_card_number_, get_json_object_field_force(from, "bank_card_number")));
  return Status::OK();
}

Status from_json(td_api::getBasicGroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.basic_group_id_, get_json_object_field_force(from, "basic_group_id")));
  return Status::OK();
}

Status from_json(td_api::getBasicGroupFullInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.basic_group_id_, get_json_object_field_force(from, "basic_group_id")));
  return Status::OK();
}

Status from_json(td_api::getBlockedMessageSenders &to, JsonObject &from) {
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::getBotInfoDescription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, get_json_object_field_force(from, "language_code")));
  return Status::OK();
}

Status from_json(td_api::getBotInfoShortDescription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, get_json_object_field_force(from, "language_code")));
  return Status::OK();
}

Status from_json(td_api::getBotName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, get_json_object_field_force(from, "language_code")));
  return Status::OK();
}

Status from_json(td_api::getCallbackQueryAnswer &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.payload_, get_json_object_field_force(from, "payload")));
  return Status::OK();
}

Status from_json(td_api::getCallbackQueryMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.callback_query_id_, get_json_object_field_force(from, "callback_query_id")));
  return Status::OK();
}

Status from_json(td_api::getChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatAdministrators &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatAvailableMessageSenders &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatEventLog &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  TRY_STATUS(from_json(to.from_event_id_, get_json_object_field_force(from, "from_event_id")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  TRY_STATUS(from_json(to.filters_, get_json_object_field_force(from, "filters")));
  TRY_STATUS(from_json(to.user_ids_, get_json_object_field_force(from, "user_ids")));
  return Status::OK();
}

Status from_json(td_api::getChatFolder &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, get_json_object_field_force(from, "chat_folder_id")));
  return Status::OK();
}

Status from_json(td_api::getChatFolderChatsToLeave &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, get_json_object_field_force(from, "chat_folder_id")));
  return Status::OK();
}

Status from_json(td_api::getChatFolderDefaultIconName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.folder_, get_json_object_field_force(from, "folder")));
  return Status::OK();
}

Status from_json(td_api::getChatFolderInviteLinks &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, get_json_object_field_force(from, "chat_folder_id")));
  return Status::OK();
}

Status from_json(td_api::getChatFolderNewChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, get_json_object_field_force(from, "chat_folder_id")));
  return Status::OK();
}

Status from_json(td_api::getChatHistory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.from_message_id_, get_json_object_field_force(from, "from_message_id")));
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  TRY_STATUS(from_json(to.only_local_, get_json_object_field_force(from, "only_local")));
  return Status::OK();
}

Status from_json(td_api::getChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.invite_link_, get_json_object_field_force(from, "invite_link")));
  return Status::OK();
}

Status from_json(td_api::getChatInviteLinkCounts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatInviteLinkMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.invite_link_, get_json_object_field_force(from, "invite_link")));
  TRY_STATUS(from_json(to.offset_member_, get_json_object_field_force(from, "offset_member")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::getChatInviteLinks &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.creator_user_id_, get_json_object_field_force(from, "creator_user_id")));
  TRY_STATUS(from_json(to.is_revoked_, get_json_object_field_force(from, "is_revoked")));
  TRY_STATUS(from_json(to.offset_date_, get_json_object_field_force(from, "offset_date")));
  TRY_STATUS(from_json(to.offset_invite_link_, get_json_object_field_force(from, "offset_invite_link")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::getChatJoinRequests &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.invite_link_, get_json_object_field_force(from, "invite_link")));
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  TRY_STATUS(from_json(to.offset_request_, get_json_object_field_force(from, "offset_request")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::getChatListsToAddChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.member_id_, get_json_object_field_force(from, "member_id")));
  return Status::OK();
}

Status from_json(td_api::getChatMessageByDate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.date_, get_json_object_field_force(from, "date")));
  return Status::OK();
}

Status from_json(td_api::getChatMessageCalendar &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.filter_, get_json_object_field_force(from, "filter")));
  TRY_STATUS(from_json(to.from_message_id_, get_json_object_field_force(from, "from_message_id")));
  return Status::OK();
}

Status from_json(td_api::getChatMessageCount &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.filter_, get_json_object_field_force(from, "filter")));
  TRY_STATUS(from_json(to.return_local_, get_json_object_field_force(from, "return_local")));
  return Status::OK();
}

Status from_json(td_api::getChatMessagePosition &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.filter_, get_json_object_field_force(from, "filter")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  return Status::OK();
}

Status from_json(td_api::getChatNotificationSettingsExceptions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, get_json_object_field_force(from, "scope")));
  TRY_STATUS(from_json(to.compare_sound_, get_json_object_field_force(from, "compare_sound")));
  return Status::OK();
}

Status from_json(td_api::getChatPinnedMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatScheduledMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatSparseMessagePositions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.filter_, get_json_object_field_force(from, "filter")));
  TRY_STATUS(from_json(to.from_message_id_, get_json_object_field_force(from, "from_message_id")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::getChatSponsoredMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::getChatStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.is_dark_, get_json_object_field_force(from, "is_dark")));
  return Status::OK();
}

Status from_json(td_api::getChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_list_, get_json_object_field_force(from, "chat_list")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::getChatsForChatFolderInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, get_json_object_field_force(from, "chat_folder_id")));
  return Status::OK();
}

Status from_json(td_api::getCommands &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, get_json_object_field_force(from, "scope")));
  TRY_STATUS(from_json(to.language_code_, get_json_object_field_force(from, "language_code")));
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

Status from_json(td_api::getCreatedPublicChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  return Status::OK();
}

Status from_json(td_api::getCurrentState &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getCustomEmojiReactionAnimations &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getCustomEmojiStickers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.custom_emoji_ids_, get_json_object_field_force(from, "custom_emoji_ids")));
  return Status::OK();
}

Status from_json(td_api::getDatabaseStatistics &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getDeepLinkInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, get_json_object_field_force(from, "link")));
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

Status from_json(td_api::getEmojiCategories &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  return Status::OK();
}

Status from_json(td_api::getEmojiReaction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.emoji_, get_json_object_field_force(from, "emoji")));
  return Status::OK();
}

Status from_json(td_api::getEmojiSuggestionsUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_code_, get_json_object_field_force(from, "language_code")));
  return Status::OK();
}

Status from_json(td_api::getExternalLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, get_json_object_field_force(from, "link")));
  TRY_STATUS(from_json(to.allow_write_access_, get_json_object_field_force(from, "allow_write_access")));
  return Status::OK();
}

Status from_json(td_api::getExternalLinkInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, get_json_object_field_force(from, "link")));
  return Status::OK();
}

Status from_json(td_api::getFavoriteStickers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, get_json_object_field_force(from, "file_id")));
  return Status::OK();
}

Status from_json(td_api::getFileDownloadedPrefixSize &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, get_json_object_field_force(from, "file_id")));
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  return Status::OK();
}

Status from_json(td_api::getFileExtension &to, JsonObject &from) {
  TRY_STATUS(from_json(to.mime_type_, get_json_object_field_force(from, "mime_type")));
  return Status::OK();
}

Status from_json(td_api::getFileMimeType &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_name_, get_json_object_field_force(from, "file_name")));
  return Status::OK();
}

Status from_json(td_api::getForumTopic &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  return Status::OK();
}

Status from_json(td_api::getForumTopicDefaultIcons &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getForumTopicLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  return Status::OK();
}

Status from_json(td_api::getForumTopics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  TRY_STATUS(from_json(to.offset_date_, get_json_object_field_force(from, "offset_date")));
  TRY_STATUS(from_json(to.offset_message_id_, get_json_object_field_force(from, "offset_message_id")));
  TRY_STATUS(from_json(to.offset_message_thread_id_, get_json_object_field_force(from, "offset_message_thread_id")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::getGameHighScores &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  return Status::OK();
}

Status from_json(td_api::getGroupCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  return Status::OK();
}

Status from_json(td_api::getGroupCallInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  TRY_STATUS(from_json(to.can_self_unmute_, get_json_object_field_force(from, "can_self_unmute")));
  return Status::OK();
}

Status from_json(td_api::getGroupCallStreamSegment &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  TRY_STATUS(from_json(to.time_offset_, get_json_object_field_force(from, "time_offset")));
  TRY_STATUS(from_json(to.scale_, get_json_object_field_force(from, "scale")));
  TRY_STATUS(from_json(to.channel_id_, get_json_object_field_force(from, "channel_id")));
  TRY_STATUS(from_json(to.video_quality_, get_json_object_field_force(from, "video_quality")));
  return Status::OK();
}

Status from_json(td_api::getGroupCallStreams &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  return Status::OK();
}

Status from_json(td_api::getGroupsInCommon &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.offset_chat_id_, get_json_object_field_force(from, "offset_chat_id")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::getImportedContactCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getInactiveSupergroupChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getInlineGameHighScores &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_message_id_, get_json_object_field_force(from, "inline_message_id")));
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  return Status::OK();
}

Status from_json(td_api::getInlineQueryResults &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.user_location_, get_json_object_field_force(from, "user_location")));
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  return Status::OK();
}

Status from_json(td_api::getInstalledStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, get_json_object_field_force(from, "sticker_type")));
  return Status::OK();
}

Status from_json(td_api::getInternalLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  TRY_STATUS(from_json(to.is_http_, get_json_object_field_force(from, "is_http")));
  return Status::OK();
}

Status from_json(td_api::getInternalLinkType &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, get_json_object_field_force(from, "link")));
  return Status::OK();
}

Status from_json(td_api::getJsonString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.json_value_, get_json_object_field_force(from, "json_value")));
  return Status::OK();
}

Status from_json(td_api::getJsonValue &to, JsonObject &from) {
  TRY_STATUS(from_json(to.json_, get_json_object_field_force(from, "json")));
  return Status::OK();
}

Status from_json(td_api::getLanguagePackInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, get_json_object_field_force(from, "language_pack_id")));
  return Status::OK();
}

Status from_json(td_api::getLanguagePackString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_database_path_, get_json_object_field_force(from, "language_pack_database_path")));
  TRY_STATUS(from_json(to.localization_target_, get_json_object_field_force(from, "localization_target")));
  TRY_STATUS(from_json(to.language_pack_id_, get_json_object_field_force(from, "language_pack_id")));
  TRY_STATUS(from_json(to.key_, get_json_object_field_force(from, "key")));
  return Status::OK();
}

Status from_json(td_api::getLanguagePackStrings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, get_json_object_field_force(from, "language_pack_id")));
  TRY_STATUS(from_json(to.keys_, get_json_object_field_force(from, "keys")));
  return Status::OK();
}

Status from_json(td_api::getLocalizationTargetInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.only_local_, get_json_object_field_force(from, "only_local")));
  return Status::OK();
}

Status from_json(td_api::getLogStream &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getLogTagVerbosityLevel &to, JsonObject &from) {
  TRY_STATUS(from_json(to.tag_, get_json_object_field_force(from, "tag")));
  return Status::OK();
}

Status from_json(td_api::getLogTags &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getLogVerbosityLevel &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getLoginUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.button_id_, get_json_object_field_force(from, "button_id")));
  TRY_STATUS(from_json(to.allow_write_access_, get_json_object_field_force(from, "allow_write_access")));
  return Status::OK();
}

Status from_json(td_api::getLoginUrlInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.button_id_, get_json_object_field_force(from, "button_id")));
  return Status::OK();
}

Status from_json(td_api::getMapThumbnailFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, get_json_object_field_force(from, "location")));
  TRY_STATUS(from_json(to.zoom_, get_json_object_field_force(from, "zoom")));
  TRY_STATUS(from_json(to.width_, get_json_object_field_force(from, "width")));
  TRY_STATUS(from_json(to.height_, get_json_object_field_force(from, "height")));
  TRY_STATUS(from_json(to.scale_, get_json_object_field_force(from, "scale")));
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::getMarkdownText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, get_json_object_field_force(from, "text")));
  return Status::OK();
}

Status from_json(td_api::getMe &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getMenuButton &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  return Status::OK();
}

Status from_json(td_api::getMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  return Status::OK();
}

Status from_json(td_api::getMessageAddedReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.reaction_type_, get_json_object_field_force(from, "reaction_type")));
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::getMessageAvailableReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.row_size_, get_json_object_field_force(from, "row_size")));
  return Status::OK();
}

Status from_json(td_api::getMessageEmbeddingCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.for_album_, get_json_object_field_force(from, "for_album")));
  return Status::OK();
}

Status from_json(td_api::getMessageFileType &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_file_head_, get_json_object_field_force(from, "message_file_head")));
  return Status::OK();
}

Status from_json(td_api::getMessageImportConfirmationText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::getMessageLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.media_timestamp_, get_json_object_field_force(from, "media_timestamp")));
  TRY_STATUS(from_json(to.for_album_, get_json_object_field_force(from, "for_album")));
  TRY_STATUS(from_json(to.in_message_thread_, get_json_object_field_force(from, "in_message_thread")));
  return Status::OK();
}

Status from_json(td_api::getMessageLinkInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, get_json_object_field_force(from, "url")));
  return Status::OK();
}

Status from_json(td_api::getMessageLocally &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  return Status::OK();
}

Status from_json(td_api::getMessagePublicForwards &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::getMessageStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.is_dark_, get_json_object_field_force(from, "is_dark")));
  return Status::OK();
}

Status from_json(td_api::getMessageThread &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  return Status::OK();
}

Status from_json(td_api::getMessageThreadHistory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.from_message_id_, get_json_object_field_force(from, "from_message_id")));
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::getMessageViewers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  return Status::OK();
}

Status from_json(td_api::getMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_ids_, get_json_object_field_force(from, "message_ids")));
  return Status::OK();
}

Status from_json(td_api::getNetworkStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.only_current_, get_json_object_field_force(from, "only_current")));
  return Status::OK();
}

Status from_json(td_api::getOption &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  return Status::OK();
}

Status from_json(td_api::getPassportAuthorizationForm &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.scope_, get_json_object_field_force(from, "scope")));
  TRY_STATUS(from_json(to.public_key_, get_json_object_field_force(from, "public_key")));
  TRY_STATUS(from_json(to.nonce_, get_json_object_field_force(from, "nonce")));
  return Status::OK();
}

Status from_json(td_api::getPassportAuthorizationFormAvailableElements &to, JsonObject &from) {
  TRY_STATUS(from_json(to.authorization_form_id_, get_json_object_field_force(from, "authorization_form_id")));
  TRY_STATUS(from_json(to.password_, get_json_object_field_force(from, "password")));
  return Status::OK();
}

Status from_json(td_api::getPassportElement &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  TRY_STATUS(from_json(to.password_, get_json_object_field_force(from, "password")));
  return Status::OK();
}

Status from_json(td_api::getPasswordState &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getPaymentForm &to, JsonObject &from) {
  TRY_STATUS(from_json(to.input_invoice_, get_json_object_field_force(from, "input_invoice")));
  TRY_STATUS(from_json(to.theme_, get_json_object_field_force(from, "theme")));
  return Status::OK();
}

Status from_json(td_api::getPaymentReceipt &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  return Status::OK();
}

Status from_json(td_api::getPhoneNumberInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_prefix_, get_json_object_field_force(from, "phone_number_prefix")));
  return Status::OK();
}

Status from_json(td_api::getPhoneNumberInfoSync &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_code_, get_json_object_field_force(from, "language_code")));
  TRY_STATUS(from_json(to.phone_number_prefix_, get_json_object_field_force(from, "phone_number_prefix")));
  return Status::OK();
}

Status from_json(td_api::getPollVoters &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.option_id_, get_json_object_field_force(from, "option_id")));
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::getPreferredCountryLanguage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.country_code_, get_json_object_field_force(from, "country_code")));
  return Status::OK();
}

Status from_json(td_api::getPremiumFeatures &to, JsonObject &from) {
  TRY_STATUS(from_json(to.source_, get_json_object_field_force(from, "source")));
  return Status::OK();
}

Status from_json(td_api::getPremiumLimit &to, JsonObject &from) {
  TRY_STATUS(from_json(to.limit_type_, get_json_object_field_force(from, "limit_type")));
  return Status::OK();
}

Status from_json(td_api::getPremiumState &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getPremiumStickerExamples &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getPremiumStickers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::getProxies &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getProxyLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.proxy_id_, get_json_object_field_force(from, "proxy_id")));
  return Status::OK();
}

Status from_json(td_api::getPushReceiverId &to, JsonObject &from) {
  TRY_STATUS(from_json(to.payload_, get_json_object_field_force(from, "payload")));
  return Status::OK();
}

Status from_json(td_api::getRecentEmojiStatuses &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getRecentInlineBots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getRecentStickers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_attached_, get_json_object_field_force(from, "is_attached")));
  return Status::OK();
}

Status from_json(td_api::getRecentlyOpenedChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::getRecentlyVisitedTMeUrls &to, JsonObject &from) {
  TRY_STATUS(from_json(to.referrer_, get_json_object_field_force(from, "referrer")));
  return Status::OK();
}

Status from_json(td_api::getRecommendedChatFolders &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getRecoveryEmailAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.password_, get_json_object_field_force(from, "password")));
  return Status::OK();
}

Status from_json(td_api::getRemoteFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.remote_file_id_, get_json_object_field_force(from, "remote_file_id")));
  TRY_STATUS(from_json(to.file_type_, get_json_object_field_force(from, "file_type")));
  return Status::OK();
}

Status from_json(td_api::getRepliedMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  return Status::OK();
}

Status from_json(td_api::getSavedAnimations &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getSavedNotificationSound &to, JsonObject &from) {
  TRY_STATUS(from_json(to.notification_sound_id_, get_json_object_field_force(from, "notification_sound_id")));
  return Status::OK();
}

Status from_json(td_api::getSavedNotificationSounds &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getSavedOrderInfo &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getScopeNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, get_json_object_field_force(from, "scope")));
  return Status::OK();
}

Status from_json(td_api::getSecretChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.secret_chat_id_, get_json_object_field_force(from, "secret_chat_id")));
  return Status::OK();
}

Status from_json(td_api::getStatisticalGraph &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.token_, get_json_object_field_force(from, "token")));
  TRY_STATUS(from_json(to.x_, get_json_object_field_force(from, "x")));
  return Status::OK();
}

Status from_json(td_api::getStickerEmojis &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, get_json_object_field_force(from, "sticker")));
  return Status::OK();
}

Status from_json(td_api::getStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.set_id_, get_json_object_field_force(from, "set_id")));
  return Status::OK();
}

Status from_json(td_api::getStickers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, get_json_object_field_force(from, "sticker_type")));
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::getStorageStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_limit_, get_json_object_field_force(from, "chat_limit")));
  return Status::OK();
}

Status from_json(td_api::getStorageStatisticsFast &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getSuggestedFileName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, get_json_object_field_force(from, "file_id")));
  TRY_STATUS(from_json(to.directory_, get_json_object_field_force(from, "directory")));
  return Status::OK();
}

Status from_json(td_api::getSuggestedStickerSetName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  return Status::OK();
}

Status from_json(td_api::getSuitableDiscussionChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getSupergroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  return Status::OK();
}

Status from_json(td_api::getSupergroupFullInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  return Status::OK();
}

Status from_json(td_api::getSupergroupMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  TRY_STATUS(from_json(to.filter_, get_json_object_field_force(from, "filter")));
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
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
  TRY_STATUS(from_json(to.text_, get_json_object_field_force(from, "text")));
  return Status::OK();
}

Status from_json(td_api::getThemeParametersJsonString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.theme_, get_json_object_field_force(from, "theme")));
  return Status::OK();
}

Status from_json(td_api::getThemedEmojiStatuses &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getTopChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.category_, get_json_object_field_force(from, "category")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::getTrendingStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, get_json_object_field_force(from, "sticker_type")));
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::getUser &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  return Status::OK();
}

Status from_json(td_api::getUserFullInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  return Status::OK();
}

Status from_json(td_api::getUserLink &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getUserPrivacySettingRules &to, JsonObject &from) {
  TRY_STATUS(from_json(to.setting_, get_json_object_field_force(from, "setting")));
  return Status::OK();
}

Status from_json(td_api::getUserProfilePhotos &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::getUserSupportInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  return Status::OK();
}

Status from_json(td_api::getVideoChatAvailableParticipants &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::getVideoChatRtmpUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::getWebAppLinkUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.web_app_short_name_, get_json_object_field_force(from, "web_app_short_name")));
  TRY_STATUS(from_json(to.start_parameter_, get_json_object_field_force(from, "start_parameter")));
  TRY_STATUS(from_json(to.theme_, get_json_object_field_force(from, "theme")));
  TRY_STATUS(from_json(to.application_name_, get_json_object_field_force(from, "application_name")));
  TRY_STATUS(from_json(to.allow_write_access_, get_json_object_field_force(from, "allow_write_access")));
  return Status::OK();
}

Status from_json(td_api::getWebAppUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.url_, get_json_object_field_force(from, "url")));
  TRY_STATUS(from_json(to.theme_, get_json_object_field_force(from, "theme")));
  TRY_STATUS(from_json(to.application_name_, get_json_object_field_force(from, "application_name")));
  return Status::OK();
}

Status from_json(td_api::getWebPageInstantView &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, get_json_object_field_force(from, "url")));
  TRY_STATUS(from_json(to.force_full_, get_json_object_field_force(from, "force_full")));
  return Status::OK();
}

Status from_json(td_api::getWebPagePreview &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, get_json_object_field_force(from, "text")));
  return Status::OK();
}

Status from_json(td_api::hideSuggestedAction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.action_, get_json_object_field_force(from, "action")));
  return Status::OK();
}

Status from_json(td_api::importContacts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.contacts_, get_json_object_field_force(from, "contacts")));
  return Status::OK();
}

Status from_json(td_api::importMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_file_, get_json_object_field_force(from, "message_file")));
  TRY_STATUS(from_json(to.attached_files_, get_json_object_field_force(from, "attached_files")));
  return Status::OK();
}

Status from_json(td_api::inviteGroupCallParticipants &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  TRY_STATUS(from_json(to.user_ids_, get_json_object_field_force(from, "user_ids")));
  return Status::OK();
}

Status from_json(td_api::joinChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::joinChatByInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, get_json_object_field_force(from, "invite_link")));
  return Status::OK();
}

Status from_json(td_api::joinGroupCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  TRY_STATUS(from_json(to.participant_id_, get_json_object_field_force(from, "participant_id")));
  TRY_STATUS(from_json(to.audio_source_id_, get_json_object_field_force(from, "audio_source_id")));
  TRY_STATUS(from_json(to.payload_, get_json_object_field_force(from, "payload")));
  TRY_STATUS(from_json(to.is_muted_, get_json_object_field_force(from, "is_muted")));
  TRY_STATUS(from_json(to.is_my_video_enabled_, get_json_object_field_force(from, "is_my_video_enabled")));
  TRY_STATUS(from_json(to.invite_hash_, get_json_object_field_force(from, "invite_hash")));
  return Status::OK();
}

Status from_json(td_api::leaveChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::leaveGroupCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  return Status::OK();
}

Status from_json(td_api::loadChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_list_, get_json_object_field_force(from, "chat_list")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::loadGroupCallParticipants &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::logOut &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::openChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::openMessageContent &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  return Status::OK();
}

Status from_json(td_api::openWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.url_, get_json_object_field_force(from, "url")));
  TRY_STATUS(from_json(to.theme_, get_json_object_field_force(from, "theme")));
  TRY_STATUS(from_json(to.application_name_, get_json_object_field_force(from, "application_name")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  TRY_STATUS(from_json(to.reply_to_message_id_, get_json_object_field_force(from, "reply_to_message_id")));
  return Status::OK();
}

Status from_json(td_api::optimizeStorage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.size_, get_json_object_field_force(from, "size")));
  TRY_STATUS(from_json(to.ttl_, get_json_object_field_force(from, "ttl")));
  TRY_STATUS(from_json(to.count_, get_json_object_field_force(from, "count")));
  TRY_STATUS(from_json(to.immunity_delay_, get_json_object_field_force(from, "immunity_delay")));
  TRY_STATUS(from_json(to.file_types_, get_json_object_field_force(from, "file_types")));
  TRY_STATUS(from_json(to.chat_ids_, get_json_object_field_force(from, "chat_ids")));
  TRY_STATUS(from_json(to.exclude_chat_ids_, get_json_object_field_force(from, "exclude_chat_ids")));
  TRY_STATUS(from_json(to.return_deleted_file_statistics_, get_json_object_field_force(from, "return_deleted_file_statistics")));
  TRY_STATUS(from_json(to.chat_limit_, get_json_object_field_force(from, "chat_limit")));
  return Status::OK();
}

Status from_json(td_api::parseMarkdown &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, get_json_object_field_force(from, "text")));
  return Status::OK();
}

Status from_json(td_api::parseTextEntities &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, get_json_object_field_force(from, "text")));
  TRY_STATUS(from_json(to.parse_mode_, get_json_object_field_force(from, "parse_mode")));
  return Status::OK();
}

Status from_json(td_api::pinChatMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.disable_notification_, get_json_object_field_force(from, "disable_notification")));
  TRY_STATUS(from_json(to.only_for_self_, get_json_object_field_force(from, "only_for_self")));
  return Status::OK();
}

Status from_json(td_api::pingProxy &to, JsonObject &from) {
  TRY_STATUS(from_json(to.proxy_id_, get_json_object_field_force(from, "proxy_id")));
  return Status::OK();
}

Status from_json(td_api::preliminaryUploadFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_, get_json_object_field_force(from, "file")));
  TRY_STATUS(from_json(to.file_type_, get_json_object_field_force(from, "file_type")));
  TRY_STATUS(from_json(to.priority_, get_json_object_field_force(from, "priority")));
  return Status::OK();
}

Status from_json(td_api::processChatFolderNewChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, get_json_object_field_force(from, "chat_folder_id")));
  TRY_STATUS(from_json(to.added_chat_ids_, get_json_object_field_force(from, "added_chat_ids")));
  return Status::OK();
}

Status from_json(td_api::processChatJoinRequest &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.approve_, get_json_object_field_force(from, "approve")));
  return Status::OK();
}

Status from_json(td_api::processChatJoinRequests &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.invite_link_, get_json_object_field_force(from, "invite_link")));
  TRY_STATUS(from_json(to.approve_, get_json_object_field_force(from, "approve")));
  return Status::OK();
}

Status from_json(td_api::processPushNotification &to, JsonObject &from) {
  TRY_STATUS(from_json(to.payload_, get_json_object_field_force(from, "payload")));
  return Status::OK();
}

Status from_json(td_api::rateSpeechRecognition &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.is_good_, get_json_object_field_force(from, "is_good")));
  return Status::OK();
}

Status from_json(td_api::readAllChatMentions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::readAllChatReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::readAllMessageThreadMentions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  return Status::OK();
}

Status from_json(td_api::readAllMessageThreadReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  return Status::OK();
}

Status from_json(td_api::readChatList &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_list_, get_json_object_field_force(from, "chat_list")));
  return Status::OK();
}

Status from_json(td_api::readFilePart &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, get_json_object_field_force(from, "file_id")));
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  TRY_STATUS(from_json(to.count_, get_json_object_field_force(from, "count")));
  return Status::OK();
}

Status from_json(td_api::recognizeSpeech &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  return Status::OK();
}

Status from_json(td_api::recoverAuthenticationPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.recovery_code_, get_json_object_field_force(from, "recovery_code")));
  TRY_STATUS(from_json(to.new_password_, get_json_object_field_force(from, "new_password")));
  TRY_STATUS(from_json(to.new_hint_, get_json_object_field_force(from, "new_hint")));
  return Status::OK();
}

Status from_json(td_api::recoverPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.recovery_code_, get_json_object_field_force(from, "recovery_code")));
  TRY_STATUS(from_json(to.new_password_, get_json_object_field_force(from, "new_password")));
  TRY_STATUS(from_json(to.new_hint_, get_json_object_field_force(from, "new_hint")));
  return Status::OK();
}

Status from_json(td_api::registerDevice &to, JsonObject &from) {
  TRY_STATUS(from_json(to.device_token_, get_json_object_field_force(from, "device_token")));
  TRY_STATUS(from_json(to.other_user_ids_, get_json_object_field_force(from, "other_user_ids")));
  return Status::OK();
}

Status from_json(td_api::registerUser &to, JsonObject &from) {
  TRY_STATUS(from_json(to.first_name_, get_json_object_field_force(from, "first_name")));
  TRY_STATUS(from_json(to.last_name_, get_json_object_field_force(from, "last_name")));
  return Status::OK();
}

Status from_json(td_api::removeAllFilesFromDownloads &to, JsonObject &from) {
  TRY_STATUS(from_json(to.only_active_, get_json_object_field_force(from, "only_active")));
  TRY_STATUS(from_json(to.only_completed_, get_json_object_field_force(from, "only_completed")));
  TRY_STATUS(from_json(to.delete_from_cache_, get_json_object_field_force(from, "delete_from_cache")));
  return Status::OK();
}

Status from_json(td_api::removeBackground &to, JsonObject &from) {
  TRY_STATUS(from_json(to.background_id_, get_json_object_field_force(from, "background_id")));
  return Status::OK();
}

Status from_json(td_api::removeChatActionBar &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::removeContacts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_ids_, get_json_object_field_force(from, "user_ids")));
  return Status::OK();
}

Status from_json(td_api::removeFavoriteSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, get_json_object_field_force(from, "sticker")));
  return Status::OK();
}

Status from_json(td_api::removeFileFromDownloads &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, get_json_object_field_force(from, "file_id")));
  TRY_STATUS(from_json(to.delete_from_cache_, get_json_object_field_force(from, "delete_from_cache")));
  return Status::OK();
}

Status from_json(td_api::removeMessageReaction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.reaction_type_, get_json_object_field_force(from, "reaction_type")));
  return Status::OK();
}

Status from_json(td_api::removeNotification &to, JsonObject &from) {
  TRY_STATUS(from_json(to.notification_group_id_, get_json_object_field_force(from, "notification_group_id")));
  TRY_STATUS(from_json(to.notification_id_, get_json_object_field_force(from, "notification_id")));
  return Status::OK();
}

Status from_json(td_api::removeNotificationGroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.notification_group_id_, get_json_object_field_force(from, "notification_group_id")));
  TRY_STATUS(from_json(to.max_notification_id_, get_json_object_field_force(from, "max_notification_id")));
  return Status::OK();
}

Status from_json(td_api::removeProxy &to, JsonObject &from) {
  TRY_STATUS(from_json(to.proxy_id_, get_json_object_field_force(from, "proxy_id")));
  return Status::OK();
}

Status from_json(td_api::removeRecentHashtag &to, JsonObject &from) {
  TRY_STATUS(from_json(to.hashtag_, get_json_object_field_force(from, "hashtag")));
  return Status::OK();
}

Status from_json(td_api::removeRecentSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_attached_, get_json_object_field_force(from, "is_attached")));
  TRY_STATUS(from_json(to.sticker_, get_json_object_field_force(from, "sticker")));
  return Status::OK();
}

Status from_json(td_api::removeRecentlyFoundChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::removeSavedAnimation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.animation_, get_json_object_field_force(from, "animation")));
  return Status::OK();
}

Status from_json(td_api::removeSavedNotificationSound &to, JsonObject &from) {
  TRY_STATUS(from_json(to.notification_sound_id_, get_json_object_field_force(from, "notification_sound_id")));
  return Status::OK();
}

Status from_json(td_api::removeStickerFromSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, get_json_object_field_force(from, "sticker")));
  return Status::OK();
}

Status from_json(td_api::removeTopChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.category_, get_json_object_field_force(from, "category")));
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::reorderActiveBotUsernames &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.usernames_, get_json_object_field_force(from, "usernames")));
  return Status::OK();
}

Status from_json(td_api::reorderActiveUsernames &to, JsonObject &from) {
  TRY_STATUS(from_json(to.usernames_, get_json_object_field_force(from, "usernames")));
  return Status::OK();
}

Status from_json(td_api::reorderChatFolders &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_ids_, get_json_object_field_force(from, "chat_folder_ids")));
  TRY_STATUS(from_json(to.main_chat_list_position_, get_json_object_field_force(from, "main_chat_list_position")));
  return Status::OK();
}

Status from_json(td_api::reorderInstalledStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, get_json_object_field_force(from, "sticker_type")));
  TRY_STATUS(from_json(to.sticker_set_ids_, get_json_object_field_force(from, "sticker_set_ids")));
  return Status::OK();
}

Status from_json(td_api::reorderSupergroupActiveUsernames &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  TRY_STATUS(from_json(to.usernames_, get_json_object_field_force(from, "usernames")));
  return Status::OK();
}

Status from_json(td_api::replacePrimaryChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::replaceVideoChatRtmpUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::reportChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_ids_, get_json_object_field_force(from, "message_ids")));
  TRY_STATUS(from_json(to.reason_, get_json_object_field_force(from, "reason")));
  TRY_STATUS(from_json(to.text_, get_json_object_field_force(from, "text")));
  return Status::OK();
}

Status from_json(td_api::reportChatPhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.file_id_, get_json_object_field_force(from, "file_id")));
  TRY_STATUS(from_json(to.reason_, get_json_object_field_force(from, "reason")));
  TRY_STATUS(from_json(to.text_, get_json_object_field_force(from, "text")));
  return Status::OK();
}

Status from_json(td_api::reportMessageReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.sender_id_, get_json_object_field_force(from, "sender_id")));
  return Status::OK();
}

Status from_json(td_api::reportSupergroupAntiSpamFalsePositive &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  return Status::OK();
}

Status from_json(td_api::reportSupergroupSpam &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  TRY_STATUS(from_json(to.message_ids_, get_json_object_field_force(from, "message_ids")));
  return Status::OK();
}

Status from_json(td_api::requestAuthenticationPasswordRecovery &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::requestPasswordRecovery &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::requestQrCodeAuthentication &to, JsonObject &from) {
  TRY_STATUS(from_json(to.other_user_ids_, get_json_object_field_force(from, "other_user_ids")));
  return Status::OK();
}

Status from_json(td_api::resendAuthenticationCode &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resendChangePhoneNumberCode &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resendEmailAddressVerificationCode &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resendLoginEmailAddressCode &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resendMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_ids_, get_json_object_field_force(from, "message_ids")));
  return Status::OK();
}

Status from_json(td_api::resendPhoneNumberConfirmationCode &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resendPhoneNumberVerificationCode &to, JsonObject &from) {
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

Status from_json(td_api::resetBackgrounds &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resetNetworkStatistics &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::resetPassword &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::revokeChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.invite_link_, get_json_object_field_force(from, "invite_link")));
  return Status::OK();
}

Status from_json(td_api::revokeGroupCallInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  return Status::OK();
}

Status from_json(td_api::saveApplicationLogEvent &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.data_, get_json_object_field_force(from, "data")));
  return Status::OK();
}

Status from_json(td_api::searchBackground &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  return Status::OK();
}

Status from_json(td_api::searchCallMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  TRY_STATUS(from_json(to.only_missed_, get_json_object_field_force(from, "only_missed")));
  return Status::OK();
}

Status from_json(td_api::searchChatMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  TRY_STATUS(from_json(to.filter_, get_json_object_field_force(from, "filter")));
  return Status::OK();
}

Status from_json(td_api::searchChatMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  TRY_STATUS(from_json(to.sender_id_, get_json_object_field_force(from, "sender_id")));
  TRY_STATUS(from_json(to.from_message_id_, get_json_object_field_force(from, "from_message_id")));
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  TRY_STATUS(from_json(to.filter_, get_json_object_field_force(from, "filter")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  return Status::OK();
}

Status from_json(td_api::searchChatRecentLocationMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::searchChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::searchChatsNearby &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, get_json_object_field_force(from, "location")));
  return Status::OK();
}

Status from_json(td_api::searchChatsOnServer &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::searchContacts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::searchEmojis &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, get_json_object_field_force(from, "text")));
  TRY_STATUS(from_json(to.exact_match_, get_json_object_field_force(from, "exact_match")));
  TRY_STATUS(from_json(to.input_language_codes_, get_json_object_field_force(from, "input_language_codes")));
  return Status::OK();
}

Status from_json(td_api::searchFileDownloads &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  TRY_STATUS(from_json(to.only_active_, get_json_object_field_force(from, "only_active")));
  TRY_STATUS(from_json(to.only_completed_, get_json_object_field_force(from, "only_completed")));
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::searchHashtags &to, JsonObject &from) {
  TRY_STATUS(from_json(to.prefix_, get_json_object_field_force(from, "prefix")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::searchInstalledStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, get_json_object_field_force(from, "sticker_type")));
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::searchMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_list_, get_json_object_field_force(from, "chat_list")));
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  TRY_STATUS(from_json(to.filter_, get_json_object_field_force(from, "filter")));
  TRY_STATUS(from_json(to.min_date_, get_json_object_field_force(from, "min_date")));
  TRY_STATUS(from_json(to.max_date_, get_json_object_field_force(from, "max_date")));
  return Status::OK();
}

Status from_json(td_api::searchOutgoingDocumentMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::searchPublicChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.username_, get_json_object_field_force(from, "username")));
  return Status::OK();
}

Status from_json(td_api::searchPublicChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  return Status::OK();
}

Status from_json(td_api::searchSecretMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  TRY_STATUS(from_json(to.filter_, get_json_object_field_force(from, "filter")));
  return Status::OK();
}

Status from_json(td_api::searchStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  return Status::OK();
}

Status from_json(td_api::searchStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, get_json_object_field_force(from, "query")));
  return Status::OK();
}

Status from_json(td_api::searchStickers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, get_json_object_field_force(from, "sticker_type")));
  TRY_STATUS(from_json(to.emojis_, get_json_object_field_force(from, "emojis")));
  TRY_STATUS(from_json(to.limit_, get_json_object_field_force(from, "limit")));
  return Status::OK();
}

Status from_json(td_api::searchUserByPhoneNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, get_json_object_field_force(from, "phone_number")));
  return Status::OK();
}

Status from_json(td_api::searchUserByToken &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, get_json_object_field_force(from, "token")));
  return Status::OK();
}

Status from_json(td_api::searchWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.web_app_short_name_, get_json_object_field_force(from, "web_app_short_name")));
  return Status::OK();
}

Status from_json(td_api::sendAuthenticationFirebaseSms &to, JsonObject &from) {
  TRY_STATUS(from_json(to.token_, get_json_object_field_force(from, "token")));
  return Status::OK();
}

Status from_json(td_api::sendBotStartMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.parameter_, get_json_object_field_force(from, "parameter")));
  return Status::OK();
}

Status from_json(td_api::sendCallDebugInformation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.call_id_, get_json_object_field_force(from, "call_id")));
  TRY_STATUS(from_json(to.debug_information_, get_json_object_field_force(from, "debug_information")));
  return Status::OK();
}

Status from_json(td_api::sendCallLog &to, JsonObject &from) {
  TRY_STATUS(from_json(to.call_id_, get_json_object_field_force(from, "call_id")));
  TRY_STATUS(from_json(to.log_file_, get_json_object_field_force(from, "log_file")));
  return Status::OK();
}

Status from_json(td_api::sendCallRating &to, JsonObject &from) {
  TRY_STATUS(from_json(to.call_id_, get_json_object_field_force(from, "call_id")));
  TRY_STATUS(from_json(to.rating_, get_json_object_field_force(from, "rating")));
  TRY_STATUS(from_json(to.comment_, get_json_object_field_force(from, "comment")));
  TRY_STATUS(from_json(to.problems_, get_json_object_field_force(from, "problems")));
  return Status::OK();
}

Status from_json(td_api::sendCallSignalingData &to, JsonObject &from) {
  TRY_STATUS(from_json(to.call_id_, get_json_object_field_force(from, "call_id")));
  TRY_STATUS(from_json_bytes(to.data_, get_json_object_field_force(from, "data")));
  return Status::OK();
}

Status from_json(td_api::sendChatAction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  TRY_STATUS(from_json(to.action_, get_json_object_field_force(from, "action")));
  return Status::OK();
}

Status from_json(td_api::sendChatScreenshotTakenNotification &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::sendCustomRequest &to, JsonObject &from) {
  TRY_STATUS(from_json(to.method_, get_json_object_field_force(from, "method")));
  TRY_STATUS(from_json(to.parameters_, get_json_object_field_force(from, "parameters")));
  return Status::OK();
}

Status from_json(td_api::sendEmailAddressVerificationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.email_address_, get_json_object_field_force(from, "email_address")));
  return Status::OK();
}

Status from_json(td_api::sendInlineQueryResultMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  TRY_STATUS(from_json(to.reply_to_message_id_, get_json_object_field_force(from, "reply_to_message_id")));
  TRY_STATUS(from_json(to.options_, get_json_object_field_force(from, "options")));
  TRY_STATUS(from_json(to.query_id_, get_json_object_field_force(from, "query_id")));
  TRY_STATUS(from_json(to.result_id_, get_json_object_field_force(from, "result_id")));
  TRY_STATUS(from_json(to.hide_via_bot_, get_json_object_field_force(from, "hide_via_bot")));
  return Status::OK();
}

Status from_json(td_api::sendMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  TRY_STATUS(from_json(to.reply_to_message_id_, get_json_object_field_force(from, "reply_to_message_id")));
  TRY_STATUS(from_json(to.options_, get_json_object_field_force(from, "options")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, get_json_object_field_force(from, "input_message_content")));
  return Status::OK();
}

Status from_json(td_api::sendMessageAlbum &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  TRY_STATUS(from_json(to.reply_to_message_id_, get_json_object_field_force(from, "reply_to_message_id")));
  TRY_STATUS(from_json(to.options_, get_json_object_field_force(from, "options")));
  TRY_STATUS(from_json(to.input_message_contents_, get_json_object_field_force(from, "input_message_contents")));
  TRY_STATUS(from_json(to.only_preview_, get_json_object_field_force(from, "only_preview")));
  return Status::OK();
}

Status from_json(td_api::sendPassportAuthorizationForm &to, JsonObject &from) {
  TRY_STATUS(from_json(to.authorization_form_id_, get_json_object_field_force(from, "authorization_form_id")));
  TRY_STATUS(from_json(to.types_, get_json_object_field_force(from, "types")));
  return Status::OK();
}

Status from_json(td_api::sendPaymentForm &to, JsonObject &from) {
  TRY_STATUS(from_json(to.input_invoice_, get_json_object_field_force(from, "input_invoice")));
  TRY_STATUS(from_json(to.payment_form_id_, get_json_object_field_force(from, "payment_form_id")));
  TRY_STATUS(from_json(to.order_info_id_, get_json_object_field_force(from, "order_info_id")));
  TRY_STATUS(from_json(to.shipping_option_id_, get_json_object_field_force(from, "shipping_option_id")));
  TRY_STATUS(from_json(to.credentials_, get_json_object_field_force(from, "credentials")));
  TRY_STATUS(from_json(to.tip_amount_, get_json_object_field_force(from, "tip_amount")));
  return Status::OK();
}

Status from_json(td_api::sendPhoneNumberConfirmationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.hash_, get_json_object_field_force(from, "hash")));
  TRY_STATUS(from_json(to.phone_number_, get_json_object_field_force(from, "phone_number")));
  TRY_STATUS(from_json(to.settings_, get_json_object_field_force(from, "settings")));
  return Status::OK();
}

Status from_json(td_api::sendPhoneNumberVerificationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, get_json_object_field_force(from, "phone_number")));
  TRY_STATUS(from_json(to.settings_, get_json_object_field_force(from, "settings")));
  return Status::OK();
}

Status from_json(td_api::sendWebAppData &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.button_text_, get_json_object_field_force(from, "button_text")));
  TRY_STATUS(from_json(to.data_, get_json_object_field_force(from, "data")));
  return Status::OK();
}

Status from_json(td_api::setAccountTtl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.ttl_, get_json_object_field_force(from, "ttl")));
  return Status::OK();
}

Status from_json(td_api::setAlarm &to, JsonObject &from) {
  TRY_STATUS(from_json(to.seconds_, get_json_object_field_force(from, "seconds")));
  return Status::OK();
}

Status from_json(td_api::setAuthenticationEmailAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.email_address_, get_json_object_field_force(from, "email_address")));
  return Status::OK();
}

Status from_json(td_api::setAuthenticationPhoneNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_, get_json_object_field_force(from, "phone_number")));
  TRY_STATUS(from_json(to.settings_, get_json_object_field_force(from, "settings")));
  return Status::OK();
}

Status from_json(td_api::setAutoDownloadSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.settings_, get_json_object_field_force(from, "settings")));
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  return Status::OK();
}

Status from_json(td_api::setAutosaveSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, get_json_object_field_force(from, "scope")));
  TRY_STATUS(from_json(to.settings_, get_json_object_field_force(from, "settings")));
  return Status::OK();
}

Status from_json(td_api::setBackground &to, JsonObject &from) {
  TRY_STATUS(from_json(to.background_, get_json_object_field_force(from, "background")));
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  TRY_STATUS(from_json(to.for_dark_theme_, get_json_object_field_force(from, "for_dark_theme")));
  return Status::OK();
}

Status from_json(td_api::setBio &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bio_, get_json_object_field_force(from, "bio")));
  return Status::OK();
}

Status from_json(td_api::setBotInfoDescription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, get_json_object_field_force(from, "language_code")));
  TRY_STATUS(from_json(to.description_, get_json_object_field_force(from, "description")));
  return Status::OK();
}

Status from_json(td_api::setBotInfoShortDescription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, get_json_object_field_force(from, "language_code")));
  TRY_STATUS(from_json(to.short_description_, get_json_object_field_force(from, "short_description")));
  return Status::OK();
}

Status from_json(td_api::setBotName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, get_json_object_field_force(from, "language_code")));
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  return Status::OK();
}

Status from_json(td_api::setBotProfilePhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.photo_, get_json_object_field_force(from, "photo")));
  return Status::OK();
}

Status from_json(td_api::setBotUpdatesStatus &to, JsonObject &from) {
  TRY_STATUS(from_json(to.pending_update_count_, get_json_object_field_force(from, "pending_update_count")));
  TRY_STATUS(from_json(to.error_message_, get_json_object_field_force(from, "error_message")));
  return Status::OK();
}

Status from_json(td_api::setChatAvailableReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.available_reactions_, get_json_object_field_force(from, "available_reactions")));
  return Status::OK();
}

Status from_json(td_api::setChatBackground &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.background_, get_json_object_field_force(from, "background")));
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  TRY_STATUS(from_json(to.dark_theme_dimming_, get_json_object_field_force(from, "dark_theme_dimming")));
  return Status::OK();
}

Status from_json(td_api::setChatClientData &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.client_data_, get_json_object_field_force(from, "client_data")));
  return Status::OK();
}

Status from_json(td_api::setChatDescription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.description_, get_json_object_field_force(from, "description")));
  return Status::OK();
}

Status from_json(td_api::setChatDiscussionGroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.discussion_chat_id_, get_json_object_field_force(from, "discussion_chat_id")));
  return Status::OK();
}

Status from_json(td_api::setChatDraftMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  TRY_STATUS(from_json(to.draft_message_, get_json_object_field_force(from, "draft_message")));
  return Status::OK();
}

Status from_json(td_api::setChatLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.location_, get_json_object_field_force(from, "location")));
  return Status::OK();
}

Status from_json(td_api::setChatMemberStatus &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.member_id_, get_json_object_field_force(from, "member_id")));
  TRY_STATUS(from_json(to.status_, get_json_object_field_force(from, "status")));
  return Status::OK();
}

Status from_json(td_api::setChatMessageAutoDeleteTime &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_auto_delete_time_, get_json_object_field_force(from, "message_auto_delete_time")));
  return Status::OK();
}

Status from_json(td_api::setChatMessageSender &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_sender_id_, get_json_object_field_force(from, "message_sender_id")));
  return Status::OK();
}

Status from_json(td_api::setChatNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.notification_settings_, get_json_object_field_force(from, "notification_settings")));
  return Status::OK();
}

Status from_json(td_api::setChatPermissions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.permissions_, get_json_object_field_force(from, "permissions")));
  return Status::OK();
}

Status from_json(td_api::setChatPhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.photo_, get_json_object_field_force(from, "photo")));
  return Status::OK();
}

Status from_json(td_api::setChatSlowModeDelay &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.slow_mode_delay_, get_json_object_field_force(from, "slow_mode_delay")));
  return Status::OK();
}

Status from_json(td_api::setChatTheme &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.theme_name_, get_json_object_field_force(from, "theme_name")));
  return Status::OK();
}

Status from_json(td_api::setChatTitle &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  return Status::OK();
}

Status from_json(td_api::setCommands &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, get_json_object_field_force(from, "scope")));
  TRY_STATUS(from_json(to.language_code_, get_json_object_field_force(from, "language_code")));
  TRY_STATUS(from_json(to.commands_, get_json_object_field_force(from, "commands")));
  return Status::OK();
}

Status from_json(td_api::setCustomEmojiStickerSetThumbnail &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  TRY_STATUS(from_json(to.custom_emoji_id_, get_json_object_field_force(from, "custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::setCustomLanguagePack &to, JsonObject &from) {
  TRY_STATUS(from_json(to.info_, get_json_object_field_force(from, "info")));
  TRY_STATUS(from_json(to.strings_, get_json_object_field_force(from, "strings")));
  return Status::OK();
}

Status from_json(td_api::setCustomLanguagePackString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, get_json_object_field_force(from, "language_pack_id")));
  TRY_STATUS(from_json(to.new_string_, get_json_object_field_force(from, "new_string")));
  return Status::OK();
}

Status from_json(td_api::setDatabaseEncryptionKey &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.new_encryption_key_, get_json_object_field_force(from, "new_encryption_key")));
  return Status::OK();
}

Status from_json(td_api::setDefaultChannelAdministratorRights &to, JsonObject &from) {
  TRY_STATUS(from_json(to.default_channel_administrator_rights_, get_json_object_field_force(from, "default_channel_administrator_rights")));
  return Status::OK();
}

Status from_json(td_api::setDefaultGroupAdministratorRights &to, JsonObject &from) {
  TRY_STATUS(from_json(to.default_group_administrator_rights_, get_json_object_field_force(from, "default_group_administrator_rights")));
  return Status::OK();
}

Status from_json(td_api::setDefaultMessageAutoDeleteTime &to, JsonObject &from) {
  TRY_STATUS(from_json(to.message_auto_delete_time_, get_json_object_field_force(from, "message_auto_delete_time")));
  return Status::OK();
}

Status from_json(td_api::setDefaultReactionType &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reaction_type_, get_json_object_field_force(from, "reaction_type")));
  return Status::OK();
}

Status from_json(td_api::setEmojiStatus &to, JsonObject &from) {
  TRY_STATUS(from_json(to.emoji_status_, get_json_object_field_force(from, "emoji_status")));
  TRY_STATUS(from_json(to.duration_, get_json_object_field_force(from, "duration")));
  return Status::OK();
}

Status from_json(td_api::setFileGenerationProgress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.generation_id_, get_json_object_field_force(from, "generation_id")));
  TRY_STATUS(from_json(to.expected_size_, get_json_object_field_force(from, "expected_size")));
  TRY_STATUS(from_json(to.local_prefix_size_, get_json_object_field_force(from, "local_prefix_size")));
  return Status::OK();
}

Status from_json(td_api::setForumTopicNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  TRY_STATUS(from_json(to.notification_settings_, get_json_object_field_force(from, "notification_settings")));
  return Status::OK();
}

Status from_json(td_api::setGameScore &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.edit_message_, get_json_object_field_force(from, "edit_message")));
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.score_, get_json_object_field_force(from, "score")));
  TRY_STATUS(from_json(to.force_, get_json_object_field_force(from, "force")));
  return Status::OK();
}

Status from_json(td_api::setGroupCallParticipantIsSpeaking &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  TRY_STATUS(from_json(to.audio_source_, get_json_object_field_force(from, "audio_source")));
  TRY_STATUS(from_json(to.is_speaking_, get_json_object_field_force(from, "is_speaking")));
  return Status::OK();
}

Status from_json(td_api::setGroupCallParticipantVolumeLevel &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  TRY_STATUS(from_json(to.participant_id_, get_json_object_field_force(from, "participant_id")));
  TRY_STATUS(from_json(to.volume_level_, get_json_object_field_force(from, "volume_level")));
  return Status::OK();
}

Status from_json(td_api::setGroupCallTitle &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  return Status::OK();
}

Status from_json(td_api::setInactiveSessionTtl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inactive_session_ttl_days_, get_json_object_field_force(from, "inactive_session_ttl_days")));
  return Status::OK();
}

Status from_json(td_api::setInlineGameScore &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_message_id_, get_json_object_field_force(from, "inline_message_id")));
  TRY_STATUS(from_json(to.edit_message_, get_json_object_field_force(from, "edit_message")));
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.score_, get_json_object_field_force(from, "score")));
  TRY_STATUS(from_json(to.force_, get_json_object_field_force(from, "force")));
  return Status::OK();
}

Status from_json(td_api::setLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, get_json_object_field_force(from, "location")));
  return Status::OK();
}

Status from_json(td_api::setLogStream &to, JsonObject &from) {
  TRY_STATUS(from_json(to.log_stream_, get_json_object_field_force(from, "log_stream")));
  return Status::OK();
}

Status from_json(td_api::setLogTagVerbosityLevel &to, JsonObject &from) {
  TRY_STATUS(from_json(to.tag_, get_json_object_field_force(from, "tag")));
  TRY_STATUS(from_json(to.new_verbosity_level_, get_json_object_field_force(from, "new_verbosity_level")));
  return Status::OK();
}

Status from_json(td_api::setLogVerbosityLevel &to, JsonObject &from) {
  TRY_STATUS(from_json(to.new_verbosity_level_, get_json_object_field_force(from, "new_verbosity_level")));
  return Status::OK();
}

Status from_json(td_api::setLoginEmailAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.new_login_email_address_, get_json_object_field_force(from, "new_login_email_address")));
  return Status::OK();
}

Status from_json(td_api::setMenuButton &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.menu_button_, get_json_object_field_force(from, "menu_button")));
  return Status::OK();
}

Status from_json(td_api::setName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.first_name_, get_json_object_field_force(from, "first_name")));
  TRY_STATUS(from_json(to.last_name_, get_json_object_field_force(from, "last_name")));
  return Status::OK();
}

Status from_json(td_api::setNetworkType &to, JsonObject &from) {
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  return Status::OK();
}

Status from_json(td_api::setOption &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  TRY_STATUS(from_json(to.value_, get_json_object_field_force(from, "value")));
  return Status::OK();
}

Status from_json(td_api::setPassportElement &to, JsonObject &from) {
  TRY_STATUS(from_json(to.element_, get_json_object_field_force(from, "element")));
  TRY_STATUS(from_json(to.password_, get_json_object_field_force(from, "password")));
  return Status::OK();
}

Status from_json(td_api::setPassportElementErrors &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.errors_, get_json_object_field_force(from, "errors")));
  return Status::OK();
}

Status from_json(td_api::setPassword &to, JsonObject &from) {
  TRY_STATUS(from_json(to.old_password_, get_json_object_field_force(from, "old_password")));
  TRY_STATUS(from_json(to.new_password_, get_json_object_field_force(from, "new_password")));
  TRY_STATUS(from_json(to.new_hint_, get_json_object_field_force(from, "new_hint")));
  TRY_STATUS(from_json(to.set_recovery_email_address_, get_json_object_field_force(from, "set_recovery_email_address")));
  TRY_STATUS(from_json(to.new_recovery_email_address_, get_json_object_field_force(from, "new_recovery_email_address")));
  return Status::OK();
}

Status from_json(td_api::setPinnedChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_list_, get_json_object_field_force(from, "chat_list")));
  TRY_STATUS(from_json(to.chat_ids_, get_json_object_field_force(from, "chat_ids")));
  return Status::OK();
}

Status from_json(td_api::setPinnedForumTopics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_thread_ids_, get_json_object_field_force(from, "message_thread_ids")));
  return Status::OK();
}

Status from_json(td_api::setPollAnswer &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.option_ids_, get_json_object_field_force(from, "option_ids")));
  return Status::OK();
}

Status from_json(td_api::setProfilePhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.photo_, get_json_object_field_force(from, "photo")));
  TRY_STATUS(from_json(to.is_public_, get_json_object_field_force(from, "is_public")));
  return Status::OK();
}

Status from_json(td_api::setRecoveryEmailAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.password_, get_json_object_field_force(from, "password")));
  TRY_STATUS(from_json(to.new_recovery_email_address_, get_json_object_field_force(from, "new_recovery_email_address")));
  return Status::OK();
}

Status from_json(td_api::setScopeNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.scope_, get_json_object_field_force(from, "scope")));
  TRY_STATUS(from_json(to.notification_settings_, get_json_object_field_force(from, "notification_settings")));
  return Status::OK();
}

Status from_json(td_api::setStickerEmojis &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, get_json_object_field_force(from, "sticker")));
  TRY_STATUS(from_json(to.emojis_, get_json_object_field_force(from, "emojis")));
  return Status::OK();
}

Status from_json(td_api::setStickerKeywords &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, get_json_object_field_force(from, "sticker")));
  TRY_STATUS(from_json(to.keywords_, get_json_object_field_force(from, "keywords")));
  return Status::OK();
}

Status from_json(td_api::setStickerMaskPosition &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, get_json_object_field_force(from, "sticker")));
  TRY_STATUS(from_json(to.mask_position_, get_json_object_field_force(from, "mask_position")));
  return Status::OK();
}

Status from_json(td_api::setStickerPositionInSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, get_json_object_field_force(from, "sticker")));
  TRY_STATUS(from_json(to.position_, get_json_object_field_force(from, "position")));
  return Status::OK();
}

Status from_json(td_api::setStickerSetThumbnail &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  TRY_STATUS(from_json(to.thumbnail_, get_json_object_field_force(from, "thumbnail")));
  return Status::OK();
}

Status from_json(td_api::setStickerSetTitle &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, get_json_object_field_force(from, "name")));
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  return Status::OK();
}

Status from_json(td_api::setSupergroupStickerSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  TRY_STATUS(from_json(to.sticker_set_id_, get_json_object_field_force(from, "sticker_set_id")));
  return Status::OK();
}

Status from_json(td_api::setSupergroupUsername &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  TRY_STATUS(from_json(to.username_, get_json_object_field_force(from, "username")));
  return Status::OK();
}

Status from_json(td_api::setTdlibParameters &to, JsonObject &from) {
  TRY_STATUS(from_json(to.use_test_dc_, get_json_object_field_force(from, "use_test_dc")));
  TRY_STATUS(from_json(to.database_directory_, get_json_object_field_force(from, "database_directory")));
  TRY_STATUS(from_json(to.files_directory_, get_json_object_field_force(from, "files_directory")));
  TRY_STATUS(from_json_bytes(to.database_encryption_key_, get_json_object_field_force(from, "database_encryption_key")));
  TRY_STATUS(from_json(to.use_file_database_, get_json_object_field_force(from, "use_file_database")));
  TRY_STATUS(from_json(to.use_chat_info_database_, get_json_object_field_force(from, "use_chat_info_database")));
  TRY_STATUS(from_json(to.use_message_database_, get_json_object_field_force(from, "use_message_database")));
  TRY_STATUS(from_json(to.use_secret_chats_, get_json_object_field_force(from, "use_secret_chats")));
  TRY_STATUS(from_json(to.api_id_, get_json_object_field_force(from, "api_id")));
  TRY_STATUS(from_json(to.api_hash_, get_json_object_field_force(from, "api_hash")));
  TRY_STATUS(from_json(to.system_language_code_, get_json_object_field_force(from, "system_language_code")));
  TRY_STATUS(from_json(to.device_model_, get_json_object_field_force(from, "device_model")));
  TRY_STATUS(from_json(to.system_version_, get_json_object_field_force(from, "system_version")));
  TRY_STATUS(from_json(to.application_version_, get_json_object_field_force(from, "application_version")));
  TRY_STATUS(from_json(to.enable_storage_optimizer_, get_json_object_field_force(from, "enable_storage_optimizer")));
  TRY_STATUS(from_json(to.ignore_file_names_, get_json_object_field_force(from, "ignore_file_names")));
  return Status::OK();
}

Status from_json(td_api::setUserPersonalProfilePhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.photo_, get_json_object_field_force(from, "photo")));
  return Status::OK();
}

Status from_json(td_api::setUserPrivacySettingRules &to, JsonObject &from) {
  TRY_STATUS(from_json(to.setting_, get_json_object_field_force(from, "setting")));
  TRY_STATUS(from_json(to.rules_, get_json_object_field_force(from, "rules")));
  return Status::OK();
}

Status from_json(td_api::setUserSupportInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.message_, get_json_object_field_force(from, "message")));
  return Status::OK();
}

Status from_json(td_api::setUsername &to, JsonObject &from) {
  TRY_STATUS(from_json(to.username_, get_json_object_field_force(from, "username")));
  return Status::OK();
}

Status from_json(td_api::setVideoChatDefaultParticipant &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.default_participant_id_, get_json_object_field_force(from, "default_participant_id")));
  return Status::OK();
}

Status from_json(td_api::shareChatWithBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.button_id_, get_json_object_field_force(from, "button_id")));
  TRY_STATUS(from_json(to.shared_chat_id_, get_json_object_field_force(from, "shared_chat_id")));
  TRY_STATUS(from_json(to.only_check_, get_json_object_field_force(from, "only_check")));
  return Status::OK();
}

Status from_json(td_api::sharePhoneNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  return Status::OK();
}

Status from_json(td_api::shareUserWithBot &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.button_id_, get_json_object_field_force(from, "button_id")));
  TRY_STATUS(from_json(to.shared_user_id_, get_json_object_field_force(from, "shared_user_id")));
  TRY_STATUS(from_json(to.only_check_, get_json_object_field_force(from, "only_check")));
  return Status::OK();
}

Status from_json(td_api::startGroupCallRecording &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  TRY_STATUS(from_json(to.title_, get_json_object_field_force(from, "title")));
  TRY_STATUS(from_json(to.record_video_, get_json_object_field_force(from, "record_video")));
  TRY_STATUS(from_json(to.use_portrait_orientation_, get_json_object_field_force(from, "use_portrait_orientation")));
  return Status::OK();
}

Status from_json(td_api::startGroupCallScreenSharing &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  TRY_STATUS(from_json(to.audio_source_id_, get_json_object_field_force(from, "audio_source_id")));
  TRY_STATUS(from_json(to.payload_, get_json_object_field_force(from, "payload")));
  return Status::OK();
}

Status from_json(td_api::startScheduledGroupCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  return Status::OK();
}

Status from_json(td_api::stopPoll &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.reply_markup_, get_json_object_field_force(from, "reply_markup")));
  return Status::OK();
}

Status from_json(td_api::suggestUserProfilePhoto &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.photo_, get_json_object_field_force(from, "photo")));
  return Status::OK();
}

Status from_json(td_api::synchronizeLanguagePack &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, get_json_object_field_force(from, "language_pack_id")));
  return Status::OK();
}

Status from_json(td_api::terminateAllOtherSessions &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::terminateSession &to, JsonObject &from) {
  TRY_STATUS(from_json(to.session_id_, get_json_object_field_force(from, "session_id")));
  return Status::OK();
}

Status from_json(td_api::testCallBytes &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.x_, get_json_object_field_force(from, "x")));
  return Status::OK();
}

Status from_json(td_api::testCallEmpty &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::testCallString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.x_, get_json_object_field_force(from, "x")));
  return Status::OK();
}

Status from_json(td_api::testCallVectorInt &to, JsonObject &from) {
  TRY_STATUS(from_json(to.x_, get_json_object_field_force(from, "x")));
  return Status::OK();
}

Status from_json(td_api::testCallVectorIntObject &to, JsonObject &from) {
  TRY_STATUS(from_json(to.x_, get_json_object_field_force(from, "x")));
  return Status::OK();
}

Status from_json(td_api::testCallVectorString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.x_, get_json_object_field_force(from, "x")));
  return Status::OK();
}

Status from_json(td_api::testCallVectorStringObject &to, JsonObject &from) {
  TRY_STATUS(from_json(to.x_, get_json_object_field_force(from, "x")));
  return Status::OK();
}

Status from_json(td_api::testGetDifference &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::testNetwork &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::testProxy &to, JsonObject &from) {
  TRY_STATUS(from_json(to.server_, get_json_object_field_force(from, "server")));
  TRY_STATUS(from_json(to.port_, get_json_object_field_force(from, "port")));
  TRY_STATUS(from_json(to.type_, get_json_object_field_force(from, "type")));
  TRY_STATUS(from_json(to.dc_id_, get_json_object_field_force(from, "dc_id")));
  TRY_STATUS(from_json(to.timeout_, get_json_object_field_force(from, "timeout")));
  return Status::OK();
}

Status from_json(td_api::testReturnError &to, JsonObject &from) {
  TRY_STATUS(from_json(to.error_, get_json_object_field_force(from, "error")));
  return Status::OK();
}

Status from_json(td_api::testSquareInt &to, JsonObject &from) {
  TRY_STATUS(from_json(to.x_, get_json_object_field_force(from, "x")));
  return Status::OK();
}

Status from_json(td_api::testUseUpdate &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::toggleAllDownloadsArePaused &to, JsonObject &from) {
  TRY_STATUS(from_json(to.are_paused_, get_json_object_field_force(from, "are_paused")));
  return Status::OK();
}

Status from_json(td_api::toggleBotIsAddedToAttachmentMenu &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.is_added_, get_json_object_field_force(from, "is_added")));
  TRY_STATUS(from_json(to.allow_write_access_, get_json_object_field_force(from, "allow_write_access")));
  return Status::OK();
}

Status from_json(td_api::toggleBotUsernameIsActive &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, get_json_object_field_force(from, "bot_user_id")));
  TRY_STATUS(from_json(to.username_, get_json_object_field_force(from, "username")));
  TRY_STATUS(from_json(to.is_active_, get_json_object_field_force(from, "is_active")));
  return Status::OK();
}

Status from_json(td_api::toggleChatDefaultDisableNotification &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.default_disable_notification_, get_json_object_field_force(from, "default_disable_notification")));
  return Status::OK();
}

Status from_json(td_api::toggleChatHasProtectedContent &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.has_protected_content_, get_json_object_field_force(from, "has_protected_content")));
  return Status::OK();
}

Status from_json(td_api::toggleChatIsMarkedAsUnread &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.is_marked_as_unread_, get_json_object_field_force(from, "is_marked_as_unread")));
  return Status::OK();
}

Status from_json(td_api::toggleChatIsPinned &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_list_, get_json_object_field_force(from, "chat_list")));
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.is_pinned_, get_json_object_field_force(from, "is_pinned")));
  return Status::OK();
}

Status from_json(td_api::toggleChatIsTranslatable &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.is_translatable_, get_json_object_field_force(from, "is_translatable")));
  return Status::OK();
}

Status from_json(td_api::toggleDownloadIsPaused &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, get_json_object_field_force(from, "file_id")));
  TRY_STATUS(from_json(to.is_paused_, get_json_object_field_force(from, "is_paused")));
  return Status::OK();
}

Status from_json(td_api::toggleForumTopicIsClosed &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  TRY_STATUS(from_json(to.is_closed_, get_json_object_field_force(from, "is_closed")));
  return Status::OK();
}

Status from_json(td_api::toggleForumTopicIsPinned &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  TRY_STATUS(from_json(to.is_pinned_, get_json_object_field_force(from, "is_pinned")));
  return Status::OK();
}

Status from_json(td_api::toggleGeneralForumTopicIsHidden &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.is_hidden_, get_json_object_field_force(from, "is_hidden")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallEnabledStartNotification &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  TRY_STATUS(from_json(to.enabled_start_notification_, get_json_object_field_force(from, "enabled_start_notification")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallIsMyVideoEnabled &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  TRY_STATUS(from_json(to.is_my_video_enabled_, get_json_object_field_force(from, "is_my_video_enabled")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallIsMyVideoPaused &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  TRY_STATUS(from_json(to.is_my_video_paused_, get_json_object_field_force(from, "is_my_video_paused")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallMuteNewParticipants &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  TRY_STATUS(from_json(to.mute_new_participants_, get_json_object_field_force(from, "mute_new_participants")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallParticipantIsHandRaised &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  TRY_STATUS(from_json(to.participant_id_, get_json_object_field_force(from, "participant_id")));
  TRY_STATUS(from_json(to.is_hand_raised_, get_json_object_field_force(from, "is_hand_raised")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallParticipantIsMuted &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  TRY_STATUS(from_json(to.participant_id_, get_json_object_field_force(from, "participant_id")));
  TRY_STATUS(from_json(to.is_muted_, get_json_object_field_force(from, "is_muted")));
  return Status::OK();
}

Status from_json(td_api::toggleGroupCallScreenSharingIsPaused &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, get_json_object_field_force(from, "group_call_id")));
  TRY_STATUS(from_json(to.is_paused_, get_json_object_field_force(from, "is_paused")));
  return Status::OK();
}

Status from_json(td_api::toggleMessageSenderIsBlocked &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sender_id_, get_json_object_field_force(from, "sender_id")));
  TRY_STATUS(from_json(to.is_blocked_, get_json_object_field_force(from, "is_blocked")));
  return Status::OK();
}

Status from_json(td_api::toggleSessionCanAcceptCalls &to, JsonObject &from) {
  TRY_STATUS(from_json(to.session_id_, get_json_object_field_force(from, "session_id")));
  TRY_STATUS(from_json(to.can_accept_calls_, get_json_object_field_force(from, "can_accept_calls")));
  return Status::OK();
}

Status from_json(td_api::toggleSessionCanAcceptSecretChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.session_id_, get_json_object_field_force(from, "session_id")));
  TRY_STATUS(from_json(to.can_accept_secret_chats_, get_json_object_field_force(from, "can_accept_secret_chats")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupHasAggressiveAntiSpamEnabled &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  TRY_STATUS(from_json(to.has_aggressive_anti_spam_enabled_, get_json_object_field_force(from, "has_aggressive_anti_spam_enabled")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupHasHiddenMembers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  TRY_STATUS(from_json(to.has_hidden_members_, get_json_object_field_force(from, "has_hidden_members")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupIsAllHistoryAvailable &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  TRY_STATUS(from_json(to.is_all_history_available_, get_json_object_field_force(from, "is_all_history_available")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupIsBroadcastGroup &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupIsForum &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  TRY_STATUS(from_json(to.is_forum_, get_json_object_field_force(from, "is_forum")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupJoinByRequest &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  TRY_STATUS(from_json(to.join_by_request_, get_json_object_field_force(from, "join_by_request")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupJoinToSendMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  TRY_STATUS(from_json(to.join_to_send_messages_, get_json_object_field_force(from, "join_to_send_messages")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupSignMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  TRY_STATUS(from_json(to.sign_messages_, get_json_object_field_force(from, "sign_messages")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupUsernameIsActive &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, get_json_object_field_force(from, "supergroup_id")));
  TRY_STATUS(from_json(to.username_, get_json_object_field_force(from, "username")));
  TRY_STATUS(from_json(to.is_active_, get_json_object_field_force(from, "is_active")));
  return Status::OK();
}

Status from_json(td_api::toggleUsernameIsActive &to, JsonObject &from) {
  TRY_STATUS(from_json(to.username_, get_json_object_field_force(from, "username")));
  TRY_STATUS(from_json(to.is_active_, get_json_object_field_force(from, "is_active")));
  return Status::OK();
}

Status from_json(td_api::transferChatOwnership &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.password_, get_json_object_field_force(from, "password")));
  return Status::OK();
}

Status from_json(td_api::translateMessageText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  TRY_STATUS(from_json(to.to_language_code_, get_json_object_field_force(from, "to_language_code")));
  return Status::OK();
}

Status from_json(td_api::translateText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, get_json_object_field_force(from, "text")));
  TRY_STATUS(from_json(to.to_language_code_, get_json_object_field_force(from, "to_language_code")));
  return Status::OK();
}

Status from_json(td_api::unpinAllChatMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::unpinAllMessageThreadMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_thread_id_, get_json_object_field_force(from, "message_thread_id")));
  return Status::OK();
}

Status from_json(td_api::unpinChatMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_id_, get_json_object_field_force(from, "message_id")));
  return Status::OK();
}

Status from_json(td_api::upgradeBasicGroupChatToSupergroupChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  return Status::OK();
}

Status from_json(td_api::uploadStickerFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, get_json_object_field_force(from, "user_id")));
  TRY_STATUS(from_json(to.sticker_format_, get_json_object_field_force(from, "sticker_format")));
  TRY_STATUS(from_json(to.sticker_, get_json_object_field_force(from, "sticker")));
  return Status::OK();
}

Status from_json(td_api::validateOrderInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.input_invoice_, get_json_object_field_force(from, "input_invoice")));
  TRY_STATUS(from_json(to.order_info_, get_json_object_field_force(from, "order_info")));
  TRY_STATUS(from_json(to.allow_save_, get_json_object_field_force(from, "allow_save")));
  return Status::OK();
}

Status from_json(td_api::viewMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, get_json_object_field_force(from, "chat_id")));
  TRY_STATUS(from_json(to.message_ids_, get_json_object_field_force(from, "message_ids")));
  TRY_STATUS(from_json(to.source_, get_json_object_field_force(from, "source")));
  TRY_STATUS(from_json(to.force_read_, get_json_object_field_force(from, "force_read")));
  return Status::OK();
}

Status from_json(td_api::viewPremiumFeature &to, JsonObject &from) {
  TRY_STATUS(from_json(to.feature_, get_json_object_field_force(from, "feature")));
  return Status::OK();
}

Status from_json(td_api::viewTrendingStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_set_ids_, get_json_object_field_force(from, "sticker_set_ids")));
  return Status::OK();
}

Status from_json(td_api::writeGeneratedFilePart &to, JsonObject &from) {
  TRY_STATUS(from_json(to.generation_id_, get_json_object_field_force(from, "generation_id")));
  TRY_STATUS(from_json(to.offset_, get_json_object_field_force(from, "offset")));
  TRY_STATUS(from_json_bytes(to.data_, get_json_object_field_force(from, "data")));
  return Status::OK();
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

void to_json(JsonValueScope &jv, const td_api::attachmentMenuBot &object) {
  auto jo = jv.enter_object();
  jo("@type", "attachmentMenuBot");
  jo("bot_user_id", object.bot_user_id_);
  jo("supports_self_chat", JsonBool{object.supports_self_chat_});
  jo("supports_user_chats", JsonBool{object.supports_user_chats_});
  jo("supports_bot_chats", JsonBool{object.supports_bot_chats_});
  jo("supports_group_chats", JsonBool{object.supports_group_chats_});
  jo("supports_channel_chats", JsonBool{object.supports_channel_chats_});
  jo("supports_settings", JsonBool{object.supports_settings_});
  jo("request_write_access", JsonBool{object.request_write_access_});
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
  if (object.android_icon_) {
    jo("android_icon", ToJson(*object.android_icon_));
  }
  if (object.macos_icon_) {
    jo("macos_icon", ToJson(*object.macos_icon_));
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
  jo("nonce", base64_encode(object.nonce_));
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
  if (object.default_group_administrator_rights_) {
    jo("default_group_administrator_rights", ToJson(*object.default_group_administrator_rights_));
  }
  if (object.default_channel_administrator_rights_) {
    jo("default_channel_administrator_rights", ToJson(*object.default_channel_administrator_rights_));
  }
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

void to_json(JsonValueScope &jv, const td_api::botMenuButton &object) {
  auto jo = jv.enter_object();
  jo("@type", "botMenuButton");
  jo("text", object.text_);
  jo("url", object.url_);
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
  if (object.permissions_) {
    jo("permissions", ToJson(*object.permissions_));
  }
  if (object.last_message_) {
    jo("last_message", ToJson(*object.last_message_));
  }
  jo("positions", ToJson(object.positions_));
  if (object.message_sender_id_) {
    jo("message_sender_id", ToJson(*object.message_sender_id_));
  }
  jo("has_protected_content", JsonBool{object.has_protected_content_});
  jo("is_translatable", JsonBool{object.is_translatable_});
  jo("is_marked_as_unread", JsonBool{object.is_marked_as_unread_});
  jo("is_blocked", JsonBool{object.is_blocked_});
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
  if (object.background_) {
    jo("background", ToJson(*object.background_));
  }
  jo("theme_name", object.theme_name_);
  if (object.action_bar_) {
    jo("action_bar", ToJson(*object.action_bar_));
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
}

void to_json(JsonValueScope &jv, const td_api::chatAvailableReactionsSome &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatAvailableReactionsSome");
  jo("reactions", ToJson(object.reactions_));
}

void to_json(JsonValueScope &jv, const td_api::chatBackground &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatBackground");
  if (object.background_) {
    jo("background", ToJson(*object.background_));
  }
  jo("dark_theme_dimming", object.dark_theme_dimming_);
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

void to_json(JsonValueScope &jv, const td_api::chatEventDescriptionChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatEventDescriptionChanged");
  jo("old_description", object.old_description_);
  jo("new_description", object.new_description_);
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
  jo("member_limit", object.member_limit_);
  jo("member_count", object.member_count_);
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
  jo("description", object.description_);
  jo("member_count", object.member_count_);
  jo("member_user_ids", ToJson(object.member_user_ids_));
  jo("creates_join_request", JsonBool{object.creates_join_request_});
  jo("is_public", JsonBool{object.is_public_});
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
  jo("can_add_web_page_previews", JsonBool{object.can_add_web_page_previews_});
  jo("can_change_info", JsonBool{object.can_change_info_});
  jo("can_invite_users", JsonBool{object.can_invite_users_});
  jo("can_pin_messages", JsonBool{object.can_pin_messages_});
  jo("can_manage_topics", JsonBool{object.can_manage_topics_});
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
  if (object.mean_view_count_) {
    jo("mean_view_count", ToJson(*object.mean_view_count_));
  }
  if (object.mean_share_count_) {
    jo("mean_share_count", ToJson(*object.mean_share_count_));
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
  if (object.instant_view_interaction_graph_) {
    jo("instant_view_interaction_graph", ToJson(*object.instant_view_interaction_graph_));
  }
  jo("recent_message_interactions", ToJson(object.recent_message_interactions_));
}

void to_json(JsonValueScope &jv, const td_api::chatStatisticsAdministratorActionsInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatStatisticsAdministratorActionsInfo");
  jo("user_id", object.user_id_);
  jo("deleted_message_count", object.deleted_message_count_);
  jo("banned_user_count", object.banned_user_count_);
  jo("restricted_user_count", object.restricted_user_count_);
}

void to_json(JsonValueScope &jv, const td_api::chatStatisticsInviterInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatStatisticsInviterInfo");
  jo("user_id", object.user_id_);
  jo("added_member_count", object.added_member_count_);
}

void to_json(JsonValueScope &jv, const td_api::chatStatisticsMessageInteractionInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatStatisticsMessageInteractionInfo");
  jo("message_id", object.message_id_);
  jo("view_count", object.view_count_);
  jo("forward_count", object.forward_count_);
}

void to_json(JsonValueScope &jv, const td_api::chatStatisticsMessageSenderInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatStatisticsMessageSenderInfo");
  jo("user_id", object.user_id_);
  jo("sent_message_count", object.sent_message_count_);
  jo("average_character_count", object.average_character_count_);
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

void to_json(JsonValueScope &jv, const td_api::closedVectorPath &object) {
  auto jo = jv.enter_object();
  jo("@type", "closedVectorPath");
  jo("commands", ToJson(object.commands_));
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
  jo("ip", object.ip_);
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
  jo("reply_to_message_id", object.reply_to_message_id_);
  jo("date", object.date_);
  if (object.input_message_text_) {
    jo("input_message_text", ToJson(*object.input_message_text_));
  }
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
  jo("emojis", ToJson(object.emojis_));
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
}

void to_json(JsonValueScope &jv, const td_api::emojiStatuses &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojiStatuses");
  jo("emoji_statuses", ToJson(object.emoji_statuses_));
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

void to_json(JsonValueScope &jv, const td_api::fileTypeVoiceNote &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeVoiceNote");
}

void to_json(JsonValueScope &jv, const td_api::fileTypeWallpaper &object) {
  auto jo = jv.enter_object();
  jo("@type", "fileTypeWallpaper");
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
  if (object.expiry_date_) {
    jo("expiry_date", ToJson(*object.expiry_date_));
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
  jo("disable_web_page_preview", JsonBool{object.disable_web_page_preview_});
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
  jo("self_destruct_time", object.self_destruct_time_);
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
  jo("self_destruct_time", object.self_destruct_time_);
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
  if (object.extended_media_content_) {
    jo("extended_media_content", ToJson(*object.extended_media_content_));
  }
}

void to_json(JsonValueScope &jv, const td_api::inputMessagePoll &object) {
  auto jo = jv.enter_object();
  jo("@type", "inputMessagePoll");
  jo("question", object.question_);
  jo("options", ToJson(object.options_));
  jo("is_anonymous", JsonBool{object.is_anonymous_});
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("open_period", object.open_period_);
  jo("close_date", object.close_date_);
  jo("is_closed", JsonBool{object.is_closed_});
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

void to_json(JsonValueScope &jv, const td_api::internalLinkTypeChangePhoneNumber &object) {
  auto jo = jv.enter_object();
  jo("@type", "internalLinkTypeChangePhoneNumber");
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
}

void to_json(JsonValueScope &jv, const td_api::invoice &object) {
  auto jo = jv.enter_object();
  jo("@type", "invoice");
  jo("currency", object.currency_);
  jo("price_parts", ToJson(object.price_parts_));
  jo("max_tip_amount", object.max_tip_amount_);
  jo("suggested_tip_amounts", ToJson(object.suggested_tip_amounts_));
  jo("recurring_payment_terms_of_service_url", object.recurring_payment_terms_of_service_url_);
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

void to_json(JsonValueScope &jv, const td_api::keyboardButtonTypeRequestUser &object) {
  auto jo = jv.enter_object();
  jo("@type", "keyboardButtonTypeRequestUser");
  jo("id", object.id_);
  jo("restrict_user_is_bot", JsonBool{object.restrict_user_is_bot_});
  jo("user_is_bot", JsonBool{object.user_is_bot_});
  jo("restrict_user_is_premium", JsonBool{object.restrict_user_is_premium_});
  jo("user_is_premium", JsonBool{object.user_is_premium_});
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
  jo("can_be_edited", JsonBool{object.can_be_edited_});
  jo("can_be_forwarded", JsonBool{object.can_be_forwarded_});
  jo("can_be_saved", JsonBool{object.can_be_saved_});
  jo("can_be_deleted_only_for_self", JsonBool{object.can_be_deleted_only_for_self_});
  jo("can_be_deleted_for_all_users", JsonBool{object.can_be_deleted_for_all_users_});
  jo("can_get_added_reactions", JsonBool{object.can_get_added_reactions_});
  jo("can_get_statistics", JsonBool{object.can_get_statistics_});
  jo("can_get_message_thread", JsonBool{object.can_get_message_thread_});
  jo("can_get_viewers", JsonBool{object.can_get_viewers_});
  jo("can_get_media_timestamp_links", JsonBool{object.can_get_media_timestamp_links_});
  jo("can_report_reactions", JsonBool{object.can_report_reactions_});
  jo("has_timestamped_media", JsonBool{object.has_timestamped_media_});
  jo("is_channel_post", JsonBool{object.is_channel_post_});
  jo("is_topic_message", JsonBool{object.is_topic_message_});
  jo("contains_unread_mention", JsonBool{object.contains_unread_mention_});
  jo("date", object.date_);
  jo("edit_date", object.edit_date_);
  if (object.forward_info_) {
    jo("forward_info", ToJson(*object.forward_info_));
  }
  if (object.interaction_info_) {
    jo("interaction_info", ToJson(*object.interaction_info_));
  }
  jo("unread_reactions", ToJson(object.unread_reactions_));
  jo("reply_in_chat_id", object.reply_in_chat_id_);
  jo("reply_to_message_id", object.reply_to_message_id_);
  jo("message_thread_id", object.message_thread_id_);
  jo("self_destruct_time", object.self_destruct_time_);
  jo("self_destruct_in", object.self_destruct_in_);
  jo("auto_delete_in", object.auto_delete_in_);
  jo("via_bot_user_id", object.via_bot_user_id_);
  jo("author_signature", object.author_signature_);
  jo("media_album_id", ToJson(JsonInt64{object.media_album_id_}));
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
  if (object.web_page_) {
    jo("web_page", ToJson(*object.web_page_));
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

void to_json(JsonValueScope &jv, const td_api::messagePhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "messagePhoto");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
  jo("has_spoiler", JsonBool{object.has_spoiler_});
  jo("is_secret", JsonBool{object.is_secret_});
}

void to_json(JsonValueScope &jv, const td_api::messageExpiredPhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageExpiredPhoto");
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
  jo("has_spoiler", JsonBool{object.has_spoiler_});
  jo("is_secret", JsonBool{object.is_secret_});
}

void to_json(JsonValueScope &jv, const td_api::messageExpiredVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageExpiredVideo");
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

void to_json(JsonValueScope &jv, const td_api::messageInvoice &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageInvoice");
  jo("title", object.title_);
  if (object.description_) {
    jo("description", ToJson(*object.description_));
  }
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  jo("currency", object.currency_);
  jo("total_amount", object.total_amount_);
  jo("start_parameter", object.start_parameter_);
  jo("is_test", JsonBool{object.is_test_});
  jo("need_shipping_address", JsonBool{object.need_shipping_address_});
  jo("receipt_message_id", object.receipt_message_id_);
  if (object.extended_media_) {
    jo("extended_media", ToJson(*object.extended_media_));
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

void to_json(JsonValueScope &jv, const td_api::messageGiftedPremium &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageGiftedPremium");
  jo("gifter_user_id", object.gifter_user_id_);
  jo("currency", object.currency_);
  jo("amount", object.amount_);
  jo("cryptocurrency", object.cryptocurrency_);
  jo("cryptocurrency_amount", ToJson(JsonInt64{object.cryptocurrency_amount_}));
  jo("month_count", object.month_count_);
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageContactRegistered &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageContactRegistered");
}

void to_json(JsonValueScope &jv, const td_api::messageUserShared &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageUserShared");
  jo("user_id", object.user_id_);
  jo("button_id", object.button_id_);
}

void to_json(JsonValueScope &jv, const td_api::messageChatShared &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageChatShared");
  jo("chat_id", object.chat_id_);
  jo("button_id", object.button_id_);
}

void to_json(JsonValueScope &jv, const td_api::messageWebsiteConnected &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageWebsiteConnected");
  jo("domain_name", object.domain_name_);
}

void to_json(JsonValueScope &jv, const td_api::messageBotWriteAccessAllowed &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageBotWriteAccessAllowed");
  if (object.web_app_) {
    jo("web_app", ToJson(*object.web_app_));
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
}

void to_json(JsonValueScope &jv, const td_api::MessageExtendedMedia &object) {
  td_api::downcast_call(const_cast<td_api::MessageExtendedMedia &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::messageExtendedMediaPreview &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageExtendedMediaPreview");
  jo("width", object.width_);
  jo("height", object.height_);
  jo("duration", object.duration_);
  if (object.minithumbnail_) {
    jo("minithumbnail", ToJson(*object.minithumbnail_));
  }
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageExtendedMediaPhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageExtendedMediaPhoto");
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageExtendedMediaVideo &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageExtendedMediaVideo");
  if (object.video_) {
    jo("video", ToJson(*object.video_));
  }
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageExtendedMediaUnsupported &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageExtendedMediaUnsupported");
  if (object.caption_) {
    jo("caption", ToJson(*object.caption_));
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
  jo("public_service_announcement_type", object.public_service_announcement_type_);
  jo("from_chat_id", object.from_chat_id_);
  jo("from_message_id", object.from_message_id_);
}

void to_json(JsonValueScope &jv, const td_api::MessageForwardOrigin &object) {
  td_api::downcast_call(const_cast<td_api::MessageForwardOrigin &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::messageForwardOriginUser &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageForwardOriginUser");
  jo("sender_user_id", object.sender_user_id_);
}

void to_json(JsonValueScope &jv, const td_api::messageForwardOriginChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageForwardOriginChat");
  jo("sender_chat_id", object.sender_chat_id_);
  jo("author_signature", object.author_signature_);
}

void to_json(JsonValueScope &jv, const td_api::messageForwardOriginHiddenUser &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageForwardOriginHiddenUser");
  jo("sender_name", object.sender_name_);
}

void to_json(JsonValueScope &jv, const td_api::messageForwardOriginChannel &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageForwardOriginChannel");
  jo("chat_id", object.chat_id_);
  jo("message_id", object.message_id_);
  jo("author_signature", object.author_signature_);
}

void to_json(JsonValueScope &jv, const td_api::messageForwardOriginMessageImport &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageForwardOriginMessageImport");
  jo("sender_name", object.sender_name_);
}

void to_json(JsonValueScope &jv, const td_api::messageInteractionInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageInteractionInfo");
  jo("view_count", object.view_count_);
  jo("forward_count", object.forward_count_);
  if (object.reply_info_) {
    jo("reply_info", ToJson(*object.reply_info_));
  }
  jo("reactions", ToJson(object.reactions_));
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

void to_json(JsonValueScope &jv, const td_api::messageReaction &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageReaction");
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("total_count", object.total_count_);
  jo("is_chosen", JsonBool{object.is_chosen_});
  jo("recent_sender_ids", ToJson(object.recent_sender_ids_));
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
  jo("error_code", object.error_code_);
  jo("error_message", object.error_message_);
  jo("can_retry", JsonBool{object.can_retry_});
  jo("need_another_sender", JsonBool{object.need_another_sender_});
  jo("retry_after", object.retry_after_);
}

void to_json(JsonValueScope &jv, const td_api::messageStatistics &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageStatistics");
  if (object.message_interaction_graph_) {
    jo("message_interaction_graph", ToJson(*object.message_interaction_graph_));
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
  if (object.invoice_) {
    jo("invoice", ToJson(*object.invoice_));
  }
  jo("seller_bot_user_id", object.seller_bot_user_id_);
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
  jo("product_title", object.product_title_);
  if (object.product_description_) {
    jo("product_description", ToJson(*object.product_description_));
  }
  if (object.product_photo_) {
    jo("product_photo", ToJson(*object.product_photo_));
  }
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
  jo("title", object.title_);
  if (object.description_) {
    jo("description", ToJson(*object.description_));
  }
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  jo("date", object.date_);
  jo("seller_bot_user_id", object.seller_bot_user_id_);
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
  jo("question", object.question_);
  jo("options", ToJson(object.options_));
  jo("total_voter_count", object.total_voter_count_);
  jo("recent_voter_user_ids", ToJson(object.recent_voter_user_ids_));
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
  jo("text", object.text_);
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
  jo("ip", object.ip_);
  jo("country", object.country_);
  jo("region", object.region_);
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
  jo("sponsor_chat_id", object.sponsor_chat_id_);
  if (object.sponsor_chat_info_) {
    jo("sponsor_chat_info", ToJson(*object.sponsor_chat_info_));
  }
  jo("show_chat_photo", JsonBool{object.show_chat_photo_});
  if (object.link_) {
    jo("link", ToJson(*object.link_));
  }
  if (object.content_) {
    jo("content", ToJson(*object.content_));
  }
  jo("sponsor_info", object.sponsor_info_);
  jo("additional_info", object.additional_info_);
}

void to_json(JsonValueScope &jv, const td_api::sponsoredMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "sponsoredMessages");
  jo("messages", ToJson(object.messages_));
  jo("messages_between", object.messages_between_);
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
  jo("is_installed", JsonBool{object.is_installed_});
  jo("is_archived", JsonBool{object.is_archived_});
  jo("is_official", JsonBool{object.is_official_});
  if (object.sticker_format_) {
    jo("sticker_format", ToJson(*object.sticker_format_));
  }
  if (object.sticker_type_) {
    jo("sticker_type", ToJson(*object.sticker_type_));
  }
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
  jo("is_installed", JsonBool{object.is_installed_});
  jo("is_archived", JsonBool{object.is_archived_});
  jo("is_official", JsonBool{object.is_official_});
  if (object.sticker_format_) {
    jo("sticker_format", ToJson(*object.sticker_format_));
  }
  if (object.sticker_type_) {
    jo("sticker_type", ToJson(*object.sticker_type_));
  }
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

void to_json(JsonValueScope &jv, const td_api::suggestedActionSubscribeToAnnualPremium &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedActionSubscribeToAnnualPremium");
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
  jo("has_linked_chat", JsonBool{object.has_linked_chat_});
  jo("has_location", JsonBool{object.has_location_});
  jo("sign_messages", JsonBool{object.sign_messages_});
  jo("join_to_send_messages", JsonBool{object.join_to_send_messages_});
  jo("join_by_request", JsonBool{object.join_by_request_});
  jo("is_slow_mode_enabled", JsonBool{object.is_slow_mode_enabled_});
  jo("is_channel", JsonBool{object.is_channel_});
  jo("is_broadcast_group", JsonBool{object.is_broadcast_group_});
  jo("is_forum", JsonBool{object.is_forum_});
  jo("is_verified", JsonBool{object.is_verified_});
  jo("restriction_reason", object.restriction_reason_);
  jo("is_scam", JsonBool{object.is_scam_});
  jo("is_fake", JsonBool{object.is_fake_});
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
  jo("can_get_members", JsonBool{object.can_get_members_});
  jo("has_hidden_members", JsonBool{object.has_hidden_members_});
  jo("can_hide_members", JsonBool{object.can_hide_members_});
  jo("can_set_username", JsonBool{object.can_set_username_});
  jo("can_set_sticker_set", JsonBool{object.can_set_sticker_set_});
  jo("can_set_location", JsonBool{object.can_set_location_});
  jo("can_get_statistics", JsonBool{object.can_get_statistics_});
  jo("can_toggle_aggressive_anti_spam", JsonBool{object.can_toggle_aggressive_anti_spam_});
  jo("is_all_history_available", JsonBool{object.is_all_history_available_});
  jo("has_aggressive_anti_spam_enabled", JsonBool{object.has_aggressive_anti_spam_enabled_});
  jo("sticker_set_id", ToJson(JsonInt64{object.sticker_set_id_}));
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

void to_json(JsonValueScope &jv, const td_api::trendingStickerSets &object) {
  auto jo = jv.enter_object();
  jo("@type", "trendingStickerSets");
  jo("total_count", object.total_count_);
  jo("sets", ToJson(object.sets_));
  jo("is_premium", JsonBool{object.is_premium_});
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
  jo("error_code", object.error_code_);
  jo("error_message", object.error_message_);
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

void to_json(JsonValueScope &jv, const td_api::updateChatIsBlocked &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatIsBlocked");
  jo("chat_id", object.chat_id_);
  jo("is_blocked", JsonBool{object.is_blocked_});
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
}

void to_json(JsonValueScope &jv, const td_api::updateChatOnlineMemberCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateChatOnlineMemberCount");
  jo("chat_id", object.chat_id_);
  jo("online_member_count", object.online_member_count_);
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

void to_json(JsonValueScope &jv, const td_api::updateSelectedBackground &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateSelectedBackground");
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

void to_json(JsonValueScope &jv, const td_api::updateDefaultReactionType &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateDefaultReactionType");
  if (object.reaction_type_) {
    jo("reaction_type", ToJson(*object.reaction_type_));
  }
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

void to_json(JsonValueScope &jv, const td_api::updateAddChatMembersPrivacyForbidden &object) {
  auto jo = jv.enter_object();
  jo("@type", "updateAddChatMembersPrivacyForbidden");
  jo("chat_id", object.chat_id_);
  jo("user_ids", ToJson(object.user_ids_));
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
  jo("user_id", object.user_id_);
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
  if (object.emoji_status_) {
    jo("emoji_status", ToJson(*object.emoji_status_));
  }
  jo("is_contact", JsonBool{object.is_contact_});
  jo("is_mutual_contact", JsonBool{object.is_mutual_contact_});
  jo("is_verified", JsonBool{object.is_verified_});
  jo("is_premium", JsonBool{object.is_premium_});
  jo("is_support", JsonBool{object.is_support_});
  jo("restriction_reason", object.restriction_reason_);
  jo("is_scam", JsonBool{object.is_scam_});
  jo("is_fake", JsonBool{object.is_fake_});
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
  jo("is_blocked", JsonBool{object.is_blocked_});
  jo("can_be_called", JsonBool{object.can_be_called_});
  jo("supports_video_calls", JsonBool{object.supports_video_calls_});
  jo("has_private_calls", JsonBool{object.has_private_calls_});
  jo("has_private_forwards", JsonBool{object.has_private_forwards_});
  jo("has_restricted_voice_and_video_note_messages", JsonBool{object.has_restricted_voice_and_video_note_messages_});
  jo("need_phone_number_privacy_exception", JsonBool{object.need_phone_number_privacy_exception_});
  if (object.bio_) {
    jo("bio", ToJson(*object.bio_));
  }
  jo("premium_gift_options", ToJson(object.premium_gift_options_));
  jo("group_in_common_count", object.group_in_common_count_);
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
}

void to_json(JsonValueScope &jv, const td_api::userStatusLastWeek &object) {
  auto jo = jv.enter_object();
  jo("@type", "userStatusLastWeek");
}

void to_json(JsonValueScope &jv, const td_api::userStatusLastMonth &object) {
  auto jo = jv.enter_object();
  jo("@type", "userStatusLastMonth");
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
  jo("is_inline", JsonBool{object.is_inline_});
  jo("inline_query_placeholder", object.inline_query_placeholder_);
  jo("need_location", JsonBool{object.need_location_});
  jo("can_be_added_to_attachment_menu", JsonBool{object.can_be_added_to_attachment_menu_});
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

void to_json(JsonValueScope &jv, const td_api::webPage &object) {
  auto jo = jv.enter_object();
  jo("@type", "webPage");
  jo("url", object.url_);
  jo("display_url", object.display_url_);
  jo("type", object.type_);
  jo("site_name", object.site_name_);
  jo("title", object.title_);
  if (object.description_) {
    jo("description", ToJson(*object.description_));
  }
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  jo("embed_url", object.embed_url_);
  jo("embed_type", object.embed_type_);
  jo("embed_width", object.embed_width_);
  jo("embed_height", object.embed_height_);
  jo("duration", object.duration_);
  jo("author", object.author_);
  if (object.animation_) {
    jo("animation", ToJson(*object.animation_));
  }
  if (object.audio_) {
    jo("audio", ToJson(*object.audio_));
  }
  if (object.document_) {
    jo("document", ToJson(*object.document_));
  }
  if (object.sticker_) {
    jo("sticker", ToJson(*object.sticker_));
  }
  if (object.video_) {
    jo("video", ToJson(*object.video_));
  }
  if (object.video_note_) {
    jo("video_note", ToJson(*object.video_note_));
  }
  if (object.voice_note_) {
    jo("voice_note", ToJson(*object.voice_note_));
  }
  jo("instant_view_version", object.instant_view_version_);
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
