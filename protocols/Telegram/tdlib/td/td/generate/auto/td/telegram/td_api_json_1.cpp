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

Result<int32> tl_constructor_from_string(td_api::GroupCallDataChannel *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"groupCallDataChannelMain", -32177779},
    {"groupCallDataChannelScreenSharing", -601649103}
  };
  auto it = m.find(str);
  if (it == m.end()) {
    return Status::Error(PSLICE() << "Unknown class \"" << str << "\"");
  }
  return it->second;
}

Result<int32> tl_constructor_from_string(td_api::InputGroupCall *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"inputGroupCallLink", -812157480},
    {"inputGroupCallMessage", -341793768}
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

Result<int32> tl_constructor_from_string(td_api::StorePaymentPurpose *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"storePaymentPurposePremiumSubscription", 1263894804},
    {"storePaymentPurposePremiumGift", -39502443},
    {"storePaymentPurposePremiumGiftCodes", -1072286736},
    {"storePaymentPurposePremiumGiveaway", 1302624938},
    {"storePaymentPurposeStarGiveaway", 211212441},
    {"storePaymentPurposeStars", 410189263},
    {"storePaymentPurposeGiftedStars", 893691428}
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

Result<int32> tl_constructor_from_string(td_api::Object *object, const std::string &str) {
  static const FlatHashMap<Slice, int32, SliceHash> m = {
    {"acceptedGiftTypes", 1783521300},
    {"accountTtl", 1324495492},
    {"address", -2043654342},
    {"affiliateProgramParameters", 1642662996},
    {"affiliateProgramSortOrderProfitability", -1963282585},
    {"affiliateProgramSortOrderCreationDate", -1558628083},
    {"affiliateProgramSortOrderRevenue", 1923269304},
    {"affiliateTypeCurrentUser", 1453785589},
    {"affiliateTypeBot", -1032587200},
    {"affiliateTypeChannel", -683939735},
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
    {"businessBotRights", 1224839038},
    {"businessConnectedBot", -1815439021},
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
    {"chatAdministratorRights", -1142880374},
    {"chatAvailableReactionsAll", 694160279},
    {"chatAvailableReactionsSome", 152513153},
    {"chatEventLogFilters", -1032965711},
    {"chatFolder", 1596164696},
    {"chatFolderIcon", -146104090},
    {"chatFolderName", -330482274},
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
    {"chatMembersFilterMention", 1932296772},
    {"chatMembersFilterRestricted", 1256282813},
    {"chatMembersFilterBanned", -1863102648},
    {"chatMembersFilterBots", -1422567288},
    {"chatNotificationSettings", 1459533846},
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
    {"draftMessage", -1165040650},
    {"emailAddressAuthenticationCode", -993257022},
    {"emailAddressAuthenticationAppleId", 633948265},
    {"emailAddressAuthenticationGoogleId", -19142846},
    {"emojiCategoryTypeDefault", 1188782699},
    {"emojiCategoryTypeRegularStickers", -1337484846},
    {"emojiCategoryTypeEmojiStatus", 1381282631},
    {"emojiCategoryTypeChatPhoto", 1059063081},
    {"emojiStatus", 973424912},
    {"emojiStatusTypeCustomEmoji", -1666780939},
    {"emojiStatusTypeUpgradedGift", -837921804},
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
    {"fileTypeWallpaper", 1854930076},
    {"firebaseAuthenticationSettingsAndroid", -1771112932},
    {"firebaseAuthenticationSettingsIos", 222930116},
    {"formattedText", -252624564},
    {"forumTopicIcon", -818765421},
    {"giftForResaleOrderPrice", 1371740258},
    {"giftForResaleOrderPriceChangeDate", -1694144054},
    {"giftForResaleOrderNumber", -1301157632},
    {"giftResalePriceStar", 1184402054},
    {"giftResalePriceTon", -415435950},
    {"giftSettings", 45783168},
    {"giveawayParameters", 1171549354},
    {"groupCallDataChannelMain", -32177779},
    {"groupCallDataChannelScreenSharing", -601649103},
    {"groupCallJoinParameters", 1763438054},
    {"groupCallVideoQualityThumbnail", -379186304},
    {"groupCallVideoQualityMedium", 394968234},
    {"groupCallVideoQualityFull", -2125916617},
    {"importedContact", 1818209156},
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
    {"inlineKeyboardButtonTypeCopyText", 68883206},
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
    {"inputChatThemeEmoji", -1461787199},
    {"inputChatThemeGift", 2026976301},
    {"inputChecklist", -145125739},
    {"inputChecklistTask", 1633462225},
    {"inputCredentialsSaved", -2034385364},
    {"inputCredentialsNew", -829689558},
    {"inputCredentialsApplePay", -1246570799},
    {"inputCredentialsGooglePay", 844384100},
    {"inputFileId", 1788906253},
    {"inputFileRemote", -107574466},
    {"inputFileLocal", 2056030919},
    {"inputFileGenerated", -1333385216},
    {"inputGroupCallLink", -812157480},
    {"inputGroupCallMessage", -341793768},
    {"inputIdentityDocument", 767353688},
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
    {"inputMessageForwarded", -1076506316},
    {"inputMessageReplyToMessage", -782038760},
    {"inputMessageReplyToExternalMessage", -505276703},
    {"inputMessageReplyToStory", -1723842320},
    {"inputPaidMedia", 475844035},
    {"inputPaidMediaTypePhoto", -761660134},
    {"inputPaidMediaTypeVideo", 1793741625},
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
    {"inputStoryAreaTypeUpgradedGift", 793059694},
    {"inputStoryAreas", -883247088},
    {"inputStoryContentPhoto", -309196727},
    {"inputStoryContentVideo", 3809243},
    {"inputSuggestedPostInfo", -1246794382},
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
    {"internalLinkTypeWebApp", 2062112045},
    {"invoice", 113204876},
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
    {"messageSchedulingStateSendWhenVideoProcessed", 2101578734},
    {"messageSelfDestructTypeTimer", 1351440333},
    {"messageSelfDestructTypeImmediately", -1036218363},
    {"messageSendOptions", -1725581906},
    {"messageSenderUser", -336109341},
    {"messageSenderChat", -239660751},
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
    {"messageSourceOther", 901818114},
    {"messageTopicThread", 1360920071},
    {"messageTopicForum", 2119440112},
    {"messageTopicDirectMessages", -1285378599},
    {"messageTopicSavedMessages", 588026991},
    {"networkStatisticsEntryFile", 188452706},
    {"networkStatisticsEntryCall", 737000365},
    {"networkTypeNone", -1971691759},
    {"networkTypeMobile", 819228239},
    {"networkTypeMobileRoaming", -1435199760},
    {"networkTypeWiFi", -633872070},
    {"networkTypeOther", 1942128539},
    {"newChatPrivacySettings", 123716192},
    {"notificationSettingsScopePrivateChats", 937446759},
    {"notificationSettingsScopeGroupChats", 1212142067},
    {"notificationSettingsScopeChannelChats", 548013448},
    {"optionValueBoolean", 63135518},
    {"optionValueEmpty", 918955155},
    {"optionValueInteger", -186858780},
    {"optionValueString", 756248212},
    {"orderInfo", 783997294},
    {"paidReactionTypeRegular", -1199187333},
    {"paidReactionTypeAnonymous", 47892621},
    {"paidReactionTypeChat", -675782044},
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
    {"premiumFeatureChecklists", -1128709251},
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
    {"profileTabPosts", -1181952362},
    {"profileTabGifts", 1296815210},
    {"profileTabMedia", 1925597525},
    {"profileTabFiles", -1422681088},
    {"profileTabLinks", -748329831},
    {"profileTabMusic", -1624780178},
    {"profileTabVoice", -461960914},
    {"profileTabGifs", -1564412267},
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
    {"scopeNotificationSettings", 88369150},
    {"searchMessagesChatTypeFilterPrivate", 1169248975},
    {"searchMessagesChatTypeFilterGroup", -2059426022},
    {"searchMessagesChatTypeFilterChannel", -773540139},
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
    {"stickerFormatWebp", -2123043040},
    {"stickerFormatTgs", 1614588662},
    {"stickerFormatWebm", -2070162097},
    {"stickerTypeRegular", 56345973},
    {"stickerTypeMask", -1765394796},
    {"stickerTypeCustomEmoji", -120752249},
    {"storePaymentPurposePremiumSubscription", 1263894804},
    {"storePaymentPurposePremiumGift", -39502443},
    {"storePaymentPurposePremiumGiftCodes", -1072286736},
    {"storePaymentPurposePremiumGiveaway", 1302624938},
    {"storePaymentPurposeStarGiveaway", 211212441},
    {"storePaymentPurposeStars", 410189263},
    {"storePaymentPurposeGiftedStars", 893691428},
    {"storeTransactionAppStore", 1625562441},
    {"storeTransactionGooglePlay", 1094018617},
    {"storyAreaPosition", -1533023124},
    {"storyFullId", 765952419},
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
    {"suggestedActionSetProfilePhoto", -1612563093},
    {"suggestedActionExtendPremium", -566207286},
    {"suggestedActionExtendStarSubscriptions", -47000234},
    {"suggestedActionCustom", 2092876611},
    {"suggestedPostPriceStar", 216488903},
    {"suggestedPostPriceTon", -1095222334},
    {"supergroupMembersFilterRecent", 1178199509},
    {"supergroupMembersFilterContacts", -1282910856},
    {"supergroupMembersFilterAdministrators", -2097380265},
    {"supergroupMembersFilterSearch", -1696358469},
    {"supergroupMembersFilterRestricted", -1107800034},
    {"supergroupMembersFilterBanned", -1210621683},
    {"supergroupMembersFilterMention", 1151301973},
    {"supergroupMembersFilterBots", 492138918},
    {"targetChatCurrent", -416689904},
    {"targetChatChosen", -1392978522},
    {"targetChatInternalLink", -579301408},
    {"targetChatTypes", 1513098833},
    {"telegramPaymentPurposePremiumGift", -1600286150},
    {"telegramPaymentPurposePremiumGiftCodes", -1863495348},
    {"telegramPaymentPurposePremiumGiveaway", -760757441},
    {"telegramPaymentPurposeStars", 1204968037},
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
    {"transactionDirectionIncoming", -271074103},
    {"transactionDirectionOutgoing", 1638241254},
    {"upgradedGiftAttributeIdModel", 1053287307},
    {"upgradedGiftAttributeIdSymbol", 1188205608},
    {"upgradedGiftAttributeIdBackdrop", 1461997935},
    {"upgradedGiftBackdropColors", 4227529},
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
    {"userPrivacySettingAllowUnpaidMessages", 1430051047},
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
    {"userPrivacySettingRuleRestrictChatMembers", 392530897},
    {"userPrivacySettingRules", 322477541},
    {"venue", 1070406393},
    {"webAppOpenModeCompact", 1711603675},
    {"webAppOpenModeFullSize", 189320513},
    {"webAppOpenModeFullScreen", 1871315357},
    {"webAppOpenParameters", 1375356527}
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
    {"addChecklistTasks", 1554619499},
    {"addContact", -2098628252},
    {"addCustomServerLanguagePack", 4492771},
    {"addFavoriteSticker", 324504799},
    {"addFileToDownloads", 867533751},
    {"addGiftCollectionGifts", -263464606},
    {"addLocalMessage", -166217823},
    {"addLogMessage", 1597427692},
    {"addMessageReaction", 1419269613},
    {"addNetworkStatistics", 1264825305},
    {"addOffer", 682451739},
    {"addPendingPaidMessageReaction", -342110765},
    {"addProfileAudio", 393218847},
    {"addProxy", 331529432},
    {"addQuickReplyShortcutInlineQueryResultMessage", -2017449468},
    {"addQuickReplyShortcutMessage", 1058573098},
    {"addQuickReplyShortcutMessageAlbum", 1348436244},
    {"addRecentSticker", -1478109026},
    {"addRecentlyFoundChat", -1746396787},
    {"addSavedAnimation", -1538525088},
    {"addSavedNotificationSound", 1043956975},
    {"addStickerToSet", 1457266235},
    {"addStoryAlbumStories", -537692984},
    {"allowBotToSendMessages", 1776928142},
    {"allowUnpaidMessagesFromUser", 803569495},
    {"answerCallbackQuery", -1153028490},
    {"answerCustomQuery", -1293603521},
    {"answerInlineQuery", 1343853844},
    {"answerPreCheckoutQuery", -1486789653},
    {"answerShippingQuery", -434601324},
    {"answerWebAppQuery", -1598776079},
    {"applyPremiumGiftCode", -1347138530},
    {"approveSuggestedPost", -924807202},
    {"assignStoreTransaction", -2046202900},
    {"banChatMember", -888111748},
    {"banGroupCallParticipants", 624883173},
    {"blockMessageSenderFromReplies", -1214384757},
    {"boostChat", 1945750252},
    {"buyGiftUpgrade", 154296026},
    {"canBotSendMessages", 544052364},
    {"canPostStory", 668621518},
    {"canPurchaseFromStore", 1017811816},
    {"canSendGift", 1307059830},
    {"canSendMessageToUser", 1529489462},
    {"canTransferOwnership", 634602508},
    {"cancelDownloadFile", -1954524450},
    {"cancelPasswordReset", 940733538},
    {"cancelPreliminaryUploadFile", 823412414},
    {"cancelRecoveryEmailAddressVerification", -1516728691},
    {"changeImportedContacts", 1119625871},
    {"changeStickerSet", 449357293},
    {"checkAuthenticationBotToken", 639321206},
    {"checkAuthenticationCode", -302103382},
    {"checkAuthenticationEmailCode", -582827361},
    {"checkAuthenticationPassword", -2025698400},
    {"checkAuthenticationPasswordRecoveryCode", -603309083},
    {"checkAuthenticationPremiumPurchase", 1588959934},
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
    {"checkWebAppFileDownload", -389397278},
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
    {"clickChatSponsoredMessage", 971995671},
    {"clickPremiumSubscriptionButton", -369319162},
    {"clickVideoMessageAdvertisement", 1367156622},
    {"close", -1187782273},
    {"closeChat", 39749353},
    {"closeSecretChat", -471006133},
    {"closeStory", 1553967851},
    {"closeWebApp", 1755391174},
    {"commitPendingPaidMessageReactions", -171354618},
    {"confirmQrCodeAuthentication", -376199379},
    {"confirmSession", -674647009},
    {"connectAffiliateProgram", 1661392684},
    {"createBasicGroupChat", 1972024548},
    {"createBusinessChatLink", -1861018304},
    {"createCall", -1104663024},
    {"createChatFolder", 1015399680},
    {"createChatFolderInviteLink", -2037911099},
    {"createChatInviteLink", 287744833},
    {"createChatSubscriptionInviteLink", 2255717},
    {"createForumTopic", -15242103},
    {"createGiftCollection", -397088099},
    {"createGroupCall", 1930068672},
    {"createInvoiceLink", -814692249},
    {"createNewBasicGroupChat", 1806454709},
    {"createNewSecretChat", -620682651},
    {"createNewStickerSet", -481065727},
    {"createNewSupergroupChat", 804058822},
    {"createPrivateChat", -947758327},
    {"createSecretChat", 1930285615},
    {"createStoryAlbum", -804362185},
    {"createSupergroupChat", 1187475691},
    {"createTemporaryPassword", -1626509434},
    {"createVideoChat", 2124715405},
    {"declineGroupCallInvitation", 1843919377},
    {"declineSuggestedPost", -1758260609},
    {"decryptGroupCallData", 1781743076},
    {"deleteAccount", 1395816134},
    {"deleteAllCallMessages", -1466445325},
    {"deleteAllRevokedChatInviteLinks", 1112020698},
    {"deleteBotMediaPreviews", -1397512722},
    {"deleteBusinessChatLink", -1101895865},
    {"deleteBusinessConnectedBot", -1633976747},
    {"deleteBusinessMessages", 1425721828},
    {"deleteBusinessStory", 1024585042},
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
    {"deleteDirectMessagesChatTopicHistory", -1906080196},
    {"deleteDirectMessagesChatTopicMessagesByDate", 945080841},
    {"deleteFile", 1807653676},
    {"deleteForumTopic", -2121212039},
    {"deleteGiftCollection", -183749499},
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
    {"deleteStory", -2020144472},
    {"deleteStoryAlbum", -658327628},
    {"destroy", 685331274},
    {"disableAllSupergroupUsernames", 843511216},
    {"disableProxy", -2100095102},
    {"discardCall", -1545983346},
    {"disconnectAffiliateProgram", -105831172},
    {"disconnectAllWebsites", -1082985981},
    {"disconnectWebsite", -778767395},
    {"downloadFile", 1059402292},
    {"dropGiftOriginalDetails", -1394445843},
    {"editBotMediaPreview", -2037031582},
    {"editBusinessChatLink", 1594947110},
    {"editBusinessMessageCaption", -1071562045},
    {"editBusinessMessageChecklist", -445013904},
    {"editBusinessMessageLiveLocation", 494972447},
    {"editBusinessMessageMedia", -60733576},
    {"editBusinessMessageReplyMarkup", 701787159},
    {"editBusinessMessageText", -1149169252},
    {"editBusinessStory", 472538940},
    {"editChatFolder", 53672754},
    {"editChatFolderInviteLink", -2141872095},
    {"editChatInviteLink", 1320303996},
    {"editChatSubscriptionInviteLink", -951826989},
    {"editCustomLanguagePackInfo", 1320751257},
    {"editForumTopic", 1112389547},
    {"editInlineMessageCaption", 1409762552},
    {"editInlineMessageLiveLocation", 2134352044},
    {"editInlineMessageMedia", 23553921},
    {"editInlineMessageReplyMarkup", -67565858},
    {"editInlineMessageText", -855457307},
    {"editMessageCaption", -2020117951},
    {"editMessageChecklist", 1497856700},
    {"editMessageLiveLocation", -1890511980},
    {"editMessageMedia", -1152678125},
    {"editMessageReplyMarkup", 332127881},
    {"editMessageSchedulingState", -1372976192},
    {"editMessageText", 196272567},
    {"editProxy", -1605883821},
    {"editQuickReplyMessage", 80517006},
    {"editStarSubscription", 2048538904},
    {"editStory", 355296788},
    {"editStoryCover", -1035823266},
    {"editUserStarSubscription", 1370582665},
    {"enableProxy", 1494450838},
    {"encryptGroupCallData", -377997690},
    {"endGroupCall", 573131959},
    {"endGroupCallRecording", -75799927},
    {"endGroupCallScreenSharing", -2047599540},
    {"finishFileGeneration", -1055060835},
    {"forwardMessages", -247889118},
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
    {"getAvailableGifts", -4559695},
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
    {"getBotSimilarBotCount", -1271545369},
    {"getBotSimilarBots", -825139275},
    {"getBusinessAccountStarAmount", -1817136693},
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
    {"getChatAvailablePaidMessageReactionSenders", -1244619639},
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
    {"getChatMessageCalendar", 1644001372},
    {"getChatMessageCount", 1641001101},
    {"getChatMessagePosition", -1468174577},
    {"getChatNotificationSettingsExceptions", 201199121},
    {"getChatPinnedMessage", 359865008},
    {"getChatPostedToChatPageStories", -46414037},
    {"getChatRevenueStatistics", 701995836},
    {"getChatRevenueTransactions", -1120110117},
    {"getChatRevenueWithdrawalUrl", 506595104},
    {"getChatScheduledMessages", -549638149},
    {"getChatSimilarChatCount", 1178506894},
    {"getChatSimilarChats", -1152348285},
    {"getChatSparseMessagePositions", 994389757},
    {"getChatSponsoredMessages", 1353203864},
    {"getChatStatistics", 327057816},
    {"getChatStoryAlbums", -1559582892},
    {"getChatStoryInteractions", 354545268},
    {"getChats", -972768574},
    {"getChatsForChatFolderInviteLink", 1873561929},
    {"getChatsToPostStories", 1893901427},
    {"getCloseFriends", -1445628722},
    {"getCollectibleItemInfo", -217797238},
    {"getCommands", 1488621559},
    {"getConnectedAffiliateProgram", -1755191440},
    {"getConnectedAffiliatePrograms", -1960029582},
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
    {"getDefaultChatEmojiStatuses", 1553698018},
    {"getDefaultChatPhotoCustomEmojiStickers", -376342683},
    {"getDefaultEmojiStatuses", -539392025},
    {"getDefaultMessageAutoDeleteTime", -450857574},
    {"getDefaultProfilePhotoCustomEmojiStickers", 1280041655},
    {"getDirectMessagesChatTopic", 1990530052},
    {"getDirectMessagesChatTopicHistory", 1035221188},
    {"getDirectMessagesChatTopicMessageByDate", 1837500879},
    {"getDirectMessagesChatTopicRevenue", -792382961},
    {"getDisallowedChatEmojiStatuses", -2004787831},
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
    {"getForumTopic", 1931459984},
    {"getForumTopicDefaultIcons", 1479898332},
    {"getForumTopicHistory", -339129791},
    {"getForumTopicLink", 330405972},
    {"getForumTopics", 1368275676},
    {"getGameHighScores", 15746459},
    {"getGiftChatThemes", -1051327876},
    {"getGiftCollections", -1533138835},
    {"getGiftUpgradePreview", -1110719907},
    {"getGiveawayInfo", -1215852357},
    {"getGreetingStickers", 374873372},
    {"getGrossingWebAppBots", 1696779802},
    {"getGroupCall", 1468491406},
    {"getGroupCallParticipants", 1986739394},
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
    {"getMainWebApp", 594050214},
    {"getMapThumbnailFile", -152660070},
    {"getMarkdownText", 164524584},
    {"getMe", -191516033},
    {"getMenuButton", -437324736},
    {"getMessage", -1821196160},
    {"getMessageAddedReactions", 2110172754},
    {"getMessageAuthor", 1890166449},
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
    {"getOwnedBots", -1954035715},
    {"getOwnedStickerSets", 1493074208},
    {"getPaidMessageRevenue", 1976589102},
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
    {"getPremiumGiftPaymentOptions", -480334244},
    {"getPremiumGiveawayPaymentOptions", 1222168073},
    {"getPremiumInfoSticker", 2043562651},
    {"getPremiumLimit", 1075313898},
    {"getPremiumState", 663632610},
    {"getPremiumStickerExamples", 1399442328},
    {"getPremiumStickers", -280950192},
    {"getPreparedInlineMessage", -83179701},
    {"getProxies", -95026381},
    {"getProxyLink", -1054495112},
    {"getPublicPostSearchLimits", -1724216773},
    {"getPushReceiverId", -286505294},
    {"getReadDatePrivacySettings", 451435451},
    {"getReceivedGift", -446535239},
    {"getReceivedGifts", 1411782110},
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
    {"getSearchSponsoredChats", 1568505164},
    {"getSearchedForTags", -1692716851},
    {"getSecretChat", 40599169},
    {"getStarAdAccountUrl", 1940473181},
    {"getStarGiftPaymentOptions", -500735773},
    {"getStarGiveawayPaymentOptions", -883172578},
    {"getStarPaymentOptions", 1838351940},
    {"getStarRevenueStatistics", -260356841},
    {"getStarSubscriptions", -641223956},
    {"getStarTransactions", -304141747},
    {"getStarWithdrawalUrl", -1445841134},
    {"getStatisticalGraph", 1100975515},
    {"getStickerEmojis", -1895508665},
    {"getStickerOutline", -1550504539},
    {"getStickerSet", 1052318659},
    {"getStickerSetName", 1039849089},
    {"getStickers", 1158058819},
    {"getStorageStatistics", -853193929},
    {"getStorageStatisticsFast", 61368066},
    {"getStory", -2011076366},
    {"getStoryAlbumStories", 1915090076},
    {"getStoryAvailableReactions", 595938619},
    {"getStoryInteractions", 483475469},
    {"getStoryNotificationSettingsExceptions", 627715760},
    {"getStoryPublicForwards", 1810378546},
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
    {"getThemedChatEmojiStatuses", 1924568314},
    {"getThemedEmojiStatuses", -1468220543},
    {"getTimeZones", 1340268632},
    {"getTonRevenueStatistics", -1315591160},
    {"getTonTransactions", -1935011115},
    {"getTonWithdrawalUrl", -1482519601},
    {"getTopChats", -388410847},
    {"getTrendingStickerSets", -531085986},
    {"getUpgradedGift", -1331821135},
    {"getUpgradedGiftEmojiStatuses", -1748975723},
    {"getUpgradedGiftValueInfo", 1818813417},
    {"getUpgradedGiftWithdrawalUrl", -784331188},
    {"getUser", 1117363211},
    {"getUserChatBoosts", -1190205543},
    {"getUserFullInfo", -776823720},
    {"getUserLink", 1226839270},
    {"getUserPrivacySettingRules", -2077223311},
    {"getUserProfileAudios", -208876086},
    {"getUserProfilePhotos", -908132798},
    {"getUserSupportInfo", 1957008133},
    {"getVideoChatAvailableParticipants", -1000496379},
    {"getVideoChatInviteLink", -1394707321},
    {"getVideoChatRtmpUrl", 1210784543},
    {"getVideoChatStreamSegment", 773343220},
    {"getVideoChatStreams", 531757765},
    {"getVideoMessageAdvertisements", -285681331},
    {"getWebAppLinkUrl", 1627284161},
    {"getWebAppPlaceholder", 583470479},
    {"getWebAppUrl", -1526784188},
    {"getWebPageInstantView", 1741395197},
    {"giftPremiumWithStars", -2058395432},
    {"hideContactCloseBirthdays", -1163065221},
    {"hideSuggestedAction", -1561384065},
    {"importContacts", -662555959},
    {"importMessages", -1864116784},
    {"inviteGroupCallParticipant", -631535414},
    {"inviteVideoChatParticipants", -473149298},
    {"isProfileAudio", -245729314},
    {"joinChat", 326769313},
    {"joinChatByInviteLink", -1049973882},
    {"joinGroupCall", -2026079917},
    {"joinVideoChat", 1322989999},
    {"launchPrepaidGiveaway", 639465530},
    {"leaveChat", -1825080735},
    {"leaveGroupCall", 980152233},
    {"loadActiveStories", 2106390328},
    {"loadChats", -1885635205},
    {"loadDirectMessagesChatTopics", 1488065975},
    {"loadGroupCallParticipants", 938720974},
    {"loadQuickReplyShortcutMessages", -46092588},
    {"loadQuickReplyShortcuts", -1016614243},
    {"loadSavedMessagesTopics", 289855160},
    {"logOut", -1581923301},
    {"markChecklistTasksAsDone", 386950739},
    {"openBotSimilarBot", -369688872},
    {"openChat", -323371509},
    {"openChatSimilarChat", -1884883949},
    {"openMessageContent", -739088005},
    {"openSponsoredChat", 865985573},
    {"openStory", -696723005},
    {"openWebApp", -950685122},
    {"optimizeStorage", 853186759},
    {"parseMarkdown", 756366063},
    {"parseTextEntities", -1709194593},
    {"pinChatMessage", 2034719663},
    {"pingProxy", -979681103},
    {"postStory", -1725643742},
    {"preliminaryUploadFile", 1894239129},
    {"processChatFolderNewChats", 1498280672},
    {"processChatJoinRequest", 1004876963},
    {"processChatJoinRequests", 1048722894},
    {"processPushNotification", 786679952},
    {"rateSpeechRecognition", -287521867},
    {"readAllChatMentions", 1357558453},
    {"readAllChatReactions", 1421973357},
    {"readAllDirectMessagesChatTopicReactions", 1154665542},
    {"readAllForumTopicMentions", -996864148},
    {"readAllForumTopicReactions", -1986498949},
    {"readBusinessMessage", -1723531538},
    {"readChatList", -1117480790},
    {"readFilePart", -174576822},
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
    {"removeGiftCollectionGifts", -692316949},
    {"removeInstalledBackground", 1346446652},
    {"removeMessageReaction", -1756934789},
    {"removeMessageSenderBotVerification", -1710174374},
    {"removeNotification", 862630734},
    {"removeNotificationGroup", 1713005454},
    {"removePendingPaidMessageReactions", 1100258555},
    {"removeProfileAudio", 1292263034},
    {"removeProxy", 1369219847},
    {"removeRecentHashtag", -1013735260},
    {"removeRecentSticker", 1246577677},
    {"removeRecentlyFoundChat", 717340444},
    {"removeSavedAnimation", -495605479},
    {"removeSavedNotificationSound", -480032946},
    {"removeSearchedForTag", 891382730},
    {"removeStickerFromSet", 1642196644},
    {"removeStoryAlbumStories", 1901872465},
    {"removeTopChat", -1907876267},
    {"reorderActiveUsernames", -455399375},
    {"reorderBotActiveUsernames", -1602301664},
    {"reorderBotMediaPreviews", 630851043},
    {"reorderChatFolders", 1665299546},
    {"reorderGiftCollectionGifts", -1545340419},
    {"reorderGiftCollections", -1607216912},
    {"reorderInstalledStickerSets", 1074928158},
    {"reorderQuickReplyShortcuts", -2052799232},
    {"reorderStoryAlbumStories", 438104756},
    {"reorderStoryAlbums", 514423948},
    {"reorderSupergroupActiveUsernames", -1962466095},
    {"replacePrimaryChatInviteLink", 1067350941},
    {"replaceStickerInSet", -406311399},
    {"replaceVideoChatRtmpUrl", 558862304},
    {"reportAuthenticationCodeMissing", -1846555064},
    {"reportChat", 1058475058},
    {"reportChatPhoto", -646966648},
    {"reportChatSponsoredMessage", -979984820},
    {"reportMessageReactions", 919111719},
    {"reportPhoneNumberCodeMissing", -895175341},
    {"reportSponsoredChat", -902673019},
    {"reportStory", 1823256372},
    {"reportSupergroupAntiSpamFalsePositive", -516050872},
    {"reportSupergroupSpam", -94825000},
    {"reportVideoMessageAdvertisement", 617137942},
    {"requestAuthenticationPasswordRecovery", 1393896118},
    {"requestPasswordRecovery", -13777582},
    {"requestQrCodeAuthentication", 1363496527},
    {"resendAuthenticationCode", -1506755656},
    {"resendEmailAddressVerificationCode", -1872416732},
    {"resendLoginEmailAddressCode", 292966933},
    {"resendMessages", 526374678},
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
    {"savePreparedInlineMessage", -954963751},
    {"searchAffiliatePrograms", 681156625},
    {"searchBackground", -2130996959},
    {"searchCallMessages", -1942229221},
    {"searchChatAffiliateProgram", -1339291206},
    {"searchChatMembers", -445823291},
    {"searchChatMessages", 1072442212},
    {"searchChatRecentLocationMessages", 950238950},
    {"searchChats", -1879787060},
    {"searchChatsOnServer", -1158402188},
    {"searchContacts", -1794690715},
    {"searchEmojis", -1456187668},
    {"searchFileDownloads", 706611286},
    {"searchGiftsForResale", 1659300894},
    {"searchHashtags", 1043637617},
    {"searchInstalledStickerSets", 2120122276},
    {"searchMessages", 1225448885},
    {"searchOutgoingDocumentMessages", -1071397762},
    {"searchPublicChat", 857135533},
    {"searchPublicChats", 970385337},
    {"searchPublicMessagesByTag", 630680746},
    {"searchPublicPosts", 618133347},
    {"searchPublicStoriesByLocation", 1596709256},
    {"searchPublicStoriesByTag", 1778102602},
    {"searchPublicStoriesByVenue", -686136790},
    {"searchQuote", 1751384351},
    {"searchRecentlyFoundChats", 1647445393},
    {"searchSavedMessages", -1969512554},
    {"searchSecretMessages", -852865892},
    {"searchStickerSet", 1676592898},
    {"searchStickerSets", 262801004},
    {"searchStickers", -1856294754},
    {"searchStringsByPrefix", -2023251463},
    {"searchUserByPhoneNumber", -343757368},
    {"searchUserByToken", -666766282},
    {"searchWebApp", -1241740747},
    {"sellGift", -298298375},
    {"sendAuthenticationFirebaseSms", 364994111},
    {"sendBotStartMessage", -1435877650},
    {"sendBusinessMessage", 159888387},
    {"sendBusinessMessageAlbum", 788608366},
    {"sendCallDebugInformation", 2019243839},
    {"sendCallLog", 1057638353},
    {"sendCallRating", -1402719502},
    {"sendCallSignalingData", 1412280732},
    {"sendChatAction", -1586808788},
    {"sendCustomRequest", 285045153},
    {"sendEmailAddressVerificationCode", -221621379},
    {"sendGift", -1199356118},
    {"sendGroupCallMessage", -543237212},
    {"sendInlineQueryResultMessage", -1445935970},
    {"sendMessage", 964116012},
    {"sendMessageAlbum", -1488607732},
    {"sendPassportAuthorizationForm", 652160701},
    {"sendPaymentForm", -965855094},
    {"sendPhoneNumberCode", 1084112144},
    {"sendPhoneNumberFirebaseSms", 261910660},
    {"sendQuickReplyShortcutMessages", 232068765},
    {"sendResoldGift", -1427446834},
    {"sendTextMessageDraft", 2121571326},
    {"sendWebAppCustomRequest", 922705352},
    {"sendWebAppData", -1423978996},
    {"setAccentColor", 1669974841},
    {"setAccountTtl", 701389032},
    {"setAlarm", -873497067},
    {"setApplicationVerificationToken", 927248261},
    {"setArchiveChatListSettings", -884650998},
    {"setAuthenticationEmailAddress", 1773323522},
    {"setAuthenticationPhoneNumber", 868276259},
    {"setAuthenticationPremiumPurchaseTransaction", -450986887},
    {"setAutoDownloadSettings", -353671948},
    {"setAutosaveSettings", 6846656},
    {"setBio", -1619582124},
    {"setBirthdate", 1319755160},
    {"setBotInfoDescription", 693574984},
    {"setBotInfoShortDescription", 982956771},
    {"setBotName", -761922959},
    {"setBotProfilePhoto", -1115272346},
    {"setBotUpdatesStatus", -1154926191},
    {"setBusinessAccountBio", -1698538041},
    {"setBusinessAccountGiftSettings", 1757763090},
    {"setBusinessAccountName", 999582546},
    {"setBusinessAccountProfilePhoto", 1174440149},
    {"setBusinessAccountUsername", -1520126367},
    {"setBusinessAwayMessageSettings", 1232357484},
    {"setBusinessConnectedBot", -1393459472},
    {"setBusinessGreetingMessageSettings", -873120707},
    {"setBusinessLocation", -344717547},
    {"setBusinessMessageIsPinned", -15403536},
    {"setBusinessOpeningHours", -462379918},
    {"setBusinessStartPage", -1628616290},
    {"setChatAccentColor", 882857930},
    {"setChatActiveStoriesList", -521970415},
    {"setChatAffiliateProgram", 14680631},
    {"setChatAvailableReactions", 267075078},
    {"setChatBackground", 246727678},
    {"setChatClientData", -827119811},
    {"setChatDescription", 1957213277},
    {"setChatDirectMessagesGroup", 1633150115},
    {"setChatDiscussionGroup", -918801736},
    {"setChatDraftMessage", -555614927},
    {"setChatEmojiStatus", 1434982674},
    {"setChatLocation", -767091286},
    {"setChatMemberStatus", 81794847},
    {"setChatMessageAutoDeleteTime", -1505643265},
    {"setChatMessageSender", -1421513858},
    {"setChatNotificationSettings", 777199614},
    {"setChatPaidMessageStarCount", -1187053289},
    {"setChatPermissions", 2138507006},
    {"setChatPhoto", -377778941},
    {"setChatPinnedStories", -669062355},
    {"setChatProfileAccentColor", 1109896826},
    {"setChatSlowModeDelay", -540350914},
    {"setChatTheme", 1474791506},
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
    {"setDirectMessagesChatTopicIsMarkedAsUnread", 1569655059},
    {"setEmojiStatus", -1829224867},
    {"setFileGenerationProgress", 1836403518},
    {"setForumTopicNotificationSettings", -1989500300},
    {"setGameScore", 2127359430},
    {"setGiftCollectionName", 1108355593},
    {"setGiftResalePrice", 373916170},
    {"setGiftSettings", -519330046},
    {"setGroupCallParticipantIsSpeaking", -1019676164},
    {"setGroupCallParticipantVolumeLevel", -1753769944},
    {"setInactiveSessionTtl", 1570548048},
    {"setInlineGameScore", -948871797},
    {"setLogStream", -1364199535},
    {"setLogTagVerbosityLevel", -2095589738},
    {"setLogVerbosityLevel", -303429678},
    {"setLoginEmailAddress", 935019476},
    {"setMainProfileTab", 1663496423},
    {"setMenuButton", -1269841599},
    {"setMessageFactCheck", -4309752},
    {"setMessageReactions", -372524900},
    {"setMessageSenderBlockList", -1987355503},
    {"setMessageSenderBotVerification", -1262364086},
    {"setName", 1711693584},
    {"setNetworkType", -701635234},
    {"setNewChatPrivacySettings", 1774139215},
    {"setOption", 2114670322},
    {"setPaidMessageReactionType", -829934930},
    {"setPassportElement", 2068173212},
    {"setPassportElementErrors", -2056754881},
    {"setPassword", -1193589027},
    {"setPersonalChat", -1068782668},
    {"setPinnedChats", -695640000},
    {"setPinnedForumTopics", -1871668497},
    {"setPinnedGifts", 1613526306},
    {"setPinnedSavedMessagesTopics", -194818924},
    {"setPollAnswer", -1399388792},
    {"setProfileAccentColor", -1986281112},
    {"setProfileAudioPosition", 1209963614},
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
    {"setStoryAlbumName", -1143129794},
    {"setStoryPrivacySettings", -655801550},
    {"setStoryReaction", 250731529},
    {"setSupergroupCustomEmojiStickerSet", 1328894639},
    {"setSupergroupMainProfileTab", 1314899548},
    {"setSupergroupStickerSet", -2056344215},
    {"setSupergroupUnrestrictBoostCount", 969814179},
    {"setSupergroupUsername", 1346325252},
    {"setTdlibParameters", -775883218},
    {"setUpgradedGiftColors", -966832402},
    {"setUserEmojiStatus", -451519541},
    {"setUserNote", -351487655},
    {"setUserPersonalProfilePhoto", 464136438},
    {"setUserPrivacySettingRules", -473812741},
    {"setUserSupportInfo", -2088986621},
    {"setUsername", 439901214},
    {"setVideoChatDefaultParticipant", -240749901},
    {"setVideoChatTitle", 1915482994},
    {"shareChatWithBot", -1504507166},
    {"sharePhoneNumber", 1097130069},
    {"shareUsersWithBot", -1574608333},
    {"startGroupCallRecording", 1757774971},
    {"startGroupCallScreenSharing", -884068051},
    {"startScheduledVideoChat", -1300829822},
    {"stopBusinessPoll", -1142218400},
    {"stopPoll", 1659374253},
    {"suggestUserBirthdate", 39506613},
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
    {"toggleBotCanManageEmojiStatus", 622495770},
    {"toggleBotIsAddedToAttachmentMenu", -1906712934},
    {"toggleBotUsernameIsActive", 2036569097},
    {"toggleBusinessConnectedBotChatIsPaused", 1328957509},
    {"toggleChatDefaultDisableNotification", 314794002},
    {"toggleChatFolderTags", -2092209084},
    {"toggleChatGiftNotifications", -2069429154},
    {"toggleChatHasProtectedContent", 975231309},
    {"toggleChatIsMarkedAsUnread", -986129697},
    {"toggleChatIsPinned", -1485429186},
    {"toggleChatIsTranslatable", -1812345889},
    {"toggleChatViewAsTopics", 724009948},
    {"toggleDirectMessagesChatTopicCanSendUnpaidMessages", -335898703},
    {"toggleDownloadIsPaused", -947493099},
    {"toggleForumTopicIsClosed", 982153376},
    {"toggleForumTopicIsPinned", 1474651795},
    {"toggleGeneralForumTopicIsHidden", 1595741256},
    {"toggleGiftIsSaved", 693198065},
    {"toggleGroupCallCanSendMessages", -617467060},
    {"toggleGroupCallIsMyVideoEnabled", -1624289030},
    {"toggleGroupCallIsMyVideoPaused", -478875239},
    {"toggleGroupCallParticipantIsHandRaised", -1896127519},
    {"toggleGroupCallParticipantIsMuted", -1308093433},
    {"toggleGroupCallScreenSharingIsPaused", -1602530464},
    {"toggleHasSponsoredMessagesEnabled", 1963285740},
    {"toggleSavedMessagesTopicIsPinned", -1588378164},
    {"toggleSessionCanAcceptCalls", 1819027208},
    {"toggleSessionCanAcceptSecretChats", 1000843390},
    {"toggleStoryIsPostedToChatPage", -2141806228},
    {"toggleSupergroupCanHaveSponsoredMessages", -1098204302},
    {"toggleSupergroupHasAggressiveAntiSpamEnabled", 1748956943},
    {"toggleSupergroupHasAutomaticTranslation", -184993048},
    {"toggleSupergroupHasHiddenMembers", -1537892918},
    {"toggleSupergroupIsAllHistoryAvailable", 1155110478},
    {"toggleSupergroupIsBroadcastGroup", 884089365},
    {"toggleSupergroupIsForum", 371064337},
    {"toggleSupergroupJoinByRequest", 2111807454},
    {"toggleSupergroupJoinToSendMessages", -182022642},
    {"toggleSupergroupSignMessages", 572268491},
    {"toggleSupergroupUsernameIsActive", -1500811777},
    {"toggleUsernameIsActive", 1244098019},
    {"toggleVideoChatEnabledStartNotification", 1851489086},
    {"toggleVideoChatMuteNewParticipants", 987023756},
    {"transferBusinessAccountStars", 732562464},
    {"transferChatOwnership", 2006977043},
    {"transferGift", -1167293126},
    {"translateMessageText", 1405427410},
    {"translateText", 623011058},
    {"unpinAllChatMessages", -1437805385},
    {"unpinAllDirectMessagesChatTopicMessages", 89671100},
    {"unpinAllForumTopicMessages", -1113340771},
    {"unpinChatMessage", 2065448670},
    {"upgradeBasicGroupChatToSupergroupChat", 300488122},
    {"upgradeGift", -1782136103},
    {"uploadStickerFile", 647385283},
    {"validateOrderInfo", -1248305201},
    {"viewMessages", 960236656},
    {"viewPremiumFeature", 192950706},
    {"viewSponsoredChat", 1722644778},
    {"viewTrendingStickerSets", -952416520},
    {"viewVideoMessageAdvertisement", -808563006},
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

Status from_json(td_api::archiveChatListSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.archive_and_mute_new_chats_from_unknown_users_, from.extract_field("archive_and_mute_new_chats_from_unknown_users")));
  TRY_STATUS(from_json(to.keep_unmuted_chats_archived_, from.extract_field("keep_unmuted_chats_archived")));
  TRY_STATUS(from_json(to.keep_chats_from_folders_archived_, from.extract_field("keep_chats_from_folders_archived")));
  return Status::OK();
}

Status from_json(td_api::backgroundTypeWallpaper &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_blurred_, from.extract_field("is_blurred")));
  TRY_STATUS(from_json(to.is_moving_, from.extract_field("is_moving")));
  return Status::OK();
}

Status from_json(td_api::botCommandScopeAllPrivateChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessAwayMessageScheduleCustom &to, JsonObject &from) {
  TRY_STATUS(from_json(to.start_date_, from.extract_field("start_date")));
  TRY_STATUS(from_json(to.end_date_, from.extract_field("end_date")));
  return Status::OK();
}

Status from_json(td_api::businessFeatureAccountLinks &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::businessOpeningHoursInterval &to, JsonObject &from) {
  TRY_STATUS(from_json(to.start_minute_, from.extract_field("start_minute")));
  TRY_STATUS(from_json(to.end_minute_, from.extract_field("end_minute")));
  return Status::OK();
}

Status from_json(td_api::callProblemDistortedVideo &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatActionRecordingVoiceNote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::chatActionUploadingVideoNote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.progress_, from.extract_field("progress")));
  return Status::OK();
}

Status from_json(td_api::chatFolderName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.animate_custom_emoji_, from.extract_field("animate_custom_emoji")));
  return Status::OK();
}

Status from_json(td_api::chatMemberStatusMember &to, JsonObject &from) {
  TRY_STATUS(from_json(to.member_until_date_, from.extract_field("member_until_date")));
  return Status::OK();
}

Status from_json(td_api::chatMembersFilterBanned &to, JsonObject &from) {
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

Status from_json(td_api::deviceTokenSimplePush &to, JsonObject &from) {
  TRY_STATUS(from_json(to.endpoint_, from.extract_field("endpoint")));
  return Status::OK();
}

Status from_json(td_api::emojiCategoryTypeDefault &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeAnimation &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeSecure &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::fileTypeVideoNote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::giftForResaleOrderPriceChangeDate &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::groupCallVideoQualityThumbnail &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::inlineKeyboardButtonTypeCallbackWithPassword &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.data_, from.extract_field("data")));
  return Status::OK();
}

Status from_json(td_api::inputBackgroundLocal &to, JsonObject &from) {
  TRY_STATUS(from_json(to.background_, from.extract_field("background")));
  return Status::OK();
}

Status from_json(td_api::inputChatThemeEmoji &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::inputFileRemote &to, JsonObject &from) {
  TRY_STATUS(from_json(to.id_, from.extract_field("id")));
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

Status from_json(td_api::inputInvoiceMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
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

Status from_json(td_api::inputPaidMediaTypePhoto &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::inputPassportElementBankStatement &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bank_statement_, from.extract_field("bank_statement")));
  return Status::OK();
}

Status from_json(td_api::inputPassportElementErrorSourceFrontSide &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.file_hash_, from.extract_field("file_hash")));
  return Status::OK();
}

Status from_json(td_api::inputStoryArea &to, JsonObject &from) {
  TRY_STATUS(from_json(to.position_, from.extract_field("position")));
  TRY_STATUS(from_json(to.type_, from.extract_field("type")));
  return Status::OK();
}

Status from_json(td_api::inputStoryAreas &to, JsonObject &from) {
  TRY_STATUS(from_json(to.areas_, from.extract_field("areas")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeBackground &to, JsonObject &from) {
  TRY_STATUS(from_json(to.background_name_, from.extract_field("background_name")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeChatFolderInvite &to, JsonObject &from) {
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeInstantView &to, JsonObject &from) {
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  TRY_STATUS(from_json(to.fallback_url_, from.extract_field("fallback_url")));
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

Status from_json(td_api::internalLinkTypeRestorePurchases &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::internalLinkTypeUpgradedGift &to, JsonObject &from) {
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::jsonValueNumber &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
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

Status from_json(td_api::linkPreviewOptions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_disabled_, from.extract_field("is_disabled")));
  TRY_STATUS(from_json(to.url_, from.extract_field("url")));
  TRY_STATUS(from_json(to.force_small_media_, from.extract_field("force_small_media")));
  TRY_STATUS(from_json(to.force_large_media_, from.extract_field("force_large_media")));
  TRY_STATUS(from_json(to.show_above_text_, from.extract_field("show_above_text")));
  return Status::OK();
}

Status from_json(td_api::maskPointChin &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::messageSendOptions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.suggested_post_info_, from.extract_field("suggested_post_info")));
  TRY_STATUS(from_json(to.disable_notification_, from.extract_field("disable_notification")));
  TRY_STATUS(from_json(to.from_background_, from.extract_field("from_background")));
  TRY_STATUS(from_json(to.protect_content_, from.extract_field("protect_content")));
  TRY_STATUS(from_json(to.allow_paid_broadcast_, from.extract_field("allow_paid_broadcast")));
  TRY_STATUS(from_json(to.paid_message_star_count_, from.extract_field("paid_message_star_count")));
  TRY_STATUS(from_json(to.update_order_of_installed_sticker_sets_, from.extract_field("update_order_of_installed_sticker_sets")));
  TRY_STATUS(from_json(to.scheduling_state_, from.extract_field("scheduling_state")));
  TRY_STATUS(from_json(to.effect_id_, from.extract_field("effect_id")));
  TRY_STATUS(from_json(to.sending_id_, from.extract_field("sending_id")));
  TRY_STATUS(from_json(to.only_preview_, from.extract_field("only_preview")));
  return Status::OK();
}

Status from_json(td_api::messageSourceSearch &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::networkStatisticsEntryFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_type_, from.extract_field("file_type")));
  TRY_STATUS(from_json(to.network_type_, from.extract_field("network_type")));
  TRY_STATUS(from_json(to.sent_bytes_, from.extract_field("sent_bytes")));
  TRY_STATUS(from_json(to.received_bytes_, from.extract_field("received_bytes")));
  return Status::OK();
}

Status from_json(td_api::notificationSettingsScopeGroupChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::paidReactionTypeChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::passportElementTypeRentalAgreement &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::phoneNumberCodeTypeConfirmOwnership &to, JsonObject &from) {
  TRY_STATUS(from_json(to.hash_, from.extract_field("hash")));
  return Status::OK();
}

Status from_json(td_api::premiumFeatureUniqueStickers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureUpgradedStories &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumFeatureChecklists &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypePinnedSavedMessagesTopicCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumLimitTypeStorySuggestedReactionAreaCount &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::premiumStoryFeatureStealthMode &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::profileTabFiles &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::publicChatTypeIsLocationBased &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::replyMarkupRemoveKeyboard &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_personal_, from.extract_field("is_personal")));
  return Status::OK();
}

Status from_json(td_api::reportReasonUnrelatedLocation &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesChatTypeFilterPrivate &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterVoiceNote &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchMessagesFilterFailedToSend &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::stickerTypeCustomEmoji &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::storeTransactionGooglePlay &to, JsonObject &from) {
  TRY_STATUS(from_json(to.package_name_, from.extract_field("package_name")));
  TRY_STATUS(from_json(to.store_product_id_, from.extract_field("store_product_id")));
  TRY_STATUS(from_json(to.purchase_token_, from.extract_field("purchase_token")));
  return Status::OK();
}

Status from_json(td_api::suggestedActionEnableArchiveAndMuteNewChats &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::suggestedActionGiftPremiumForChristmas &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::supergroupMembersFilterContacts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  return Status::OK();
}

Status from_json(td_api::targetChatInternalLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, from.extract_field("link")));
  return Status::OK();
}

Status from_json(td_api::testInt &to, JsonObject &from) {
  TRY_STATUS(from_json(to.value_, from.extract_field("value")));
  return Status::OK();
}

Status from_json(td_api::textEntityTypePhoneNumber &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::textEntityTypePreCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_, from.extract_field("language")));
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

Status from_json(td_api::transactionDirectionIncoming &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingShowPhoneNumber &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingAllowUnpaidMessages &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::userPrivacySettingRuleRestrictBots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::acceptCall &to, JsonObject &from) {
  TRY_STATUS(from_json(to.call_id_, from.extract_field("call_id")));
  TRY_STATUS(from_json(to.protocol_, from.extract_field("protocol")));
  return Status::OK();
}

Status from_json(td_api::addContact &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  TRY_STATUS(from_json(to.contact_, from.extract_field("contact")));
  TRY_STATUS(from_json(to.share_phone_number_, from.extract_field("share_phone_number")));
  return Status::OK();
}

Status from_json(td_api::addOffer &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.options_, from.extract_field("options")));
  return Status::OK();
}

Status from_json(td_api::addSavedAnimation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.animation_, from.extract_field("animation")));
  return Status::OK();
}

Status from_json(td_api::answerPreCheckoutQuery &to, JsonObject &from) {
  TRY_STATUS(from_json(to.pre_checkout_query_id_, from.extract_field("pre_checkout_query_id")));
  TRY_STATUS(from_json(to.error_message_, from.extract_field("error_message")));
  return Status::OK();
}

Status from_json(td_api::boostChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.slot_ids_, from.extract_field("slot_ids")));
  return Status::OK();
}

Status from_json(td_api::cancelPasswordReset &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::checkAuthenticationPasswordRecoveryCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.recovery_code_, from.extract_field("recovery_code")));
  return Status::OK();
}

Status from_json(td_api::checkPhoneNumberCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.code_, from.extract_field("code")));
  return Status::OK();
}

Status from_json(td_api::clearImportedContacts &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::clickVideoMessageAdvertisement &to, JsonObject &from) {
  TRY_STATUS(from_json(to.advertisement_unique_id_, from.extract_field("advertisement_unique_id")));
  return Status::OK();
}

Status from_json(td_api::connectAffiliateProgram &to, JsonObject &from) {
  TRY_STATUS(from_json(to.affiliate_, from.extract_field("affiliate")));
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  return Status::OK();
}

Status from_json(td_api::createGiftCollection &to, JsonObject &from) {
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.received_gift_ids_, from.extract_field("received_gift_ids")));
  return Status::OK();
}

Status from_json(td_api::createStoryAlbum &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_poster_chat_id_, from.extract_field("story_poster_chat_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.story_ids_, from.extract_field("story_ids")));
  return Status::OK();
}

Status from_json(td_api::deleteAllRevokedChatInviteLinks &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.creator_user_id_, from.extract_field("creator_user_id")));
  return Status::OK();
}

Status from_json(td_api::deleteChatFolderInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::deleteFile &to, JsonObject &from) {
  TRY_STATUS(from_json(to.file_id_, from.extract_field("file_id")));
  return Status::OK();
}

Status from_json(td_api::deleteRevokedChatInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  return Status::OK();
}

Status from_json(td_api::disableAllSupergroupUsernames &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  return Status::OK();
}

Status from_json(td_api::editBusinessChatLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.link_, from.extract_field("link")));
  TRY_STATUS(from_json(to.link_info_, from.extract_field("link_info")));
  return Status::OK();
}

Status from_json(td_api::editChatFolderInviteLink &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  TRY_STATUS(from_json(to.invite_link_, from.extract_field("invite_link")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  TRY_STATUS(from_json(to.chat_ids_, from.extract_field("chat_ids")));
  return Status::OK();
}

Status from_json(td_api::editInlineMessageText &to, JsonObject &from) {
  TRY_STATUS(from_json(to.inline_message_id_, from.extract_field("inline_message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::editQuickReplyMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_id_, from.extract_field("shortcut_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.input_message_content_, from.extract_field("input_message_content")));
  return Status::OK();
}

Status from_json(td_api::endGroupCallScreenSharing &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  return Status::OK();
}

Status from_json(td_api::getApplicationDownloadLink &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getAvailableGifts &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getBotMediaPreviews &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  return Status::OK();
}

Status from_json(td_api::getBusinessFeatures &to, JsonObject &from) {
  TRY_STATUS(from_json(to.source_, from.extract_field("source")));
  return Status::OK();
}

Status from_json(td_api::getChatBoostFeatures &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_channel_, from.extract_field("is_channel")));
  return Status::OK();
}

Status from_json(td_api::getChatFolderChatsToLeave &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
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

Status from_json(td_api::getChatPostedToChatPageStories &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.from_story_id_, from.extract_field("from_story_id")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  return Status::OK();
}

Status from_json(td_api::getChatStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.is_dark_, from.extract_field("is_dark")));
  return Status::OK();
}

Status from_json(td_api::getConnectedAffiliateProgram &to, JsonObject &from) {
  TRY_STATUS(from_json(to.affiliate_, from.extract_field("affiliate")));
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  return Status::OK();
}

Status from_json(td_api::getCurrentWeather &to, JsonObject &from) {
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  return Status::OK();
}

Status from_json(td_api::getDefaultMessageAutoDeleteTime &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getEmojiSuggestionsUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  return Status::OK();
}

Status from_json(td_api::getForumTopicDefaultIcons &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getGreetingStickers &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getInstalledBackgrounds &to, JsonObject &from) {
  TRY_STATUS(from_json(to.for_dark_theme_, from.extract_field("for_dark_theme")));
  return Status::OK();
}

Status from_json(td_api::getLanguagePackStrings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.language_pack_id_, from.extract_field("language_pack_id")));
  TRY_STATUS(from_json(to.keys_, from.extract_field("keys")));
  return Status::OK();
}

Status from_json(td_api::getMainWebApp &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.start_parameter_, from.extract_field("start_parameter")));
  TRY_STATUS(from_json(to.parameters_, from.extract_field("parameters")));
  return Status::OK();
}

Status from_json(td_api::getMessageEffect &to, JsonObject &from) {
  TRY_STATUS(from_json(to.effect_id_, from.extract_field("effect_id")));
  return Status::OK();
}

Status from_json(td_api::getMessageReadDate &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::getOwnedBots &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getPhoneNumberInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.phone_number_prefix_, from.extract_field("phone_number_prefix")));
  return Status::OK();
}

Status from_json(td_api::getPremiumState &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getReceivedGift &to, JsonObject &from) {
  TRY_STATUS(from_json(to.received_gift_id_, from.extract_field("received_gift_id")));
  return Status::OK();
}

Status from_json(td_api::getRecoveryEmailAddress &to, JsonObject &from) {
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::getSavedOrderInfo &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getStarRevenueStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  TRY_STATUS(from_json(to.is_dark_, from.extract_field("is_dark")));
  return Status::OK();
}

Status from_json(td_api::getStickers &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, from.extract_field("sticker_type")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::getStoryStatistics &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  TRY_STATUS(from_json(to.is_dark_, from.extract_field("is_dark")));
  return Status::OK();
}

Status from_json(td_api::getSupportUser &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::getTonWithdrawalUrl &to, JsonObject &from) {
  TRY_STATUS(from_json(to.password_, from.extract_field("password")));
  return Status::OK();
}

Status from_json(td_api::getUserFullInfo &to, JsonObject &from) {
  TRY_STATUS(from_json(to.user_id_, from.extract_field("user_id")));
  return Status::OK();
}

Status from_json(td_api::getVideoChatStreamSegment &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.time_offset_, from.extract_field("time_offset")));
  TRY_STATUS(from_json(to.scale_, from.extract_field("scale")));
  TRY_STATUS(from_json(to.channel_id_, from.extract_field("channel_id")));
  TRY_STATUS(from_json(to.video_quality_, from.extract_field("video_quality")));
  return Status::OK();
}

Status from_json(td_api::hideSuggestedAction &to, JsonObject &from) {
  TRY_STATUS(from_json(to.action_, from.extract_field("action")));
  return Status::OK();
}

Status from_json(td_api::joinVideoChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.participant_id_, from.extract_field("participant_id")));
  TRY_STATUS(from_json(to.join_parameters_, from.extract_field("join_parameters")));
  TRY_STATUS(from_json(to.invite_hash_, from.extract_field("invite_hash")));
  return Status::OK();
}

Status from_json(td_api::loadQuickReplyShortcuts &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::openStory &to, JsonObject &from) {
  TRY_STATUS(from_json(to.story_poster_chat_id_, from.extract_field("story_poster_chat_id")));
  TRY_STATUS(from_json(to.story_id_, from.extract_field("story_id")));
  return Status::OK();
}

Status from_json(td_api::processChatFolderNewChats &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_folder_id_, from.extract_field("chat_folder_id")));
  TRY_STATUS(from_json(to.added_chat_ids_, from.extract_field("added_chat_ids")));
  return Status::OK();
}

Status from_json(td_api::readAllForumTopicReactions &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.forum_topic_id_, from.extract_field("forum_topic_id")));
  return Status::OK();
}

Status from_json(td_api::registerDevice &to, JsonObject &from) {
  TRY_STATUS(from_json(to.device_token_, from.extract_field("device_token")));
  TRY_STATUS(from_json(to.other_user_ids_, from.extract_field("other_user_ids")));
  return Status::OK();
}

Status from_json(td_api::removeInstalledBackground &to, JsonObject &from) {
  TRY_STATUS(from_json(to.background_id_, from.extract_field("background_id")));
  return Status::OK();
}

Status from_json(td_api::removeRecentSticker &to, JsonObject &from) {
  TRY_STATUS(from_json(to.is_attached_, from.extract_field("is_attached")));
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  return Status::OK();
}

Status from_json(td_api::reorderBotActiveUsernames &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.usernames_, from.extract_field("usernames")));
  return Status::OK();
}

Status from_json(td_api::reorderSupergroupActiveUsernames &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.usernames_, from.extract_field("usernames")));
  return Status::OK();
}

Status from_json(td_api::reportPhoneNumberCodeMissing &to, JsonObject &from) {
  TRY_STATUS(from_json(to.mobile_network_code_, from.extract_field("mobile_network_code")));
  return Status::OK();
}

Status from_json(td_api::resendAuthenticationCode &to, JsonObject &from) {
  TRY_STATUS(from_json(to.reason_, from.extract_field("reason")));
  return Status::OK();
}

Status from_json(td_api::resetNetworkStatistics &to, JsonObject &from) {
  return Status::OK();
}

Status from_json(td_api::searchCallMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  TRY_STATUS(from_json(to.only_missed_, from.extract_field("only_missed")));
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

Status from_json(td_api::searchPublicPosts &to, JsonObject &from) {
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
  TRY_STATUS(from_json(to.offset_, from.extract_field("offset")));
  TRY_STATUS(from_json(to.limit_, from.extract_field("limit")));
  TRY_STATUS(from_json(to.star_count_, from.extract_field("star_count")));
  return Status::OK();
}

Status from_json(td_api::searchStickerSets &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_type_, from.extract_field("sticker_type")));
  TRY_STATUS(from_json(to.query_, from.extract_field("query")));
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

Status from_json(td_api::sendGift &to, JsonObject &from) {
  TRY_STATUS(from_json(to.gift_id_, from.extract_field("gift_id")));
  TRY_STATUS(from_json(to.owner_id_, from.extract_field("owner_id")));
  TRY_STATUS(from_json(to.text_, from.extract_field("text")));
  TRY_STATUS(from_json(to.is_private_, from.extract_field("is_private")));
  TRY_STATUS(from_json(to.pay_for_upgrade_, from.extract_field("pay_for_upgrade")));
  return Status::OK();
}

Status from_json(td_api::sendQuickReplyShortcutMessages &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.shortcut_id_, from.extract_field("shortcut_id")));
  TRY_STATUS(from_json(to.sending_id_, from.extract_field("sending_id")));
  return Status::OK();
}

Status from_json(td_api::setArchiveChatListSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.settings_, from.extract_field("settings")));
  return Status::OK();
}

Status from_json(td_api::setBotInfoShortDescription &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.language_code_, from.extract_field("language_code")));
  TRY_STATUS(from_json(to.short_description_, from.extract_field("short_description")));
  return Status::OK();
}

Status from_json(td_api::setBusinessAwayMessageSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.away_message_settings_, from.extract_field("away_message_settings")));
  return Status::OK();
}

Status from_json(td_api::setChatAffiliateProgram &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.parameters_, from.extract_field("parameters")));
  return Status::OK();
}

Status from_json(td_api::setChatLocation &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.location_, from.extract_field("location")));
  return Status::OK();
}

Status from_json(td_api::setChatProfileAccentColor &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.profile_accent_color_id_, from.extract_field("profile_accent_color_id")));
  TRY_STATUS(from_json(to.profile_background_custom_emoji_id_, from.extract_field("profile_background_custom_emoji_id")));
  return Status::OK();
}

Status from_json(td_api::setDatabaseEncryptionKey &to, JsonObject &from) {
  TRY_STATUS(from_json_bytes(to.new_encryption_key_, from.extract_field("new_encryption_key")));
  return Status::OK();
}

Status from_json(td_api::setForumTopicNotificationSettings &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.forum_topic_id_, from.extract_field("forum_topic_id")));
  TRY_STATUS(from_json(to.notification_settings_, from.extract_field("notification_settings")));
  return Status::OK();
}

Status from_json(td_api::setLogStream &to, JsonObject &from) {
  TRY_STATUS(from_json(to.log_stream_, from.extract_field("log_stream")));
  return Status::OK();
}

Status from_json(td_api::setMessageSenderBotVerification &to, JsonObject &from) {
  TRY_STATUS(from_json(to.bot_user_id_, from.extract_field("bot_user_id")));
  TRY_STATUS(from_json(to.verified_id_, from.extract_field("verified_id")));
  TRY_STATUS(from_json(to.custom_description_, from.extract_field("custom_description")));
  return Status::OK();
}

Status from_json(td_api::setPersonalChat &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  return Status::OK();
}

Status from_json(td_api::setQuickReplyShortcutName &to, JsonObject &from) {
  TRY_STATUS(from_json(to.shortcut_id_, from.extract_field("shortcut_id")));
  TRY_STATUS(from_json(to.name_, from.extract_field("name")));
  return Status::OK();
}

Status from_json(td_api::setStickerPositionInSet &to, JsonObject &from) {
  TRY_STATUS(from_json(to.sticker_, from.extract_field("sticker")));
  TRY_STATUS(from_json(to.position_, from.extract_field("position")));
  return Status::OK();
}

Status from_json(td_api::setSupergroupUnrestrictBoostCount &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.unrestrict_boost_count_, from.extract_field("unrestrict_boost_count")));
  return Status::OK();
}

Status from_json(td_api::setUsername &to, JsonObject &from) {
  TRY_STATUS(from_json(to.username_, from.extract_field("username")));
  return Status::OK();
}

Status from_json(td_api::stopBusinessPoll &to, JsonObject &from) {
  TRY_STATUS(from_json(to.business_connection_id_, from.extract_field("business_connection_id")));
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  TRY_STATUS(from_json(to.reply_markup_, from.extract_field("reply_markup")));
  return Status::OK();
}

Status from_json(td_api::testCallString &to, JsonObject &from) {
  TRY_STATUS(from_json(to.x_, from.extract_field("x")));
  return Status::OK();
}

Status from_json(td_api::testSquareInt &to, JsonObject &from) {
  TRY_STATUS(from_json(to.x_, from.extract_field("x")));
  return Status::OK();
}

Status from_json(td_api::toggleChatGiftNotifications &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.are_enabled_, from.extract_field("are_enabled")));
  return Status::OK();
}

Status from_json(td_api::toggleForumTopicIsPinned &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.forum_topic_id_, from.extract_field("forum_topic_id")));
  TRY_STATUS(from_json(to.is_pinned_, from.extract_field("is_pinned")));
  return Status::OK();
}

Status from_json(td_api::toggleHasSponsoredMessagesEnabled &to, JsonObject &from) {
  TRY_STATUS(from_json(to.has_sponsored_messages_enabled_, from.extract_field("has_sponsored_messages_enabled")));
  return Status::OK();
}

Status from_json(td_api::toggleSupergroupIsAllHistoryAvailable &to, JsonObject &from) {
  TRY_STATUS(from_json(to.supergroup_id_, from.extract_field("supergroup_id")));
  TRY_STATUS(from_json(to.is_all_history_available_, from.extract_field("is_all_history_available")));
  return Status::OK();
}

Status from_json(td_api::toggleVideoChatMuteNewParticipants &to, JsonObject &from) {
  TRY_STATUS(from_json(to.group_call_id_, from.extract_field("group_call_id")));
  TRY_STATUS(from_json(to.mute_new_participants_, from.extract_field("mute_new_participants")));
  return Status::OK();
}

Status from_json(td_api::unpinChatMessage &to, JsonObject &from) {
  TRY_STATUS(from_json(to.chat_id_, from.extract_field("chat_id")));
  TRY_STATUS(from_json(to.message_id_, from.extract_field("message_id")));
  return Status::OK();
}

Status from_json(td_api::viewVideoMessageAdvertisement &to, JsonObject &from) {
  TRY_STATUS(from_json(to.advertisement_unique_id_, from.extract_field("advertisement_unique_id")));
  return Status::OK();
}

void to_json(JsonValueScope &jv, const td_api::advertisementSponsor &object) {
  auto jo = jv.enter_object();
  jo("@type", "advertisementSponsor");
  jo("url", object.url_);
  if (object.photo_) {
    jo("photo", ToJson(*object.photo_));
  }
  jo("info", object.info_);
}

void to_json(JsonValueScope &jv, const td_api::animations &object) {
  auto jo = jv.enter_object();
  jo("@type", "animations");
  jo("animations", ToJson(object.animations_));
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

void to_json(JsonValueScope &jv, const td_api::botVerificationParameters &object) {
  auto jo = jv.enter_object();
  jo("@type", "botVerificationParameters");
  jo("icon_custom_emoji_id", ToJson(JsonInt64{object.icon_custom_emoji_id_}));
  jo("organization_name", object.organization_name_);
  if (object.default_custom_description_) {
    jo("default_custom_description", ToJson(*object.default_custom_description_));
  }
  jo("can_set_custom_description", JsonBool{object.can_set_custom_description_});
}

void to_json(JsonValueScope &jv, const td_api::businessChatLinks &object) {
  auto jo = jv.enter_object();
  jo("@type", "businessChatLinks");
  jo("links", ToJson(object.links_));
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

void to_json(JsonValueScope &jv, const td_api::callProtocol &object) {
  auto jo = jv.enter_object();
  jo("@type", "callProtocol");
  jo("udp_p2p", JsonBool{object.udp_p2p_});
  jo("udp_reflector", JsonBool{object.udp_reflector_});
  jo("min_layer", object.min_layer_);
  jo("max_layer", object.max_layer_);
  jo("library_versions", ToJson(object.library_versions_));
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

void to_json(JsonValueScope &jv, const td_api::chatBackground &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatBackground");
  if (object.background_) {
    jo("background", ToJson(*object.background_));
  }
  jo("dark_theme_dimming", object.dark_theme_dimming_);
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

void to_json(JsonValueScope &jv, const td_api::chatFolderInviteLinks &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatFolderInviteLinks");
  jo("invite_links", ToJson(object.invite_links_));
}

void to_json(JsonValueScope &jv, const td_api::chatInviteLinks &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatInviteLinks");
  jo("total_count", object.total_count_);
  jo("invite_links", ToJson(object.invite_links_));
}

void to_json(JsonValueScope &jv, const td_api::chatMembers &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatMembers");
  jo("total_count", object.total_count_);
  jo("members", ToJson(object.members_));
}

void to_json(JsonValueScope &jv, const td_api::chatPhotos &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatPhotos");
  jo("total_count", object.total_count_);
  jo("photos", ToJson(object.photos_));
}

void to_json(JsonValueScope &jv, const td_api::chatStatisticsAdministratorActionsInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "chatStatisticsAdministratorActionsInfo");
  jo("user_id", object.user_id_);
  jo("deleted_message_count", object.deleted_message_count_);
  jo("banned_user_count", object.banned_user_count_);
  jo("restricted_user_count", object.restricted_user_count_);
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

void to_json(JsonValueScope &jv, const td_api::connectedWebsites &object) {
  auto jo = jv.enter_object();
  jo("@type", "connectedWebsites");
  jo("websites", ToJson(object.websites_));
}

void to_json(JsonValueScope &jv, const td_api::data &object) {
  auto jo = jv.enter_object();
  jo("@type", "data");
  jo("data", base64_encode(object.data_));
}

void to_json(JsonValueScope &jv, const td_api::downloadedFileCounts &object) {
  auto jo = jv.enter_object();
  jo("@type", "downloadedFileCounts");
  jo("active_count", object.active_count_);
  jo("paused_count", object.paused_count_);
  jo("completed_count", object.completed_count_);
}

void to_json(JsonValueScope &jv, const td_api::emojiKeywords &object) {
  auto jo = jv.enter_object();
  jo("@type", "emojiKeywords");
  jo("emoji_keywords", ToJson(object.emoji_keywords_));
}

void to_json(JsonValueScope &jv, const td_api::error &object) {
  auto jo = jv.enter_object();
  jo("@type", "error");
  jo("code", object.code_);
  jo("message", object.message_);
}

void to_json(JsonValueScope &jv, const td_api::formattedText &object) {
  auto jo = jv.enter_object();
  jo("@type", "formattedText");
  jo("text", object.text_);
  jo("entities", ToJson(object.entities_));
}

void to_json(JsonValueScope &jv, const td_api::foundChatMessages &object) {
  auto jo = jv.enter_object();
  jo("@type", "foundChatMessages");
  jo("total_count", object.total_count_);
  jo("messages", ToJson(object.messages_));
  jo("next_from_message_id", object.next_from_message_id_);
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

void to_json(JsonValueScope &jv, const td_api::giftPurchaseLimits &object) {
  auto jo = jv.enter_object();
  jo("@type", "giftPurchaseLimits");
  jo("total_count", object.total_count_);
  jo("remaining_count", object.remaining_count_);
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

void to_json(JsonValueScope &jv, const td_api::groupCallRecentSpeaker &object) {
  auto jo = jv.enter_object();
  jo("@type", "groupCallRecentSpeaker");
  if (object.participant_id_) {
    jo("participant_id", ToJson(*object.participant_id_));
  }
  jo("is_speaking", JsonBool{object.is_speaking_});
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
  if (object.cover_) {
    jo("cover", ToJson(*object.cover_));
  }
  jo("start_timestamp", object.start_timestamp_);
  jo("duration", object.duration_);
  jo("supports_streaming", JsonBool{object.supports_streaming_});
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

void to_json(JsonValueScope &jv, const td_api::logVerbosityLevel &object) {
  auto jo = jv.enter_object();
  jo("@type", "logVerbosityLevel");
  jo("verbosity_level", object.verbosity_level_);
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
  jo("alternative_videos", ToJson(object.alternative_videos_));
  jo("storyboards", ToJson(object.storyboards_));
  if (object.cover_) {
    jo("cover", ToJson(*object.cover_));
  }
  jo("start_timestamp", object.start_timestamp_);
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
  jo("story_poster_chat_id", object.story_poster_chat_id_);
  jo("story_id", object.story_id_);
  jo("via_mention", JsonBool{object.via_mention_});
}

void to_json(JsonValueScope &jv, const td_api::messageChecklist &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageChecklist");
  if (object.list_) {
    jo("list", ToJson(*object.list_));
  }
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

void to_json(JsonValueScope &jv, const td_api::messageGroupCall &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageGroupCall");
  jo("is_active", JsonBool{object.is_active_});
  jo("was_missed", JsonBool{object.was_missed_});
  jo("is_video", JsonBool{object.is_video_});
  jo("duration", object.duration_);
  jo("other_participant_ids", ToJson(object.other_participant_ids_));
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
  if (object.theme_) {
    jo("theme", ToJson(*object.theme_));
  }
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
  jo("is_name_implicit", JsonBool{object.is_name_implicit_});
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

void to_json(JsonValueScope &jv, const td_api::messageSuggestBirthdate &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSuggestBirthdate");
  if (object.birthdate_) {
    jo("birthdate", ToJson(*object.birthdate_));
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
  jo("subscription_until_date", object.subscription_until_date_);
  jo("is_recurring", JsonBool{object.is_recurring_});
  jo("is_first_recurring", JsonBool{object.is_first_recurring_});
  jo("invoice_name", object.invoice_name_);
}

void to_json(JsonValueScope &jv, const td_api::messagePaymentSuccessfulBot &object) {
  auto jo = jv.enter_object();
  jo("@type", "messagePaymentSuccessfulBot");
  jo("currency", object.currency_);
  jo("total_amount", object.total_amount_);
  jo("subscription_until_date", object.subscription_until_date_);
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
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
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
  if (object.text_) {
    jo("text", ToJson(*object.text_));
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

void to_json(JsonValueScope &jv, const td_api::messageGiftedTon &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageGiftedTon");
  jo("gifter_user_id", object.gifter_user_id_);
  jo("receiver_user_id", object.receiver_user_id_);
  jo("ton_amount", object.ton_amount_);
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

void to_json(JsonValueScope &jv, const td_api::messageGift &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageGift");
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
  if (object.sender_id_) {
    jo("sender_id", ToJson(*object.sender_id_));
  }
  if (object.receiver_id_) {
    jo("receiver_id", ToJson(*object.receiver_id_));
  }
  jo("received_gift_id", object.received_gift_id_);
  if (object.text_) {
    jo("text", ToJson(*object.text_));
  }
  jo("sell_star_count", object.sell_star_count_);
  jo("prepaid_upgrade_star_count", object.prepaid_upgrade_star_count_);
  jo("is_upgrade_separate", JsonBool{object.is_upgrade_separate_});
  jo("is_private", JsonBool{object.is_private_});
  jo("is_saved", JsonBool{object.is_saved_});
  jo("is_prepaid_upgrade", JsonBool{object.is_prepaid_upgrade_});
  jo("can_be_upgraded", JsonBool{object.can_be_upgraded_});
  jo("was_converted", JsonBool{object.was_converted_});
  jo("was_upgraded", JsonBool{object.was_upgraded_});
  jo("was_refunded", JsonBool{object.was_refunded_});
  jo("upgraded_received_gift_id", object.upgraded_received_gift_id_);
  jo("prepaid_upgrade_hash", object.prepaid_upgrade_hash_);
}

void to_json(JsonValueScope &jv, const td_api::messageUpgradedGift &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageUpgradedGift");
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
  if (object.sender_id_) {
    jo("sender_id", ToJson(*object.sender_id_));
  }
  if (object.receiver_id_) {
    jo("receiver_id", ToJson(*object.receiver_id_));
  }
  if (object.origin_) {
    jo("origin", ToJson(*object.origin_));
  }
  jo("received_gift_id", object.received_gift_id_);
  jo("is_saved", JsonBool{object.is_saved_});
  jo("can_be_transferred", JsonBool{object.can_be_transferred_});
  jo("was_transferred", JsonBool{object.was_transferred_});
  jo("transfer_star_count", object.transfer_star_count_);
  jo("drop_original_details_star_count", object.drop_original_details_star_count_);
  jo("next_transfer_date", object.next_transfer_date_);
  jo("next_resale_date", object.next_resale_date_);
  jo("export_date", object.export_date_);
}

void to_json(JsonValueScope &jv, const td_api::messageRefundedUpgradedGift &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageRefundedUpgradedGift");
  if (object.gift_) {
    jo("gift", ToJson(*object.gift_));
  }
  if (object.sender_id_) {
    jo("sender_id", ToJson(*object.sender_id_));
  }
  if (object.receiver_id_) {
    jo("receiver_id", ToJson(*object.receiver_id_));
  }
  if (object.origin_) {
    jo("origin", ToJson(*object.origin_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messagePaidMessagesRefunded &object) {
  auto jo = jv.enter_object();
  jo("@type", "messagePaidMessagesRefunded");
  jo("message_count", object.message_count_);
  jo("star_count", object.star_count_);
}

void to_json(JsonValueScope &jv, const td_api::messagePaidMessagePriceChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "messagePaidMessagePriceChanged");
  jo("paid_message_star_count", object.paid_message_star_count_);
}

void to_json(JsonValueScope &jv, const td_api::messageDirectMessagePriceChanged &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageDirectMessagePriceChanged");
  jo("is_enabled", JsonBool{object.is_enabled_});
  jo("paid_message_star_count", object.paid_message_star_count_);
}

void to_json(JsonValueScope &jv, const td_api::messageChecklistTasksDone &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageChecklistTasksDone");
  jo("checklist_message_id", object.checklist_message_id_);
  jo("marked_as_done_task_ids", ToJson(object.marked_as_done_task_ids_));
  jo("marked_as_not_done_task_ids", ToJson(object.marked_as_not_done_task_ids_));
}

void to_json(JsonValueScope &jv, const td_api::messageChecklistTasksAdded &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageChecklistTasksAdded");
  jo("checklist_message_id", object.checklist_message_id_);
  jo("tasks", ToJson(object.tasks_));
}

void to_json(JsonValueScope &jv, const td_api::messageSuggestedPostApprovalFailed &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSuggestedPostApprovalFailed");
  jo("suggested_post_message_id", object.suggested_post_message_id_);
  if (object.price_) {
    jo("price", ToJson(*object.price_));
  }
}

void to_json(JsonValueScope &jv, const td_api::messageSuggestedPostApproved &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSuggestedPostApproved");
  jo("suggested_post_message_id", object.suggested_post_message_id_);
  if (object.price_) {
    jo("price", ToJson(*object.price_));
  }
  jo("send_date", object.send_date_);
}

void to_json(JsonValueScope &jv, const td_api::messageSuggestedPostDeclined &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSuggestedPostDeclined");
  jo("suggested_post_message_id", object.suggested_post_message_id_);
  jo("comment", object.comment_);
}

void to_json(JsonValueScope &jv, const td_api::messageSuggestedPostPaid &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSuggestedPostPaid");
  jo("suggested_post_message_id", object.suggested_post_message_id_);
  if (object.star_amount_) {
    jo("star_amount", ToJson(*object.star_amount_));
  }
  jo("ton_amount", object.ton_amount_);
}

void to_json(JsonValueScope &jv, const td_api::messageSuggestedPostRefunded &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageSuggestedPostRefunded");
  jo("suggested_post_message_id", object.suggested_post_message_id_);
  if (object.reason_) {
    jo("reason", ToJson(*object.reason_));
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

void to_json(JsonValueScope &jv, const td_api::messageLinkInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageLinkInfo");
  jo("is_public", JsonBool{object.is_public_});
  jo("chat_id", object.chat_id_);
  if (object.topic_id_) {
    jo("topic_id", ToJson(*object.topic_id_));
  }
  if (object.message_) {
    jo("message", ToJson(*object.message_));
  }
  jo("media_timestamp", object.media_timestamp_);
  jo("for_album", JsonBool{object.for_album_});
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
  jo("checklist_task_id", object.checklist_task_id_);
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
  jo("story_poster_chat_id", object.story_poster_chat_id_);
  jo("story_id", object.story_id_);
}

void to_json(JsonValueScope &jv, const td_api::messageViewer &object) {
  auto jo = jv.enter_object();
  jo("@type", "messageViewer");
  jo("user_id", object.user_id_);
  jo("view_date", object.view_date_);
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

void to_json(JsonValueScope &jv, const td_api::outline &object) {
  auto jo = jv.enter_object();
  jo("@type", "outline");
  jo("paths", ToJson(object.paths_));
}

void to_json(JsonValueScope &jv, const td_api::PaidReactionType &object) {
  td_api::downcast_call(const_cast<td_api::PaidReactionType &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::paidReactionTypeRegular &object) {
  auto jo = jv.enter_object();
  jo("@type", "paidReactionTypeRegular");
}

void to_json(JsonValueScope &jv, const td_api::paidReactionTypeAnonymous &object) {
  auto jo = jv.enter_object();
  jo("@type", "paidReactionTypeAnonymous");
}

void to_json(JsonValueScope &jv, const td_api::paidReactionTypeChat &object) {
  auto jo = jv.enter_object();
  jo("@type", "paidReactionTypeChat");
  jo("chat_id", object.chat_id_);
}

void to_json(JsonValueScope &jv, const td_api::passportRequiredElement &object) {
  auto jo = jv.enter_object();
  jo("@type", "passportRequiredElement");
  jo("suitable_elements", ToJson(object.suitable_elements_));
}

void to_json(JsonValueScope &jv, const td_api::paymentResult &object) {
  auto jo = jv.enter_object();
  jo("@type", "paymentResult");
  jo("success", JsonBool{object.success_});
  jo("verification_url", object.verification_url_);
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

void to_json(JsonValueScope &jv, const td_api::premiumLimit &object) {
  auto jo = jv.enter_object();
  jo("@type", "premiumLimit");
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("default_value", object.default_value_);
  jo("premium_value", object.premium_value_);
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

void to_json(JsonValueScope &jv, const td_api::publicPostSearchLimits &object) {
  auto jo = jv.enter_object();
  jo("@type", "publicPostSearchLimits");
  jo("daily_free_query_count", object.daily_free_query_count_);
  jo("remaining_free_query_count", object.remaining_free_query_count_);
  jo("next_free_query_in", object.next_free_query_in_);
  jo("star_count", ToJson(JsonInt64{object.star_count_}));
  jo("is_current_query_free", JsonBool{object.is_current_query_free_});
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

void to_json(JsonValueScope &jv, const td_api::ReportChatResult &object) {
  td_api::downcast_call(const_cast<td_api::ReportChatResult &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::reportChatResultOk &object) {
  auto jo = jv.enter_object();
  jo("@type", "reportChatResultOk");
}

void to_json(JsonValueScope &jv, const td_api::reportChatResultOptionRequired &object) {
  auto jo = jv.enter_object();
  jo("@type", "reportChatResultOptionRequired");
  jo("title", object.title_);
  jo("options", ToJson(object.options_));
}

void to_json(JsonValueScope &jv, const td_api::reportChatResultTextRequired &object) {
  auto jo = jv.enter_object();
  jo("@type", "reportChatResultTextRequired");
  jo("option_id", base64_encode(object.option_id_));
  jo("is_optional", JsonBool{object.is_optional_});
}

void to_json(JsonValueScope &jv, const td_api::reportChatResultMessagesRequired &object) {
  auto jo = jv.enter_object();
  jo("@type", "reportChatResultMessagesRequired");
}

void to_json(JsonValueScope &jv, const td_api::savedCredentials &object) {
  auto jo = jv.enter_object();
  jo("@type", "savedCredentials");
  jo("id", object.id_);
  jo("title", object.title_);
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

void to_json(JsonValueScope &jv, const td_api::starGiveawayWinnerOption &object) {
  auto jo = jv.enter_object();
  jo("@type", "starGiveawayWinnerOption");
  jo("winner_count", object.winner_count_);
  jo("won_star_count", object.won_star_count_);
  jo("is_default", JsonBool{object.is_default_});
}

void to_json(JsonValueScope &jv, const td_api::starTransaction &object) {
  auto jo = jv.enter_object();
  jo("@type", "starTransaction");
  jo("id", object.id_);
  if (object.star_amount_) {
    jo("star_amount", ToJson(*object.star_amount_));
  }
  jo("is_refund", JsonBool{object.is_refund_});
  jo("date", object.date_);
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
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
  jo("size", object.size_);
  jo("covers", ToJson(object.covers_));
}

void to_json(JsonValueScope &jv, const td_api::story &object) {
  auto jo = jv.enter_object();
  jo("@type", "story");
  jo("id", object.id_);
  jo("poster_chat_id", object.poster_chat_id_);
  if (object.poster_id_) {
    jo("poster_id", ToJson(*object.poster_id_));
  }
  jo("date", object.date_);
  jo("is_being_posted", JsonBool{object.is_being_posted_});
  jo("is_being_edited", JsonBool{object.is_being_edited_});
  jo("is_edited", JsonBool{object.is_edited_});
  jo("is_posted_to_chat_page", JsonBool{object.is_posted_to_chat_page_});
  jo("is_visible_only_for_self", JsonBool{object.is_visible_only_for_self_});
  jo("can_be_added_to_album", JsonBool{object.can_be_added_to_album_});
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
  jo("album_ids", ToJson(object.album_ids_));
}

void to_json(JsonValueScope &jv, const td_api::storyInteractionInfo &object) {
  auto jo = jv.enter_object();
  jo("@type", "storyInteractionInfo");
  jo("view_count", object.view_count_);
  jo("forward_count", object.forward_count_);
  jo("reaction_count", object.reaction_count_);
  jo("recent_viewer_user_ids", ToJson(object.recent_viewer_user_ids_));
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

void to_json(JsonValueScope &jv, const td_api::suggestedActionSetProfilePhoto &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedActionSetProfilePhoto");
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

void to_json(JsonValueScope &jv, const td_api::suggestedActionCustom &object) {
  auto jo = jv.enter_object();
  jo("@type", "suggestedActionCustom");
  jo("name", object.name_);
  if (object.title_) {
    jo("title", ToJson(*object.title_));
  }
  if (object.description_) {
    jo("description", ToJson(*object.description_));
  }
  jo("url", object.url_);
}

void to_json(JsonValueScope &jv, const td_api::tMeUrls &object) {
  auto jo = jv.enter_object();
  jo("@type", "tMeUrls");
  jo("urls", ToJson(object.urls_));
}

void to_json(JsonValueScope &jv, const td_api::testVectorIntObject &object) {
  auto jo = jv.enter_object();
  jo("@type", "testVectorIntObject");
  jo("value", ToJson(object.value_));
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

void to_json(JsonValueScope &jv, const td_api::trendingStickerSets &object) {
  auto jo = jv.enter_object();
  jo("@type", "trendingStickerSets");
  jo("total_count", object.total_count_);
  jo("sets", ToJson(object.sets_));
  jo("is_premium", JsonBool{object.is_premium_});
}

void to_json(JsonValueScope &jv, const td_api::upgradedGiftBackdropCount &object) {
  auto jo = jv.enter_object();
  jo("@type", "upgradedGiftBackdropCount");
  if (object.backdrop_) {
    jo("backdrop", ToJson(*object.backdrop_));
  }
  jo("total_count", object.total_count_);
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
  if (object.upgraded_gift_colors_) {
    jo("upgraded_gift_colors", ToJson(*object.upgraded_gift_colors_));
  }
  jo("profile_accent_color_id", object.profile_accent_color_id_);
  jo("profile_background_custom_emoji_id", ToJson(JsonInt64{object.profile_background_custom_emoji_id_}));
  if (object.emoji_status_) {
    jo("emoji_status", ToJson(*object.emoji_status_));
  }
  jo("is_contact", JsonBool{object.is_contact_});
  jo("is_mutual_contact", JsonBool{object.is_mutual_contact_});
  jo("is_close_friend", JsonBool{object.is_close_friend_});
  if (object.verification_status_) {
    jo("verification_status", ToJson(*object.verification_status_));
  }
  jo("is_premium", JsonBool{object.is_premium_});
  jo("is_support", JsonBool{object.is_support_});
  if (object.restriction_info_) {
    jo("restriction_info", ToJson(*object.restriction_info_));
  }
  jo("has_active_stories", JsonBool{object.has_active_stories_});
  jo("has_unread_active_stories", JsonBool{object.has_unread_active_stories_});
  jo("restricts_new_chats", JsonBool{object.restricts_new_chats_});
  jo("paid_message_star_count", object.paid_message_star_count_);
  jo("have_access", JsonBool{object.have_access_});
  if (object.type_) {
    jo("type", ToJson(*object.type_));
  }
  jo("language_code", object.language_code_);
  jo("added_to_attachment_menu", JsonBool{object.added_to_attachment_menu_});
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
  jo("has_topics", JsonBool{object.has_topics_});
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

void to_json(JsonValueScope &jv, const td_api::videoChatStream &object) {
  auto jo = jv.enter_object();
  jo("@type", "videoChatStream");
  jo("channel_id", object.channel_id_);
  jo("scale", object.scale_);
  jo("time_offset", object.time_offset_);
}

void to_json(JsonValueScope &jv, const td_api::WebAppOpenMode &object) {
  td_api::downcast_call(const_cast<td_api::WebAppOpenMode &>(object), [&jv](const auto &object) { to_json(jv, object); });
}

void to_json(JsonValueScope &jv, const td_api::webAppOpenModeCompact &object) {
  auto jo = jv.enter_object();
  jo("@type", "webAppOpenModeCompact");
}

void to_json(JsonValueScope &jv, const td_api::webAppOpenModeFullSize &object) {
  auto jo = jv.enter_object();
  jo("@type", "webAppOpenModeFullSize");
}

void to_json(JsonValueScope &jv, const td_api::webAppOpenModeFullScreen &object) {
  auto jo = jv.enter_object();
  jo("@type", "webAppOpenModeFullScreen");
}

}  // namespace td_api
}  // namespace td
