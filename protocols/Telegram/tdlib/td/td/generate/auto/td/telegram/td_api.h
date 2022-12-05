#pragma once

#include "td/tl/TlObject.h"

#include <string>

#include <cstdint>
#include <utility>
#include <vector>

namespace td {
class TlStorerToString;

namespace td_api {

using int32 = std::int32_t;
using int53 = std::int64_t;
using int64 = std::int64_t;

using string = std::string;

using bytes = std::string;

template <class Type>
using array = std::vector<Type>;

using BaseObject = ::td::TlObject;

template <class Type>
using object_ptr = ::td::tl_object_ptr<Type>;

template <class Type, class... Args>
object_ptr<Type> make_object(Args &&... args) {
  return object_ptr<Type>(new Type(std::forward<Args>(args)...));
}

template <class ToType, class FromType>
object_ptr<ToType> move_object_as(FromType &&from) {
  return object_ptr<ToType>(static_cast<ToType *>(from.release()));
}

std::string to_string(const BaseObject &value);

template <class T>
std::string to_string(const object_ptr<T> &value) {
  if (value == nullptr) {
    return "null";
  }

  return to_string(*value);
}

template <class T>
std::string to_string(const std::vector<object_ptr<T>> &values) {
  std::string result = "{\n";
  for (const auto &value : values) {
    if (value == nullptr) {
      result += "null\n";
    } else {
      result += to_string(*value);
    }
  }
  result += "}\n";
  return result;
}

class accountTtl;

class addedReaction;

class addedReactions;

class address;

class animatedChatPhoto;

class animatedEmoji;

class animation;

class animations;

class attachmentMenuBot;

class attachmentMenuBotColor;

class audio;

class authenticationCodeInfo;

class AuthenticationCodeType;

class AuthorizationState;

class autoDownloadSettings;

class autoDownloadSettingsPresets;

class availableReaction;

class availableReactions;

class background;

class BackgroundFill;

class BackgroundType;

class backgrounds;

class bankCardActionOpenUrl;

class bankCardInfo;

class basicGroup;

class basicGroupFullInfo;

class botCommand;

class BotCommandScope;

class botCommands;

class botInfo;

class botMenuButton;

class call;

class CallDiscardReason;

class callId;

class CallProblem;

class callProtocol;

class callServer;

class CallServerType;

class CallState;

class callbackQueryAnswer;

class CallbackQueryPayload;

class CanTransferOwnershipResult;

class chat;

class ChatAction;

class ChatActionBar;

class chatAdministrator;

class chatAdministratorRights;

class chatAdministrators;

class ChatAvailableReactions;

class chatEvent;

class ChatEventAction;

class chatEventLogFilters;

class chatEvents;

class chatFilter;

class chatFilterInfo;

class chatInviteLink;

class chatInviteLinkCount;

class chatInviteLinkCounts;

class chatInviteLinkInfo;

class chatInviteLinkMember;

class chatInviteLinkMembers;

class chatInviteLinks;

class chatJoinRequest;

class chatJoinRequests;

class chatJoinRequestsInfo;

class ChatList;

class chatLists;

class chatLocation;

class chatMember;

class ChatMemberStatus;

class chatMembers;

class ChatMembersFilter;

class chatMessageSender;

class chatMessageSenders;

class chatNearby;

class chatNotificationSettings;

class chatPermissions;

class chatPhoto;

class chatPhotoInfo;

class chatPhotos;

class chatPosition;

class ChatReportReason;

class ChatSource;

class ChatStatistics;

class chatStatisticsAdministratorActionsInfo;

class chatStatisticsInviterInfo;

class chatStatisticsMessageInteractionInfo;

class chatStatisticsMessageSenderInfo;

class chatTheme;

class ChatType;

class chats;

class chatsNearby;

class CheckChatUsernameResult;

class CheckStickerSetNameResult;

class closedVectorPath;

class connectedWebsite;

class connectedWebsites;

class ConnectionState;

class contact;

class count;

class countries;

class countryInfo;

class customRequestResult;

class databaseStatistics;

class date;

class dateRange;

class datedFile;

class deepLinkInfo;

class DeviceToken;

class DiceStickers;

class document;

class downloadedFileCounts;

class draftMessage;

class EmailAddressAuthentication;

class emailAddressAuthenticationCodeInfo;

class emojiReaction;

class emojiStatus;

class emojiStatuses;

class emojis;

class encryptedCredentials;

class encryptedPassportElement;

class error;

class file;

class fileDownload;

class fileDownloadedPrefixSize;

class filePart;

class FileType;

class formattedText;

class forumTopic;

class forumTopicIcon;

class forumTopicInfo;

class foundFileDownloads;

class foundMessages;

class game;

class gameHighScore;

class gameHighScores;

class groupCall;

class groupCallId;

class groupCallParticipant;

class groupCallParticipantVideoInfo;

class groupCallRecentSpeaker;

class groupCallStream;

class groupCallStreams;

class GroupCallVideoQuality;

class groupCallVideoSourceGroup;

class hashtags;

class httpUrl;

class identityDocument;

class importedContacts;

class inlineKeyboardButton;

class InlineKeyboardButtonType;

class InlineQueryResult;

class inlineQueryResults;

class InputBackground;

class InputChatPhoto;

class InputCredentials;

class InputFile;

class inputIdentityDocument;

class InputInlineQueryResult;

class InputInvoice;

class InputMessageContent;

class InputPassportElement;

class inputPassportElementError;

class InputPassportElementErrorSource;

class inputPersonalDocument;

class inputSticker;

class inputThumbnail;

class InternalLinkType;

class invoice;

class jsonObjectMember;

class JsonValue;

class keyboardButton;

class KeyboardButtonType;

class labeledPricePart;

class languagePackInfo;

class languagePackString;

class LanguagePackStringValue;

class languagePackStrings;

class localFile;

class localizationTargetInfo;

class location;

class LogStream;

class logTags;

class logVerbosityLevel;

class LoginUrlInfo;

class MaskPoint;

class maskPosition;

class message;

class messageCalendar;

class messageCalendarDay;

class MessageContent;

class messageCopyOptions;

class MessageExtendedMedia;

class MessageFileType;

class messageForwardInfo;

class MessageForwardOrigin;

class messageInteractionInfo;

class messageLink;

class messageLinkInfo;

class messagePosition;

class messagePositions;

class messageReaction;

class messageReplyInfo;

class MessageSchedulingState;

class messageSendOptions;

class MessageSender;

class messageSenders;

class MessageSendingState;

class messageStatistics;

class messageThreadInfo;

class messages;

class minithumbnail;

class networkStatistics;

class NetworkStatisticsEntry;

class NetworkType;

class notification;

class notificationGroup;

class NotificationGroupType;

class NotificationSettingsScope;

class notificationSound;

class notificationSounds;

class NotificationType;

class ok;

class OptionValue;

class orderInfo;

class PageBlock;

class pageBlockCaption;

class PageBlockHorizontalAlignment;

class pageBlockListItem;

class pageBlockRelatedArticle;

class pageBlockTableCell;

class PageBlockVerticalAlignment;

class passportAuthorizationForm;

class PassportElement;

class passportElementError;

class PassportElementErrorSource;

class PassportElementType;

class passportElements;

class passportElementsWithErrors;

class passportRequiredElement;

class passportSuitableElement;

class passwordState;

class paymentForm;

class paymentOption;

class PaymentProvider;

class paymentReceipt;

class paymentResult;

class personalDetails;

class personalDocument;

class phoneNumberAuthenticationSettings;

class phoneNumberInfo;

class photo;

class photoSize;

class point;

class poll;

class pollOption;

class PollType;

class PremiumFeature;

class premiumFeaturePromotionAnimation;

class premiumFeatures;

class premiumLimit;

class PremiumLimitType;

class premiumPaymentOption;

class PremiumSource;

class premiumState;

class profilePhoto;

class proxies;

class proxy;

class ProxyType;

class PublicChatType;

class PushMessageContent;

class pushReceiverId;

class ReactionType;

class recommendedChatFilter;

class recommendedChatFilters;

class recoveryEmailAddress;

class remoteFile;

class ReplyMarkup;

class ResetPasswordResult;

class RichText;

class rtmpUrl;

class savedCredentials;

class scopeNotificationSettings;

class SearchMessagesFilter;

class seconds;

class secretChat;

class SecretChatState;

class sentWebAppMessage;

class session;

class SessionType;

class sessions;

class shippingOption;

class SpeechRecognitionResult;

class sponsoredMessage;

class sponsoredMessages;

class StatisticalGraph;

class statisticalValue;

class sticker;

class StickerFormat;

class stickerSet;

class stickerSetInfo;

class stickerSets;

class StickerType;

class stickers;

class storageStatistics;

class storageStatisticsByChat;

class storageStatisticsByFileType;

class storageStatisticsFast;

class StorePaymentPurpose;

class SuggestedAction;

class supergroup;

class supergroupFullInfo;

class SupergroupMembersFilter;

class tMeUrl;

class TMeUrlType;

class tMeUrls;

class TargetChat;

class temporaryPasswordState;

class termsOfService;

class testBytes;

class testInt;

class testString;

class testVectorInt;

class testVectorIntObject;

class testVectorString;

class testVectorStringObject;

class text;

class textEntities;

class textEntity;

class TextEntityType;

class TextParseMode;

class themeParameters;

class themeSettings;

class thumbnail;

class ThumbnailFormat;

class TopChatCategory;

class trendingStickerSets;

class unreadReaction;

class Update;

class updates;

class user;

class userFullInfo;

class UserPrivacySetting;

class UserPrivacySettingRule;

class userPrivacySettingRules;

class UserStatus;

class userSupportInfo;

class UserType;

class usernames;

class users;

class validatedOrderInfo;

class VectorPathCommand;

class venue;

class video;

class videoChat;

class videoNote;

class voiceNote;

class webAppInfo;

class webPage;

class webPageInstantView;

class Object;

class Object: public TlObject {
 public:
};

class Function: public TlObject {
 public:
};

class accountTtl final : public Object {
 public:
  int32 days_;

  accountTtl();

  explicit accountTtl(int32 days_);

  static const std::int32_t ID = 1324495492;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class addedReaction final : public Object {
 public:
  object_ptr<ReactionType> type_;
  object_ptr<MessageSender> sender_id_;

  addedReaction();

  addedReaction(object_ptr<ReactionType> &&type_, object_ptr<MessageSender> &&sender_id_);

  static const std::int32_t ID = -1130587313;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class addedReactions final : public Object {
 public:
  int32 total_count_;
  array<object_ptr<addedReaction>> reactions_;
  string next_offset_;

  addedReactions();

  addedReactions(int32 total_count_, array<object_ptr<addedReaction>> &&reactions_, string const &next_offset_);

  static const std::int32_t ID = 226352304;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class address final : public Object {
 public:
  string country_code_;
  string state_;
  string city_;
  string street_line1_;
  string street_line2_;
  string postal_code_;

  address();

  address(string const &country_code_, string const &state_, string const &city_, string const &street_line1_, string const &street_line2_, string const &postal_code_);

  static const std::int32_t ID = -2043654342;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class animatedChatPhoto final : public Object {
 public:
  int32 length_;
  object_ptr<file> file_;
  double main_frame_timestamp_;

  animatedChatPhoto();

  animatedChatPhoto(int32 length_, object_ptr<file> &&file_, double main_frame_timestamp_);

  static const std::int32_t ID = 191994926;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class animatedEmoji final : public Object {
 public:
  object_ptr<sticker> sticker_;
  int32 sticker_width_;
  int32 sticker_height_;
  int32 fitzpatrick_type_;
  object_ptr<file> sound_;

  animatedEmoji();

  animatedEmoji(object_ptr<sticker> &&sticker_, int32 sticker_width_, int32 sticker_height_, int32 fitzpatrick_type_, object_ptr<file> &&sound_);

  static const std::int32_t ID = 1378918079;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class animation final : public Object {
 public:
  int32 duration_;
  int32 width_;
  int32 height_;
  string file_name_;
  string mime_type_;
  bool has_stickers_;
  object_ptr<minithumbnail> minithumbnail_;
  object_ptr<thumbnail> thumbnail_;
  object_ptr<file> animation_;

  animation();

  animation(int32 duration_, int32 width_, int32 height_, string const &file_name_, string const &mime_type_, bool has_stickers_, object_ptr<minithumbnail> &&minithumbnail_, object_ptr<thumbnail> &&thumbnail_, object_ptr<file> &&animation_);

  static const std::int32_t ID = -872359106;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class animations final : public Object {
 public:
  array<object_ptr<animation>> animations_;

  animations();

  explicit animations(array<object_ptr<animation>> &&animations_);

  static const std::int32_t ID = 344216945;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class attachmentMenuBot final : public Object {
 public:
  int53 bot_user_id_;
  bool supports_self_chat_;
  bool supports_user_chats_;
  bool supports_bot_chats_;
  bool supports_group_chats_;
  bool supports_channel_chats_;
  bool supports_settings_;
  string name_;
  object_ptr<attachmentMenuBotColor> name_color_;
  object_ptr<file> default_icon_;
  object_ptr<file> ios_static_icon_;
  object_ptr<file> ios_animated_icon_;
  object_ptr<file> android_icon_;
  object_ptr<file> macos_icon_;
  object_ptr<attachmentMenuBotColor> icon_color_;
  object_ptr<file> web_app_placeholder_;

  attachmentMenuBot();

  attachmentMenuBot(int53 bot_user_id_, bool supports_self_chat_, bool supports_user_chats_, bool supports_bot_chats_, bool supports_group_chats_, bool supports_channel_chats_, bool supports_settings_, string const &name_, object_ptr<attachmentMenuBotColor> &&name_color_, object_ptr<file> &&default_icon_, object_ptr<file> &&ios_static_icon_, object_ptr<file> &&ios_animated_icon_, object_ptr<file> &&android_icon_, object_ptr<file> &&macos_icon_, object_ptr<attachmentMenuBotColor> &&icon_color_, object_ptr<file> &&web_app_placeholder_);

  static const std::int32_t ID = -1566056904;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class attachmentMenuBotColor final : public Object {
 public:
  int32 light_color_;
  int32 dark_color_;

  attachmentMenuBotColor();

  attachmentMenuBotColor(int32 light_color_, int32 dark_color_);

  static const std::int32_t ID = 1680039612;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class audio final : public Object {
 public:
  int32 duration_;
  string title_;
  string performer_;
  string file_name_;
  string mime_type_;
  object_ptr<minithumbnail> album_cover_minithumbnail_;
  object_ptr<thumbnail> album_cover_thumbnail_;
  array<object_ptr<thumbnail>> external_album_covers_;
  object_ptr<file> audio_;

  audio();

  audio(int32 duration_, string const &title_, string const &performer_, string const &file_name_, string const &mime_type_, object_ptr<minithumbnail> &&album_cover_minithumbnail_, object_ptr<thumbnail> &&album_cover_thumbnail_, array<object_ptr<thumbnail>> &&external_album_covers_, object_ptr<file> &&audio_);

  static const std::int32_t ID = -166398841;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class authenticationCodeInfo final : public Object {
 public:
  string phone_number_;
  object_ptr<AuthenticationCodeType> type_;
  object_ptr<AuthenticationCodeType> next_type_;
  int32 timeout_;

  authenticationCodeInfo();

  authenticationCodeInfo(string const &phone_number_, object_ptr<AuthenticationCodeType> &&type_, object_ptr<AuthenticationCodeType> &&next_type_, int32 timeout_);

  static const std::int32_t ID = -860345416;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class AuthenticationCodeType: public Object {
 public:
};

class authenticationCodeTypeTelegramMessage final : public AuthenticationCodeType {
 public:
  int32 length_;

  authenticationCodeTypeTelegramMessage();

  explicit authenticationCodeTypeTelegramMessage(int32 length_);

  static const std::int32_t ID = 2079628074;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class authenticationCodeTypeSms final : public AuthenticationCodeType {
 public:
  int32 length_;

  authenticationCodeTypeSms();

  explicit authenticationCodeTypeSms(int32 length_);

  static const std::int32_t ID = 962650760;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class authenticationCodeTypeCall final : public AuthenticationCodeType {
 public:
  int32 length_;

  authenticationCodeTypeCall();

  explicit authenticationCodeTypeCall(int32 length_);

  static const std::int32_t ID = 1636265063;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class authenticationCodeTypeFlashCall final : public AuthenticationCodeType {
 public:
  string pattern_;

  authenticationCodeTypeFlashCall();

  explicit authenticationCodeTypeFlashCall(string const &pattern_);

  static const std::int32_t ID = 1395882402;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class authenticationCodeTypeMissedCall final : public AuthenticationCodeType {
 public:
  string phone_number_prefix_;
  int32 length_;

  authenticationCodeTypeMissedCall();

  authenticationCodeTypeMissedCall(string const &phone_number_prefix_, int32 length_);

  static const std::int32_t ID = 700123783;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class AuthorizationState: public Object {
 public:
};

class authorizationStateWaitTdlibParameters final : public AuthorizationState {
 public:

  authorizationStateWaitTdlibParameters();

  static const std::int32_t ID = 904720988;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class authorizationStateWaitPhoneNumber final : public AuthorizationState {
 public:

  authorizationStateWaitPhoneNumber();

  static const std::int32_t ID = 306402531;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class authorizationStateWaitEmailAddress final : public AuthorizationState {
 public:
  bool allow_apple_id_;
  bool allow_google_id_;

  authorizationStateWaitEmailAddress();

  authorizationStateWaitEmailAddress(bool allow_apple_id_, bool allow_google_id_);

  static const std::int32_t ID = 1040478663;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class authorizationStateWaitEmailCode final : public AuthorizationState {
 public:
  bool allow_apple_id_;
  bool allow_google_id_;
  object_ptr<emailAddressAuthenticationCodeInfo> code_info_;
  int32 next_phone_number_authorization_date_;

  authorizationStateWaitEmailCode();

  authorizationStateWaitEmailCode(bool allow_apple_id_, bool allow_google_id_, object_ptr<emailAddressAuthenticationCodeInfo> &&code_info_, int32 next_phone_number_authorization_date_);

  static const std::int32_t ID = 174262505;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class authorizationStateWaitCode final : public AuthorizationState {
 public:
  object_ptr<authenticationCodeInfo> code_info_;

  authorizationStateWaitCode();

  explicit authorizationStateWaitCode(object_ptr<authenticationCodeInfo> &&code_info_);

  static const std::int32_t ID = 52643073;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class authorizationStateWaitOtherDeviceConfirmation final : public AuthorizationState {
 public:
  string link_;

  authorizationStateWaitOtherDeviceConfirmation();

  explicit authorizationStateWaitOtherDeviceConfirmation(string const &link_);

  static const std::int32_t ID = 860166378;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class authorizationStateWaitRegistration final : public AuthorizationState {
 public:
  object_ptr<termsOfService> terms_of_service_;

  authorizationStateWaitRegistration();

  explicit authorizationStateWaitRegistration(object_ptr<termsOfService> &&terms_of_service_);

  static const std::int32_t ID = 550350511;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class authorizationStateWaitPassword final : public AuthorizationState {
 public:
  string password_hint_;
  bool has_recovery_email_address_;
  string recovery_email_address_pattern_;

  authorizationStateWaitPassword();

  authorizationStateWaitPassword(string const &password_hint_, bool has_recovery_email_address_, string const &recovery_email_address_pattern_);

  static const std::int32_t ID = 187548796;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class authorizationStateReady final : public AuthorizationState {
 public:

  authorizationStateReady();

  static const std::int32_t ID = -1834871737;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class authorizationStateLoggingOut final : public AuthorizationState {
 public:

  authorizationStateLoggingOut();

  static const std::int32_t ID = 154449270;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class authorizationStateClosing final : public AuthorizationState {
 public:

  authorizationStateClosing();

  static const std::int32_t ID = 445855311;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class authorizationStateClosed final : public AuthorizationState {
 public:

  authorizationStateClosed();

  static const std::int32_t ID = 1526047584;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class autoDownloadSettings final : public Object {
 public:
  bool is_auto_download_enabled_;
  int32 max_photo_file_size_;
  int53 max_video_file_size_;
  int53 max_other_file_size_;
  int32 video_upload_bitrate_;
  bool preload_large_videos_;
  bool preload_next_audio_;
  bool use_less_data_for_calls_;

  autoDownloadSettings();

  autoDownloadSettings(bool is_auto_download_enabled_, int32 max_photo_file_size_, int53 max_video_file_size_, int53 max_other_file_size_, int32 video_upload_bitrate_, bool preload_large_videos_, bool preload_next_audio_, bool use_less_data_for_calls_);

  static const std::int32_t ID = -813805749;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class autoDownloadSettingsPresets final : public Object {
 public:
  object_ptr<autoDownloadSettings> low_;
  object_ptr<autoDownloadSettings> medium_;
  object_ptr<autoDownloadSettings> high_;

  autoDownloadSettingsPresets();

  autoDownloadSettingsPresets(object_ptr<autoDownloadSettings> &&low_, object_ptr<autoDownloadSettings> &&medium_, object_ptr<autoDownloadSettings> &&high_);

  static const std::int32_t ID = -782099166;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class availableReaction final : public Object {
 public:
  object_ptr<ReactionType> type_;
  bool needs_premium_;

  availableReaction();

  availableReaction(object_ptr<ReactionType> &&type_, bool needs_premium_);

  static const std::int32_t ID = -117292153;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class availableReactions final : public Object {
 public:
  array<object_ptr<availableReaction>> top_reactions_;
  array<object_ptr<availableReaction>> recent_reactions_;
  array<object_ptr<availableReaction>> popular_reactions_;
  bool allow_custom_emoji_;

  availableReactions();

  availableReactions(array<object_ptr<availableReaction>> &&top_reactions_, array<object_ptr<availableReaction>> &&recent_reactions_, array<object_ptr<availableReaction>> &&popular_reactions_, bool allow_custom_emoji_);

  static const std::int32_t ID = 1995943616;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class background final : public Object {
 public:
  int64 id_;
  bool is_default_;
  bool is_dark_;
  string name_;
  object_ptr<document> document_;
  object_ptr<BackgroundType> type_;

  background();

  background(int64 id_, bool is_default_, bool is_dark_, string const &name_, object_ptr<document> &&document_, object_ptr<BackgroundType> &&type_);

  static const std::int32_t ID = -429971172;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class BackgroundFill: public Object {
 public:
};

class backgroundFillSolid final : public BackgroundFill {
 public:
  int32 color_;

  backgroundFillSolid();

  explicit backgroundFillSolid(int32 color_);

  static const std::int32_t ID = 1010678813;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class backgroundFillGradient final : public BackgroundFill {
 public:
  int32 top_color_;
  int32 bottom_color_;
  int32 rotation_angle_;

  backgroundFillGradient();

  backgroundFillGradient(int32 top_color_, int32 bottom_color_, int32 rotation_angle_);

  static const std::int32_t ID = -1839206017;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class backgroundFillFreeformGradient final : public BackgroundFill {
 public:
  array<int32> colors_;

  backgroundFillFreeformGradient();

  explicit backgroundFillFreeformGradient(array<int32> &&colors_);

  static const std::int32_t ID = -1145469255;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class BackgroundType: public Object {
 public:
};

class backgroundTypeWallpaper final : public BackgroundType {
 public:
  bool is_blurred_;
  bool is_moving_;

  backgroundTypeWallpaper();

  backgroundTypeWallpaper(bool is_blurred_, bool is_moving_);

  static const std::int32_t ID = 1972128891;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class backgroundTypePattern final : public BackgroundType {
 public:
  object_ptr<BackgroundFill> fill_;
  int32 intensity_;
  bool is_inverted_;
  bool is_moving_;

  backgroundTypePattern();

  backgroundTypePattern(object_ptr<BackgroundFill> &&fill_, int32 intensity_, bool is_inverted_, bool is_moving_);

  static const std::int32_t ID = 1290213117;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class backgroundTypeFill final : public BackgroundType {
 public:
  object_ptr<BackgroundFill> fill_;

  backgroundTypeFill();

  explicit backgroundTypeFill(object_ptr<BackgroundFill> &&fill_);

  static const std::int32_t ID = 993008684;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class backgrounds final : public Object {
 public:
  array<object_ptr<background>> backgrounds_;

  backgrounds();

  explicit backgrounds(array<object_ptr<background>> &&backgrounds_);

  static const std::int32_t ID = 724728704;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class bankCardActionOpenUrl final : public Object {
 public:
  string text_;
  string url_;

  bankCardActionOpenUrl();

  bankCardActionOpenUrl(string const &text_, string const &url_);

  static const std::int32_t ID = -196454267;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class bankCardInfo final : public Object {
 public:
  string title_;
  array<object_ptr<bankCardActionOpenUrl>> actions_;

  bankCardInfo();

  bankCardInfo(string const &title_, array<object_ptr<bankCardActionOpenUrl>> &&actions_);

  static const std::int32_t ID = -2116647730;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class basicGroup final : public Object {
 public:
  int53 id_;
  int32 member_count_;
  object_ptr<ChatMemberStatus> status_;
  bool is_active_;
  int53 upgraded_to_supergroup_id_;

  basicGroup();

  basicGroup(int53 id_, int32 member_count_, object_ptr<ChatMemberStatus> &&status_, bool is_active_, int53 upgraded_to_supergroup_id_);

  static const std::int32_t ID = -194767217;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class basicGroupFullInfo final : public Object {
 public:
  object_ptr<chatPhoto> photo_;
  string description_;
  int53 creator_user_id_;
  array<object_ptr<chatMember>> members_;
  object_ptr<chatInviteLink> invite_link_;
  array<object_ptr<botCommands>> bot_commands_;

  basicGroupFullInfo();

  basicGroupFullInfo(object_ptr<chatPhoto> &&photo_, string const &description_, int53 creator_user_id_, array<object_ptr<chatMember>> &&members_, object_ptr<chatInviteLink> &&invite_link_, array<object_ptr<botCommands>> &&bot_commands_);

  static const std::int32_t ID = 2022233397;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botCommand final : public Object {
 public:
  string command_;
  string description_;

  botCommand();

  botCommand(string const &command_, string const &description_);

  static const std::int32_t ID = -1032140601;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class BotCommandScope: public Object {
 public:
};

class botCommandScopeDefault final : public BotCommandScope {
 public:

  botCommandScopeDefault();

  static const std::int32_t ID = 795652779;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botCommandScopeAllPrivateChats final : public BotCommandScope {
 public:

  botCommandScopeAllPrivateChats();

  static const std::int32_t ID = -344889543;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botCommandScopeAllGroupChats final : public BotCommandScope {
 public:

  botCommandScopeAllGroupChats();

  static const std::int32_t ID = -981088162;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botCommandScopeAllChatAdministrators final : public BotCommandScope {
 public:

  botCommandScopeAllChatAdministrators();

  static const std::int32_t ID = 1998329169;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botCommandScopeChat final : public BotCommandScope {
 public:
  int53 chat_id_;

  botCommandScopeChat();

  explicit botCommandScopeChat(int53 chat_id_);

  static const std::int32_t ID = -430234971;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botCommandScopeChatAdministrators final : public BotCommandScope {
 public:
  int53 chat_id_;

  botCommandScopeChatAdministrators();

  explicit botCommandScopeChatAdministrators(int53 chat_id_);

  static const std::int32_t ID = 1119682126;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botCommandScopeChatMember final : public BotCommandScope {
 public:
  int53 chat_id_;
  int53 user_id_;

  botCommandScopeChatMember();

  botCommandScopeChatMember(int53 chat_id_, int53 user_id_);

  static const std::int32_t ID = -211380494;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botCommands final : public Object {
 public:
  int53 bot_user_id_;
  array<object_ptr<botCommand>> commands_;

  botCommands();

  botCommands(int53 bot_user_id_, array<object_ptr<botCommand>> &&commands_);

  static const std::int32_t ID = 1741364468;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botInfo final : public Object {
 public:
  string share_text_;
  string description_;
  object_ptr<photo> photo_;
  object_ptr<animation> animation_;
  object_ptr<botMenuButton> menu_button_;
  array<object_ptr<botCommand>> commands_;
  object_ptr<chatAdministratorRights> default_group_administrator_rights_;
  object_ptr<chatAdministratorRights> default_channel_administrator_rights_;

  botInfo();

  botInfo(string const &share_text_, string const &description_, object_ptr<photo> &&photo_, object_ptr<animation> &&animation_, object_ptr<botMenuButton> &&menu_button_, array<object_ptr<botCommand>> &&commands_, object_ptr<chatAdministratorRights> &&default_group_administrator_rights_, object_ptr<chatAdministratorRights> &&default_channel_administrator_rights_);

  static const std::int32_t ID = 429675178;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botMenuButton final : public Object {
 public:
  string text_;
  string url_;

  botMenuButton();

  botMenuButton(string const &text_, string const &url_);

  static const std::int32_t ID = -944407322;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class call final : public Object {
 public:
  int32 id_;
  int53 user_id_;
  bool is_outgoing_;
  bool is_video_;
  object_ptr<CallState> state_;

  call();

  call(int32 id_, int53 user_id_, bool is_outgoing_, bool is_video_, object_ptr<CallState> &&state_);

  static const std::int32_t ID = 920360804;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class CallDiscardReason: public Object {
 public:
};

class callDiscardReasonEmpty final : public CallDiscardReason {
 public:

  callDiscardReasonEmpty();

  static const std::int32_t ID = -1258917949;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callDiscardReasonMissed final : public CallDiscardReason {
 public:

  callDiscardReasonMissed();

  static const std::int32_t ID = 1680358012;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callDiscardReasonDeclined final : public CallDiscardReason {
 public:

  callDiscardReasonDeclined();

  static const std::int32_t ID = -1729926094;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callDiscardReasonDisconnected final : public CallDiscardReason {
 public:

  callDiscardReasonDisconnected();

  static const std::int32_t ID = -1342872670;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callDiscardReasonHungUp final : public CallDiscardReason {
 public:

  callDiscardReasonHungUp();

  static const std::int32_t ID = 438216166;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callId final : public Object {
 public:
  int32 id_;

  callId();

  explicit callId(int32 id_);

  static const std::int32_t ID = 65717769;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class CallProblem: public Object {
 public:
};

class callProblemEcho final : public CallProblem {
 public:

  callProblemEcho();

  static const std::int32_t ID = 801116548;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callProblemNoise final : public CallProblem {
 public:

  callProblemNoise();

  static const std::int32_t ID = 1053065359;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callProblemInterruptions final : public CallProblem {
 public:

  callProblemInterruptions();

  static const std::int32_t ID = 1119493218;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callProblemDistortedSpeech final : public CallProblem {
 public:

  callProblemDistortedSpeech();

  static const std::int32_t ID = 379960581;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callProblemSilentLocal final : public CallProblem {
 public:

  callProblemSilentLocal();

  static const std::int32_t ID = 253652790;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callProblemSilentRemote final : public CallProblem {
 public:

  callProblemSilentRemote();

  static const std::int32_t ID = 573634714;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callProblemDropped final : public CallProblem {
 public:

  callProblemDropped();

  static const std::int32_t ID = -1207311487;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callProblemDistortedVideo final : public CallProblem {
 public:

  callProblemDistortedVideo();

  static const std::int32_t ID = 385245706;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callProblemPixelatedVideo final : public CallProblem {
 public:

  callProblemPixelatedVideo();

  static const std::int32_t ID = 2115315411;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callProtocol final : public Object {
 public:
  bool udp_p2p_;
  bool udp_reflector_;
  int32 min_layer_;
  int32 max_layer_;
  array<string> library_versions_;

  callProtocol();

  callProtocol(bool udp_p2p_, bool udp_reflector_, int32 min_layer_, int32 max_layer_, array<string> &&library_versions_);

  static const std::int32_t ID = -1075562897;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callServer final : public Object {
 public:
  int64 id_;
  string ip_address_;
  string ipv6_address_;
  int32 port_;
  object_ptr<CallServerType> type_;

  callServer();

  callServer(int64 id_, string const &ip_address_, string const &ipv6_address_, int32 port_, object_ptr<CallServerType> &&type_);

  static const std::int32_t ID = 1865932695;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class CallServerType: public Object {
 public:
};

class callServerTypeTelegramReflector final : public CallServerType {
 public:
  bytes peer_tag_;
  bool is_tcp_;

  callServerTypeTelegramReflector();

  callServerTypeTelegramReflector(bytes const &peer_tag_, bool is_tcp_);

  static const std::int32_t ID = 850343189;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callServerTypeWebrtc final : public CallServerType {
 public:
  string username_;
  string password_;
  bool supports_turn_;
  bool supports_stun_;

  callServerTypeWebrtc();

  callServerTypeWebrtc(string const &username_, string const &password_, bool supports_turn_, bool supports_stun_);

  static const std::int32_t ID = 1250622821;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class CallState: public Object {
 public:
};

class callStatePending final : public CallState {
 public:
  bool is_created_;
  bool is_received_;

  callStatePending();

  callStatePending(bool is_created_, bool is_received_);

  static const std::int32_t ID = 1073048620;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callStateExchangingKeys final : public CallState {
 public:

  callStateExchangingKeys();

  static const std::int32_t ID = -1848149403;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callStateReady final : public CallState {
 public:
  object_ptr<callProtocol> protocol_;
  array<object_ptr<callServer>> servers_;
  string config_;
  bytes encryption_key_;
  array<string> emojis_;
  bool allow_p2p_;

  callStateReady();

  callStateReady(object_ptr<callProtocol> &&protocol_, array<object_ptr<callServer>> &&servers_, string const &config_, bytes const &encryption_key_, array<string> &&emojis_, bool allow_p2p_);

  static const std::int32_t ID = -2000107571;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callStateHangingUp final : public CallState {
 public:

  callStateHangingUp();

  static const std::int32_t ID = -2133790038;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callStateDiscarded final : public CallState {
 public:
  object_ptr<CallDiscardReason> reason_;
  bool need_rating_;
  bool need_debug_information_;
  bool need_log_;

  callStateDiscarded();

  callStateDiscarded(object_ptr<CallDiscardReason> &&reason_, bool need_rating_, bool need_debug_information_, bool need_log_);

  static const std::int32_t ID = 1394310213;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callStateError final : public CallState {
 public:
  object_ptr<error> error_;

  callStateError();

  explicit callStateError(object_ptr<error> &&error_);

  static const std::int32_t ID = -975215467;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callbackQueryAnswer final : public Object {
 public:
  string text_;
  bool show_alert_;
  string url_;

  callbackQueryAnswer();

  callbackQueryAnswer(string const &text_, bool show_alert_, string const &url_);

  static const std::int32_t ID = 360867933;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class CallbackQueryPayload: public Object {
 public:
};

class callbackQueryPayloadData final : public CallbackQueryPayload {
 public:
  bytes data_;

  callbackQueryPayloadData();

  explicit callbackQueryPayloadData(bytes const &data_);

  static const std::int32_t ID = -1977729946;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callbackQueryPayloadDataWithPassword final : public CallbackQueryPayload {
 public:
  string password_;
  bytes data_;

  callbackQueryPayloadDataWithPassword();

  callbackQueryPayloadDataWithPassword(string const &password_, bytes const &data_);

  static const std::int32_t ID = 1340266738;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class callbackQueryPayloadGame final : public CallbackQueryPayload {
 public:
  string game_short_name_;

  callbackQueryPayloadGame();

  explicit callbackQueryPayloadGame(string const &game_short_name_);

  static const std::int32_t ID = 1303571512;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class CanTransferOwnershipResult: public Object {
 public:
};

class canTransferOwnershipResultOk final : public CanTransferOwnershipResult {
 public:

  canTransferOwnershipResultOk();

  static const std::int32_t ID = -89881021;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class canTransferOwnershipResultPasswordNeeded final : public CanTransferOwnershipResult {
 public:

  canTransferOwnershipResultPasswordNeeded();

  static const std::int32_t ID = 1548372703;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class canTransferOwnershipResultPasswordTooFresh final : public CanTransferOwnershipResult {
 public:
  int32 retry_after_;

  canTransferOwnershipResultPasswordTooFresh();

  explicit canTransferOwnershipResultPasswordTooFresh(int32 retry_after_);

  static const std::int32_t ID = 811440913;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class canTransferOwnershipResultSessionTooFresh final : public CanTransferOwnershipResult {
 public:
  int32 retry_after_;

  canTransferOwnershipResultSessionTooFresh();

  explicit canTransferOwnershipResultSessionTooFresh(int32 retry_after_);

  static const std::int32_t ID = 984664289;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chat final : public Object {
 public:
  int53 id_;
  object_ptr<ChatType> type_;
  string title_;
  object_ptr<chatPhotoInfo> photo_;
  object_ptr<chatPermissions> permissions_;
  object_ptr<message> last_message_;
  array<object_ptr<chatPosition>> positions_;
  object_ptr<MessageSender> message_sender_id_;
  bool has_protected_content_;
  bool is_marked_as_unread_;
  bool is_blocked_;
  bool has_scheduled_messages_;
  bool can_be_deleted_only_for_self_;
  bool can_be_deleted_for_all_users_;
  bool can_be_reported_;
  bool default_disable_notification_;
  int32 unread_count_;
  int53 last_read_inbox_message_id_;
  int53 last_read_outbox_message_id_;
  int32 unread_mention_count_;
  int32 unread_reaction_count_;
  object_ptr<chatNotificationSettings> notification_settings_;
  object_ptr<ChatAvailableReactions> available_reactions_;
  int32 message_ttl_;
  string theme_name_;
  object_ptr<ChatActionBar> action_bar_;
  object_ptr<videoChat> video_chat_;
  object_ptr<chatJoinRequestsInfo> pending_join_requests_;
  int53 reply_markup_message_id_;
  object_ptr<draftMessage> draft_message_;
  string client_data_;

  chat();

  chat(int53 id_, object_ptr<ChatType> &&type_, string const &title_, object_ptr<chatPhotoInfo> &&photo_, object_ptr<chatPermissions> &&permissions_, object_ptr<message> &&last_message_, array<object_ptr<chatPosition>> &&positions_, object_ptr<MessageSender> &&message_sender_id_, bool has_protected_content_, bool is_marked_as_unread_, bool is_blocked_, bool has_scheduled_messages_, bool can_be_deleted_only_for_self_, bool can_be_deleted_for_all_users_, bool can_be_reported_, bool default_disable_notification_, int32 unread_count_, int53 last_read_inbox_message_id_, int53 last_read_outbox_message_id_, int32 unread_mention_count_, int32 unread_reaction_count_, object_ptr<chatNotificationSettings> &&notification_settings_, object_ptr<ChatAvailableReactions> &&available_reactions_, int32 message_ttl_, string const &theme_name_, object_ptr<ChatActionBar> &&action_bar_, object_ptr<videoChat> &&video_chat_, object_ptr<chatJoinRequestsInfo> &&pending_join_requests_, int53 reply_markup_message_id_, object_ptr<draftMessage> &&draft_message_, string const &client_data_);

  static const std::int32_t ID = -1395697186;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChatAction: public Object {
 public:
};

class chatActionTyping final : public ChatAction {
 public:

  chatActionTyping();

  static const std::int32_t ID = 380122167;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionRecordingVideo final : public ChatAction {
 public:

  chatActionRecordingVideo();

  static const std::int32_t ID = 216553362;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionUploadingVideo final : public ChatAction {
 public:
  int32 progress_;

  chatActionUploadingVideo();

  explicit chatActionUploadingVideo(int32 progress_);

  static const std::int32_t ID = 1234185270;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionRecordingVoiceNote final : public ChatAction {
 public:

  chatActionRecordingVoiceNote();

  static const std::int32_t ID = -808850058;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionUploadingVoiceNote final : public ChatAction {
 public:
  int32 progress_;

  chatActionUploadingVoiceNote();

  explicit chatActionUploadingVoiceNote(int32 progress_);

  static const std::int32_t ID = -613643666;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionUploadingPhoto final : public ChatAction {
 public:
  int32 progress_;

  chatActionUploadingPhoto();

  explicit chatActionUploadingPhoto(int32 progress_);

  static const std::int32_t ID = 654240583;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionUploadingDocument final : public ChatAction {
 public:
  int32 progress_;

  chatActionUploadingDocument();

  explicit chatActionUploadingDocument(int32 progress_);

  static const std::int32_t ID = 167884362;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionChoosingSticker final : public ChatAction {
 public:

  chatActionChoosingSticker();

  static const std::int32_t ID = 372753697;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionChoosingLocation final : public ChatAction {
 public:

  chatActionChoosingLocation();

  static const std::int32_t ID = -2017893596;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionChoosingContact final : public ChatAction {
 public:

  chatActionChoosingContact();

  static const std::int32_t ID = -1222507496;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionStartPlayingGame final : public ChatAction {
 public:

  chatActionStartPlayingGame();

  static const std::int32_t ID = -865884164;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionRecordingVideoNote final : public ChatAction {
 public:

  chatActionRecordingVideoNote();

  static const std::int32_t ID = 16523393;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionUploadingVideoNote final : public ChatAction {
 public:
  int32 progress_;

  chatActionUploadingVideoNote();

  explicit chatActionUploadingVideoNote(int32 progress_);

  static const std::int32_t ID = 1172364918;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionWatchingAnimations final : public ChatAction {
 public:
  string emoji_;

  chatActionWatchingAnimations();

  explicit chatActionWatchingAnimations(string const &emoji_);

  static const std::int32_t ID = 2052990641;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionCancel final : public ChatAction {
 public:

  chatActionCancel();

  static const std::int32_t ID = 1160523958;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChatActionBar: public Object {
 public:
};

class chatActionBarReportSpam final : public ChatActionBar {
 public:
  bool can_unarchive_;

  chatActionBarReportSpam();

  explicit chatActionBarReportSpam(bool can_unarchive_);

  static const std::int32_t ID = -1312758246;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionBarReportUnrelatedLocation final : public ChatActionBar {
 public:

  chatActionBarReportUnrelatedLocation();

  static const std::int32_t ID = 758175489;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionBarInviteMembers final : public ChatActionBar {
 public:

  chatActionBarInviteMembers();

  static const std::int32_t ID = 1985313904;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionBarReportAddBlock final : public ChatActionBar {
 public:
  bool can_unarchive_;
  int32 distance_;

  chatActionBarReportAddBlock();

  chatActionBarReportAddBlock(bool can_unarchive_, int32 distance_);

  static const std::int32_t ID = -914150419;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionBarAddContact final : public ChatActionBar {
 public:

  chatActionBarAddContact();

  static const std::int32_t ID = -733325295;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionBarSharePhoneNumber final : public ChatActionBar {
 public:

  chatActionBarSharePhoneNumber();

  static const std::int32_t ID = 35188697;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatActionBarJoinRequest final : public ChatActionBar {
 public:
  string title_;
  bool is_channel_;
  int32 request_date_;

  chatActionBarJoinRequest();

  chatActionBarJoinRequest(string const &title_, bool is_channel_, int32 request_date_);

  static const std::int32_t ID = 1037140744;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatAdministrator final : public Object {
 public:
  int53 user_id_;
  string custom_title_;
  bool is_owner_;

  chatAdministrator();

  chatAdministrator(int53 user_id_, string const &custom_title_, bool is_owner_);

  static const std::int32_t ID = 1920449836;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatAdministratorRights final : public Object {
 public:
  bool can_manage_chat_;
  bool can_change_info_;
  bool can_post_messages_;
  bool can_edit_messages_;
  bool can_delete_messages_;
  bool can_invite_users_;
  bool can_restrict_members_;
  bool can_pin_messages_;
  bool can_manage_topics_;
  bool can_promote_members_;
  bool can_manage_video_chats_;
  bool is_anonymous_;

  chatAdministratorRights();

  chatAdministratorRights(bool can_manage_chat_, bool can_change_info_, bool can_post_messages_, bool can_edit_messages_, bool can_delete_messages_, bool can_invite_users_, bool can_restrict_members_, bool can_pin_messages_, bool can_manage_topics_, bool can_promote_members_, bool can_manage_video_chats_, bool is_anonymous_);

  static const std::int32_t ID = -1384650041;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatAdministrators final : public Object {
 public:
  array<object_ptr<chatAdministrator>> administrators_;

  chatAdministrators();

  explicit chatAdministrators(array<object_ptr<chatAdministrator>> &&administrators_);

  static const std::int32_t ID = -2126186435;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChatAvailableReactions: public Object {
 public:
};

class chatAvailableReactionsAll final : public ChatAvailableReactions {
 public:

  chatAvailableReactionsAll();

  static const std::int32_t ID = -537887666;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatAvailableReactionsSome final : public ChatAvailableReactions {
 public:
  array<object_ptr<ReactionType>> reactions_;

  chatAvailableReactionsSome();

  explicit chatAvailableReactionsSome(array<object_ptr<ReactionType>> &&reactions_);

  static const std::int32_t ID = -640810821;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEvent final : public Object {
 public:
  int64 id_;
  int32 date_;
  object_ptr<MessageSender> member_id_;
  object_ptr<ChatEventAction> action_;

  chatEvent();

  chatEvent(int64 id_, int32 date_, object_ptr<MessageSender> &&member_id_, object_ptr<ChatEventAction> &&action_);

  static const std::int32_t ID = -652102704;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChatEventAction: public Object {
 public:
};

class chatEventMessageEdited final : public ChatEventAction {
 public:
  object_ptr<message> old_message_;
  object_ptr<message> new_message_;

  chatEventMessageEdited();

  chatEventMessageEdited(object_ptr<message> &&old_message_, object_ptr<message> &&new_message_);

  static const std::int32_t ID = -430967304;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventMessageDeleted final : public ChatEventAction {
 public:
  object_ptr<message> message_;

  chatEventMessageDeleted();

  explicit chatEventMessageDeleted(object_ptr<message> &&message_);

  static const std::int32_t ID = -892974601;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventMessagePinned final : public ChatEventAction {
 public:
  object_ptr<message> message_;

  chatEventMessagePinned();

  explicit chatEventMessagePinned(object_ptr<message> &&message_);

  static const std::int32_t ID = 438742298;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventMessageUnpinned final : public ChatEventAction {
 public:
  object_ptr<message> message_;

  chatEventMessageUnpinned();

  explicit chatEventMessageUnpinned(object_ptr<message> &&message_);

  static const std::int32_t ID = -376161513;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventPollStopped final : public ChatEventAction {
 public:
  object_ptr<message> message_;

  chatEventPollStopped();

  explicit chatEventPollStopped(object_ptr<message> &&message_);

  static const std::int32_t ID = 2009893861;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventMemberJoined final : public ChatEventAction {
 public:

  chatEventMemberJoined();

  static const std::int32_t ID = -235468508;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventMemberJoinedByInviteLink final : public ChatEventAction {
 public:
  object_ptr<chatInviteLink> invite_link_;

  chatEventMemberJoinedByInviteLink();

  explicit chatEventMemberJoinedByInviteLink(object_ptr<chatInviteLink> &&invite_link_);

  static const std::int32_t ID = -253307459;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventMemberJoinedByRequest final : public ChatEventAction {
 public:
  int53 approver_user_id_;
  object_ptr<chatInviteLink> invite_link_;

  chatEventMemberJoinedByRequest();

  chatEventMemberJoinedByRequest(int53 approver_user_id_, object_ptr<chatInviteLink> &&invite_link_);

  static const std::int32_t ID = -1647804865;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventMemberInvited final : public ChatEventAction {
 public:
  int53 user_id_;
  object_ptr<ChatMemberStatus> status_;

  chatEventMemberInvited();

  chatEventMemberInvited(int53 user_id_, object_ptr<ChatMemberStatus> &&status_);

  static const std::int32_t ID = 953663433;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventMemberLeft final : public ChatEventAction {
 public:

  chatEventMemberLeft();

  static const std::int32_t ID = -948420593;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventMemberPromoted final : public ChatEventAction {
 public:
  int53 user_id_;
  object_ptr<ChatMemberStatus> old_status_;
  object_ptr<ChatMemberStatus> new_status_;

  chatEventMemberPromoted();

  chatEventMemberPromoted(int53 user_id_, object_ptr<ChatMemberStatus> &&old_status_, object_ptr<ChatMemberStatus> &&new_status_);

  static const std::int32_t ID = 525297761;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventMemberRestricted final : public ChatEventAction {
 public:
  object_ptr<MessageSender> member_id_;
  object_ptr<ChatMemberStatus> old_status_;
  object_ptr<ChatMemberStatus> new_status_;

  chatEventMemberRestricted();

  chatEventMemberRestricted(object_ptr<MessageSender> &&member_id_, object_ptr<ChatMemberStatus> &&old_status_, object_ptr<ChatMemberStatus> &&new_status_);

  static const std::int32_t ID = 1603608069;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventAvailableReactionsChanged final : public ChatEventAction {
 public:
  object_ptr<ChatAvailableReactions> old_available_reactions_;
  object_ptr<ChatAvailableReactions> new_available_reactions_;

  chatEventAvailableReactionsChanged();

  chatEventAvailableReactionsChanged(object_ptr<ChatAvailableReactions> &&old_available_reactions_, object_ptr<ChatAvailableReactions> &&new_available_reactions_);

  static const std::int32_t ID = -1749491521;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventDescriptionChanged final : public ChatEventAction {
 public:
  string old_description_;
  string new_description_;

  chatEventDescriptionChanged();

  chatEventDescriptionChanged(string const &old_description_, string const &new_description_);

  static const std::int32_t ID = 39112478;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventLinkedChatChanged final : public ChatEventAction {
 public:
  int53 old_linked_chat_id_;
  int53 new_linked_chat_id_;

  chatEventLinkedChatChanged();

  chatEventLinkedChatChanged(int53 old_linked_chat_id_, int53 new_linked_chat_id_);

  static const std::int32_t ID = 1797419439;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventLocationChanged final : public ChatEventAction {
 public:
  object_ptr<chatLocation> old_location_;
  object_ptr<chatLocation> new_location_;

  chatEventLocationChanged();

  chatEventLocationChanged(object_ptr<chatLocation> &&old_location_, object_ptr<chatLocation> &&new_location_);

  static const std::int32_t ID = -405930674;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventMessageTtlChanged final : public ChatEventAction {
 public:
  int32 old_message_ttl_;
  int32 new_message_ttl_;

  chatEventMessageTtlChanged();

  chatEventMessageTtlChanged(int32 old_message_ttl_, int32 new_message_ttl_);

  static const std::int32_t ID = 776386995;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventPermissionsChanged final : public ChatEventAction {
 public:
  object_ptr<chatPermissions> old_permissions_;
  object_ptr<chatPermissions> new_permissions_;

  chatEventPermissionsChanged();

  chatEventPermissionsChanged(object_ptr<chatPermissions> &&old_permissions_, object_ptr<chatPermissions> &&new_permissions_);

  static const std::int32_t ID = -1311557720;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventPhotoChanged final : public ChatEventAction {
 public:
  object_ptr<chatPhoto> old_photo_;
  object_ptr<chatPhoto> new_photo_;

  chatEventPhotoChanged();

  chatEventPhotoChanged(object_ptr<chatPhoto> &&old_photo_, object_ptr<chatPhoto> &&new_photo_);

  static const std::int32_t ID = -811572541;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventSlowModeDelayChanged final : public ChatEventAction {
 public:
  int32 old_slow_mode_delay_;
  int32 new_slow_mode_delay_;

  chatEventSlowModeDelayChanged();

  chatEventSlowModeDelayChanged(int32 old_slow_mode_delay_, int32 new_slow_mode_delay_);

  static const std::int32_t ID = -1653195765;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventStickerSetChanged final : public ChatEventAction {
 public:
  int64 old_sticker_set_id_;
  int64 new_sticker_set_id_;

  chatEventStickerSetChanged();

  chatEventStickerSetChanged(int64 old_sticker_set_id_, int64 new_sticker_set_id_);

  static const std::int32_t ID = -1243130481;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventTitleChanged final : public ChatEventAction {
 public:
  string old_title_;
  string new_title_;

  chatEventTitleChanged();

  chatEventTitleChanged(string const &old_title_, string const &new_title_);

  static const std::int32_t ID = 1134103250;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventUsernameChanged final : public ChatEventAction {
 public:
  string old_username_;
  string new_username_;

  chatEventUsernameChanged();

  chatEventUsernameChanged(string const &old_username_, string const &new_username_);

  static const std::int32_t ID = 1728558443;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventActiveUsernamesChanged final : public ChatEventAction {
 public:
  array<string> old_usernames_;
  array<string> new_usernames_;

  chatEventActiveUsernamesChanged();

  chatEventActiveUsernamesChanged(array<string> &&old_usernames_, array<string> &&new_usernames_);

  static const std::int32_t ID = -1508790810;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventHasProtectedContentToggled final : public ChatEventAction {
 public:
  bool has_protected_content_;

  chatEventHasProtectedContentToggled();

  explicit chatEventHasProtectedContentToggled(bool has_protected_content_);

  static const std::int32_t ID = -184270335;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventInvitesToggled final : public ChatEventAction {
 public:
  bool can_invite_users_;

  chatEventInvitesToggled();

  explicit chatEventInvitesToggled(bool can_invite_users_);

  static const std::int32_t ID = -62548373;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventIsAllHistoryAvailableToggled final : public ChatEventAction {
 public:
  bool is_all_history_available_;

  chatEventIsAllHistoryAvailableToggled();

  explicit chatEventIsAllHistoryAvailableToggled(bool is_all_history_available_);

  static const std::int32_t ID = -1599063019;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventSignMessagesToggled final : public ChatEventAction {
 public:
  bool sign_messages_;

  chatEventSignMessagesToggled();

  explicit chatEventSignMessagesToggled(bool sign_messages_);

  static const std::int32_t ID = -1313265634;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventInviteLinkEdited final : public ChatEventAction {
 public:
  object_ptr<chatInviteLink> old_invite_link_;
  object_ptr<chatInviteLink> new_invite_link_;

  chatEventInviteLinkEdited();

  chatEventInviteLinkEdited(object_ptr<chatInviteLink> &&old_invite_link_, object_ptr<chatInviteLink> &&new_invite_link_);

  static const std::int32_t ID = -460190366;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventInviteLinkRevoked final : public ChatEventAction {
 public:
  object_ptr<chatInviteLink> invite_link_;

  chatEventInviteLinkRevoked();

  explicit chatEventInviteLinkRevoked(object_ptr<chatInviteLink> &&invite_link_);

  static const std::int32_t ID = -1579417629;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventInviteLinkDeleted final : public ChatEventAction {
 public:
  object_ptr<chatInviteLink> invite_link_;

  chatEventInviteLinkDeleted();

  explicit chatEventInviteLinkDeleted(object_ptr<chatInviteLink> &&invite_link_);

  static const std::int32_t ID = -1394974361;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventVideoChatCreated final : public ChatEventAction {
 public:
  int32 group_call_id_;

  chatEventVideoChatCreated();

  explicit chatEventVideoChatCreated(int32 group_call_id_);

  static const std::int32_t ID = 1822853755;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventVideoChatEnded final : public ChatEventAction {
 public:
  int32 group_call_id_;

  chatEventVideoChatEnded();

  explicit chatEventVideoChatEnded(int32 group_call_id_);

  static const std::int32_t ID = 1630039112;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventVideoChatMuteNewParticipantsToggled final : public ChatEventAction {
 public:
  bool mute_new_participants_;

  chatEventVideoChatMuteNewParticipantsToggled();

  explicit chatEventVideoChatMuteNewParticipantsToggled(bool mute_new_participants_);

  static const std::int32_t ID = -126547970;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventVideoChatParticipantIsMutedToggled final : public ChatEventAction {
 public:
  object_ptr<MessageSender> participant_id_;
  bool is_muted_;

  chatEventVideoChatParticipantIsMutedToggled();

  chatEventVideoChatParticipantIsMutedToggled(object_ptr<MessageSender> &&participant_id_, bool is_muted_);

  static const std::int32_t ID = 521165047;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventVideoChatParticipantVolumeLevelChanged final : public ChatEventAction {
 public:
  object_ptr<MessageSender> participant_id_;
  int32 volume_level_;

  chatEventVideoChatParticipantVolumeLevelChanged();

  chatEventVideoChatParticipantVolumeLevelChanged(object_ptr<MessageSender> &&participant_id_, int32 volume_level_);

  static const std::int32_t ID = 1131385534;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventIsForumToggled final : public ChatEventAction {
 public:
  bool is_forum_;

  chatEventIsForumToggled();

  explicit chatEventIsForumToggled(bool is_forum_);

  static const std::int32_t ID = 1516491033;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventForumTopicCreated final : public ChatEventAction {
 public:
  object_ptr<forumTopicInfo> topic_info_;

  chatEventForumTopicCreated();

  explicit chatEventForumTopicCreated(object_ptr<forumTopicInfo> &&topic_info_);

  static const std::int32_t ID = 2005269314;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventForumTopicEdited final : public ChatEventAction {
 public:
  object_ptr<forumTopicInfo> old_topic_info_;
  object_ptr<forumTopicInfo> new_topic_info_;

  chatEventForumTopicEdited();

  chatEventForumTopicEdited(object_ptr<forumTopicInfo> &&old_topic_info_, object_ptr<forumTopicInfo> &&new_topic_info_);

  static const std::int32_t ID = 1624910860;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventForumTopicToggleIsClosed final : public ChatEventAction {
 public:
  object_ptr<forumTopicInfo> topic_info_;

  chatEventForumTopicToggleIsClosed();

  explicit chatEventForumTopicToggleIsClosed(object_ptr<forumTopicInfo> &&topic_info_);

  static const std::int32_t ID = -962704070;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventForumTopicDeleted final : public ChatEventAction {
 public:
  object_ptr<forumTopicInfo> topic_info_;

  chatEventForumTopicDeleted();

  explicit chatEventForumTopicDeleted(object_ptr<forumTopicInfo> &&topic_info_);

  static const std::int32_t ID = -1332795123;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventForumTopicPinned final : public ChatEventAction {
 public:
  object_ptr<forumTopicInfo> old_topic_info_;
  object_ptr<forumTopicInfo> new_topic_info_;

  chatEventForumTopicPinned();

  chatEventForumTopicPinned(object_ptr<forumTopicInfo> &&old_topic_info_, object_ptr<forumTopicInfo> &&new_topic_info_);

  static const std::int32_t ID = 2143626222;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEventLogFilters final : public Object {
 public:
  bool message_edits_;
  bool message_deletions_;
  bool message_pins_;
  bool member_joins_;
  bool member_leaves_;
  bool member_invites_;
  bool member_promotions_;
  bool member_restrictions_;
  bool info_changes_;
  bool setting_changes_;
  bool invite_link_changes_;
  bool video_chat_changes_;
  bool forum_changes_;

  chatEventLogFilters();

  chatEventLogFilters(bool message_edits_, bool message_deletions_, bool message_pins_, bool member_joins_, bool member_leaves_, bool member_invites_, bool member_promotions_, bool member_restrictions_, bool info_changes_, bool setting_changes_, bool invite_link_changes_, bool video_chat_changes_, bool forum_changes_);

  static const std::int32_t ID = 1648261677;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatEvents final : public Object {
 public:
  array<object_ptr<chatEvent>> events_;

  chatEvents();

  explicit chatEvents(array<object_ptr<chatEvent>> &&events_);

  static const std::int32_t ID = -585329664;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatFilter final : public Object {
 public:
  string title_;
  string icon_name_;
  array<int53> pinned_chat_ids_;
  array<int53> included_chat_ids_;
  array<int53> excluded_chat_ids_;
  bool exclude_muted_;
  bool exclude_read_;
  bool exclude_archived_;
  bool include_contacts_;
  bool include_non_contacts_;
  bool include_bots_;
  bool include_groups_;
  bool include_channels_;

  chatFilter();

  chatFilter(string const &title_, string const &icon_name_, array<int53> &&pinned_chat_ids_, array<int53> &&included_chat_ids_, array<int53> &&excluded_chat_ids_, bool exclude_muted_, bool exclude_read_, bool exclude_archived_, bool include_contacts_, bool include_non_contacts_, bool include_bots_, bool include_groups_, bool include_channels_);

  static const std::int32_t ID = -664815123;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatFilterInfo final : public Object {
 public:
  int32 id_;
  string title_;
  string icon_name_;

  chatFilterInfo();

  chatFilterInfo(int32 id_, string const &title_, string const &icon_name_);

  static const std::int32_t ID = -943721165;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatInviteLink final : public Object {
 public:
  string invite_link_;
  string name_;
  int53 creator_user_id_;
  int32 date_;
  int32 edit_date_;
  int32 expiration_date_;
  int32 member_limit_;
  int32 member_count_;
  int32 pending_join_request_count_;
  bool creates_join_request_;
  bool is_primary_;
  bool is_revoked_;

  chatInviteLink();

  chatInviteLink(string const &invite_link_, string const &name_, int53 creator_user_id_, int32 date_, int32 edit_date_, int32 expiration_date_, int32 member_limit_, int32 member_count_, int32 pending_join_request_count_, bool creates_join_request_, bool is_primary_, bool is_revoked_);

  static const std::int32_t ID = -205812476;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatInviteLinkCount final : public Object {
 public:
  int53 user_id_;
  int32 invite_link_count_;
  int32 revoked_invite_link_count_;

  chatInviteLinkCount();

  chatInviteLinkCount(int53 user_id_, int32 invite_link_count_, int32 revoked_invite_link_count_);

  static const std::int32_t ID = -1021999210;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatInviteLinkCounts final : public Object {
 public:
  array<object_ptr<chatInviteLinkCount>> invite_link_counts_;

  chatInviteLinkCounts();

  explicit chatInviteLinkCounts(array<object_ptr<chatInviteLinkCount>> &&invite_link_counts_);

  static const std::int32_t ID = 920326637;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatInviteLinkInfo final : public Object {
 public:
  int53 chat_id_;
  int32 accessible_for_;
  object_ptr<ChatType> type_;
  string title_;
  object_ptr<chatPhotoInfo> photo_;
  string description_;
  int32 member_count_;
  array<int53> member_user_ids_;
  bool creates_join_request_;
  bool is_public_;

  chatInviteLinkInfo();

  chatInviteLinkInfo(int53 chat_id_, int32 accessible_for_, object_ptr<ChatType> &&type_, string const &title_, object_ptr<chatPhotoInfo> &&photo_, string const &description_, int32 member_count_, array<int53> &&member_user_ids_, bool creates_join_request_, bool is_public_);

  static const std::int32_t ID = 546234276;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatInviteLinkMember final : public Object {
 public:
  int53 user_id_;
  int32 joined_chat_date_;
  int53 approver_user_id_;

  chatInviteLinkMember();

  chatInviteLinkMember(int53 user_id_, int32 joined_chat_date_, int53 approver_user_id_);

  static const std::int32_t ID = -1409060582;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatInviteLinkMembers final : public Object {
 public:
  int32 total_count_;
  array<object_ptr<chatInviteLinkMember>> members_;

  chatInviteLinkMembers();

  chatInviteLinkMembers(int32 total_count_, array<object_ptr<chatInviteLinkMember>> &&members_);

  static const std::int32_t ID = 315635051;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatInviteLinks final : public Object {
 public:
  int32 total_count_;
  array<object_ptr<chatInviteLink>> invite_links_;

  chatInviteLinks();

  chatInviteLinks(int32 total_count_, array<object_ptr<chatInviteLink>> &&invite_links_);

  static const std::int32_t ID = 112891427;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatJoinRequest final : public Object {
 public:
  int53 user_id_;
  int32 date_;
  string bio_;

  chatJoinRequest();

  chatJoinRequest(int53 user_id_, int32 date_, string const &bio_);

  static const std::int32_t ID = 59341416;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatJoinRequests final : public Object {
 public:
  int32 total_count_;
  array<object_ptr<chatJoinRequest>> requests_;

  chatJoinRequests();

  chatJoinRequests(int32 total_count_, array<object_ptr<chatJoinRequest>> &&requests_);

  static const std::int32_t ID = 1291680519;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatJoinRequestsInfo final : public Object {
 public:
  int32 total_count_;
  array<int53> user_ids_;

  chatJoinRequestsInfo();

  chatJoinRequestsInfo(int32 total_count_, array<int53> &&user_ids_);

  static const std::int32_t ID = 888534463;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChatList: public Object {
 public:
};

class chatListMain final : public ChatList {
 public:

  chatListMain();

  static const std::int32_t ID = -400991316;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatListArchive final : public ChatList {
 public:

  chatListArchive();

  static const std::int32_t ID = 362770115;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatListFilter final : public ChatList {
 public:
  int32 chat_filter_id_;

  chatListFilter();

  explicit chatListFilter(int32 chat_filter_id_);

  static const std::int32_t ID = -2022707655;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatLists final : public Object {
 public:
  array<object_ptr<ChatList>> chat_lists_;

  chatLists();

  explicit chatLists(array<object_ptr<ChatList>> &&chat_lists_);

  static const std::int32_t ID = -258292771;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatLocation final : public Object {
 public:
  object_ptr<location> location_;
  string address_;

  chatLocation();

  chatLocation(object_ptr<location> &&location_, string const &address_);

  static const std::int32_t ID = -1566863583;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatMember final : public Object {
 public:
  object_ptr<MessageSender> member_id_;
  int53 inviter_user_id_;
  int32 joined_chat_date_;
  object_ptr<ChatMemberStatus> status_;

  chatMember();

  chatMember(object_ptr<MessageSender> &&member_id_, int53 inviter_user_id_, int32 joined_chat_date_, object_ptr<ChatMemberStatus> &&status_);

  static const std::int32_t ID = 1829953909;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChatMemberStatus: public Object {
 public:
};

class chatMemberStatusCreator final : public ChatMemberStatus {
 public:
  string custom_title_;
  bool is_anonymous_;
  bool is_member_;

  chatMemberStatusCreator();

  chatMemberStatusCreator(string const &custom_title_, bool is_anonymous_, bool is_member_);

  static const std::int32_t ID = -160019714;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatMemberStatusAdministrator final : public ChatMemberStatus {
 public:
  string custom_title_;
  bool can_be_edited_;
  object_ptr<chatAdministratorRights> rights_;

  chatMemberStatusAdministrator();

  chatMemberStatusAdministrator(string const &custom_title_, bool can_be_edited_, object_ptr<chatAdministratorRights> &&rights_);

  static const std::int32_t ID = -70024163;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatMemberStatusMember final : public ChatMemberStatus {
 public:

  chatMemberStatusMember();

  static const std::int32_t ID = 844723285;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatMemberStatusRestricted final : public ChatMemberStatus {
 public:
  bool is_member_;
  int32 restricted_until_date_;
  object_ptr<chatPermissions> permissions_;

  chatMemberStatusRestricted();

  chatMemberStatusRestricted(bool is_member_, int32 restricted_until_date_, object_ptr<chatPermissions> &&permissions_);

  static const std::int32_t ID = 1661432998;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatMemberStatusLeft final : public ChatMemberStatus {
 public:

  chatMemberStatusLeft();

  static const std::int32_t ID = -5815259;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatMemberStatusBanned final : public ChatMemberStatus {
 public:
  int32 banned_until_date_;

  chatMemberStatusBanned();

  explicit chatMemberStatusBanned(int32 banned_until_date_);

  static const std::int32_t ID = -1653518666;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatMembers final : public Object {
 public:
  int32 total_count_;
  array<object_ptr<chatMember>> members_;

  chatMembers();

  chatMembers(int32 total_count_, array<object_ptr<chatMember>> &&members_);

  static const std::int32_t ID = -497558622;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChatMembersFilter: public Object {
 public:
};

class chatMembersFilterContacts final : public ChatMembersFilter {
 public:

  chatMembersFilterContacts();

  static const std::int32_t ID = 1774485671;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatMembersFilterAdministrators final : public ChatMembersFilter {
 public:

  chatMembersFilterAdministrators();

  static const std::int32_t ID = -1266893796;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatMembersFilterMembers final : public ChatMembersFilter {
 public:

  chatMembersFilterMembers();

  static const std::int32_t ID = 670504342;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatMembersFilterMention final : public ChatMembersFilter {
 public:
  int53 message_thread_id_;

  chatMembersFilterMention();

  explicit chatMembersFilterMention(int53 message_thread_id_);

  static const std::int32_t ID = 856419831;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatMembersFilterRestricted final : public ChatMembersFilter {
 public:

  chatMembersFilterRestricted();

  static const std::int32_t ID = 1256282813;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatMembersFilterBanned final : public ChatMembersFilter {
 public:

  chatMembersFilterBanned();

  static const std::int32_t ID = -1863102648;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatMembersFilterBots final : public ChatMembersFilter {
 public:

  chatMembersFilterBots();

  static const std::int32_t ID = -1422567288;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatMessageSender final : public Object {
 public:
  object_ptr<MessageSender> sender_;
  bool needs_premium_;

  chatMessageSender();

  chatMessageSender(object_ptr<MessageSender> &&sender_, bool needs_premium_);

  static const std::int32_t ID = 760590010;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatMessageSenders final : public Object {
 public:
  array<object_ptr<chatMessageSender>> senders_;

  chatMessageSenders();

  explicit chatMessageSenders(array<object_ptr<chatMessageSender>> &&senders_);

  static const std::int32_t ID = -1866230970;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatNearby final : public Object {
 public:
  int53 chat_id_;
  int32 distance_;

  chatNearby();

  chatNearby(int53 chat_id_, int32 distance_);

  static const std::int32_t ID = 48120405;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatNotificationSettings final : public Object {
 public:
  bool use_default_mute_for_;
  int32 mute_for_;
  bool use_default_sound_;
  int64 sound_id_;
  bool use_default_show_preview_;
  bool show_preview_;
  bool use_default_disable_pinned_message_notifications_;
  bool disable_pinned_message_notifications_;
  bool use_default_disable_mention_notifications_;
  bool disable_mention_notifications_;

  chatNotificationSettings();

  chatNotificationSettings(bool use_default_mute_for_, int32 mute_for_, bool use_default_sound_, int64 sound_id_, bool use_default_show_preview_, bool show_preview_, bool use_default_disable_pinned_message_notifications_, bool disable_pinned_message_notifications_, bool use_default_disable_mention_notifications_, bool disable_mention_notifications_);

  static const std::int32_t ID = 944322400;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatPermissions final : public Object {
 public:
  bool can_send_messages_;
  bool can_send_media_messages_;
  bool can_send_polls_;
  bool can_send_other_messages_;
  bool can_add_web_page_previews_;
  bool can_change_info_;
  bool can_invite_users_;
  bool can_pin_messages_;
  bool can_manage_topics_;

  chatPermissions();

  chatPermissions(bool can_send_messages_, bool can_send_media_messages_, bool can_send_polls_, bool can_send_other_messages_, bool can_add_web_page_previews_, bool can_change_info_, bool can_invite_users_, bool can_pin_messages_, bool can_manage_topics_);

  static const std::int32_t ID = -835592567;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatPhoto final : public Object {
 public:
  int64 id_;
  int32 added_date_;
  object_ptr<minithumbnail> minithumbnail_;
  array<object_ptr<photoSize>> sizes_;
  object_ptr<animatedChatPhoto> animation_;
  object_ptr<animatedChatPhoto> small_animation_;

  chatPhoto();

  chatPhoto(int64 id_, int32 added_date_, object_ptr<minithumbnail> &&minithumbnail_, array<object_ptr<photoSize>> &&sizes_, object_ptr<animatedChatPhoto> &&animation_, object_ptr<animatedChatPhoto> &&small_animation_);

  static const std::int32_t ID = 1854980206;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatPhotoInfo final : public Object {
 public:
  object_ptr<file> small_;
  object_ptr<file> big_;
  object_ptr<minithumbnail> minithumbnail_;
  bool has_animation_;

  chatPhotoInfo();

  chatPhotoInfo(object_ptr<file> &&small_, object_ptr<file> &&big_, object_ptr<minithumbnail> &&minithumbnail_, bool has_animation_);

  static const std::int32_t ID = 167058358;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatPhotos final : public Object {
 public:
  int32 total_count_;
  array<object_ptr<chatPhoto>> photos_;

  chatPhotos();

  chatPhotos(int32 total_count_, array<object_ptr<chatPhoto>> &&photos_);

  static const std::int32_t ID = -1510699180;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatPosition final : public Object {
 public:
  object_ptr<ChatList> list_;
  int64 order_;
  bool is_pinned_;
  object_ptr<ChatSource> source_;

  chatPosition();

  chatPosition(object_ptr<ChatList> &&list_, int64 order_, bool is_pinned_, object_ptr<ChatSource> &&source_);

  static const std::int32_t ID = -622557355;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChatReportReason: public Object {
 public:
};

class chatReportReasonSpam final : public ChatReportReason {
 public:

  chatReportReasonSpam();

  static const std::int32_t ID = -510848863;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatReportReasonViolence final : public ChatReportReason {
 public:

  chatReportReasonViolence();

  static const std::int32_t ID = -1330235395;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatReportReasonPornography final : public ChatReportReason {
 public:

  chatReportReasonPornography();

  static const std::int32_t ID = 722614385;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatReportReasonChildAbuse final : public ChatReportReason {
 public:

  chatReportReasonChildAbuse();

  static const std::int32_t ID = -1070686531;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatReportReasonCopyright final : public ChatReportReason {
 public:

  chatReportReasonCopyright();

  static const std::int32_t ID = 986898080;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatReportReasonUnrelatedLocation final : public ChatReportReason {
 public:

  chatReportReasonUnrelatedLocation();

  static const std::int32_t ID = 2632403;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatReportReasonFake final : public ChatReportReason {
 public:

  chatReportReasonFake();

  static const std::int32_t ID = -1713230446;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatReportReasonIllegalDrugs final : public ChatReportReason {
 public:

  chatReportReasonIllegalDrugs();

  static const std::int32_t ID = -844539307;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatReportReasonPersonalDetails final : public ChatReportReason {
 public:

  chatReportReasonPersonalDetails();

  static const std::int32_t ID = 1121159029;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatReportReasonCustom final : public ChatReportReason {
 public:

  chatReportReasonCustom();

  static const std::int32_t ID = 1288925974;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChatSource: public Object {
 public:
};

class chatSourceMtprotoProxy final : public ChatSource {
 public:

  chatSourceMtprotoProxy();

  static const std::int32_t ID = 394074115;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatSourcePublicServiceAnnouncement final : public ChatSource {
 public:
  string type_;
  string text_;

  chatSourcePublicServiceAnnouncement();

  chatSourcePublicServiceAnnouncement(string const &type_, string const &text_);

  static const std::int32_t ID = -328571244;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChatStatistics: public Object {
 public:
};

class chatStatisticsSupergroup final : public ChatStatistics {
 public:
  object_ptr<dateRange> period_;
  object_ptr<statisticalValue> member_count_;
  object_ptr<statisticalValue> message_count_;
  object_ptr<statisticalValue> viewer_count_;
  object_ptr<statisticalValue> sender_count_;
  object_ptr<StatisticalGraph> member_count_graph_;
  object_ptr<StatisticalGraph> join_graph_;
  object_ptr<StatisticalGraph> join_by_source_graph_;
  object_ptr<StatisticalGraph> language_graph_;
  object_ptr<StatisticalGraph> message_content_graph_;
  object_ptr<StatisticalGraph> action_graph_;
  object_ptr<StatisticalGraph> day_graph_;
  object_ptr<StatisticalGraph> week_graph_;
  array<object_ptr<chatStatisticsMessageSenderInfo>> top_senders_;
  array<object_ptr<chatStatisticsAdministratorActionsInfo>> top_administrators_;
  array<object_ptr<chatStatisticsInviterInfo>> top_inviters_;

  chatStatisticsSupergroup();

  chatStatisticsSupergroup(object_ptr<dateRange> &&period_, object_ptr<statisticalValue> &&member_count_, object_ptr<statisticalValue> &&message_count_, object_ptr<statisticalValue> &&viewer_count_, object_ptr<statisticalValue> &&sender_count_, object_ptr<StatisticalGraph> &&member_count_graph_, object_ptr<StatisticalGraph> &&join_graph_, object_ptr<StatisticalGraph> &&join_by_source_graph_, object_ptr<StatisticalGraph> &&language_graph_, object_ptr<StatisticalGraph> &&message_content_graph_, object_ptr<StatisticalGraph> &&action_graph_, object_ptr<StatisticalGraph> &&day_graph_, object_ptr<StatisticalGraph> &&week_graph_, array<object_ptr<chatStatisticsMessageSenderInfo>> &&top_senders_, array<object_ptr<chatStatisticsAdministratorActionsInfo>> &&top_administrators_, array<object_ptr<chatStatisticsInviterInfo>> &&top_inviters_);

  static const std::int32_t ID = -17244633;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatStatisticsChannel final : public ChatStatistics {
 public:
  object_ptr<dateRange> period_;
  object_ptr<statisticalValue> member_count_;
  object_ptr<statisticalValue> mean_view_count_;
  object_ptr<statisticalValue> mean_share_count_;
  double enabled_notifications_percentage_;
  object_ptr<StatisticalGraph> member_count_graph_;
  object_ptr<StatisticalGraph> join_graph_;
  object_ptr<StatisticalGraph> mute_graph_;
  object_ptr<StatisticalGraph> view_count_by_hour_graph_;
  object_ptr<StatisticalGraph> view_count_by_source_graph_;
  object_ptr<StatisticalGraph> join_by_source_graph_;
  object_ptr<StatisticalGraph> language_graph_;
  object_ptr<StatisticalGraph> message_interaction_graph_;
  object_ptr<StatisticalGraph> instant_view_interaction_graph_;
  array<object_ptr<chatStatisticsMessageInteractionInfo>> recent_message_interactions_;

  chatStatisticsChannel();

  chatStatisticsChannel(object_ptr<dateRange> &&period_, object_ptr<statisticalValue> &&member_count_, object_ptr<statisticalValue> &&mean_view_count_, object_ptr<statisticalValue> &&mean_share_count_, double enabled_notifications_percentage_, object_ptr<StatisticalGraph> &&member_count_graph_, object_ptr<StatisticalGraph> &&join_graph_, object_ptr<StatisticalGraph> &&mute_graph_, object_ptr<StatisticalGraph> &&view_count_by_hour_graph_, object_ptr<StatisticalGraph> &&view_count_by_source_graph_, object_ptr<StatisticalGraph> &&join_by_source_graph_, object_ptr<StatisticalGraph> &&language_graph_, object_ptr<StatisticalGraph> &&message_interaction_graph_, object_ptr<StatisticalGraph> &&instant_view_interaction_graph_, array<object_ptr<chatStatisticsMessageInteractionInfo>> &&recent_message_interactions_);

  static const std::int32_t ID = -825434183;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatStatisticsAdministratorActionsInfo final : public Object {
 public:
  int53 user_id_;
  int32 deleted_message_count_;
  int32 banned_user_count_;
  int32 restricted_user_count_;

  chatStatisticsAdministratorActionsInfo();

  chatStatisticsAdministratorActionsInfo(int53 user_id_, int32 deleted_message_count_, int32 banned_user_count_, int32 restricted_user_count_);

  static const std::int32_t ID = -406467202;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatStatisticsInviterInfo final : public Object {
 public:
  int53 user_id_;
  int32 added_member_count_;

  chatStatisticsInviterInfo();

  chatStatisticsInviterInfo(int53 user_id_, int32 added_member_count_);

  static const std::int32_t ID = 629396619;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatStatisticsMessageInteractionInfo final : public Object {
 public:
  int53 message_id_;
  int32 view_count_;
  int32 forward_count_;

  chatStatisticsMessageInteractionInfo();

  chatStatisticsMessageInteractionInfo(int53 message_id_, int32 view_count_, int32 forward_count_);

  static const std::int32_t ID = -765580756;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatStatisticsMessageSenderInfo final : public Object {
 public:
  int53 user_id_;
  int32 sent_message_count_;
  int32 average_character_count_;

  chatStatisticsMessageSenderInfo();

  chatStatisticsMessageSenderInfo(int53 user_id_, int32 sent_message_count_, int32 average_character_count_);

  static const std::int32_t ID = 1762295371;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatTheme final : public Object {
 public:
  string name_;
  object_ptr<themeSettings> light_settings_;
  object_ptr<themeSettings> dark_settings_;

  chatTheme();

  chatTheme(string const &name_, object_ptr<themeSettings> &&light_settings_, object_ptr<themeSettings> &&dark_settings_);

  static const std::int32_t ID = -113218503;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChatType: public Object {
 public:
};

class chatTypePrivate final : public ChatType {
 public:
  int53 user_id_;

  chatTypePrivate();

  explicit chatTypePrivate(int53 user_id_);

  static const std::int32_t ID = 1579049844;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatTypeBasicGroup final : public ChatType {
 public:
  int53 basic_group_id_;

  chatTypeBasicGroup();

  explicit chatTypeBasicGroup(int53 basic_group_id_);

  static const std::int32_t ID = 973884508;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatTypeSupergroup final : public ChatType {
 public:
  int53 supergroup_id_;
  bool is_channel_;

  chatTypeSupergroup();

  chatTypeSupergroup(int53 supergroup_id_, bool is_channel_);

  static const std::int32_t ID = -1472570774;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatTypeSecret final : public ChatType {
 public:
  int32 secret_chat_id_;
  int53 user_id_;

  chatTypeSecret();

  chatTypeSecret(int32 secret_chat_id_, int53 user_id_);

  static const std::int32_t ID = 862366513;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chats final : public Object {
 public:
  int32 total_count_;
  array<int53> chat_ids_;

  chats();

  chats(int32 total_count_, array<int53> &&chat_ids_);

  static const std::int32_t ID = 1809654812;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatsNearby final : public Object {
 public:
  array<object_ptr<chatNearby>> users_nearby_;
  array<object_ptr<chatNearby>> supergroups_nearby_;

  chatsNearby();

  chatsNearby(array<object_ptr<chatNearby>> &&users_nearby_, array<object_ptr<chatNearby>> &&supergroups_nearby_);

  static const std::int32_t ID = 187746081;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class CheckChatUsernameResult: public Object {
 public:
};

class checkChatUsernameResultOk final : public CheckChatUsernameResult {
 public:

  checkChatUsernameResultOk();

  static const std::int32_t ID = -1498956964;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkChatUsernameResultUsernameInvalid final : public CheckChatUsernameResult {
 public:

  checkChatUsernameResultUsernameInvalid();

  static const std::int32_t ID = -636979370;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkChatUsernameResultUsernameOccupied final : public CheckChatUsernameResult {
 public:

  checkChatUsernameResultUsernameOccupied();

  static const std::int32_t ID = 1320892201;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkChatUsernameResultPublicChatsTooMuch final : public CheckChatUsernameResult {
 public:

  checkChatUsernameResultPublicChatsTooMuch();

  static const std::int32_t ID = 858247741;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkChatUsernameResultPublicGroupsUnavailable final : public CheckChatUsernameResult {
 public:

  checkChatUsernameResultPublicGroupsUnavailable();

  static const std::int32_t ID = -51833641;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class CheckStickerSetNameResult: public Object {
 public:
};

class checkStickerSetNameResultOk final : public CheckStickerSetNameResult {
 public:

  checkStickerSetNameResultOk();

  static const std::int32_t ID = -1404308904;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkStickerSetNameResultNameInvalid final : public CheckStickerSetNameResult {
 public:

  checkStickerSetNameResultNameInvalid();

  static const std::int32_t ID = 177992244;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkStickerSetNameResultNameOccupied final : public CheckStickerSetNameResult {
 public:

  checkStickerSetNameResultNameOccupied();

  static const std::int32_t ID = 1012980872;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class closedVectorPath final : public Object {
 public:
  array<object_ptr<VectorPathCommand>> commands_;

  closedVectorPath();

  explicit closedVectorPath(array<object_ptr<VectorPathCommand>> &&commands_);

  static const std::int32_t ID = 589951657;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class connectedWebsite final : public Object {
 public:
  int64 id_;
  string domain_name_;
  int53 bot_user_id_;
  string browser_;
  string platform_;
  int32 log_in_date_;
  int32 last_active_date_;
  string ip_;
  string location_;

  connectedWebsite();

  connectedWebsite(int64 id_, string const &domain_name_, int53 bot_user_id_, string const &browser_, string const &platform_, int32 log_in_date_, int32 last_active_date_, string const &ip_, string const &location_);

  static const std::int32_t ID = 844014445;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class connectedWebsites final : public Object {
 public:
  array<object_ptr<connectedWebsite>> websites_;

  connectedWebsites();

  explicit connectedWebsites(array<object_ptr<connectedWebsite>> &&websites_);

  static const std::int32_t ID = -1727949694;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ConnectionState: public Object {
 public:
};

class connectionStateWaitingForNetwork final : public ConnectionState {
 public:

  connectionStateWaitingForNetwork();

  static const std::int32_t ID = 1695405912;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class connectionStateConnectingToProxy final : public ConnectionState {
 public:

  connectionStateConnectingToProxy();

  static const std::int32_t ID = -93187239;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class connectionStateConnecting final : public ConnectionState {
 public:

  connectionStateConnecting();

  static const std::int32_t ID = -1298400670;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class connectionStateUpdating final : public ConnectionState {
 public:

  connectionStateUpdating();

  static const std::int32_t ID = -188104009;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class connectionStateReady final : public ConnectionState {
 public:

  connectionStateReady();

  static const std::int32_t ID = 48608492;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class contact final : public Object {
 public:
  string phone_number_;
  string first_name_;
  string last_name_;
  string vcard_;
  int53 user_id_;

  contact();

  contact(string const &phone_number_, string const &first_name_, string const &last_name_, string const &vcard_, int53 user_id_);

  static const std::int32_t ID = -1993844876;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class count final : public Object {
 public:
  int32 count_;

  count();

  explicit count(int32 count_);

  static const std::int32_t ID = 1295577348;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class countries final : public Object {
 public:
  array<object_ptr<countryInfo>> countries_;

  countries();

  explicit countries(array<object_ptr<countryInfo>> &&countries_);

  static const std::int32_t ID = 1854211813;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class countryInfo final : public Object {
 public:
  string country_code_;
  string name_;
  string english_name_;
  bool is_hidden_;
  array<string> calling_codes_;

  countryInfo();

  countryInfo(string const &country_code_, string const &name_, string const &english_name_, bool is_hidden_, array<string> &&calling_codes_);

  static const std::int32_t ID = 1617195722;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class customRequestResult final : public Object {
 public:
  string result_;

  customRequestResult();

  explicit customRequestResult(string const &result_);

  static const std::int32_t ID = -2009960452;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class databaseStatistics final : public Object {
 public:
  string statistics_;

  databaseStatistics();

  explicit databaseStatistics(string const &statistics_);

  static const std::int32_t ID = -1123912880;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class date final : public Object {
 public:
  int32 day_;
  int32 month_;
  int32 year_;

  date();

  date(int32 day_, int32 month_, int32 year_);

  static const std::int32_t ID = -277956960;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class dateRange final : public Object {
 public:
  int32 start_date_;
  int32 end_date_;

  dateRange();

  dateRange(int32 start_date_, int32 end_date_);

  static const std::int32_t ID = 1360333926;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class datedFile final : public Object {
 public:
  object_ptr<file> file_;
  int32 date_;

  datedFile();

  datedFile(object_ptr<file> &&file_, int32 date_);

  static const std::int32_t ID = -1840795491;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deepLinkInfo final : public Object {
 public:
  object_ptr<formattedText> text_;
  bool need_update_application_;

  deepLinkInfo();

  deepLinkInfo(object_ptr<formattedText> &&text_, bool need_update_application_);

  static const std::int32_t ID = 1864081662;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class DeviceToken: public Object {
 public:
};

class deviceTokenFirebaseCloudMessaging final : public DeviceToken {
 public:
  string token_;
  bool encrypt_;

  deviceTokenFirebaseCloudMessaging();

  deviceTokenFirebaseCloudMessaging(string const &token_, bool encrypt_);

  static const std::int32_t ID = -797881849;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deviceTokenApplePush final : public DeviceToken {
 public:
  string device_token_;
  bool is_app_sandbox_;

  deviceTokenApplePush();

  deviceTokenApplePush(string const &device_token_, bool is_app_sandbox_);

  static const std::int32_t ID = 387541955;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deviceTokenApplePushVoIP final : public DeviceToken {
 public:
  string device_token_;
  bool is_app_sandbox_;
  bool encrypt_;

  deviceTokenApplePushVoIP();

  deviceTokenApplePushVoIP(string const &device_token_, bool is_app_sandbox_, bool encrypt_);

  static const std::int32_t ID = 804275689;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deviceTokenWindowsPush final : public DeviceToken {
 public:
  string access_token_;

  deviceTokenWindowsPush();

  explicit deviceTokenWindowsPush(string const &access_token_);

  static const std::int32_t ID = -1410514289;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deviceTokenMicrosoftPush final : public DeviceToken {
 public:
  string channel_uri_;

  deviceTokenMicrosoftPush();

  explicit deviceTokenMicrosoftPush(string const &channel_uri_);

  static const std::int32_t ID = 1224269900;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deviceTokenMicrosoftPushVoIP final : public DeviceToken {
 public:
  string channel_uri_;

  deviceTokenMicrosoftPushVoIP();

  explicit deviceTokenMicrosoftPushVoIP(string const &channel_uri_);

  static const std::int32_t ID = -785603759;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deviceTokenWebPush final : public DeviceToken {
 public:
  string endpoint_;
  string p256dh_base64url_;
  string auth_base64url_;

  deviceTokenWebPush();

  deviceTokenWebPush(string const &endpoint_, string const &p256dh_base64url_, string const &auth_base64url_);

  static const std::int32_t ID = -1694507273;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deviceTokenSimplePush final : public DeviceToken {
 public:
  string endpoint_;

  deviceTokenSimplePush();

  explicit deviceTokenSimplePush(string const &endpoint_);

  static const std::int32_t ID = 49584736;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deviceTokenUbuntuPush final : public DeviceToken {
 public:
  string token_;

  deviceTokenUbuntuPush();

  explicit deviceTokenUbuntuPush(string const &token_);

  static const std::int32_t ID = 1782320422;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deviceTokenBlackBerryPush final : public DeviceToken {
 public:
  string token_;

  deviceTokenBlackBerryPush();

  explicit deviceTokenBlackBerryPush(string const &token_);

  static const std::int32_t ID = 1559167234;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deviceTokenTizenPush final : public DeviceToken {
 public:
  string reg_id_;

  deviceTokenTizenPush();

  explicit deviceTokenTizenPush(string const &reg_id_);

  static const std::int32_t ID = -1359947213;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class DiceStickers: public Object {
 public:
};

class diceStickersRegular final : public DiceStickers {
 public:
  object_ptr<sticker> sticker_;

  diceStickersRegular();

  explicit diceStickersRegular(object_ptr<sticker> &&sticker_);

  static const std::int32_t ID = -740299570;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class diceStickersSlotMachine final : public DiceStickers {
 public:
  object_ptr<sticker> background_;
  object_ptr<sticker> lever_;
  object_ptr<sticker> left_reel_;
  object_ptr<sticker> center_reel_;
  object_ptr<sticker> right_reel_;

  diceStickersSlotMachine();

  diceStickersSlotMachine(object_ptr<sticker> &&background_, object_ptr<sticker> &&lever_, object_ptr<sticker> &&left_reel_, object_ptr<sticker> &&center_reel_, object_ptr<sticker> &&right_reel_);

  static const std::int32_t ID = -375223124;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class document final : public Object {
 public:
  string file_name_;
  string mime_type_;
  object_ptr<minithumbnail> minithumbnail_;
  object_ptr<thumbnail> thumbnail_;
  object_ptr<file> document_;

  document();

  document(string const &file_name_, string const &mime_type_, object_ptr<minithumbnail> &&minithumbnail_, object_ptr<thumbnail> &&thumbnail_, object_ptr<file> &&document_);

  static const std::int32_t ID = -1357271080;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class downloadedFileCounts final : public Object {
 public:
  int32 active_count_;
  int32 paused_count_;
  int32 completed_count_;

  downloadedFileCounts();

  downloadedFileCounts(int32 active_count_, int32 paused_count_, int32 completed_count_);

  static const std::int32_t ID = -1973999550;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class draftMessage final : public Object {
 public:
  int53 reply_to_message_id_;
  int32 date_;
  object_ptr<InputMessageContent> input_message_text_;

  draftMessage();

  draftMessage(int53 reply_to_message_id_, int32 date_, object_ptr<InputMessageContent> &&input_message_text_);

  static const std::int32_t ID = 1373050112;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class EmailAddressAuthentication: public Object {
 public:
};

class emailAddressAuthenticationCode final : public EmailAddressAuthentication {
 public:
  string code_;

  emailAddressAuthenticationCode();

  explicit emailAddressAuthenticationCode(string const &code_);

  static const std::int32_t ID = -993257022;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class emailAddressAuthenticationAppleId final : public EmailAddressAuthentication {
 public:
  string token_;

  emailAddressAuthenticationAppleId();

  explicit emailAddressAuthenticationAppleId(string const &token_);

  static const std::int32_t ID = 633948265;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class emailAddressAuthenticationGoogleId final : public EmailAddressAuthentication {
 public:
  string token_;

  emailAddressAuthenticationGoogleId();

  explicit emailAddressAuthenticationGoogleId(string const &token_);

  static const std::int32_t ID = -19142846;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class emailAddressAuthenticationCodeInfo final : public Object {
 public:
  string email_address_pattern_;
  int32 length_;

  emailAddressAuthenticationCodeInfo();

  emailAddressAuthenticationCodeInfo(string const &email_address_pattern_, int32 length_);

  static const std::int32_t ID = 1151066659;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class emojiReaction final : public Object {
 public:
  string emoji_;
  string title_;
  bool is_active_;
  object_ptr<sticker> static_icon_;
  object_ptr<sticker> appear_animation_;
  object_ptr<sticker> select_animation_;
  object_ptr<sticker> activate_animation_;
  object_ptr<sticker> effect_animation_;
  object_ptr<sticker> around_animation_;
  object_ptr<sticker> center_animation_;

  emojiReaction();

  emojiReaction(string const &emoji_, string const &title_, bool is_active_, object_ptr<sticker> &&static_icon_, object_ptr<sticker> &&appear_animation_, object_ptr<sticker> &&select_animation_, object_ptr<sticker> &&activate_animation_, object_ptr<sticker> &&effect_animation_, object_ptr<sticker> &&around_animation_, object_ptr<sticker> &&center_animation_);

  static const std::int32_t ID = 1616063583;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class emojiStatus final : public Object {
 public:
  int64 custom_emoji_id_;

  emojiStatus();

  explicit emojiStatus(int64 custom_emoji_id_);

  static const std::int32_t ID = 1092133478;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class emojiStatuses final : public Object {
 public:
  array<object_ptr<emojiStatus>> emoji_statuses_;

  emojiStatuses();

  explicit emojiStatuses(array<object_ptr<emojiStatus>> &&emoji_statuses_);

  static const std::int32_t ID = 1186104146;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class emojis final : public Object {
 public:
  array<string> emojis_;

  emojis();

  explicit emojis(array<string> &&emojis_);

  static const std::int32_t ID = 950339552;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class encryptedCredentials final : public Object {
 public:
  bytes data_;
  bytes hash_;
  bytes secret_;

  encryptedCredentials();

  encryptedCredentials(bytes const &data_, bytes const &hash_, bytes const &secret_);

  static const std::int32_t ID = 1331106766;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class encryptedPassportElement final : public Object {
 public:
  object_ptr<PassportElementType> type_;
  bytes data_;
  object_ptr<datedFile> front_side_;
  object_ptr<datedFile> reverse_side_;
  object_ptr<datedFile> selfie_;
  array<object_ptr<datedFile>> translation_;
  array<object_ptr<datedFile>> files_;
  string value_;
  string hash_;

  encryptedPassportElement();

  encryptedPassportElement(object_ptr<PassportElementType> &&type_, bytes const &data_, object_ptr<datedFile> &&front_side_, object_ptr<datedFile> &&reverse_side_, object_ptr<datedFile> &&selfie_, array<object_ptr<datedFile>> &&translation_, array<object_ptr<datedFile>> &&files_, string const &value_, string const &hash_);

  static const std::int32_t ID = 2002386193;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class error final : public Object {
 public:
  int32 code_;
  string message_;

  error();

  error(int32 code_, string const &message_);

  static const std::int32_t ID = -1679978726;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class file final : public Object {
 public:
  int32 id_;
  int53 size_;
  int53 expected_size_;
  object_ptr<localFile> local_;
  object_ptr<remoteFile> remote_;

  file();

  file(int32 id_, int53 size_, int53 expected_size_, object_ptr<localFile> &&local_, object_ptr<remoteFile> &&remote_);

  static const std::int32_t ID = 1263291956;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class fileDownload final : public Object {
 public:
  int32 file_id_;
  object_ptr<message> message_;
  int32 add_date_;
  int32 complete_date_;
  bool is_paused_;

  fileDownload();

  fileDownload(int32 file_id_, object_ptr<message> &&message_, int32 add_date_, int32 complete_date_, bool is_paused_);

  static const std::int32_t ID = -2092100780;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class fileDownloadedPrefixSize final : public Object {
 public:
  int53 size_;

  fileDownloadedPrefixSize();

  explicit fileDownloadedPrefixSize(int53 size_);

  static const std::int32_t ID = -2015205381;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class filePart final : public Object {
 public:
  bytes data_;

  filePart();

  explicit filePart(bytes const &data_);

  static const std::int32_t ID = 911821878;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class FileType: public Object {
 public:
};

class fileTypeNone final : public FileType {
 public:

  fileTypeNone();

  static const std::int32_t ID = 2003009189;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class fileTypeAnimation final : public FileType {
 public:

  fileTypeAnimation();

  static const std::int32_t ID = -290816582;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class fileTypeAudio final : public FileType {
 public:

  fileTypeAudio();

  static const std::int32_t ID = -709112160;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class fileTypeDocument final : public FileType {
 public:

  fileTypeDocument();

  static const std::int32_t ID = -564722929;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class fileTypeNotificationSound final : public FileType {
 public:

  fileTypeNotificationSound();

  static const std::int32_t ID = -1020289271;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class fileTypePhoto final : public FileType {
 public:

  fileTypePhoto();

  static const std::int32_t ID = -1718914651;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class fileTypeProfilePhoto final : public FileType {
 public:

  fileTypeProfilePhoto();

  static const std::int32_t ID = 1795089315;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class fileTypeSecret final : public FileType {
 public:

  fileTypeSecret();

  static const std::int32_t ID = -1871899401;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class fileTypeSecretThumbnail final : public FileType {
 public:

  fileTypeSecretThumbnail();

  static const std::int32_t ID = -1401326026;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class fileTypeSecure final : public FileType {
 public:

  fileTypeSecure();

  static const std::int32_t ID = -1419133146;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class fileTypeSticker final : public FileType {
 public:

  fileTypeSticker();

  static const std::int32_t ID = 475233385;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class fileTypeThumbnail final : public FileType {
 public:

  fileTypeThumbnail();

  static const std::int32_t ID = -12443298;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class fileTypeUnknown final : public FileType {
 public:

  fileTypeUnknown();

  static const std::int32_t ID = -2011566768;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class fileTypeVideo final : public FileType {
 public:

  fileTypeVideo();

  static const std::int32_t ID = 1430816539;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class fileTypeVideoNote final : public FileType {
 public:

  fileTypeVideoNote();

  static const std::int32_t ID = -518412385;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class fileTypeVoiceNote final : public FileType {
 public:

  fileTypeVoiceNote();

  static const std::int32_t ID = -588681661;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class fileTypeWallpaper final : public FileType {
 public:

  fileTypeWallpaper();

  static const std::int32_t ID = 1854930076;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class formattedText final : public Object {
 public:
  string text_;
  array<object_ptr<textEntity>> entities_;

  formattedText();

  formattedText(string const &text_, array<object_ptr<textEntity>> &&entities_);

  static const std::int32_t ID = -252624564;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class forumTopic final : public Object {
 public:
  object_ptr<forumTopicInfo> info_;
  object_ptr<message> last_message_;
  bool is_pinned_;
  int32 unread_count_;
  int53 last_read_inbox_message_id_;
  int53 last_read_outbox_message_id_;
  int32 unread_mention_count_;
  int32 unread_reaction_count_;
  object_ptr<chatNotificationSettings> notification_settings_;
  object_ptr<draftMessage> draft_message_;

  forumTopic();

  forumTopic(object_ptr<forumTopicInfo> &&info_, object_ptr<message> &&last_message_, bool is_pinned_, int32 unread_count_, int53 last_read_inbox_message_id_, int53 last_read_outbox_message_id_, int32 unread_mention_count_, int32 unread_reaction_count_, object_ptr<chatNotificationSettings> &&notification_settings_, object_ptr<draftMessage> &&draft_message_);

  static const std::int32_t ID = 303279334;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class forumTopicIcon final : public Object {
 public:
  int32 color_;
  int64 custom_emoji_id_;

  forumTopicIcon();

  forumTopicIcon(int32 color_, int64 custom_emoji_id_);

  static const std::int32_t ID = -818765421;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class forumTopicInfo final : public Object {
 public:
  int53 message_thread_id_;
  string name_;
  object_ptr<forumTopicIcon> icon_;
  int32 creation_date_;
  object_ptr<MessageSender> creator_id_;
  bool is_outgoing_;
  bool is_closed_;

  forumTopicInfo();

  forumTopicInfo(int53 message_thread_id_, string const &name_, object_ptr<forumTopicIcon> &&icon_, int32 creation_date_, object_ptr<MessageSender> &&creator_id_, bool is_outgoing_, bool is_closed_);

  static const std::int32_t ID = -745571750;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class foundFileDownloads final : public Object {
 public:
  object_ptr<downloadedFileCounts> total_counts_;
  array<object_ptr<fileDownload>> files_;
  string next_offset_;

  foundFileDownloads();

  foundFileDownloads(object_ptr<downloadedFileCounts> &&total_counts_, array<object_ptr<fileDownload>> &&files_, string const &next_offset_);

  static const std::int32_t ID = 1395890392;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class foundMessages final : public Object {
 public:
  int32 total_count_;
  array<object_ptr<message>> messages_;
  string next_offset_;

  foundMessages();

  foundMessages(int32 total_count_, array<object_ptr<message>> &&messages_, string const &next_offset_);

  static const std::int32_t ID = -529809608;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class game final : public Object {
 public:
  int64 id_;
  string short_name_;
  string title_;
  object_ptr<formattedText> text_;
  string description_;
  object_ptr<photo> photo_;
  object_ptr<animation> animation_;

  game();

  game(int64 id_, string const &short_name_, string const &title_, object_ptr<formattedText> &&text_, string const &description_, object_ptr<photo> &&photo_, object_ptr<animation> &&animation_);

  static const std::int32_t ID = -1565597752;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class gameHighScore final : public Object {
 public:
  int32 position_;
  int53 user_id_;
  int32 score_;

  gameHighScore();

  gameHighScore(int32 position_, int53 user_id_, int32 score_);

  static const std::int32_t ID = 342871838;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class gameHighScores final : public Object {
 public:
  array<object_ptr<gameHighScore>> scores_;

  gameHighScores();

  explicit gameHighScores(array<object_ptr<gameHighScore>> &&scores_);

  static const std::int32_t ID = -725770727;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class groupCall final : public Object {
 public:
  int32 id_;
  string title_;
  int32 scheduled_start_date_;
  bool enabled_start_notification_;
  bool is_active_;
  bool is_rtmp_stream_;
  bool is_joined_;
  bool need_rejoin_;
  bool can_be_managed_;
  int32 participant_count_;
  bool has_hidden_listeners_;
  bool loaded_all_participants_;
  array<object_ptr<groupCallRecentSpeaker>> recent_speakers_;
  bool is_my_video_enabled_;
  bool is_my_video_paused_;
  bool can_enable_video_;
  bool mute_new_participants_;
  bool can_toggle_mute_new_participants_;
  int32 record_duration_;
  bool is_video_recorded_;
  int32 duration_;

  groupCall();

  groupCall(int32 id_, string const &title_, int32 scheduled_start_date_, bool enabled_start_notification_, bool is_active_, bool is_rtmp_stream_, bool is_joined_, bool need_rejoin_, bool can_be_managed_, int32 participant_count_, bool has_hidden_listeners_, bool loaded_all_participants_, array<object_ptr<groupCallRecentSpeaker>> &&recent_speakers_, bool is_my_video_enabled_, bool is_my_video_paused_, bool can_enable_video_, bool mute_new_participants_, bool can_toggle_mute_new_participants_, int32 record_duration_, bool is_video_recorded_, int32 duration_);

  static const std::int32_t ID = -123443355;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class groupCallId final : public Object {
 public:
  int32 id_;

  groupCallId();

  explicit groupCallId(int32 id_);

  static const std::int32_t ID = 350534469;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class groupCallParticipant final : public Object {
 public:
  object_ptr<MessageSender> participant_id_;
  int32 audio_source_id_;
  int32 screen_sharing_audio_source_id_;
  object_ptr<groupCallParticipantVideoInfo> video_info_;
  object_ptr<groupCallParticipantVideoInfo> screen_sharing_video_info_;
  string bio_;
  bool is_current_user_;
  bool is_speaking_;
  bool is_hand_raised_;
  bool can_be_muted_for_all_users_;
  bool can_be_unmuted_for_all_users_;
  bool can_be_muted_for_current_user_;
  bool can_be_unmuted_for_current_user_;
  bool is_muted_for_all_users_;
  bool is_muted_for_current_user_;
  bool can_unmute_self_;
  int32 volume_level_;
  string order_;

  groupCallParticipant();

  groupCallParticipant(object_ptr<MessageSender> &&participant_id_, int32 audio_source_id_, int32 screen_sharing_audio_source_id_, object_ptr<groupCallParticipantVideoInfo> &&video_info_, object_ptr<groupCallParticipantVideoInfo> &&screen_sharing_video_info_, string const &bio_, bool is_current_user_, bool is_speaking_, bool is_hand_raised_, bool can_be_muted_for_all_users_, bool can_be_unmuted_for_all_users_, bool can_be_muted_for_current_user_, bool can_be_unmuted_for_current_user_, bool is_muted_for_all_users_, bool is_muted_for_current_user_, bool can_unmute_self_, int32 volume_level_, string const &order_);

  static const std::int32_t ID = 2059182571;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class groupCallParticipantVideoInfo final : public Object {
 public:
  array<object_ptr<groupCallVideoSourceGroup>> source_groups_;
  string endpoint_id_;
  bool is_paused_;

  groupCallParticipantVideoInfo();

  groupCallParticipantVideoInfo(array<object_ptr<groupCallVideoSourceGroup>> &&source_groups_, string const &endpoint_id_, bool is_paused_);

  static const std::int32_t ID = -14294645;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class groupCallRecentSpeaker final : public Object {
 public:
  object_ptr<MessageSender> participant_id_;
  bool is_speaking_;

  groupCallRecentSpeaker();

  groupCallRecentSpeaker(object_ptr<MessageSender> &&participant_id_, bool is_speaking_);

  static const std::int32_t ID = 1819519436;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class groupCallStream final : public Object {
 public:
  int32 channel_id_;
  int32 scale_;
  int53 time_offset_;

  groupCallStream();

  groupCallStream(int32 channel_id_, int32 scale_, int53 time_offset_);

  static const std::int32_t ID = -264564795;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class groupCallStreams final : public Object {
 public:
  array<object_ptr<groupCallStream>> streams_;

  groupCallStreams();

  explicit groupCallStreams(array<object_ptr<groupCallStream>> &&streams_);

  static const std::int32_t ID = -1032959578;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class GroupCallVideoQuality: public Object {
 public:
};

class groupCallVideoQualityThumbnail final : public GroupCallVideoQuality {
 public:

  groupCallVideoQualityThumbnail();

  static const std::int32_t ID = -379186304;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class groupCallVideoQualityMedium final : public GroupCallVideoQuality {
 public:

  groupCallVideoQualityMedium();

  static const std::int32_t ID = 394968234;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class groupCallVideoQualityFull final : public GroupCallVideoQuality {
 public:

  groupCallVideoQualityFull();

  static const std::int32_t ID = -2125916617;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class groupCallVideoSourceGroup final : public Object {
 public:
  string semantics_;
  array<int32> source_ids_;

  groupCallVideoSourceGroup();

  groupCallVideoSourceGroup(string const &semantics_, array<int32> &&source_ids_);

  static const std::int32_t ID = -1190900785;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class hashtags final : public Object {
 public:
  array<string> hashtags_;

  hashtags();

  explicit hashtags(array<string> &&hashtags_);

  static const std::int32_t ID = 676798885;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class httpUrl final : public Object {
 public:
  string url_;

  httpUrl();

  explicit httpUrl(string const &url_);

  static const std::int32_t ID = -2018019930;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class identityDocument final : public Object {
 public:
  string number_;
  object_ptr<date> expiry_date_;
  object_ptr<datedFile> front_side_;
  object_ptr<datedFile> reverse_side_;
  object_ptr<datedFile> selfie_;
  array<object_ptr<datedFile>> translation_;

  identityDocument();

  identityDocument(string const &number_, object_ptr<date> &&expiry_date_, object_ptr<datedFile> &&front_side_, object_ptr<datedFile> &&reverse_side_, object_ptr<datedFile> &&selfie_, array<object_ptr<datedFile>> &&translation_);

  static const std::int32_t ID = 445952972;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class importedContacts final : public Object {
 public:
  array<int53> user_ids_;
  array<int32> importer_count_;

  importedContacts();

  importedContacts(array<int53> &&user_ids_, array<int32> &&importer_count_);

  static const std::int32_t ID = 2068432290;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineKeyboardButton final : public Object {
 public:
  string text_;
  object_ptr<InlineKeyboardButtonType> type_;

  inlineKeyboardButton();

  inlineKeyboardButton(string const &text_, object_ptr<InlineKeyboardButtonType> &&type_);

  static const std::int32_t ID = -372105704;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InlineKeyboardButtonType: public Object {
 public:
};

class inlineKeyboardButtonTypeUrl final : public InlineKeyboardButtonType {
 public:
  string url_;

  inlineKeyboardButtonTypeUrl();

  explicit inlineKeyboardButtonTypeUrl(string const &url_);

  static const std::int32_t ID = 1130741420;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineKeyboardButtonTypeLoginUrl final : public InlineKeyboardButtonType {
 public:
  string url_;
  int53 id_;
  string forward_text_;

  inlineKeyboardButtonTypeLoginUrl();

  inlineKeyboardButtonTypeLoginUrl(string const &url_, int53 id_, string const &forward_text_);

  static const std::int32_t ID = -1203413081;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineKeyboardButtonTypeWebApp final : public InlineKeyboardButtonType {
 public:
  string url_;

  inlineKeyboardButtonTypeWebApp();

  explicit inlineKeyboardButtonTypeWebApp(string const &url_);

  static const std::int32_t ID = -1767471672;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineKeyboardButtonTypeCallback final : public InlineKeyboardButtonType {
 public:
  bytes data_;

  inlineKeyboardButtonTypeCallback();

  explicit inlineKeyboardButtonTypeCallback(bytes const &data_);

  static const std::int32_t ID = -1127515139;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineKeyboardButtonTypeCallbackWithPassword final : public InlineKeyboardButtonType {
 public:
  bytes data_;

  inlineKeyboardButtonTypeCallbackWithPassword();

  explicit inlineKeyboardButtonTypeCallbackWithPassword(bytes const &data_);

  static const std::int32_t ID = 908018248;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineKeyboardButtonTypeCallbackGame final : public InlineKeyboardButtonType {
 public:

  inlineKeyboardButtonTypeCallbackGame();

  static const std::int32_t ID = -383429528;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineKeyboardButtonTypeSwitchInline final : public InlineKeyboardButtonType {
 public:
  string query_;
  bool in_current_chat_;

  inlineKeyboardButtonTypeSwitchInline();

  inlineKeyboardButtonTypeSwitchInline(string const &query_, bool in_current_chat_);

  static const std::int32_t ID = -2035563307;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineKeyboardButtonTypeBuy final : public InlineKeyboardButtonType {
 public:

  inlineKeyboardButtonTypeBuy();

  static const std::int32_t ID = 1360739440;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineKeyboardButtonTypeUser final : public InlineKeyboardButtonType {
 public:
  int53 user_id_;

  inlineKeyboardButtonTypeUser();

  explicit inlineKeyboardButtonTypeUser(int53 user_id_);

  static const std::int32_t ID = 1836574114;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InlineQueryResult: public Object {
 public:
};

class inlineQueryResultArticle final : public InlineQueryResult {
 public:
  string id_;
  string url_;
  bool hide_url_;
  string title_;
  string description_;
  object_ptr<thumbnail> thumbnail_;

  inlineQueryResultArticle();

  inlineQueryResultArticle(string const &id_, string const &url_, bool hide_url_, string const &title_, string const &description_, object_ptr<thumbnail> &&thumbnail_);

  static const std::int32_t ID = 206340825;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineQueryResultContact final : public InlineQueryResult {
 public:
  string id_;
  object_ptr<contact> contact_;
  object_ptr<thumbnail> thumbnail_;

  inlineQueryResultContact();

  inlineQueryResultContact(string const &id_, object_ptr<contact> &&contact_, object_ptr<thumbnail> &&thumbnail_);

  static const std::int32_t ID = -181960174;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineQueryResultLocation final : public InlineQueryResult {
 public:
  string id_;
  object_ptr<location> location_;
  string title_;
  object_ptr<thumbnail> thumbnail_;

  inlineQueryResultLocation();

  inlineQueryResultLocation(string const &id_, object_ptr<location> &&location_, string const &title_, object_ptr<thumbnail> &&thumbnail_);

  static const std::int32_t ID = 466004752;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineQueryResultVenue final : public InlineQueryResult {
 public:
  string id_;
  object_ptr<venue> venue_;
  object_ptr<thumbnail> thumbnail_;

  inlineQueryResultVenue();

  inlineQueryResultVenue(string const &id_, object_ptr<venue> &&venue_, object_ptr<thumbnail> &&thumbnail_);

  static const std::int32_t ID = 1281036382;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineQueryResultGame final : public InlineQueryResult {
 public:
  string id_;
  object_ptr<game> game_;

  inlineQueryResultGame();

  inlineQueryResultGame(string const &id_, object_ptr<game> &&game_);

  static const std::int32_t ID = 1706916987;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineQueryResultAnimation final : public InlineQueryResult {
 public:
  string id_;
  object_ptr<animation> animation_;
  string title_;

  inlineQueryResultAnimation();

  inlineQueryResultAnimation(string const &id_, object_ptr<animation> &&animation_, string const &title_);

  static const std::int32_t ID = 2009984267;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineQueryResultAudio final : public InlineQueryResult {
 public:
  string id_;
  object_ptr<audio> audio_;

  inlineQueryResultAudio();

  inlineQueryResultAudio(string const &id_, object_ptr<audio> &&audio_);

  static const std::int32_t ID = 842650360;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineQueryResultDocument final : public InlineQueryResult {
 public:
  string id_;
  object_ptr<document> document_;
  string title_;
  string description_;

  inlineQueryResultDocument();

  inlineQueryResultDocument(string const &id_, object_ptr<document> &&document_, string const &title_, string const &description_);

  static const std::int32_t ID = -1491268539;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineQueryResultPhoto final : public InlineQueryResult {
 public:
  string id_;
  object_ptr<photo> photo_;
  string title_;
  string description_;

  inlineQueryResultPhoto();

  inlineQueryResultPhoto(string const &id_, object_ptr<photo> &&photo_, string const &title_, string const &description_);

  static const std::int32_t ID = 1848319440;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineQueryResultSticker final : public InlineQueryResult {
 public:
  string id_;
  object_ptr<sticker> sticker_;

  inlineQueryResultSticker();

  inlineQueryResultSticker(string const &id_, object_ptr<sticker> &&sticker_);

  static const std::int32_t ID = -1848224245;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineQueryResultVideo final : public InlineQueryResult {
 public:
  string id_;
  object_ptr<video> video_;
  string title_;
  string description_;

  inlineQueryResultVideo();

  inlineQueryResultVideo(string const &id_, object_ptr<video> &&video_, string const &title_, string const &description_);

  static const std::int32_t ID = -1373158683;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineQueryResultVoiceNote final : public InlineQueryResult {
 public:
  string id_;
  object_ptr<voiceNote> voice_note_;
  string title_;

  inlineQueryResultVoiceNote();

  inlineQueryResultVoiceNote(string const &id_, object_ptr<voiceNote> &&voice_note_, string const &title_);

  static const std::int32_t ID = -1897393105;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineQueryResults final : public Object {
 public:
  int64 inline_query_id_;
  string next_offset_;
  array<object_ptr<InlineQueryResult>> results_;
  string switch_pm_text_;
  string switch_pm_parameter_;

  inlineQueryResults();

  inlineQueryResults(int64 inline_query_id_, string const &next_offset_, array<object_ptr<InlineQueryResult>> &&results_, string const &switch_pm_text_, string const &switch_pm_parameter_);

  static const std::int32_t ID = 1000709656;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputBackground: public Object {
 public:
};

class inputBackgroundLocal final : public InputBackground {
 public:
  object_ptr<InputFile> background_;

  inputBackgroundLocal();

  explicit inputBackgroundLocal(object_ptr<InputFile> &&background_);

  static const std::int32_t ID = -1747094364;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputBackgroundRemote final : public InputBackground {
 public:
  int64 background_id_;

  inputBackgroundRemote();

  explicit inputBackgroundRemote(int64 background_id_);

  static const std::int32_t ID = -274976231;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputChatPhoto: public Object {
 public:
};

class inputChatPhotoPrevious final : public InputChatPhoto {
 public:
  int64 chat_photo_id_;

  inputChatPhotoPrevious();

  explicit inputChatPhotoPrevious(int64 chat_photo_id_);

  static const std::int32_t ID = 23128529;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputChatPhotoStatic final : public InputChatPhoto {
 public:
  object_ptr<InputFile> photo_;

  inputChatPhotoStatic();

  explicit inputChatPhotoStatic(object_ptr<InputFile> &&photo_);

  static const std::int32_t ID = 1979179699;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputChatPhotoAnimation final : public InputChatPhoto {
 public:
  object_ptr<InputFile> animation_;
  double main_frame_timestamp_;

  inputChatPhotoAnimation();

  inputChatPhotoAnimation(object_ptr<InputFile> &&animation_, double main_frame_timestamp_);

  static const std::int32_t ID = 90846242;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputCredentials: public Object {
 public:
};

class inputCredentialsSaved final : public InputCredentials {
 public:
  string saved_credentials_id_;

  inputCredentialsSaved();

  explicit inputCredentialsSaved(string const &saved_credentials_id_);

  static const std::int32_t ID = -2034385364;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputCredentialsNew final : public InputCredentials {
 public:
  string data_;
  bool allow_save_;

  inputCredentialsNew();

  inputCredentialsNew(string const &data_, bool allow_save_);

  static const std::int32_t ID = -829689558;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputCredentialsApplePay final : public InputCredentials {
 public:
  string data_;

  inputCredentialsApplePay();

  explicit inputCredentialsApplePay(string const &data_);

  static const std::int32_t ID = -1246570799;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputCredentialsGooglePay final : public InputCredentials {
 public:
  string data_;

  inputCredentialsGooglePay();

  explicit inputCredentialsGooglePay(string const &data_);

  static const std::int32_t ID = 844384100;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputFile: public Object {
 public:
};

class inputFileId final : public InputFile {
 public:
  int32 id_;

  inputFileId();

  explicit inputFileId(int32 id_);

  static const std::int32_t ID = 1788906253;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputFileRemote final : public InputFile {
 public:
  string id_;

  inputFileRemote();

  explicit inputFileRemote(string const &id_);

  static const std::int32_t ID = -107574466;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputFileLocal final : public InputFile {
 public:
  string path_;

  inputFileLocal();

  explicit inputFileLocal(string const &path_);

  static const std::int32_t ID = 2056030919;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputFileGenerated final : public InputFile {
 public:
  string original_path_;
  string conversion_;
  int53 expected_size_;

  inputFileGenerated();

  inputFileGenerated(string const &original_path_, string const &conversion_, int53 expected_size_);

  static const std::int32_t ID = -1333385216;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputIdentityDocument final : public Object {
 public:
  string number_;
  object_ptr<date> expiry_date_;
  object_ptr<InputFile> front_side_;
  object_ptr<InputFile> reverse_side_;
  object_ptr<InputFile> selfie_;
  array<object_ptr<InputFile>> translation_;

  inputIdentityDocument();

  inputIdentityDocument(string const &number_, object_ptr<date> &&expiry_date_, object_ptr<InputFile> &&front_side_, object_ptr<InputFile> &&reverse_side_, object_ptr<InputFile> &&selfie_, array<object_ptr<InputFile>> &&translation_);

  static const std::int32_t ID = -381776063;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputInlineQueryResult: public Object {
 public:
};

class inputInlineQueryResultAnimation final : public InputInlineQueryResult {
 public:
  string id_;
  string title_;
  string thumbnail_url_;
  string thumbnail_mime_type_;
  string video_url_;
  string video_mime_type_;
  int32 video_duration_;
  int32 video_width_;
  int32 video_height_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<InputMessageContent> input_message_content_;

  inputInlineQueryResultAnimation();

  inputInlineQueryResultAnimation(string const &id_, string const &title_, string const &thumbnail_url_, string const &thumbnail_mime_type_, string const &video_url_, string const &video_mime_type_, int32 video_duration_, int32 video_width_, int32 video_height_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_);

  static const std::int32_t ID = -1489808874;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputInlineQueryResultArticle final : public InputInlineQueryResult {
 public:
  string id_;
  string url_;
  bool hide_url_;
  string title_;
  string description_;
  string thumbnail_url_;
  int32 thumbnail_width_;
  int32 thumbnail_height_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<InputMessageContent> input_message_content_;

  inputInlineQueryResultArticle();

  inputInlineQueryResultArticle(string const &id_, string const &url_, bool hide_url_, string const &title_, string const &description_, string const &thumbnail_url_, int32 thumbnail_width_, int32 thumbnail_height_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_);

  static const std::int32_t ID = 1973670156;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputInlineQueryResultAudio final : public InputInlineQueryResult {
 public:
  string id_;
  string title_;
  string performer_;
  string audio_url_;
  int32 audio_duration_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<InputMessageContent> input_message_content_;

  inputInlineQueryResultAudio();

  inputInlineQueryResultAudio(string const &id_, string const &title_, string const &performer_, string const &audio_url_, int32 audio_duration_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_);

  static const std::int32_t ID = 1260139988;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputInlineQueryResultContact final : public InputInlineQueryResult {
 public:
  string id_;
  object_ptr<contact> contact_;
  string thumbnail_url_;
  int32 thumbnail_width_;
  int32 thumbnail_height_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<InputMessageContent> input_message_content_;

  inputInlineQueryResultContact();

  inputInlineQueryResultContact(string const &id_, object_ptr<contact> &&contact_, string const &thumbnail_url_, int32 thumbnail_width_, int32 thumbnail_height_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_);

  static const std::int32_t ID = 1846064594;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputInlineQueryResultDocument final : public InputInlineQueryResult {
 public:
  string id_;
  string title_;
  string description_;
  string document_url_;
  string mime_type_;
  string thumbnail_url_;
  int32 thumbnail_width_;
  int32 thumbnail_height_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<InputMessageContent> input_message_content_;

  inputInlineQueryResultDocument();

  inputInlineQueryResultDocument(string const &id_, string const &title_, string const &description_, string const &document_url_, string const &mime_type_, string const &thumbnail_url_, int32 thumbnail_width_, int32 thumbnail_height_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_);

  static const std::int32_t ID = 578801869;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputInlineQueryResultGame final : public InputInlineQueryResult {
 public:
  string id_;
  string game_short_name_;
  object_ptr<ReplyMarkup> reply_markup_;

  inputInlineQueryResultGame();

  inputInlineQueryResultGame(string const &id_, string const &game_short_name_, object_ptr<ReplyMarkup> &&reply_markup_);

  static const std::int32_t ID = 966074327;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputInlineQueryResultLocation final : public InputInlineQueryResult {
 public:
  string id_;
  object_ptr<location> location_;
  int32 live_period_;
  string title_;
  string thumbnail_url_;
  int32 thumbnail_width_;
  int32 thumbnail_height_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<InputMessageContent> input_message_content_;

  inputInlineQueryResultLocation();

  inputInlineQueryResultLocation(string const &id_, object_ptr<location> &&location_, int32 live_period_, string const &title_, string const &thumbnail_url_, int32 thumbnail_width_, int32 thumbnail_height_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_);

  static const std::int32_t ID = -1887650218;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputInlineQueryResultPhoto final : public InputInlineQueryResult {
 public:
  string id_;
  string title_;
  string description_;
  string thumbnail_url_;
  string photo_url_;
  int32 photo_width_;
  int32 photo_height_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<InputMessageContent> input_message_content_;

  inputInlineQueryResultPhoto();

  inputInlineQueryResultPhoto(string const &id_, string const &title_, string const &description_, string const &thumbnail_url_, string const &photo_url_, int32 photo_width_, int32 photo_height_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_);

  static const std::int32_t ID = -1123338721;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputInlineQueryResultSticker final : public InputInlineQueryResult {
 public:
  string id_;
  string thumbnail_url_;
  string sticker_url_;
  int32 sticker_width_;
  int32 sticker_height_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<InputMessageContent> input_message_content_;

  inputInlineQueryResultSticker();

  inputInlineQueryResultSticker(string const &id_, string const &thumbnail_url_, string const &sticker_url_, int32 sticker_width_, int32 sticker_height_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_);

  static const std::int32_t ID = 274007129;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputInlineQueryResultVenue final : public InputInlineQueryResult {
 public:
  string id_;
  object_ptr<venue> venue_;
  string thumbnail_url_;
  int32 thumbnail_width_;
  int32 thumbnail_height_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<InputMessageContent> input_message_content_;

  inputInlineQueryResultVenue();

  inputInlineQueryResultVenue(string const &id_, object_ptr<venue> &&venue_, string const &thumbnail_url_, int32 thumbnail_width_, int32 thumbnail_height_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_);

  static const std::int32_t ID = 541704509;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputInlineQueryResultVideo final : public InputInlineQueryResult {
 public:
  string id_;
  string title_;
  string description_;
  string thumbnail_url_;
  string video_url_;
  string mime_type_;
  int32 video_width_;
  int32 video_height_;
  int32 video_duration_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<InputMessageContent> input_message_content_;

  inputInlineQueryResultVideo();

  inputInlineQueryResultVideo(string const &id_, string const &title_, string const &description_, string const &thumbnail_url_, string const &video_url_, string const &mime_type_, int32 video_width_, int32 video_height_, int32 video_duration_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_);

  static const std::int32_t ID = 1724073191;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputInlineQueryResultVoiceNote final : public InputInlineQueryResult {
 public:
  string id_;
  string title_;
  string voice_note_url_;
  int32 voice_note_duration_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<InputMessageContent> input_message_content_;

  inputInlineQueryResultVoiceNote();

  inputInlineQueryResultVoiceNote(string const &id_, string const &title_, string const &voice_note_url_, int32 voice_note_duration_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_);

  static const std::int32_t ID = -1790072503;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputInvoice: public Object {
 public:
};

class inputInvoiceMessage final : public InputInvoice {
 public:
  int53 chat_id_;
  int53 message_id_;

  inputInvoiceMessage();

  inputInvoiceMessage(int53 chat_id_, int53 message_id_);

  static const std::int32_t ID = 1490872848;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputInvoiceName final : public InputInvoice {
 public:
  string name_;

  inputInvoiceName();

  explicit inputInvoiceName(string const &name_);

  static const std::int32_t ID = -1312155917;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputMessageContent: public Object {
 public:
};

class inputMessageText final : public InputMessageContent {
 public:
  object_ptr<formattedText> text_;
  bool disable_web_page_preview_;
  bool clear_draft_;

  inputMessageText();

  inputMessageText(object_ptr<formattedText> &&text_, bool disable_web_page_preview_, bool clear_draft_);

  static const std::int32_t ID = 247050392;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessageAnimation final : public InputMessageContent {
 public:
  object_ptr<InputFile> animation_;
  object_ptr<inputThumbnail> thumbnail_;
  array<int32> added_sticker_file_ids_;
  int32 duration_;
  int32 width_;
  int32 height_;
  object_ptr<formattedText> caption_;

  inputMessageAnimation();

  inputMessageAnimation(object_ptr<InputFile> &&animation_, object_ptr<inputThumbnail> &&thumbnail_, array<int32> &&added_sticker_file_ids_, int32 duration_, int32 width_, int32 height_, object_ptr<formattedText> &&caption_);

  static const std::int32_t ID = 1208433535;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessageAudio final : public InputMessageContent {
 public:
  object_ptr<InputFile> audio_;
  object_ptr<inputThumbnail> album_cover_thumbnail_;
  int32 duration_;
  string title_;
  string performer_;
  object_ptr<formattedText> caption_;

  inputMessageAudio();

  inputMessageAudio(object_ptr<InputFile> &&audio_, object_ptr<inputThumbnail> &&album_cover_thumbnail_, int32 duration_, string const &title_, string const &performer_, object_ptr<formattedText> &&caption_);

  static const std::int32_t ID = -626786126;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessageDocument final : public InputMessageContent {
 public:
  object_ptr<InputFile> document_;
  object_ptr<inputThumbnail> thumbnail_;
  bool disable_content_type_detection_;
  object_ptr<formattedText> caption_;

  inputMessageDocument();

  inputMessageDocument(object_ptr<InputFile> &&document_, object_ptr<inputThumbnail> &&thumbnail_, bool disable_content_type_detection_, object_ptr<formattedText> &&caption_);

  static const std::int32_t ID = 1633383097;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessagePhoto final : public InputMessageContent {
 public:
  object_ptr<InputFile> photo_;
  object_ptr<inputThumbnail> thumbnail_;
  array<int32> added_sticker_file_ids_;
  int32 width_;
  int32 height_;
  object_ptr<formattedText> caption_;
  int32 ttl_;

  inputMessagePhoto();

  inputMessagePhoto(object_ptr<InputFile> &&photo_, object_ptr<inputThumbnail> &&thumbnail_, array<int32> &&added_sticker_file_ids_, int32 width_, int32 height_, object_ptr<formattedText> &&caption_, int32 ttl_);

  static const std::int32_t ID = 1648801584;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessageSticker final : public InputMessageContent {
 public:
  object_ptr<InputFile> sticker_;
  object_ptr<inputThumbnail> thumbnail_;
  int32 width_;
  int32 height_;
  string emoji_;

  inputMessageSticker();

  inputMessageSticker(object_ptr<InputFile> &&sticker_, object_ptr<inputThumbnail> &&thumbnail_, int32 width_, int32 height_, string const &emoji_);

  static const std::int32_t ID = 1072805625;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessageVideo final : public InputMessageContent {
 public:
  object_ptr<InputFile> video_;
  object_ptr<inputThumbnail> thumbnail_;
  array<int32> added_sticker_file_ids_;
  int32 duration_;
  int32 width_;
  int32 height_;
  bool supports_streaming_;
  object_ptr<formattedText> caption_;
  int32 ttl_;

  inputMessageVideo();

  inputMessageVideo(object_ptr<InputFile> &&video_, object_ptr<inputThumbnail> &&thumbnail_, array<int32> &&added_sticker_file_ids_, int32 duration_, int32 width_, int32 height_, bool supports_streaming_, object_ptr<formattedText> &&caption_, int32 ttl_);

  static const std::int32_t ID = -2108486755;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessageVideoNote final : public InputMessageContent {
 public:
  object_ptr<InputFile> video_note_;
  object_ptr<inputThumbnail> thumbnail_;
  int32 duration_;
  int32 length_;

  inputMessageVideoNote();

  inputMessageVideoNote(object_ptr<InputFile> &&video_note_, object_ptr<inputThumbnail> &&thumbnail_, int32 duration_, int32 length_);

  static const std::int32_t ID = 279108859;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessageVoiceNote final : public InputMessageContent {
 public:
  object_ptr<InputFile> voice_note_;
  int32 duration_;
  bytes waveform_;
  object_ptr<formattedText> caption_;

  inputMessageVoiceNote();

  inputMessageVoiceNote(object_ptr<InputFile> &&voice_note_, int32 duration_, bytes const &waveform_, object_ptr<formattedText> &&caption_);

  static const std::int32_t ID = 2136519657;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessageLocation final : public InputMessageContent {
 public:
  object_ptr<location> location_;
  int32 live_period_;
  int32 heading_;
  int32 proximity_alert_radius_;

  inputMessageLocation();

  inputMessageLocation(object_ptr<location> &&location_, int32 live_period_, int32 heading_, int32 proximity_alert_radius_);

  static const std::int32_t ID = 648735088;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessageVenue final : public InputMessageContent {
 public:
  object_ptr<venue> venue_;

  inputMessageVenue();

  explicit inputMessageVenue(object_ptr<venue> &&venue_);

  static const std::int32_t ID = 1447926269;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessageContact final : public InputMessageContent {
 public:
  object_ptr<contact> contact_;

  inputMessageContact();

  explicit inputMessageContact(object_ptr<contact> &&contact_);

  static const std::int32_t ID = -982446849;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessageDice final : public InputMessageContent {
 public:
  string emoji_;
  bool clear_draft_;

  inputMessageDice();

  inputMessageDice(string const &emoji_, bool clear_draft_);

  static const std::int32_t ID = 841574313;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessageGame final : public InputMessageContent {
 public:
  int53 bot_user_id_;
  string game_short_name_;

  inputMessageGame();

  inputMessageGame(int53 bot_user_id_, string const &game_short_name_);

  static const std::int32_t ID = 1252944610;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessageInvoice final : public InputMessageContent {
 public:
  object_ptr<invoice> invoice_;
  string title_;
  string description_;
  string photo_url_;
  int32 photo_size_;
  int32 photo_width_;
  int32 photo_height_;
  bytes payload_;
  string provider_token_;
  string provider_data_;
  string start_parameter_;
  object_ptr<InputMessageContent> extended_media_content_;

  inputMessageInvoice();

  inputMessageInvoice(object_ptr<invoice> &&invoice_, string const &title_, string const &description_, string const &photo_url_, int32 photo_size_, int32 photo_width_, int32 photo_height_, bytes const &payload_, string const &provider_token_, string const &provider_data_, string const &start_parameter_, object_ptr<InputMessageContent> &&extended_media_content_);

  static const std::int32_t ID = 885857632;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessagePoll final : public InputMessageContent {
 public:
  string question_;
  array<string> options_;
  bool is_anonymous_;
  object_ptr<PollType> type_;
  int32 open_period_;
  int32 close_date_;
  bool is_closed_;

  inputMessagePoll();

  inputMessagePoll(string const &question_, array<string> &&options_, bool is_anonymous_, object_ptr<PollType> &&type_, int32 open_period_, int32 close_date_, bool is_closed_);

  static const std::int32_t ID = 2054629900;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessageForwarded final : public InputMessageContent {
 public:
  int53 from_chat_id_;
  int53 message_id_;
  bool in_game_share_;
  object_ptr<messageCopyOptions> copy_options_;

  inputMessageForwarded();

  inputMessageForwarded(int53 from_chat_id_, int53 message_id_, bool in_game_share_, object_ptr<messageCopyOptions> &&copy_options_);

  static const std::int32_t ID = 1696232440;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputPassportElement: public Object {
 public:
};

class inputPassportElementPersonalDetails final : public InputPassportElement {
 public:
  object_ptr<personalDetails> personal_details_;

  inputPassportElementPersonalDetails();

  explicit inputPassportElementPersonalDetails(object_ptr<personalDetails> &&personal_details_);

  static const std::int32_t ID = 164791359;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementPassport final : public InputPassportElement {
 public:
  object_ptr<inputIdentityDocument> passport_;

  inputPassportElementPassport();

  explicit inputPassportElementPassport(object_ptr<inputIdentityDocument> &&passport_);

  static const std::int32_t ID = -497011356;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementDriverLicense final : public InputPassportElement {
 public:
  object_ptr<inputIdentityDocument> driver_license_;

  inputPassportElementDriverLicense();

  explicit inputPassportElementDriverLicense(object_ptr<inputIdentityDocument> &&driver_license_);

  static const std::int32_t ID = 304813264;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementIdentityCard final : public InputPassportElement {
 public:
  object_ptr<inputIdentityDocument> identity_card_;

  inputPassportElementIdentityCard();

  explicit inputPassportElementIdentityCard(object_ptr<inputIdentityDocument> &&identity_card_);

  static const std::int32_t ID = -9963390;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementInternalPassport final : public InputPassportElement {
 public:
  object_ptr<inputIdentityDocument> internal_passport_;

  inputPassportElementInternalPassport();

  explicit inputPassportElementInternalPassport(object_ptr<inputIdentityDocument> &&internal_passport_);

  static const std::int32_t ID = 715360043;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementAddress final : public InputPassportElement {
 public:
  object_ptr<address> address_;

  inputPassportElementAddress();

  explicit inputPassportElementAddress(object_ptr<address> &&address_);

  static const std::int32_t ID = 461630480;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementUtilityBill final : public InputPassportElement {
 public:
  object_ptr<inputPersonalDocument> utility_bill_;

  inputPassportElementUtilityBill();

  explicit inputPassportElementUtilityBill(object_ptr<inputPersonalDocument> &&utility_bill_);

  static const std::int32_t ID = 1389203841;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementBankStatement final : public InputPassportElement {
 public:
  object_ptr<inputPersonalDocument> bank_statement_;

  inputPassportElementBankStatement();

  explicit inputPassportElementBankStatement(object_ptr<inputPersonalDocument> &&bank_statement_);

  static const std::int32_t ID = -26585208;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementRentalAgreement final : public InputPassportElement {
 public:
  object_ptr<inputPersonalDocument> rental_agreement_;

  inputPassportElementRentalAgreement();

  explicit inputPassportElementRentalAgreement(object_ptr<inputPersonalDocument> &&rental_agreement_);

  static const std::int32_t ID = 1736154155;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementPassportRegistration final : public InputPassportElement {
 public:
  object_ptr<inputPersonalDocument> passport_registration_;

  inputPassportElementPassportRegistration();

  explicit inputPassportElementPassportRegistration(object_ptr<inputPersonalDocument> &&passport_registration_);

  static const std::int32_t ID = 1314562128;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementTemporaryRegistration final : public InputPassportElement {
 public:
  object_ptr<inputPersonalDocument> temporary_registration_;

  inputPassportElementTemporaryRegistration();

  explicit inputPassportElementTemporaryRegistration(object_ptr<inputPersonalDocument> &&temporary_registration_);

  static const std::int32_t ID = -1913238047;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementPhoneNumber final : public InputPassportElement {
 public:
  string phone_number_;

  inputPassportElementPhoneNumber();

  explicit inputPassportElementPhoneNumber(string const &phone_number_);

  static const std::int32_t ID = 1319357497;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementEmailAddress final : public InputPassportElement {
 public:
  string email_address_;

  inputPassportElementEmailAddress();

  explicit inputPassportElementEmailAddress(string const &email_address_);

  static const std::int32_t ID = -248605659;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementError final : public Object {
 public:
  object_ptr<PassportElementType> type_;
  string message_;
  object_ptr<InputPassportElementErrorSource> source_;

  inputPassportElementError();

  inputPassportElementError(object_ptr<PassportElementType> &&type_, string const &message_, object_ptr<InputPassportElementErrorSource> &&source_);

  static const std::int32_t ID = 285756898;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputPassportElementErrorSource: public Object {
 public:
};

class inputPassportElementErrorSourceUnspecified final : public InputPassportElementErrorSource {
 public:
  bytes element_hash_;

  inputPassportElementErrorSourceUnspecified();

  explicit inputPassportElementErrorSourceUnspecified(bytes const &element_hash_);

  static const std::int32_t ID = 267230319;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementErrorSourceDataField final : public InputPassportElementErrorSource {
 public:
  string field_name_;
  bytes data_hash_;

  inputPassportElementErrorSourceDataField();

  inputPassportElementErrorSourceDataField(string const &field_name_, bytes const &data_hash_);

  static const std::int32_t ID = -426795002;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementErrorSourceFrontSide final : public InputPassportElementErrorSource {
 public:
  bytes file_hash_;

  inputPassportElementErrorSourceFrontSide();

  explicit inputPassportElementErrorSourceFrontSide(bytes const &file_hash_);

  static const std::int32_t ID = 588023741;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementErrorSourceReverseSide final : public InputPassportElementErrorSource {
 public:
  bytes file_hash_;

  inputPassportElementErrorSourceReverseSide();

  explicit inputPassportElementErrorSourceReverseSide(bytes const &file_hash_);

  static const std::int32_t ID = 413072891;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementErrorSourceSelfie final : public InputPassportElementErrorSource {
 public:
  bytes file_hash_;

  inputPassportElementErrorSourceSelfie();

  explicit inputPassportElementErrorSourceSelfie(bytes const &file_hash_);

  static const std::int32_t ID = -773575528;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementErrorSourceTranslationFile final : public InputPassportElementErrorSource {
 public:
  bytes file_hash_;

  inputPassportElementErrorSourceTranslationFile();

  explicit inputPassportElementErrorSourceTranslationFile(bytes const &file_hash_);

  static const std::int32_t ID = 505842299;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementErrorSourceTranslationFiles final : public InputPassportElementErrorSource {
 public:
  array<bytes> file_hashes_;

  inputPassportElementErrorSourceTranslationFiles();

  explicit inputPassportElementErrorSourceTranslationFiles(array<bytes> &&file_hashes_);

  static const std::int32_t ID = -527254048;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementErrorSourceFile final : public InputPassportElementErrorSource {
 public:
  bytes file_hash_;

  inputPassportElementErrorSourceFile();

  explicit inputPassportElementErrorSourceFile(bytes const &file_hash_);

  static const std::int32_t ID = -298492469;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPassportElementErrorSourceFiles final : public InputPassportElementErrorSource {
 public:
  array<bytes> file_hashes_;

  inputPassportElementErrorSourceFiles();

  explicit inputPassportElementErrorSourceFiles(array<bytes> &&file_hashes_);

  static const std::int32_t ID = -2008541640;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPersonalDocument final : public Object {
 public:
  array<object_ptr<InputFile>> files_;
  array<object_ptr<InputFile>> translation_;

  inputPersonalDocument();

  inputPersonalDocument(array<object_ptr<InputFile>> &&files_, array<object_ptr<InputFile>> &&translation_);

  static const std::int32_t ID = 1676966826;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputSticker final : public Object {
 public:
  object_ptr<InputFile> sticker_;
  string emojis_;
  object_ptr<StickerFormat> format_;
  object_ptr<maskPosition> mask_position_;

  inputSticker();

  inputSticker(object_ptr<InputFile> &&sticker_, string const &emojis_, object_ptr<StickerFormat> &&format_, object_ptr<maskPosition> &&mask_position_);

  static const std::int32_t ID = -1692915280;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputThumbnail final : public Object {
 public:
  object_ptr<InputFile> thumbnail_;
  int32 width_;
  int32 height_;

  inputThumbnail();

  inputThumbnail(object_ptr<InputFile> &&thumbnail_, int32 width_, int32 height_);

  static const std::int32_t ID = 1582387236;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InternalLinkType: public Object {
 public:
};

class internalLinkTypeActiveSessions final : public InternalLinkType {
 public:

  internalLinkTypeActiveSessions();

  static const std::int32_t ID = 1886108589;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeAttachmentMenuBot final : public InternalLinkType {
 public:
  object_ptr<TargetChat> target_chat_;
  string bot_username_;
  string url_;

  internalLinkTypeAttachmentMenuBot();

  internalLinkTypeAttachmentMenuBot(object_ptr<TargetChat> &&target_chat_, string const &bot_username_, string const &url_);

  static const std::int32_t ID = 1682719269;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeAuthenticationCode final : public InternalLinkType {
 public:
  string code_;

  internalLinkTypeAuthenticationCode();

  explicit internalLinkTypeAuthenticationCode(string const &code_);

  static const std::int32_t ID = -209235982;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeBackground final : public InternalLinkType {
 public:
  string background_name_;

  internalLinkTypeBackground();

  explicit internalLinkTypeBackground(string const &background_name_);

  static const std::int32_t ID = 185411848;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeBotStart final : public InternalLinkType {
 public:
  string bot_username_;
  string start_parameter_;
  bool autostart_;

  internalLinkTypeBotStart();

  internalLinkTypeBotStart(string const &bot_username_, string const &start_parameter_, bool autostart_);

  static const std::int32_t ID = 1066950637;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeBotStartInGroup final : public InternalLinkType {
 public:
  string bot_username_;
  string start_parameter_;
  object_ptr<chatAdministratorRights> administrator_rights_;

  internalLinkTypeBotStartInGroup();

  internalLinkTypeBotStartInGroup(string const &bot_username_, string const &start_parameter_, object_ptr<chatAdministratorRights> &&administrator_rights_);

  static const std::int32_t ID = -905081650;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeBotAddToChannel final : public InternalLinkType {
 public:
  string bot_username_;
  object_ptr<chatAdministratorRights> administrator_rights_;

  internalLinkTypeBotAddToChannel();

  internalLinkTypeBotAddToChannel(string const &bot_username_, object_ptr<chatAdministratorRights> &&administrator_rights_);

  static const std::int32_t ID = 1401602752;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeChangePhoneNumber final : public InternalLinkType {
 public:

  internalLinkTypeChangePhoneNumber();

  static const std::int32_t ID = -265856255;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeChatInvite final : public InternalLinkType {
 public:
  string invite_link_;

  internalLinkTypeChatInvite();

  explicit internalLinkTypeChatInvite(string const &invite_link_);

  static const std::int32_t ID = 428621017;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeFilterSettings final : public InternalLinkType {
 public:

  internalLinkTypeFilterSettings();

  static const std::int32_t ID = 1501632411;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeGame final : public InternalLinkType {
 public:
  string bot_username_;
  string game_short_name_;

  internalLinkTypeGame();

  internalLinkTypeGame(string const &bot_username_, string const &game_short_name_);

  static const std::int32_t ID = -260788787;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeInstantView final : public InternalLinkType {
 public:
  string url_;
  string fallback_url_;

  internalLinkTypeInstantView();

  internalLinkTypeInstantView(string const &url_, string const &fallback_url_);

  static const std::int32_t ID = 1776607039;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeInvoice final : public InternalLinkType {
 public:
  string invoice_name_;

  internalLinkTypeInvoice();

  explicit internalLinkTypeInvoice(string const &invoice_name_);

  static const std::int32_t ID = -213094996;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeLanguagePack final : public InternalLinkType {
 public:
  string language_pack_id_;

  internalLinkTypeLanguagePack();

  explicit internalLinkTypeLanguagePack(string const &language_pack_id_);

  static const std::int32_t ID = -1450766996;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeLanguageSettings final : public InternalLinkType {
 public:

  internalLinkTypeLanguageSettings();

  static const std::int32_t ID = -1340479770;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeMessage final : public InternalLinkType {
 public:
  string url_;

  internalLinkTypeMessage();

  explicit internalLinkTypeMessage(string const &url_);

  static const std::int32_t ID = 978541650;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeMessageDraft final : public InternalLinkType {
 public:
  object_ptr<formattedText> text_;
  bool contains_link_;

  internalLinkTypeMessageDraft();

  internalLinkTypeMessageDraft(object_ptr<formattedText> &&text_, bool contains_link_);

  static const std::int32_t ID = 661633749;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypePassportDataRequest final : public InternalLinkType {
 public:
  int53 bot_user_id_;
  string scope_;
  string public_key_;
  string nonce_;
  string callback_url_;

  internalLinkTypePassportDataRequest();

  internalLinkTypePassportDataRequest(int53 bot_user_id_, string const &scope_, string const &public_key_, string const &nonce_, string const &callback_url_);

  static const std::int32_t ID = -988819839;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypePhoneNumberConfirmation final : public InternalLinkType {
 public:
  string hash_;
  string phone_number_;

  internalLinkTypePhoneNumberConfirmation();

  internalLinkTypePhoneNumberConfirmation(string const &hash_, string const &phone_number_);

  static const std::int32_t ID = 1757375254;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypePremiumFeatures final : public InternalLinkType {
 public:
  string referrer_;

  internalLinkTypePremiumFeatures();

  explicit internalLinkTypePremiumFeatures(string const &referrer_);

  static const std::int32_t ID = 1216892745;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypePrivacyAndSecuritySettings final : public InternalLinkType {
 public:

  internalLinkTypePrivacyAndSecuritySettings();

  static const std::int32_t ID = -1386255665;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeProxy final : public InternalLinkType {
 public:
  string server_;
  int32 port_;
  object_ptr<ProxyType> type_;

  internalLinkTypeProxy();

  internalLinkTypeProxy(string const &server_, int32 port_, object_ptr<ProxyType> &&type_);

  static const std::int32_t ID = -1313788694;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypePublicChat final : public InternalLinkType {
 public:
  string chat_username_;

  internalLinkTypePublicChat();

  explicit internalLinkTypePublicChat(string const &chat_username_);

  static const std::int32_t ID = -1485547717;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeQrCodeAuthentication final : public InternalLinkType {
 public:

  internalLinkTypeQrCodeAuthentication();

  static const std::int32_t ID = -1089332956;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeRestorePurchases final : public InternalLinkType {
 public:

  internalLinkTypeRestorePurchases();

  static const std::int32_t ID = 606090371;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeSettings final : public InternalLinkType {
 public:

  internalLinkTypeSettings();

  static const std::int32_t ID = 393561524;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeStickerSet final : public InternalLinkType {
 public:
  string sticker_set_name_;

  internalLinkTypeStickerSet();

  explicit internalLinkTypeStickerSet(string const &sticker_set_name_);

  static const std::int32_t ID = -529639751;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeTheme final : public InternalLinkType {
 public:
  string theme_name_;

  internalLinkTypeTheme();

  explicit internalLinkTypeTheme(string const &theme_name_);

  static const std::int32_t ID = -200935417;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeThemeSettings final : public InternalLinkType {
 public:

  internalLinkTypeThemeSettings();

  static const std::int32_t ID = -1051903722;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeUnknownDeepLink final : public InternalLinkType {
 public:
  string link_;

  internalLinkTypeUnknownDeepLink();

  explicit internalLinkTypeUnknownDeepLink(string const &link_);

  static const std::int32_t ID = 625596379;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeUnsupportedProxy final : public InternalLinkType {
 public:

  internalLinkTypeUnsupportedProxy();

  static const std::int32_t ID = -566649079;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeUserPhoneNumber final : public InternalLinkType {
 public:
  string phone_number_;

  internalLinkTypeUserPhoneNumber();

  explicit internalLinkTypeUserPhoneNumber(string const &phone_number_);

  static const std::int32_t ID = -1955751319;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class internalLinkTypeVideoChat final : public InternalLinkType {
 public:
  string chat_username_;
  string invite_hash_;
  bool is_live_stream_;

  internalLinkTypeVideoChat();

  internalLinkTypeVideoChat(string const &chat_username_, string const &invite_hash_, bool is_live_stream_);

  static const std::int32_t ID = -2020149068;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class invoice final : public Object {
 public:
  string currency_;
  array<object_ptr<labeledPricePart>> price_parts_;
  int53 max_tip_amount_;
  array<int53> suggested_tip_amounts_;
  string recurring_payment_terms_of_service_url_;
  bool is_test_;
  bool need_name_;
  bool need_phone_number_;
  bool need_email_address_;
  bool need_shipping_address_;
  bool send_phone_number_to_provider_;
  bool send_email_address_to_provider_;
  bool is_flexible_;

  invoice();

  invoice(string const &currency_, array<object_ptr<labeledPricePart>> &&price_parts_, int53 max_tip_amount_, array<int53> &&suggested_tip_amounts_, string const &recurring_payment_terms_of_service_url_, bool is_test_, bool need_name_, bool need_phone_number_, bool need_email_address_, bool need_shipping_address_, bool send_phone_number_to_provider_, bool send_email_address_to_provider_, bool is_flexible_);

  static const std::int32_t ID = 1977205639;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class jsonObjectMember final : public Object {
 public:
  string key_;
  object_ptr<JsonValue> value_;

  jsonObjectMember();

  jsonObjectMember(string const &key_, object_ptr<JsonValue> &&value_);

  static const std::int32_t ID = -1803309418;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class JsonValue: public Object {
 public:
};

class jsonValueNull final : public JsonValue {
 public:

  jsonValueNull();

  static const std::int32_t ID = -92872499;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class jsonValueBoolean final : public JsonValue {
 public:
  bool value_;

  jsonValueBoolean();

  explicit jsonValueBoolean(bool value_);

  static const std::int32_t ID = -2142186576;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class jsonValueNumber final : public JsonValue {
 public:
  double value_;

  jsonValueNumber();

  explicit jsonValueNumber(double value_);

  static const std::int32_t ID = -1010822033;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class jsonValueString final : public JsonValue {
 public:
  string value_;

  jsonValueString();

  explicit jsonValueString(string const &value_);

  static const std::int32_t ID = 1597947313;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class jsonValueArray final : public JsonValue {
 public:
  array<object_ptr<JsonValue>> values_;

  jsonValueArray();

  explicit jsonValueArray(array<object_ptr<JsonValue>> &&values_);

  static const std::int32_t ID = -183913546;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class jsonValueObject final : public JsonValue {
 public:
  array<object_ptr<jsonObjectMember>> members_;

  jsonValueObject();

  explicit jsonValueObject(array<object_ptr<jsonObjectMember>> &&members_);

  static const std::int32_t ID = 520252026;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class keyboardButton final : public Object {
 public:
  string text_;
  object_ptr<KeyboardButtonType> type_;

  keyboardButton();

  keyboardButton(string const &text_, object_ptr<KeyboardButtonType> &&type_);

  static const std::int32_t ID = -2069836172;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class KeyboardButtonType: public Object {
 public:
};

class keyboardButtonTypeText final : public KeyboardButtonType {
 public:

  keyboardButtonTypeText();

  static const std::int32_t ID = -1773037256;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class keyboardButtonTypeRequestPhoneNumber final : public KeyboardButtonType {
 public:

  keyboardButtonTypeRequestPhoneNumber();

  static const std::int32_t ID = -1529235527;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class keyboardButtonTypeRequestLocation final : public KeyboardButtonType {
 public:

  keyboardButtonTypeRequestLocation();

  static const std::int32_t ID = -125661955;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class keyboardButtonTypeRequestPoll final : public KeyboardButtonType {
 public:
  bool force_regular_;
  bool force_quiz_;

  keyboardButtonTypeRequestPoll();

  keyboardButtonTypeRequestPoll(bool force_regular_, bool force_quiz_);

  static const std::int32_t ID = 1902435512;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class keyboardButtonTypeWebApp final : public KeyboardButtonType {
 public:
  string url_;

  keyboardButtonTypeWebApp();

  explicit keyboardButtonTypeWebApp(string const &url_);

  static const std::int32_t ID = 1892220770;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class labeledPricePart final : public Object {
 public:
  string label_;
  int53 amount_;

  labeledPricePart();

  labeledPricePart(string const &label_, int53 amount_);

  static const std::int32_t ID = 552789798;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class languagePackInfo final : public Object {
 public:
  string id_;
  string base_language_pack_id_;
  string name_;
  string native_name_;
  string plural_code_;
  bool is_official_;
  bool is_rtl_;
  bool is_beta_;
  bool is_installed_;
  int32 total_string_count_;
  int32 translated_string_count_;
  int32 local_string_count_;
  string translation_url_;

  languagePackInfo();

  languagePackInfo(string const &id_, string const &base_language_pack_id_, string const &name_, string const &native_name_, string const &plural_code_, bool is_official_, bool is_rtl_, bool is_beta_, bool is_installed_, int32 total_string_count_, int32 translated_string_count_, int32 local_string_count_, string const &translation_url_);

  static const std::int32_t ID = 542199642;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class languagePackString final : public Object {
 public:
  string key_;
  object_ptr<LanguagePackStringValue> value_;

  languagePackString();

  languagePackString(string const &key_, object_ptr<LanguagePackStringValue> &&value_);

  static const std::int32_t ID = 1307632736;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class LanguagePackStringValue: public Object {
 public:
};

class languagePackStringValueOrdinary final : public LanguagePackStringValue {
 public:
  string value_;

  languagePackStringValueOrdinary();

  explicit languagePackStringValueOrdinary(string const &value_);

  static const std::int32_t ID = -249256352;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class languagePackStringValuePluralized final : public LanguagePackStringValue {
 public:
  string zero_value_;
  string one_value_;
  string two_value_;
  string few_value_;
  string many_value_;
  string other_value_;

  languagePackStringValuePluralized();

  languagePackStringValuePluralized(string const &zero_value_, string const &one_value_, string const &two_value_, string const &few_value_, string const &many_value_, string const &other_value_);

  static const std::int32_t ID = 1906840261;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class languagePackStringValueDeleted final : public LanguagePackStringValue {
 public:

  languagePackStringValueDeleted();

  static const std::int32_t ID = 1834792698;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class languagePackStrings final : public Object {
 public:
  array<object_ptr<languagePackString>> strings_;

  languagePackStrings();

  explicit languagePackStrings(array<object_ptr<languagePackString>> &&strings_);

  static const std::int32_t ID = 1172082922;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class localFile final : public Object {
 public:
  string path_;
  bool can_be_downloaded_;
  bool can_be_deleted_;
  bool is_downloading_active_;
  bool is_downloading_completed_;
  int53 download_offset_;
  int53 downloaded_prefix_size_;
  int53 downloaded_size_;

  localFile();

  localFile(string const &path_, bool can_be_downloaded_, bool can_be_deleted_, bool is_downloading_active_, bool is_downloading_completed_, int53 download_offset_, int53 downloaded_prefix_size_, int53 downloaded_size_);

  static const std::int32_t ID = -1562732153;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class localizationTargetInfo final : public Object {
 public:
  array<object_ptr<languagePackInfo>> language_packs_;

  localizationTargetInfo();

  explicit localizationTargetInfo(array<object_ptr<languagePackInfo>> &&language_packs_);

  static const std::int32_t ID = -2048670809;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class location final : public Object {
 public:
  double latitude_;
  double longitude_;
  double horizontal_accuracy_;

  location();

  location(double latitude_, double longitude_, double horizontal_accuracy_);

  static const std::int32_t ID = -443392141;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class LogStream: public Object {
 public:
};

class logStreamDefault final : public LogStream {
 public:

  logStreamDefault();

  static const std::int32_t ID = 1390581436;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class logStreamFile final : public LogStream {
 public:
  string path_;
  int53 max_file_size_;
  bool redirect_stderr_;

  logStreamFile();

  logStreamFile(string const &path_, int53 max_file_size_, bool redirect_stderr_);

  static const std::int32_t ID = 1532136933;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class logStreamEmpty final : public LogStream {
 public:

  logStreamEmpty();

  static const std::int32_t ID = -499912244;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class logTags final : public Object {
 public:
  array<string> tags_;

  logTags();

  explicit logTags(array<string> &&tags_);

  static const std::int32_t ID = -1604930601;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class logVerbosityLevel final : public Object {
 public:
  int32 verbosity_level_;

  logVerbosityLevel();

  explicit logVerbosityLevel(int32 verbosity_level_);

  static const std::int32_t ID = 1734624234;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class LoginUrlInfo: public Object {
 public:
};

class loginUrlInfoOpen final : public LoginUrlInfo {
 public:
  string url_;
  bool skip_confirm_;

  loginUrlInfoOpen();

  loginUrlInfoOpen(string const &url_, bool skip_confirm_);

  static const std::int32_t ID = -1079045420;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class loginUrlInfoRequestConfirmation final : public LoginUrlInfo {
 public:
  string url_;
  string domain_;
  int53 bot_user_id_;
  bool request_write_access_;

  loginUrlInfoRequestConfirmation();

  loginUrlInfoRequestConfirmation(string const &url_, string const &domain_, int53 bot_user_id_, bool request_write_access_);

  static const std::int32_t ID = 2128290863;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class MaskPoint: public Object {
 public:
};

class maskPointForehead final : public MaskPoint {
 public:

  maskPointForehead();

  static const std::int32_t ID = 1027512005;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class maskPointEyes final : public MaskPoint {
 public:

  maskPointEyes();

  static const std::int32_t ID = 1748310861;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class maskPointMouth final : public MaskPoint {
 public:

  maskPointMouth();

  static const std::int32_t ID = 411773406;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class maskPointChin final : public MaskPoint {
 public:

  maskPointChin();

  static const std::int32_t ID = 534995335;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class maskPosition final : public Object {
 public:
  object_ptr<MaskPoint> point_;
  double x_shift_;
  double y_shift_;
  double scale_;

  maskPosition();

  maskPosition(object_ptr<MaskPoint> &&point_, double x_shift_, double y_shift_, double scale_);

  static const std::int32_t ID = -2097433026;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class message final : public Object {
 public:
  int53 id_;
  object_ptr<MessageSender> sender_id_;
  int53 chat_id_;
  object_ptr<MessageSendingState> sending_state_;
  object_ptr<MessageSchedulingState> scheduling_state_;
  bool is_outgoing_;
  bool is_pinned_;
  bool can_be_edited_;
  bool can_be_forwarded_;
  bool can_be_saved_;
  bool can_be_deleted_only_for_self_;
  bool can_be_deleted_for_all_users_;
  bool can_get_added_reactions_;
  bool can_get_statistics_;
  bool can_get_message_thread_;
  bool can_get_viewers_;
  bool can_get_media_timestamp_links_;
  bool can_report_reactions_;
  bool has_timestamped_media_;
  bool is_channel_post_;
  bool is_topic_message_;
  bool contains_unread_mention_;
  int32 date_;
  int32 edit_date_;
  object_ptr<messageForwardInfo> forward_info_;
  object_ptr<messageInteractionInfo> interaction_info_;
  array<object_ptr<unreadReaction>> unread_reactions_;
  int53 reply_in_chat_id_;
  int53 reply_to_message_id_;
  int53 message_thread_id_;
  int32 ttl_;
  double ttl_expires_in_;
  int53 via_bot_user_id_;
  string author_signature_;
  int64 media_album_id_;
  string restriction_reason_;
  object_ptr<MessageContent> content_;
  object_ptr<ReplyMarkup> reply_markup_;

  message();

  message(int53 id_, object_ptr<MessageSender> &&sender_id_, int53 chat_id_, object_ptr<MessageSendingState> &&sending_state_, object_ptr<MessageSchedulingState> &&scheduling_state_, bool is_outgoing_, bool is_pinned_, bool can_be_edited_, bool can_be_forwarded_, bool can_be_saved_, bool can_be_deleted_only_for_self_, bool can_be_deleted_for_all_users_, bool can_get_added_reactions_, bool can_get_statistics_, bool can_get_message_thread_, bool can_get_viewers_, bool can_get_media_timestamp_links_, bool can_report_reactions_, bool has_timestamped_media_, bool is_channel_post_, bool is_topic_message_, bool contains_unread_mention_, int32 date_, int32 edit_date_, object_ptr<messageForwardInfo> &&forward_info_, object_ptr<messageInteractionInfo> &&interaction_info_, array<object_ptr<unreadReaction>> &&unread_reactions_, int53 reply_in_chat_id_, int53 reply_to_message_id_, int53 message_thread_id_, int32 ttl_, double ttl_expires_in_, int53 via_bot_user_id_, string const &author_signature_, int64 media_album_id_, string const &restriction_reason_, object_ptr<MessageContent> &&content_, object_ptr<ReplyMarkup> &&reply_markup_);

  static const std::int32_t ID = -2008710896;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageCalendar final : public Object {
 public:
  int32 total_count_;
  array<object_ptr<messageCalendarDay>> days_;

  messageCalendar();

  messageCalendar(int32 total_count_, array<object_ptr<messageCalendarDay>> &&days_);

  static const std::int32_t ID = -1682890519;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageCalendarDay final : public Object {
 public:
  int32 total_count_;
  object_ptr<message> message_;

  messageCalendarDay();

  messageCalendarDay(int32 total_count_, object_ptr<message> &&message_);

  static const std::int32_t ID = -376467614;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class MessageContent: public Object {
 public:
};

class messageText final : public MessageContent {
 public:
  object_ptr<formattedText> text_;
  object_ptr<webPage> web_page_;

  messageText();

  messageText(object_ptr<formattedText> &&text_, object_ptr<webPage> &&web_page_);

  static const std::int32_t ID = 1989037971;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageAnimation final : public MessageContent {
 public:
  object_ptr<animation> animation_;
  object_ptr<formattedText> caption_;
  bool is_secret_;

  messageAnimation();

  messageAnimation(object_ptr<animation> &&animation_, object_ptr<formattedText> &&caption_, bool is_secret_);

  static const std::int32_t ID = 1306939396;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageAudio final : public MessageContent {
 public:
  object_ptr<audio> audio_;
  object_ptr<formattedText> caption_;

  messageAudio();

  messageAudio(object_ptr<audio> &&audio_, object_ptr<formattedText> &&caption_);

  static const std::int32_t ID = 276722716;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageDocument final : public MessageContent {
 public:
  object_ptr<document> document_;
  object_ptr<formattedText> caption_;

  messageDocument();

  messageDocument(object_ptr<document> &&document_, object_ptr<formattedText> &&caption_);

  static const std::int32_t ID = 596945783;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messagePhoto final : public MessageContent {
 public:
  object_ptr<photo> photo_;
  object_ptr<formattedText> caption_;
  bool is_secret_;

  messagePhoto();

  messagePhoto(object_ptr<photo> &&photo_, object_ptr<formattedText> &&caption_, bool is_secret_);

  static const std::int32_t ID = -1851395174;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageExpiredPhoto final : public MessageContent {
 public:

  messageExpiredPhoto();

  static const std::int32_t ID = -1404641801;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageSticker final : public MessageContent {
 public:
  object_ptr<sticker> sticker_;
  bool is_premium_;

  messageSticker();

  messageSticker(object_ptr<sticker> &&sticker_, bool is_premium_);

  static const std::int32_t ID = -437199670;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageVideo final : public MessageContent {
 public:
  object_ptr<video> video_;
  object_ptr<formattedText> caption_;
  bool is_secret_;

  messageVideo();

  messageVideo(object_ptr<video> &&video_, object_ptr<formattedText> &&caption_, bool is_secret_);

  static const std::int32_t ID = 2021281344;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageExpiredVideo final : public MessageContent {
 public:

  messageExpiredVideo();

  static const std::int32_t ID = -1212209981;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageVideoNote final : public MessageContent {
 public:
  object_ptr<videoNote> video_note_;
  bool is_viewed_;
  bool is_secret_;

  messageVideoNote();

  messageVideoNote(object_ptr<videoNote> &&video_note_, bool is_viewed_, bool is_secret_);

  static const std::int32_t ID = 963323014;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageVoiceNote final : public MessageContent {
 public:
  object_ptr<voiceNote> voice_note_;
  object_ptr<formattedText> caption_;
  bool is_listened_;

  messageVoiceNote();

  messageVoiceNote(object_ptr<voiceNote> &&voice_note_, object_ptr<formattedText> &&caption_, bool is_listened_);

  static const std::int32_t ID = 527777781;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageLocation final : public MessageContent {
 public:
  object_ptr<location> location_;
  int32 live_period_;
  int32 expires_in_;
  int32 heading_;
  int32 proximity_alert_radius_;

  messageLocation();

  messageLocation(object_ptr<location> &&location_, int32 live_period_, int32 expires_in_, int32 heading_, int32 proximity_alert_radius_);

  static const std::int32_t ID = 303973492;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageVenue final : public MessageContent {
 public:
  object_ptr<venue> venue_;

  messageVenue();

  explicit messageVenue(object_ptr<venue> &&venue_);

  static const std::int32_t ID = -2146492043;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageContact final : public MessageContent {
 public:
  object_ptr<contact> contact_;

  messageContact();

  explicit messageContact(object_ptr<contact> &&contact_);

  static const std::int32_t ID = -512684966;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageAnimatedEmoji final : public MessageContent {
 public:
  object_ptr<animatedEmoji> animated_emoji_;
  string emoji_;

  messageAnimatedEmoji();

  messageAnimatedEmoji(object_ptr<animatedEmoji> &&animated_emoji_, string const &emoji_);

  static const std::int32_t ID = 908195298;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageDice final : public MessageContent {
 public:
  object_ptr<DiceStickers> initial_state_;
  object_ptr<DiceStickers> final_state_;
  string emoji_;
  int32 value_;
  int32 success_animation_frame_number_;

  messageDice();

  messageDice(object_ptr<DiceStickers> &&initial_state_, object_ptr<DiceStickers> &&final_state_, string const &emoji_, int32 value_, int32 success_animation_frame_number_);

  static const std::int32_t ID = 1115779641;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageGame final : public MessageContent {
 public:
  object_ptr<game> game_;

  messageGame();

  explicit messageGame(object_ptr<game> &&game_);

  static const std::int32_t ID = -69441162;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messagePoll final : public MessageContent {
 public:
  object_ptr<poll> poll_;

  messagePoll();

  explicit messagePoll(object_ptr<poll> &&poll_);

  static const std::int32_t ID = -662130099;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageInvoice final : public MessageContent {
 public:
  string title_;
  object_ptr<formattedText> description_;
  object_ptr<photo> photo_;
  string currency_;
  int53 total_amount_;
  string start_parameter_;
  bool is_test_;
  bool need_shipping_address_;
  int53 receipt_message_id_;
  object_ptr<MessageExtendedMedia> extended_media_;

  messageInvoice();

  messageInvoice(string const &title_, object_ptr<formattedText> &&description_, object_ptr<photo> &&photo_, string const &currency_, int53 total_amount_, string const &start_parameter_, bool is_test_, bool need_shipping_address_, int53 receipt_message_id_, object_ptr<MessageExtendedMedia> &&extended_media_);

  static const std::int32_t ID = 818077142;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageCall final : public MessageContent {
 public:
  bool is_video_;
  object_ptr<CallDiscardReason> discard_reason_;
  int32 duration_;

  messageCall();

  messageCall(bool is_video_, object_ptr<CallDiscardReason> &&discard_reason_, int32 duration_);

  static const std::int32_t ID = 538893824;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageVideoChatScheduled final : public MessageContent {
 public:
  int32 group_call_id_;
  int32 start_date_;

  messageVideoChatScheduled();

  messageVideoChatScheduled(int32 group_call_id_, int32 start_date_);

  static const std::int32_t ID = -1855185481;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageVideoChatStarted final : public MessageContent {
 public:
  int32 group_call_id_;

  messageVideoChatStarted();

  explicit messageVideoChatStarted(int32 group_call_id_);

  static const std::int32_t ID = 521225561;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageVideoChatEnded final : public MessageContent {
 public:
  int32 duration_;

  messageVideoChatEnded();

  explicit messageVideoChatEnded(int32 duration_);

  static const std::int32_t ID = 2032544855;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageInviteVideoChatParticipants final : public MessageContent {
 public:
  int32 group_call_id_;
  array<int53> user_ids_;

  messageInviteVideoChatParticipants();

  messageInviteVideoChatParticipants(int32 group_call_id_, array<int53> &&user_ids_);

  static const std::int32_t ID = -1459065585;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageBasicGroupChatCreate final : public MessageContent {
 public:
  string title_;
  array<int53> member_user_ids_;

  messageBasicGroupChatCreate();

  messageBasicGroupChatCreate(string const &title_, array<int53> &&member_user_ids_);

  static const std::int32_t ID = 795404060;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageSupergroupChatCreate final : public MessageContent {
 public:
  string title_;

  messageSupergroupChatCreate();

  explicit messageSupergroupChatCreate(string const &title_);

  static const std::int32_t ID = -434325733;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageChatChangeTitle final : public MessageContent {
 public:
  string title_;

  messageChatChangeTitle();

  explicit messageChatChangeTitle(string const &title_);

  static const std::int32_t ID = 748272449;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageChatChangePhoto final : public MessageContent {
 public:
  object_ptr<chatPhoto> photo_;

  messageChatChangePhoto();

  explicit messageChatChangePhoto(object_ptr<chatPhoto> &&photo_);

  static const std::int32_t ID = -813415093;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageChatDeletePhoto final : public MessageContent {
 public:

  messageChatDeletePhoto();

  static const std::int32_t ID = -184374809;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageChatAddMembers final : public MessageContent {
 public:
  array<int53> member_user_ids_;

  messageChatAddMembers();

  explicit messageChatAddMembers(array<int53> &&member_user_ids_);

  static const std::int32_t ID = 1701117908;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageChatJoinByLink final : public MessageContent {
 public:

  messageChatJoinByLink();

  static const std::int32_t ID = 1846493311;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageChatJoinByRequest final : public MessageContent {
 public:

  messageChatJoinByRequest();

  static const std::int32_t ID = 1195428732;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageChatDeleteMember final : public MessageContent {
 public:
  int53 user_id_;

  messageChatDeleteMember();

  explicit messageChatDeleteMember(int53 user_id_);

  static const std::int32_t ID = 938029481;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageChatUpgradeTo final : public MessageContent {
 public:
  int53 supergroup_id_;

  messageChatUpgradeTo();

  explicit messageChatUpgradeTo(int53 supergroup_id_);

  static const std::int32_t ID = 104813723;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageChatUpgradeFrom final : public MessageContent {
 public:
  string title_;
  int53 basic_group_id_;

  messageChatUpgradeFrom();

  messageChatUpgradeFrom(string const &title_, int53 basic_group_id_);

  static const std::int32_t ID = 325954268;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messagePinMessage final : public MessageContent {
 public:
  int53 message_id_;

  messagePinMessage();

  explicit messagePinMessage(int53 message_id_);

  static const std::int32_t ID = 953503801;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageScreenshotTaken final : public MessageContent {
 public:

  messageScreenshotTaken();

  static const std::int32_t ID = -1564971605;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageChatSetTheme final : public MessageContent {
 public:
  string theme_name_;

  messageChatSetTheme();

  explicit messageChatSetTheme(string const &theme_name_);

  static const std::int32_t ID = -1716612088;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageChatSetTtl final : public MessageContent {
 public:
  int32 ttl_;

  messageChatSetTtl();

  explicit messageChatSetTtl(int32 ttl_);

  static const std::int32_t ID = 1810060209;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageForumTopicCreated final : public MessageContent {
 public:
  string name_;
  object_ptr<forumTopicIcon> icon_;

  messageForumTopicCreated();

  messageForumTopicCreated(string const &name_, object_ptr<forumTopicIcon> &&icon_);

  static const std::int32_t ID = -1194440751;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageForumTopicEdited final : public MessageContent {
 public:
  string name_;
  bool edit_icon_custom_emoji_id_;
  int64 icon_custom_emoji_id_;

  messageForumTopicEdited();

  messageForumTopicEdited(string const &name_, bool edit_icon_custom_emoji_id_, int64 icon_custom_emoji_id_);

  static const std::int32_t ID = 12629888;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageForumTopicIsClosedToggled final : public MessageContent {
 public:
  bool is_closed_;

  messageForumTopicIsClosedToggled();

  explicit messageForumTopicIsClosedToggled(bool is_closed_);

  static const std::int32_t ID = 1264029664;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageCustomServiceAction final : public MessageContent {
 public:
  string text_;

  messageCustomServiceAction();

  explicit messageCustomServiceAction(string const &text_);

  static const std::int32_t ID = 1435879282;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageGameScore final : public MessageContent {
 public:
  int53 game_message_id_;
  int64 game_id_;
  int32 score_;

  messageGameScore();

  messageGameScore(int53 game_message_id_, int64 game_id_, int32 score_);

  static const std::int32_t ID = 1344904575;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messagePaymentSuccessful final : public MessageContent {
 public:
  int53 invoice_chat_id_;
  int53 invoice_message_id_;
  string currency_;
  int53 total_amount_;
  bool is_recurring_;
  bool is_first_recurring_;
  string invoice_name_;

  messagePaymentSuccessful();

  messagePaymentSuccessful(int53 invoice_chat_id_, int53 invoice_message_id_, string const &currency_, int53 total_amount_, bool is_recurring_, bool is_first_recurring_, string const &invoice_name_);

  static const std::int32_t ID = 1406745820;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messagePaymentSuccessfulBot final : public MessageContent {
 public:
  string currency_;
  int53 total_amount_;
  bool is_recurring_;
  bool is_first_recurring_;
  bytes invoice_payload_;
  string shipping_option_id_;
  object_ptr<orderInfo> order_info_;
  string telegram_payment_charge_id_;
  string provider_payment_charge_id_;

  messagePaymentSuccessfulBot();

  messagePaymentSuccessfulBot(string const &currency_, int53 total_amount_, bool is_recurring_, bool is_first_recurring_, bytes const &invoice_payload_, string const &shipping_option_id_, object_ptr<orderInfo> &&order_info_, string const &telegram_payment_charge_id_, string const &provider_payment_charge_id_);

  static const std::int32_t ID = 1759592121;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageGiftedPremium final : public MessageContent {
 public:
  string currency_;
  int53 amount_;
  int32 month_count_;
  object_ptr<sticker> sticker_;

  messageGiftedPremium();

  messageGiftedPremium(string const &currency_, int53 amount_, int32 month_count_, object_ptr<sticker> &&sticker_);

  static const std::int32_t ID = 1580804249;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageContactRegistered final : public MessageContent {
 public:

  messageContactRegistered();

  static const std::int32_t ID = -1502020353;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageWebsiteConnected final : public MessageContent {
 public:
  string domain_name_;

  messageWebsiteConnected();

  explicit messageWebsiteConnected(string const &domain_name_);

  static const std::int32_t ID = -1074551800;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageWebAppDataSent final : public MessageContent {
 public:
  string button_text_;

  messageWebAppDataSent();

  explicit messageWebAppDataSent(string const &button_text_);

  static const std::int32_t ID = -83674862;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageWebAppDataReceived final : public MessageContent {
 public:
  string button_text_;
  string data_;

  messageWebAppDataReceived();

  messageWebAppDataReceived(string const &button_text_, string const &data_);

  static const std::int32_t ID = -8578539;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messagePassportDataSent final : public MessageContent {
 public:
  array<object_ptr<PassportElementType>> types_;

  messagePassportDataSent();

  explicit messagePassportDataSent(array<object_ptr<PassportElementType>> &&types_);

  static const std::int32_t ID = 1017405171;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messagePassportDataReceived final : public MessageContent {
 public:
  array<object_ptr<encryptedPassportElement>> elements_;
  object_ptr<encryptedCredentials> credentials_;

  messagePassportDataReceived();

  messagePassportDataReceived(array<object_ptr<encryptedPassportElement>> &&elements_, object_ptr<encryptedCredentials> &&credentials_);

  static const std::int32_t ID = -1367863624;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageProximityAlertTriggered final : public MessageContent {
 public:
  object_ptr<MessageSender> traveler_id_;
  object_ptr<MessageSender> watcher_id_;
  int32 distance_;

  messageProximityAlertTriggered();

  messageProximityAlertTriggered(object_ptr<MessageSender> &&traveler_id_, object_ptr<MessageSender> &&watcher_id_, int32 distance_);

  static const std::int32_t ID = 67761875;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageUnsupported final : public MessageContent {
 public:

  messageUnsupported();

  static const std::int32_t ID = -1816726139;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageCopyOptions final : public Object {
 public:
  bool send_copy_;
  bool replace_caption_;
  object_ptr<formattedText> new_caption_;

  messageCopyOptions();

  messageCopyOptions(bool send_copy_, bool replace_caption_, object_ptr<formattedText> &&new_caption_);

  static const std::int32_t ID = 1208442937;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class MessageExtendedMedia: public Object {
 public:
};

class messageExtendedMediaPreview final : public MessageExtendedMedia {
 public:
  int32 width_;
  int32 height_;
  int32 duration_;
  object_ptr<minithumbnail> minithumbnail_;
  object_ptr<formattedText> caption_;

  messageExtendedMediaPreview();

  messageExtendedMediaPreview(int32 width_, int32 height_, int32 duration_, object_ptr<minithumbnail> &&minithumbnail_, object_ptr<formattedText> &&caption_);

  static const std::int32_t ID = 1996727111;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageExtendedMediaPhoto final : public MessageExtendedMedia {
 public:
  object_ptr<photo> photo_;
  object_ptr<formattedText> caption_;

  messageExtendedMediaPhoto();

  messageExtendedMediaPhoto(object_ptr<photo> &&photo_, object_ptr<formattedText> &&caption_);

  static const std::int32_t ID = -963951312;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageExtendedMediaVideo final : public MessageExtendedMedia {
 public:
  object_ptr<video> video_;
  object_ptr<formattedText> caption_;

  messageExtendedMediaVideo();

  messageExtendedMediaVideo(object_ptr<video> &&video_, object_ptr<formattedText> &&caption_);

  static const std::int32_t ID = 296533819;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageExtendedMediaUnsupported final : public MessageExtendedMedia {
 public:
  object_ptr<formattedText> caption_;

  messageExtendedMediaUnsupported();

  explicit messageExtendedMediaUnsupported(object_ptr<formattedText> &&caption_);

  static const std::int32_t ID = 337596448;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class MessageFileType: public Object {
 public:
};

class messageFileTypePrivate final : public MessageFileType {
 public:
  string name_;

  messageFileTypePrivate();

  explicit messageFileTypePrivate(string const &name_);

  static const std::int32_t ID = -521908524;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageFileTypeGroup final : public MessageFileType {
 public:
  string title_;

  messageFileTypeGroup();

  explicit messageFileTypeGroup(string const &title_);

  static const std::int32_t ID = -219836568;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageFileTypeUnknown final : public MessageFileType {
 public:

  messageFileTypeUnknown();

  static const std::int32_t ID = 1176353458;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageForwardInfo final : public Object {
 public:
  object_ptr<MessageForwardOrigin> origin_;
  int32 date_;
  string public_service_announcement_type_;
  int53 from_chat_id_;
  int53 from_message_id_;

  messageForwardInfo();

  messageForwardInfo(object_ptr<MessageForwardOrigin> &&origin_, int32 date_, string const &public_service_announcement_type_, int53 from_chat_id_, int53 from_message_id_);

  static const std::int32_t ID = -327300408;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class MessageForwardOrigin: public Object {
 public:
};

class messageForwardOriginUser final : public MessageForwardOrigin {
 public:
  int53 sender_user_id_;

  messageForwardOriginUser();

  explicit messageForwardOriginUser(int53 sender_user_id_);

  static const std::int32_t ID = -355174191;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageForwardOriginChat final : public MessageForwardOrigin {
 public:
  int53 sender_chat_id_;
  string author_signature_;

  messageForwardOriginChat();

  messageForwardOriginChat(int53 sender_chat_id_, string const &author_signature_);

  static const std::int32_t ID = 1526010724;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageForwardOriginHiddenUser final : public MessageForwardOrigin {
 public:
  string sender_name_;

  messageForwardOriginHiddenUser();

  explicit messageForwardOriginHiddenUser(string const &sender_name_);

  static const std::int32_t ID = -271257885;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageForwardOriginChannel final : public MessageForwardOrigin {
 public:
  int53 chat_id_;
  int53 message_id_;
  string author_signature_;

  messageForwardOriginChannel();

  messageForwardOriginChannel(int53 chat_id_, int53 message_id_, string const &author_signature_);

  static const std::int32_t ID = 1490730723;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageForwardOriginMessageImport final : public MessageForwardOrigin {
 public:
  string sender_name_;

  messageForwardOriginMessageImport();

  explicit messageForwardOriginMessageImport(string const &sender_name_);

  static const std::int32_t ID = -739561951;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageInteractionInfo final : public Object {
 public:
  int32 view_count_;
  int32 forward_count_;
  object_ptr<messageReplyInfo> reply_info_;
  array<object_ptr<messageReaction>> reactions_;

  messageInteractionInfo();

  messageInteractionInfo(int32 view_count_, int32 forward_count_, object_ptr<messageReplyInfo> &&reply_info_, array<object_ptr<messageReaction>> &&reactions_);

  static const std::int32_t ID = -574858485;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageLink final : public Object {
 public:
  string link_;
  bool is_public_;

  messageLink();

  messageLink(string const &link_, bool is_public_);

  static const std::int32_t ID = -1354089818;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageLinkInfo final : public Object {
 public:
  bool is_public_;
  int53 chat_id_;
  int53 message_thread_id_;
  object_ptr<message> message_;
  int32 media_timestamp_;
  bool for_album_;

  messageLinkInfo();

  messageLinkInfo(bool is_public_, int53 chat_id_, int53 message_thread_id_, object_ptr<message> &&message_, int32 media_timestamp_, bool for_album_);

  static const std::int32_t ID = 731315024;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messagePosition final : public Object {
 public:
  int32 position_;
  int53 message_id_;
  int32 date_;

  messagePosition();

  messagePosition(int32 position_, int53 message_id_, int32 date_);

  static const std::int32_t ID = 1292189935;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messagePositions final : public Object {
 public:
  int32 total_count_;
  array<object_ptr<messagePosition>> positions_;

  messagePositions();

  messagePositions(int32 total_count_, array<object_ptr<messagePosition>> &&positions_);

  static const std::int32_t ID = -1930466649;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageReaction final : public Object {
 public:
  object_ptr<ReactionType> type_;
  int32 total_count_;
  bool is_chosen_;
  array<object_ptr<MessageSender>> recent_sender_ids_;

  messageReaction();

  messageReaction(object_ptr<ReactionType> &&type_, int32 total_count_, bool is_chosen_, array<object_ptr<MessageSender>> &&recent_sender_ids_);

  static const std::int32_t ID = 583566666;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageReplyInfo final : public Object {
 public:
  int32 reply_count_;
  array<object_ptr<MessageSender>> recent_replier_ids_;
  int53 last_read_inbox_message_id_;
  int53 last_read_outbox_message_id_;
  int53 last_message_id_;

  messageReplyInfo();

  messageReplyInfo(int32 reply_count_, array<object_ptr<MessageSender>> &&recent_replier_ids_, int53 last_read_inbox_message_id_, int53 last_read_outbox_message_id_, int53 last_message_id_);

  static const std::int32_t ID = -2093702263;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class MessageSchedulingState: public Object {
 public:
};

class messageSchedulingStateSendAtDate final : public MessageSchedulingState {
 public:
  int32 send_date_;

  messageSchedulingStateSendAtDate();

  explicit messageSchedulingStateSendAtDate(int32 send_date_);

  static const std::int32_t ID = -1485570073;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageSchedulingStateSendWhenOnline final : public MessageSchedulingState {
 public:

  messageSchedulingStateSendWhenOnline();

  static const std::int32_t ID = 2092947464;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageSendOptions final : public Object {
 public:
  bool disable_notification_;
  bool from_background_;
  bool protect_content_;
  bool update_order_of_installed_sticker_sets_;
  object_ptr<MessageSchedulingState> scheduling_state_;

  messageSendOptions();

  messageSendOptions(bool disable_notification_, bool from_background_, bool protect_content_, bool update_order_of_installed_sticker_sets_, object_ptr<MessageSchedulingState> &&scheduling_state_);

  static const std::int32_t ID = -783267022;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class MessageSender: public Object {
 public:
};

class messageSenderUser final : public MessageSender {
 public:
  int53 user_id_;

  messageSenderUser();

  explicit messageSenderUser(int53 user_id_);

  static const std::int32_t ID = -336109341;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageSenderChat final : public MessageSender {
 public:
  int53 chat_id_;

  messageSenderChat();

  explicit messageSenderChat(int53 chat_id_);

  static const std::int32_t ID = -239660751;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageSenders final : public Object {
 public:
  int32 total_count_;
  array<object_ptr<MessageSender>> senders_;

  messageSenders();

  messageSenders(int32 total_count_, array<object_ptr<MessageSender>> &&senders_);

  static const std::int32_t ID = -690158467;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class MessageSendingState: public Object {
 public:
};

class messageSendingStatePending final : public MessageSendingState {
 public:

  messageSendingStatePending();

  static const std::int32_t ID = -1381803582;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageSendingStateFailed final : public MessageSendingState {
 public:
  int32 error_code_;
  string error_message_;
  bool can_retry_;
  bool need_another_sender_;
  double retry_after_;

  messageSendingStateFailed();

  messageSendingStateFailed(int32 error_code_, string const &error_message_, bool can_retry_, bool need_another_sender_, double retry_after_);

  static const std::int32_t ID = -1741887228;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageStatistics final : public Object {
 public:
  object_ptr<StatisticalGraph> message_interaction_graph_;

  messageStatistics();

  explicit messageStatistics(object_ptr<StatisticalGraph> &&message_interaction_graph_);

  static const std::int32_t ID = -1011383888;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageThreadInfo final : public Object {
 public:
  int53 chat_id_;
  int53 message_thread_id_;
  object_ptr<messageReplyInfo> reply_info_;
  int32 unread_message_count_;
  array<object_ptr<message>> messages_;
  object_ptr<draftMessage> draft_message_;

  messageThreadInfo();

  messageThreadInfo(int53 chat_id_, int53 message_thread_id_, object_ptr<messageReplyInfo> &&reply_info_, int32 unread_message_count_, array<object_ptr<message>> &&messages_, object_ptr<draftMessage> &&draft_message_);

  static const std::int32_t ID = -248536056;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages final : public Object {
 public:
  int32 total_count_;
  array<object_ptr<message>> messages_;

  messages();

  messages(int32 total_count_, array<object_ptr<message>> &&messages_);

  static const std::int32_t ID = -16498159;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class minithumbnail final : public Object {
 public:
  int32 width_;
  int32 height_;
  bytes data_;

  minithumbnail();

  minithumbnail(int32 width_, int32 height_, bytes const &data_);

  static const std::int32_t ID = -328540758;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class networkStatistics final : public Object {
 public:
  int32 since_date_;
  array<object_ptr<NetworkStatisticsEntry>> entries_;

  networkStatistics();

  networkStatistics(int32 since_date_, array<object_ptr<NetworkStatisticsEntry>> &&entries_);

  static const std::int32_t ID = 1615554212;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class NetworkStatisticsEntry: public Object {
 public:
};

class networkStatisticsEntryFile final : public NetworkStatisticsEntry {
 public:
  object_ptr<FileType> file_type_;
  object_ptr<NetworkType> network_type_;
  int53 sent_bytes_;
  int53 received_bytes_;

  networkStatisticsEntryFile();

  networkStatisticsEntryFile(object_ptr<FileType> &&file_type_, object_ptr<NetworkType> &&network_type_, int53 sent_bytes_, int53 received_bytes_);

  static const std::int32_t ID = 188452706;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class networkStatisticsEntryCall final : public NetworkStatisticsEntry {
 public:
  object_ptr<NetworkType> network_type_;
  int53 sent_bytes_;
  int53 received_bytes_;
  double duration_;

  networkStatisticsEntryCall();

  networkStatisticsEntryCall(object_ptr<NetworkType> &&network_type_, int53 sent_bytes_, int53 received_bytes_, double duration_);

  static const std::int32_t ID = 737000365;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class NetworkType: public Object {
 public:
};

class networkTypeNone final : public NetworkType {
 public:

  networkTypeNone();

  static const std::int32_t ID = -1971691759;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class networkTypeMobile final : public NetworkType {
 public:

  networkTypeMobile();

  static const std::int32_t ID = 819228239;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class networkTypeMobileRoaming final : public NetworkType {
 public:

  networkTypeMobileRoaming();

  static const std::int32_t ID = -1435199760;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class networkTypeWiFi final : public NetworkType {
 public:

  networkTypeWiFi();

  static const std::int32_t ID = -633872070;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class networkTypeOther final : public NetworkType {
 public:

  networkTypeOther();

  static const std::int32_t ID = 1942128539;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class notification final : public Object {
 public:
  int32 id_;
  int32 date_;
  bool is_silent_;
  object_ptr<NotificationType> type_;

  notification();

  notification(int32 id_, int32 date_, bool is_silent_, object_ptr<NotificationType> &&type_);

  static const std::int32_t ID = 788743120;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class notificationGroup final : public Object {
 public:
  int32 id_;
  object_ptr<NotificationGroupType> type_;
  int53 chat_id_;
  int32 total_count_;
  array<object_ptr<notification>> notifications_;

  notificationGroup();

  notificationGroup(int32 id_, object_ptr<NotificationGroupType> &&type_, int53 chat_id_, int32 total_count_, array<object_ptr<notification>> &&notifications_);

  static const std::int32_t ID = 780691541;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class NotificationGroupType: public Object {
 public:
};

class notificationGroupTypeMessages final : public NotificationGroupType {
 public:

  notificationGroupTypeMessages();

  static const std::int32_t ID = -1702481123;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class notificationGroupTypeMentions final : public NotificationGroupType {
 public:

  notificationGroupTypeMentions();

  static const std::int32_t ID = -2050324051;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class notificationGroupTypeSecretChat final : public NotificationGroupType {
 public:

  notificationGroupTypeSecretChat();

  static const std::int32_t ID = 1390759476;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class notificationGroupTypeCalls final : public NotificationGroupType {
 public:

  notificationGroupTypeCalls();

  static const std::int32_t ID = 1379123538;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class NotificationSettingsScope: public Object {
 public:
};

class notificationSettingsScopePrivateChats final : public NotificationSettingsScope {
 public:

  notificationSettingsScopePrivateChats();

  static const std::int32_t ID = 937446759;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class notificationSettingsScopeGroupChats final : public NotificationSettingsScope {
 public:

  notificationSettingsScopeGroupChats();

  static const std::int32_t ID = 1212142067;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class notificationSettingsScopeChannelChats final : public NotificationSettingsScope {
 public:

  notificationSettingsScopeChannelChats();

  static const std::int32_t ID = 548013448;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class notificationSound final : public Object {
 public:
  int64 id_;
  int32 duration_;
  int32 date_;
  string title_;
  string data_;
  object_ptr<file> sound_;

  notificationSound();

  notificationSound(int64 id_, int32 duration_, int32 date_, string const &title_, string const &data_, object_ptr<file> &&sound_);

  static const std::int32_t ID = -185638601;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class notificationSounds final : public Object {
 public:
  array<object_ptr<notificationSound>> notification_sounds_;

  notificationSounds();

  explicit notificationSounds(array<object_ptr<notificationSound>> &&notification_sounds_);

  static const std::int32_t ID = -630813169;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class NotificationType: public Object {
 public:
};

class notificationTypeNewMessage final : public NotificationType {
 public:
  object_ptr<message> message_;
  bool show_preview_;

  notificationTypeNewMessage();

  notificationTypeNewMessage(object_ptr<message> &&message_, bool show_preview_);

  static const std::int32_t ID = -254745614;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class notificationTypeNewSecretChat final : public NotificationType {
 public:

  notificationTypeNewSecretChat();

  static const std::int32_t ID = 1198638768;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class notificationTypeNewCall final : public NotificationType {
 public:
  int32 call_id_;

  notificationTypeNewCall();

  explicit notificationTypeNewCall(int32 call_id_);

  static const std::int32_t ID = 1712734585;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class notificationTypeNewPushMessage final : public NotificationType {
 public:
  int53 message_id_;
  object_ptr<MessageSender> sender_id_;
  string sender_name_;
  bool is_outgoing_;
  object_ptr<PushMessageContent> content_;

  notificationTypeNewPushMessage();

  notificationTypeNewPushMessage(int53 message_id_, object_ptr<MessageSender> &&sender_id_, string const &sender_name_, bool is_outgoing_, object_ptr<PushMessageContent> &&content_);

  static const std::int32_t ID = -711680462;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ok final : public Object {
 public:

  ok();

  static const std::int32_t ID = -722616727;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class OptionValue: public Object {
 public:
};

class optionValueBoolean final : public OptionValue {
 public:
  bool value_;

  optionValueBoolean();

  explicit optionValueBoolean(bool value_);

  static const std::int32_t ID = 63135518;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class optionValueEmpty final : public OptionValue {
 public:

  optionValueEmpty();

  static const std::int32_t ID = 918955155;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class optionValueInteger final : public OptionValue {
 public:
  int64 value_;

  optionValueInteger();

  explicit optionValueInteger(int64 value_);

  static const std::int32_t ID = -186858780;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class optionValueString final : public OptionValue {
 public:
  string value_;

  optionValueString();

  explicit optionValueString(string const &value_);

  static const std::int32_t ID = 756248212;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class orderInfo final : public Object {
 public:
  string name_;
  string phone_number_;
  string email_address_;
  object_ptr<address> shipping_address_;

  orderInfo();

  orderInfo(string const &name_, string const &phone_number_, string const &email_address_, object_ptr<address> &&shipping_address_);

  static const std::int32_t ID = 783997294;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PageBlock: public Object {
 public:
};

class pageBlockTitle final : public PageBlock {
 public:
  object_ptr<RichText> title_;

  pageBlockTitle();

  explicit pageBlockTitle(object_ptr<RichText> &&title_);

  static const std::int32_t ID = 1629664784;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockSubtitle final : public PageBlock {
 public:
  object_ptr<RichText> subtitle_;

  pageBlockSubtitle();

  explicit pageBlockSubtitle(object_ptr<RichText> &&subtitle_);

  static const std::int32_t ID = 264524263;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockAuthorDate final : public PageBlock {
 public:
  object_ptr<RichText> author_;
  int32 publish_date_;

  pageBlockAuthorDate();

  pageBlockAuthorDate(object_ptr<RichText> &&author_, int32 publish_date_);

  static const std::int32_t ID = 1300231184;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockHeader final : public PageBlock {
 public:
  object_ptr<RichText> header_;

  pageBlockHeader();

  explicit pageBlockHeader(object_ptr<RichText> &&header_);

  static const std::int32_t ID = 1402854811;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockSubheader final : public PageBlock {
 public:
  object_ptr<RichText> subheader_;

  pageBlockSubheader();

  explicit pageBlockSubheader(object_ptr<RichText> &&subheader_);

  static const std::int32_t ID = 1263956774;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockKicker final : public PageBlock {
 public:
  object_ptr<RichText> kicker_;

  pageBlockKicker();

  explicit pageBlockKicker(object_ptr<RichText> &&kicker_);

  static const std::int32_t ID = 1361282635;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockParagraph final : public PageBlock {
 public:
  object_ptr<RichText> text_;

  pageBlockParagraph();

  explicit pageBlockParagraph(object_ptr<RichText> &&text_);

  static const std::int32_t ID = 1182402406;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockPreformatted final : public PageBlock {
 public:
  object_ptr<RichText> text_;
  string language_;

  pageBlockPreformatted();

  pageBlockPreformatted(object_ptr<RichText> &&text_, string const &language_);

  static const std::int32_t ID = -1066346178;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockFooter final : public PageBlock {
 public:
  object_ptr<RichText> footer_;

  pageBlockFooter();

  explicit pageBlockFooter(object_ptr<RichText> &&footer_);

  static const std::int32_t ID = 886429480;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockDivider final : public PageBlock {
 public:

  pageBlockDivider();

  static const std::int32_t ID = -618614392;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockAnchor final : public PageBlock {
 public:
  string name_;

  pageBlockAnchor();

  explicit pageBlockAnchor(string const &name_);

  static const std::int32_t ID = -837994576;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockList final : public PageBlock {
 public:
  array<object_ptr<pageBlockListItem>> items_;

  pageBlockList();

  explicit pageBlockList(array<object_ptr<pageBlockListItem>> &&items_);

  static const std::int32_t ID = -1037074852;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockBlockQuote final : public PageBlock {
 public:
  object_ptr<RichText> text_;
  object_ptr<RichText> credit_;

  pageBlockBlockQuote();

  pageBlockBlockQuote(object_ptr<RichText> &&text_, object_ptr<RichText> &&credit_);

  static const std::int32_t ID = 1657834142;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockPullQuote final : public PageBlock {
 public:
  object_ptr<RichText> text_;
  object_ptr<RichText> credit_;

  pageBlockPullQuote();

  pageBlockPullQuote(object_ptr<RichText> &&text_, object_ptr<RichText> &&credit_);

  static const std::int32_t ID = 490242317;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockAnimation final : public PageBlock {
 public:
  object_ptr<animation> animation_;
  object_ptr<pageBlockCaption> caption_;
  bool need_autoplay_;

  pageBlockAnimation();

  pageBlockAnimation(object_ptr<animation> &&animation_, object_ptr<pageBlockCaption> &&caption_, bool need_autoplay_);

  static const std::int32_t ID = 1355669513;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockAudio final : public PageBlock {
 public:
  object_ptr<audio> audio_;
  object_ptr<pageBlockCaption> caption_;

  pageBlockAudio();

  pageBlockAudio(object_ptr<audio> &&audio_, object_ptr<pageBlockCaption> &&caption_);

  static const std::int32_t ID = -63371245;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockPhoto final : public PageBlock {
 public:
  object_ptr<photo> photo_;
  object_ptr<pageBlockCaption> caption_;
  string url_;

  pageBlockPhoto();

  pageBlockPhoto(object_ptr<photo> &&photo_, object_ptr<pageBlockCaption> &&caption_, string const &url_);

  static const std::int32_t ID = 417601156;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockVideo final : public PageBlock {
 public:
  object_ptr<video> video_;
  object_ptr<pageBlockCaption> caption_;
  bool need_autoplay_;
  bool is_looped_;

  pageBlockVideo();

  pageBlockVideo(object_ptr<video> &&video_, object_ptr<pageBlockCaption> &&caption_, bool need_autoplay_, bool is_looped_);

  static const std::int32_t ID = 510041394;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockVoiceNote final : public PageBlock {
 public:
  object_ptr<voiceNote> voice_note_;
  object_ptr<pageBlockCaption> caption_;

  pageBlockVoiceNote();

  pageBlockVoiceNote(object_ptr<voiceNote> &&voice_note_, object_ptr<pageBlockCaption> &&caption_);

  static const std::int32_t ID = 1823310463;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockCover final : public PageBlock {
 public:
  object_ptr<PageBlock> cover_;

  pageBlockCover();

  explicit pageBlockCover(object_ptr<PageBlock> &&cover_);

  static const std::int32_t ID = 972174080;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockEmbedded final : public PageBlock {
 public:
  string url_;
  string html_;
  object_ptr<photo> poster_photo_;
  int32 width_;
  int32 height_;
  object_ptr<pageBlockCaption> caption_;
  bool is_full_width_;
  bool allow_scrolling_;

  pageBlockEmbedded();

  pageBlockEmbedded(string const &url_, string const &html_, object_ptr<photo> &&poster_photo_, int32 width_, int32 height_, object_ptr<pageBlockCaption> &&caption_, bool is_full_width_, bool allow_scrolling_);

  static const std::int32_t ID = -1942577763;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockEmbeddedPost final : public PageBlock {
 public:
  string url_;
  string author_;
  object_ptr<photo> author_photo_;
  int32 date_;
  array<object_ptr<PageBlock>> page_blocks_;
  object_ptr<pageBlockCaption> caption_;

  pageBlockEmbeddedPost();

  pageBlockEmbeddedPost(string const &url_, string const &author_, object_ptr<photo> &&author_photo_, int32 date_, array<object_ptr<PageBlock>> &&page_blocks_, object_ptr<pageBlockCaption> &&caption_);

  static const std::int32_t ID = 397600949;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockCollage final : public PageBlock {
 public:
  array<object_ptr<PageBlock>> page_blocks_;
  object_ptr<pageBlockCaption> caption_;

  pageBlockCollage();

  pageBlockCollage(array<object_ptr<PageBlock>> &&page_blocks_, object_ptr<pageBlockCaption> &&caption_);

  static const std::int32_t ID = 1163760110;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockSlideshow final : public PageBlock {
 public:
  array<object_ptr<PageBlock>> page_blocks_;
  object_ptr<pageBlockCaption> caption_;

  pageBlockSlideshow();

  pageBlockSlideshow(array<object_ptr<PageBlock>> &&page_blocks_, object_ptr<pageBlockCaption> &&caption_);

  static const std::int32_t ID = 539217375;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockChatLink final : public PageBlock {
 public:
  string title_;
  object_ptr<chatPhotoInfo> photo_;
  string username_;

  pageBlockChatLink();

  pageBlockChatLink(string const &title_, object_ptr<chatPhotoInfo> &&photo_, string const &username_);

  static const std::int32_t ID = -202091253;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockTable final : public PageBlock {
 public:
  object_ptr<RichText> caption_;
  array<array<object_ptr<pageBlockTableCell>>> cells_;
  bool is_bordered_;
  bool is_striped_;

  pageBlockTable();

  pageBlockTable(object_ptr<RichText> &&caption_, array<array<object_ptr<pageBlockTableCell>>> &&cells_, bool is_bordered_, bool is_striped_);

  static const std::int32_t ID = -942649288;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockDetails final : public PageBlock {
 public:
  object_ptr<RichText> header_;
  array<object_ptr<PageBlock>> page_blocks_;
  bool is_open_;

  pageBlockDetails();

  pageBlockDetails(object_ptr<RichText> &&header_, array<object_ptr<PageBlock>> &&page_blocks_, bool is_open_);

  static const std::int32_t ID = -1599869809;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockRelatedArticles final : public PageBlock {
 public:
  object_ptr<RichText> header_;
  array<object_ptr<pageBlockRelatedArticle>> articles_;

  pageBlockRelatedArticles();

  pageBlockRelatedArticles(object_ptr<RichText> &&header_, array<object_ptr<pageBlockRelatedArticle>> &&articles_);

  static const std::int32_t ID = -1807324374;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockMap final : public PageBlock {
 public:
  object_ptr<location> location_;
  int32 zoom_;
  int32 width_;
  int32 height_;
  object_ptr<pageBlockCaption> caption_;

  pageBlockMap();

  pageBlockMap(object_ptr<location> &&location_, int32 zoom_, int32 width_, int32 height_, object_ptr<pageBlockCaption> &&caption_);

  static const std::int32_t ID = 1510961171;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockCaption final : public Object {
 public:
  object_ptr<RichText> text_;
  object_ptr<RichText> credit_;

  pageBlockCaption();

  pageBlockCaption(object_ptr<RichText> &&text_, object_ptr<RichText> &&credit_);

  static const std::int32_t ID = -1180064650;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PageBlockHorizontalAlignment: public Object {
 public:
};

class pageBlockHorizontalAlignmentLeft final : public PageBlockHorizontalAlignment {
 public:

  pageBlockHorizontalAlignmentLeft();

  static const std::int32_t ID = 848701417;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockHorizontalAlignmentCenter final : public PageBlockHorizontalAlignment {
 public:

  pageBlockHorizontalAlignmentCenter();

  static const std::int32_t ID = -1009203990;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockHorizontalAlignmentRight final : public PageBlockHorizontalAlignment {
 public:

  pageBlockHorizontalAlignmentRight();

  static const std::int32_t ID = 1371369214;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockListItem final : public Object {
 public:
  string label_;
  array<object_ptr<PageBlock>> page_blocks_;

  pageBlockListItem();

  pageBlockListItem(string const &label_, array<object_ptr<PageBlock>> &&page_blocks_);

  static const std::int32_t ID = 323186259;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockRelatedArticle final : public Object {
 public:
  string url_;
  string title_;
  string description_;
  object_ptr<photo> photo_;
  string author_;
  int32 publish_date_;

  pageBlockRelatedArticle();

  pageBlockRelatedArticle(string const &url_, string const &title_, string const &description_, object_ptr<photo> &&photo_, string const &author_, int32 publish_date_);

  static const std::int32_t ID = 481199251;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockTableCell final : public Object {
 public:
  object_ptr<RichText> text_;
  bool is_header_;
  int32 colspan_;
  int32 rowspan_;
  object_ptr<PageBlockHorizontalAlignment> align_;
  object_ptr<PageBlockVerticalAlignment> valign_;

  pageBlockTableCell();

  pageBlockTableCell(object_ptr<RichText> &&text_, bool is_header_, int32 colspan_, int32 rowspan_, object_ptr<PageBlockHorizontalAlignment> &&align_, object_ptr<PageBlockVerticalAlignment> &&valign_);

  static const std::int32_t ID = 1417658214;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PageBlockVerticalAlignment: public Object {
 public:
};

class pageBlockVerticalAlignmentTop final : public PageBlockVerticalAlignment {
 public:

  pageBlockVerticalAlignmentTop();

  static const std::int32_t ID = 195500454;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockVerticalAlignmentMiddle final : public PageBlockVerticalAlignment {
 public:

  pageBlockVerticalAlignmentMiddle();

  static const std::int32_t ID = -2123096587;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockVerticalAlignmentBottom final : public PageBlockVerticalAlignment {
 public:

  pageBlockVerticalAlignmentBottom();

  static const std::int32_t ID = 2092531158;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportAuthorizationForm final : public Object {
 public:
  int32 id_;
  array<object_ptr<passportRequiredElement>> required_elements_;
  string privacy_policy_url_;

  passportAuthorizationForm();

  passportAuthorizationForm(int32 id_, array<object_ptr<passportRequiredElement>> &&required_elements_, string const &privacy_policy_url_);

  static const std::int32_t ID = -1070673218;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PassportElement: public Object {
 public:
};

class passportElementPersonalDetails final : public PassportElement {
 public:
  object_ptr<personalDetails> personal_details_;

  passportElementPersonalDetails();

  explicit passportElementPersonalDetails(object_ptr<personalDetails> &&personal_details_);

  static const std::int32_t ID = 1217724035;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementPassport final : public PassportElement {
 public:
  object_ptr<identityDocument> passport_;

  passportElementPassport();

  explicit passportElementPassport(object_ptr<identityDocument> &&passport_);

  static const std::int32_t ID = -263985373;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementDriverLicense final : public PassportElement {
 public:
  object_ptr<identityDocument> driver_license_;

  passportElementDriverLicense();

  explicit passportElementDriverLicense(object_ptr<identityDocument> &&driver_license_);

  static const std::int32_t ID = 1643580589;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementIdentityCard final : public PassportElement {
 public:
  object_ptr<identityDocument> identity_card_;

  passportElementIdentityCard();

  explicit passportElementIdentityCard(object_ptr<identityDocument> &&identity_card_);

  static const std::int32_t ID = 2083775797;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementInternalPassport final : public PassportElement {
 public:
  object_ptr<identityDocument> internal_passport_;

  passportElementInternalPassport();

  explicit passportElementInternalPassport(object_ptr<identityDocument> &&internal_passport_);

  static const std::int32_t ID = 36220295;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementAddress final : public PassportElement {
 public:
  object_ptr<address> address_;

  passportElementAddress();

  explicit passportElementAddress(object_ptr<address> &&address_);

  static const std::int32_t ID = -782625232;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementUtilityBill final : public PassportElement {
 public:
  object_ptr<personalDocument> utility_bill_;

  passportElementUtilityBill();

  explicit passportElementUtilityBill(object_ptr<personalDocument> &&utility_bill_);

  static const std::int32_t ID = -234611246;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementBankStatement final : public PassportElement {
 public:
  object_ptr<personalDocument> bank_statement_;

  passportElementBankStatement();

  explicit passportElementBankStatement(object_ptr<personalDocument> &&bank_statement_);

  static const std::int32_t ID = -366464408;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementRentalAgreement final : public PassportElement {
 public:
  object_ptr<personalDocument> rental_agreement_;

  passportElementRentalAgreement();

  explicit passportElementRentalAgreement(object_ptr<personalDocument> &&rental_agreement_);

  static const std::int32_t ID = -290141400;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementPassportRegistration final : public PassportElement {
 public:
  object_ptr<personalDocument> passport_registration_;

  passportElementPassportRegistration();

  explicit passportElementPassportRegistration(object_ptr<personalDocument> &&passport_registration_);

  static const std::int32_t ID = 618323071;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementTemporaryRegistration final : public PassportElement {
 public:
  object_ptr<personalDocument> temporary_registration_;

  passportElementTemporaryRegistration();

  explicit passportElementTemporaryRegistration(object_ptr<personalDocument> &&temporary_registration_);

  static const std::int32_t ID = 1237626864;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementPhoneNumber final : public PassportElement {
 public:
  string phone_number_;

  passportElementPhoneNumber();

  explicit passportElementPhoneNumber(string const &phone_number_);

  static const std::int32_t ID = -1320118375;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementEmailAddress final : public PassportElement {
 public:
  string email_address_;

  passportElementEmailAddress();

  explicit passportElementEmailAddress(string const &email_address_);

  static const std::int32_t ID = -1528129531;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementError final : public Object {
 public:
  object_ptr<PassportElementType> type_;
  string message_;
  object_ptr<PassportElementErrorSource> source_;

  passportElementError();

  passportElementError(object_ptr<PassportElementType> &&type_, string const &message_, object_ptr<PassportElementErrorSource> &&source_);

  static const std::int32_t ID = -1861902395;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PassportElementErrorSource: public Object {
 public:
};

class passportElementErrorSourceUnspecified final : public PassportElementErrorSource {
 public:

  passportElementErrorSourceUnspecified();

  static const std::int32_t ID = -378320830;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementErrorSourceDataField final : public PassportElementErrorSource {
 public:
  string field_name_;

  passportElementErrorSourceDataField();

  explicit passportElementErrorSourceDataField(string const &field_name_);

  static const std::int32_t ID = -308650776;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementErrorSourceFrontSide final : public PassportElementErrorSource {
 public:

  passportElementErrorSourceFrontSide();

  static const std::int32_t ID = 1895658292;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementErrorSourceReverseSide final : public PassportElementErrorSource {
 public:

  passportElementErrorSourceReverseSide();

  static const std::int32_t ID = 1918630391;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementErrorSourceSelfie final : public PassportElementErrorSource {
 public:

  passportElementErrorSourceSelfie();

  static const std::int32_t ID = -797043672;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementErrorSourceTranslationFile final : public PassportElementErrorSource {
 public:
  int32 file_index_;

  passportElementErrorSourceTranslationFile();

  explicit passportElementErrorSourceTranslationFile(int32 file_index_);

  static const std::int32_t ID = -689621228;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementErrorSourceTranslationFiles final : public PassportElementErrorSource {
 public:

  passportElementErrorSourceTranslationFiles();

  static const std::int32_t ID = 581280796;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementErrorSourceFile final : public PassportElementErrorSource {
 public:
  int32 file_index_;

  passportElementErrorSourceFile();

  explicit passportElementErrorSourceFile(int32 file_index_);

  static const std::int32_t ID = 2020358960;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementErrorSourceFiles final : public PassportElementErrorSource {
 public:

  passportElementErrorSourceFiles();

  static const std::int32_t ID = 1894164178;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PassportElementType: public Object {
 public:
};

class passportElementTypePersonalDetails final : public PassportElementType {
 public:

  passportElementTypePersonalDetails();

  static const std::int32_t ID = -1032136365;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementTypePassport final : public PassportElementType {
 public:

  passportElementTypePassport();

  static const std::int32_t ID = -436360376;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementTypeDriverLicense final : public PassportElementType {
 public:

  passportElementTypeDriverLicense();

  static const std::int32_t ID = 1827298379;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementTypeIdentityCard final : public PassportElementType {
 public:

  passportElementTypeIdentityCard();

  static const std::int32_t ID = -502356132;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementTypeInternalPassport final : public PassportElementType {
 public:

  passportElementTypeInternalPassport();

  static const std::int32_t ID = -793781959;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementTypeAddress final : public PassportElementType {
 public:

  passportElementTypeAddress();

  static const std::int32_t ID = 496327874;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementTypeUtilityBill final : public PassportElementType {
 public:

  passportElementTypeUtilityBill();

  static const std::int32_t ID = 627084906;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementTypeBankStatement final : public PassportElementType {
 public:

  passportElementTypeBankStatement();

  static const std::int32_t ID = 574095667;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementTypeRentalAgreement final : public PassportElementType {
 public:

  passportElementTypeRentalAgreement();

  static const std::int32_t ID = -2060583280;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementTypePassportRegistration final : public PassportElementType {
 public:

  passportElementTypePassportRegistration();

  static const std::int32_t ID = -159478209;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementTypeTemporaryRegistration final : public PassportElementType {
 public:

  passportElementTypeTemporaryRegistration();

  static const std::int32_t ID = 1092498527;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementTypePhoneNumber final : public PassportElementType {
 public:

  passportElementTypePhoneNumber();

  static const std::int32_t ID = -995361172;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementTypeEmailAddress final : public PassportElementType {
 public:

  passportElementTypeEmailAddress();

  static const std::int32_t ID = -79321405;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElements final : public Object {
 public:
  array<object_ptr<PassportElement>> elements_;

  passportElements();

  explicit passportElements(array<object_ptr<PassportElement>> &&elements_);

  static const std::int32_t ID = 1264617556;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportElementsWithErrors final : public Object {
 public:
  array<object_ptr<PassportElement>> elements_;
  array<object_ptr<passportElementError>> errors_;

  passportElementsWithErrors();

  passportElementsWithErrors(array<object_ptr<PassportElement>> &&elements_, array<object_ptr<passportElementError>> &&errors_);

  static const std::int32_t ID = 1308923044;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportRequiredElement final : public Object {
 public:
  array<object_ptr<passportSuitableElement>> suitable_elements_;

  passportRequiredElement();

  explicit passportRequiredElement(array<object_ptr<passportSuitableElement>> &&suitable_elements_);

  static const std::int32_t ID = -1983641651;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passportSuitableElement final : public Object {
 public:
  object_ptr<PassportElementType> type_;
  bool is_selfie_required_;
  bool is_translation_required_;
  bool is_native_name_required_;

  passportSuitableElement();

  passportSuitableElement(object_ptr<PassportElementType> &&type_, bool is_selfie_required_, bool is_translation_required_, bool is_native_name_required_);

  static const std::int32_t ID = -789019876;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passwordState final : public Object {
 public:
  bool has_password_;
  string password_hint_;
  bool has_recovery_email_address_;
  bool has_passport_data_;
  object_ptr<emailAddressAuthenticationCodeInfo> recovery_email_address_code_info_;
  string login_email_address_pattern_;
  int32 pending_reset_date_;

  passwordState();

  passwordState(bool has_password_, string const &password_hint_, bool has_recovery_email_address_, bool has_passport_data_, object_ptr<emailAddressAuthenticationCodeInfo> &&recovery_email_address_code_info_, string const &login_email_address_pattern_, int32 pending_reset_date_);

  static const std::int32_t ID = 483801128;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class paymentForm final : public Object {
 public:
  int64 id_;
  object_ptr<invoice> invoice_;
  int53 seller_bot_user_id_;
  int53 payment_provider_user_id_;
  object_ptr<PaymentProvider> payment_provider_;
  array<object_ptr<paymentOption>> additional_payment_options_;
  object_ptr<orderInfo> saved_order_info_;
  array<object_ptr<savedCredentials>> saved_credentials_;
  bool can_save_credentials_;
  bool need_password_;
  string product_title_;
  object_ptr<formattedText> product_description_;
  object_ptr<photo> product_photo_;

  paymentForm();

  paymentForm(int64 id_, object_ptr<invoice> &&invoice_, int53 seller_bot_user_id_, int53 payment_provider_user_id_, object_ptr<PaymentProvider> &&payment_provider_, array<object_ptr<paymentOption>> &&additional_payment_options_, object_ptr<orderInfo> &&saved_order_info_, array<object_ptr<savedCredentials>> &&saved_credentials_, bool can_save_credentials_, bool need_password_, string const &product_title_, object_ptr<formattedText> &&product_description_, object_ptr<photo> &&product_photo_);

  static const std::int32_t ID = -1468471378;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class paymentOption final : public Object {
 public:
  string title_;
  string url_;

  paymentOption();

  paymentOption(string const &title_, string const &url_);

  static const std::int32_t ID = -294020965;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PaymentProvider: public Object {
 public:
};

class paymentProviderSmartGlocal final : public PaymentProvider {
 public:
  string public_token_;

  paymentProviderSmartGlocal();

  explicit paymentProviderSmartGlocal(string const &public_token_);

  static const std::int32_t ID = 1800479470;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class paymentProviderStripe final : public PaymentProvider {
 public:
  string publishable_key_;
  bool need_country_;
  bool need_postal_code_;
  bool need_cardholder_name_;

  paymentProviderStripe();

  paymentProviderStripe(string const &publishable_key_, bool need_country_, bool need_postal_code_, bool need_cardholder_name_);

  static const std::int32_t ID = 370467227;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class paymentProviderOther final : public PaymentProvider {
 public:
  string url_;

  paymentProviderOther();

  explicit paymentProviderOther(string const &url_);

  static const std::int32_t ID = -1336876828;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class paymentReceipt final : public Object {
 public:
  string title_;
  object_ptr<formattedText> description_;
  object_ptr<photo> photo_;
  int32 date_;
  int53 seller_bot_user_id_;
  int53 payment_provider_user_id_;
  object_ptr<invoice> invoice_;
  object_ptr<orderInfo> order_info_;
  object_ptr<shippingOption> shipping_option_;
  string credentials_title_;
  int53 tip_amount_;

  paymentReceipt();

  paymentReceipt(string const &title_, object_ptr<formattedText> &&description_, object_ptr<photo> &&photo_, int32 date_, int53 seller_bot_user_id_, int53 payment_provider_user_id_, object_ptr<invoice> &&invoice_, object_ptr<orderInfo> &&order_info_, object_ptr<shippingOption> &&shipping_option_, string const &credentials_title_, int53 tip_amount_);

  static const std::int32_t ID = -400955711;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class paymentResult final : public Object {
 public:
  bool success_;
  string verification_url_;

  paymentResult();

  paymentResult(bool success_, string const &verification_url_);

  static const std::int32_t ID = -804263843;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class personalDetails final : public Object {
 public:
  string first_name_;
  string middle_name_;
  string last_name_;
  string native_first_name_;
  string native_middle_name_;
  string native_last_name_;
  object_ptr<date> birthdate_;
  string gender_;
  string country_code_;
  string residence_country_code_;

  personalDetails();

  personalDetails(string const &first_name_, string const &middle_name_, string const &last_name_, string const &native_first_name_, string const &native_middle_name_, string const &native_last_name_, object_ptr<date> &&birthdate_, string const &gender_, string const &country_code_, string const &residence_country_code_);

  static const std::int32_t ID = -1061656137;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class personalDocument final : public Object {
 public:
  array<object_ptr<datedFile>> files_;
  array<object_ptr<datedFile>> translation_;

  personalDocument();

  personalDocument(array<object_ptr<datedFile>> &&files_, array<object_ptr<datedFile>> &&translation_);

  static const std::int32_t ID = -1011634661;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class phoneNumberAuthenticationSettings final : public Object {
 public:
  bool allow_flash_call_;
  bool allow_missed_call_;
  bool is_current_phone_number_;
  bool allow_sms_retriever_api_;
  array<string> authentication_tokens_;

  phoneNumberAuthenticationSettings();

  phoneNumberAuthenticationSettings(bool allow_flash_call_, bool allow_missed_call_, bool is_current_phone_number_, bool allow_sms_retriever_api_, array<string> &&authentication_tokens_);

  static const std::int32_t ID = -421219756;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class phoneNumberInfo final : public Object {
 public:
  object_ptr<countryInfo> country_;
  string country_calling_code_;
  string formatted_phone_number_;

  phoneNumberInfo();

  phoneNumberInfo(object_ptr<countryInfo> &&country_, string const &country_calling_code_, string const &formatted_phone_number_);

  static const std::int32_t ID = 560180961;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class photo final : public Object {
 public:
  bool has_stickers_;
  object_ptr<minithumbnail> minithumbnail_;
  array<object_ptr<photoSize>> sizes_;

  photo();

  photo(bool has_stickers_, object_ptr<minithumbnail> &&minithumbnail_, array<object_ptr<photoSize>> &&sizes_);

  static const std::int32_t ID = -2022871583;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class photoSize final : public Object {
 public:
  string type_;
  object_ptr<file> photo_;
  int32 width_;
  int32 height_;
  array<int32> progressive_sizes_;

  photoSize();

  photoSize(string const &type_, object_ptr<file> &&photo_, int32 width_, int32 height_, array<int32> &&progressive_sizes_);

  static const std::int32_t ID = 1609182352;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class point final : public Object {
 public:
  double x_;
  double y_;

  point();

  point(double x_, double y_);

  static const std::int32_t ID = 437515705;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class poll final : public Object {
 public:
  int64 id_;
  string question_;
  array<object_ptr<pollOption>> options_;
  int32 total_voter_count_;
  array<int53> recent_voter_user_ids_;
  bool is_anonymous_;
  object_ptr<PollType> type_;
  int32 open_period_;
  int32 close_date_;
  bool is_closed_;

  poll();

  poll(int64 id_, string const &question_, array<object_ptr<pollOption>> &&options_, int32 total_voter_count_, array<int53> &&recent_voter_user_ids_, bool is_anonymous_, object_ptr<PollType> &&type_, int32 open_period_, int32 close_date_, bool is_closed_);

  static const std::int32_t ID = 116940085;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pollOption final : public Object {
 public:
  string text_;
  int32 voter_count_;
  int32 vote_percentage_;
  bool is_chosen_;
  bool is_being_chosen_;

  pollOption();

  pollOption(string const &text_, int32 voter_count_, int32 vote_percentage_, bool is_chosen_, bool is_being_chosen_);

  static const std::int32_t ID = 1473893797;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PollType: public Object {
 public:
};

class pollTypeRegular final : public PollType {
 public:
  bool allow_multiple_answers_;

  pollTypeRegular();

  explicit pollTypeRegular(bool allow_multiple_answers_);

  static const std::int32_t ID = 641265698;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pollTypeQuiz final : public PollType {
 public:
  int32 correct_option_id_;
  object_ptr<formattedText> explanation_;

  pollTypeQuiz();

  pollTypeQuiz(int32 correct_option_id_, object_ptr<formattedText> &&explanation_);

  static const std::int32_t ID = 657013913;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PremiumFeature: public Object {
 public:
};

class premiumFeatureIncreasedLimits final : public PremiumFeature {
 public:

  premiumFeatureIncreasedLimits();

  static const std::int32_t ID = 1785455031;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumFeatureIncreasedUploadFileSize final : public PremiumFeature {
 public:

  premiumFeatureIncreasedUploadFileSize();

  static const std::int32_t ID = 1825367155;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumFeatureImprovedDownloadSpeed final : public PremiumFeature {
 public:

  premiumFeatureImprovedDownloadSpeed();

  static const std::int32_t ID = -267695554;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumFeatureVoiceRecognition final : public PremiumFeature {
 public:

  premiumFeatureVoiceRecognition();

  static const std::int32_t ID = 1288216542;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumFeatureDisabledAds final : public PremiumFeature {
 public:

  premiumFeatureDisabledAds();

  static const std::int32_t ID = -2008587702;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumFeatureUniqueReactions final : public PremiumFeature {
 public:

  premiumFeatureUniqueReactions();

  static const std::int32_t ID = 766750743;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumFeatureUniqueStickers final : public PremiumFeature {
 public:

  premiumFeatureUniqueStickers();

  static const std::int32_t ID = -2101773312;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumFeatureCustomEmoji final : public PremiumFeature {
 public:

  premiumFeatureCustomEmoji();

  static const std::int32_t ID = 1332599628;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumFeatureAdvancedChatManagement final : public PremiumFeature {
 public:

  premiumFeatureAdvancedChatManagement();

  static const std::int32_t ID = 796347674;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumFeatureProfileBadge final : public PremiumFeature {
 public:

  premiumFeatureProfileBadge();

  static const std::int32_t ID = 233648322;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumFeatureEmojiStatus final : public PremiumFeature {
 public:

  premiumFeatureEmojiStatus();

  static const std::int32_t ID = -36516639;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumFeatureAnimatedProfilePhoto final : public PremiumFeature {
 public:

  premiumFeatureAnimatedProfilePhoto();

  static const std::int32_t ID = -100741914;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumFeatureForumTopicIcon final : public PremiumFeature {
 public:

  premiumFeatureForumTopicIcon();

  static const std::int32_t ID = -823172286;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumFeatureAppIcons final : public PremiumFeature {
 public:

  premiumFeatureAppIcons();

  static const std::int32_t ID = 1585050761;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumFeaturePromotionAnimation final : public Object {
 public:
  object_ptr<PremiumFeature> feature_;
  object_ptr<animation> animation_;

  premiumFeaturePromotionAnimation();

  premiumFeaturePromotionAnimation(object_ptr<PremiumFeature> &&feature_, object_ptr<animation> &&animation_);

  static const std::int32_t ID = -1986155748;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumFeatures final : public Object {
 public:
  array<object_ptr<PremiumFeature>> features_;
  array<object_ptr<premiumLimit>> limits_;
  object_ptr<InternalLinkType> payment_link_;

  premiumFeatures();

  premiumFeatures(array<object_ptr<PremiumFeature>> &&features_, array<object_ptr<premiumLimit>> &&limits_, object_ptr<InternalLinkType> &&payment_link_);

  static const std::int32_t ID = 1875162172;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumLimit final : public Object {
 public:
  object_ptr<PremiumLimitType> type_;
  int32 default_value_;
  int32 premium_value_;

  premiumLimit();

  premiumLimit(object_ptr<PremiumLimitType> &&type_, int32 default_value_, int32 premium_value_);

  static const std::int32_t ID = 2127786726;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PremiumLimitType: public Object {
 public:
};

class premiumLimitTypeSupergroupCount final : public PremiumLimitType {
 public:

  premiumLimitTypeSupergroupCount();

  static const std::int32_t ID = -247467131;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumLimitTypePinnedChatCount final : public PremiumLimitType {
 public:

  premiumLimitTypePinnedChatCount();

  static const std::int32_t ID = -998947871;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumLimitTypeCreatedPublicChatCount final : public PremiumLimitType {
 public:

  premiumLimitTypeCreatedPublicChatCount();

  static const std::int32_t ID = 446086841;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumLimitTypeSavedAnimationCount final : public PremiumLimitType {
 public:

  premiumLimitTypeSavedAnimationCount();

  static const std::int32_t ID = -19759735;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumLimitTypeFavoriteStickerCount final : public PremiumLimitType {
 public:

  premiumLimitTypeFavoriteStickerCount();

  static const std::int32_t ID = 639754787;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumLimitTypeChatFilterCount final : public PremiumLimitType {
 public:

  premiumLimitTypeChatFilterCount();

  static const std::int32_t ID = -644517214;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumLimitTypeChatFilterChosenChatCount final : public PremiumLimitType {
 public:

  premiumLimitTypeChatFilterChosenChatCount();

  static const std::int32_t ID = 2057969759;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumLimitTypePinnedArchivedChatCount final : public PremiumLimitType {
 public:

  premiumLimitTypePinnedArchivedChatCount();

  static const std::int32_t ID = 1485515276;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumLimitTypeCaptionLength final : public PremiumLimitType {
 public:

  premiumLimitTypeCaptionLength();

  static const std::int32_t ID = 293984314;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumLimitTypeBioLength final : public PremiumLimitType {
 public:

  premiumLimitTypeBioLength();

  static const std::int32_t ID = -1146976765;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumPaymentOption final : public Object {
 public:
  string currency_;
  int53 amount_;
  int32 discount_percentage_;
  int32 month_count_;
  string store_product_id_;
  object_ptr<InternalLinkType> payment_link_;

  premiumPaymentOption();

  premiumPaymentOption(string const &currency_, int53 amount_, int32 discount_percentage_, int32 month_count_, string const &store_product_id_, object_ptr<InternalLinkType> &&payment_link_);

  static const std::int32_t ID = -1945346126;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PremiumSource: public Object {
 public:
};

class premiumSourceLimitExceeded final : public PremiumSource {
 public:
  object_ptr<PremiumLimitType> limit_type_;

  premiumSourceLimitExceeded();

  explicit premiumSourceLimitExceeded(object_ptr<PremiumLimitType> &&limit_type_);

  static const std::int32_t ID = -2052159742;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumSourceFeature final : public PremiumSource {
 public:
  object_ptr<PremiumFeature> feature_;

  premiumSourceFeature();

  explicit premiumSourceFeature(object_ptr<PremiumFeature> &&feature_);

  static const std::int32_t ID = 445813541;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumSourceLink final : public PremiumSource {
 public:
  string referrer_;

  premiumSourceLink();

  explicit premiumSourceLink(string const &referrer_);

  static const std::int32_t ID = 2135071132;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumSourceSettings final : public PremiumSource {
 public:

  premiumSourceSettings();

  static const std::int32_t ID = -285702859;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumState final : public Object {
 public:
  object_ptr<formattedText> state_;
  array<object_ptr<premiumPaymentOption>> payment_options_;
  array<object_ptr<premiumFeaturePromotionAnimation>> animations_;

  premiumState();

  premiumState(object_ptr<formattedText> &&state_, array<object_ptr<premiumPaymentOption>> &&payment_options_, array<object_ptr<premiumFeaturePromotionAnimation>> &&animations_);

  static const std::int32_t ID = -1502199392;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class profilePhoto final : public Object {
 public:
  int64 id_;
  object_ptr<file> small_;
  object_ptr<file> big_;
  object_ptr<minithumbnail> minithumbnail_;
  bool has_animation_;

  profilePhoto();

  profilePhoto(int64 id_, object_ptr<file> &&small_, object_ptr<file> &&big_, object_ptr<minithumbnail> &&minithumbnail_, bool has_animation_);

  static const std::int32_t ID = -131097523;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class proxies final : public Object {
 public:
  array<object_ptr<proxy>> proxies_;

  proxies();

  explicit proxies(array<object_ptr<proxy>> &&proxies_);

  static const std::int32_t ID = 1200447205;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class proxy final : public Object {
 public:
  int32 id_;
  string server_;
  int32 port_;
  int32 last_used_date_;
  bool is_enabled_;
  object_ptr<ProxyType> type_;

  proxy();

  proxy(int32 id_, string const &server_, int32 port_, int32 last_used_date_, bool is_enabled_, object_ptr<ProxyType> &&type_);

  static const std::int32_t ID = 196049779;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ProxyType: public Object {
 public:
};

class proxyTypeSocks5 final : public ProxyType {
 public:
  string username_;
  string password_;

  proxyTypeSocks5();

  proxyTypeSocks5(string const &username_, string const &password_);

  static const std::int32_t ID = -890027341;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class proxyTypeHttp final : public ProxyType {
 public:
  string username_;
  string password_;
  bool http_only_;

  proxyTypeHttp();

  proxyTypeHttp(string const &username_, string const &password_, bool http_only_);

  static const std::int32_t ID = -1547188361;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class proxyTypeMtproto final : public ProxyType {
 public:
  string secret_;

  proxyTypeMtproto();

  explicit proxyTypeMtproto(string const &secret_);

  static const std::int32_t ID = -1964826627;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PublicChatType: public Object {
 public:
};

class publicChatTypeHasUsername final : public PublicChatType {
 public:

  publicChatTypeHasUsername();

  static const std::int32_t ID = 350789758;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class publicChatTypeIsLocationBased final : public PublicChatType {
 public:

  publicChatTypeIsLocationBased();

  static const std::int32_t ID = 1183735952;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PushMessageContent: public Object {
 public:
};

class pushMessageContentHidden final : public PushMessageContent {
 public:
  bool is_pinned_;

  pushMessageContentHidden();

  explicit pushMessageContentHidden(bool is_pinned_);

  static const std::int32_t ID = -316950436;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentAnimation final : public PushMessageContent {
 public:
  object_ptr<animation> animation_;
  string caption_;
  bool is_pinned_;

  pushMessageContentAnimation();

  pushMessageContentAnimation(object_ptr<animation> &&animation_, string const &caption_, bool is_pinned_);

  static const std::int32_t ID = 1034215396;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentAudio final : public PushMessageContent {
 public:
  object_ptr<audio> audio_;
  bool is_pinned_;

  pushMessageContentAudio();

  pushMessageContentAudio(object_ptr<audio> &&audio_, bool is_pinned_);

  static const std::int32_t ID = 381581426;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentContact final : public PushMessageContent {
 public:
  string name_;
  bool is_pinned_;

  pushMessageContentContact();

  pushMessageContentContact(string const &name_, bool is_pinned_);

  static const std::int32_t ID = -12219820;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentContactRegistered final : public PushMessageContent {
 public:

  pushMessageContentContactRegistered();

  static const std::int32_t ID = -303962720;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentDocument final : public PushMessageContent {
 public:
  object_ptr<document> document_;
  bool is_pinned_;

  pushMessageContentDocument();

  pushMessageContentDocument(object_ptr<document> &&document_, bool is_pinned_);

  static const std::int32_t ID = -458379775;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentGame final : public PushMessageContent {
 public:
  string title_;
  bool is_pinned_;

  pushMessageContentGame();

  pushMessageContentGame(string const &title_, bool is_pinned_);

  static const std::int32_t ID = -515131109;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentGameScore final : public PushMessageContent {
 public:
  string title_;
  int32 score_;
  bool is_pinned_;

  pushMessageContentGameScore();

  pushMessageContentGameScore(string const &title_, int32 score_, bool is_pinned_);

  static const std::int32_t ID = 901303688;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentInvoice final : public PushMessageContent {
 public:
  string price_;
  bool is_pinned_;

  pushMessageContentInvoice();

  pushMessageContentInvoice(string const &price_, bool is_pinned_);

  static const std::int32_t ID = -1731687492;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentLocation final : public PushMessageContent {
 public:
  bool is_live_;
  bool is_pinned_;

  pushMessageContentLocation();

  pushMessageContentLocation(bool is_live_, bool is_pinned_);

  static const std::int32_t ID = -1288005709;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentPhoto final : public PushMessageContent {
 public:
  object_ptr<photo> photo_;
  string caption_;
  bool is_secret_;
  bool is_pinned_;

  pushMessageContentPhoto();

  pushMessageContentPhoto(object_ptr<photo> &&photo_, string const &caption_, bool is_secret_, bool is_pinned_);

  static const std::int32_t ID = 140631122;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentPoll final : public PushMessageContent {
 public:
  string question_;
  bool is_regular_;
  bool is_pinned_;

  pushMessageContentPoll();

  pushMessageContentPoll(string const &question_, bool is_regular_, bool is_pinned_);

  static const std::int32_t ID = -44403654;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentScreenshotTaken final : public PushMessageContent {
 public:

  pushMessageContentScreenshotTaken();

  static const std::int32_t ID = 214245369;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentSticker final : public PushMessageContent {
 public:
  object_ptr<sticker> sticker_;
  string emoji_;
  bool is_pinned_;

  pushMessageContentSticker();

  pushMessageContentSticker(object_ptr<sticker> &&sticker_, string const &emoji_, bool is_pinned_);

  static const std::int32_t ID = 1553513939;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentText final : public PushMessageContent {
 public:
  string text_;
  bool is_pinned_;

  pushMessageContentText();

  pushMessageContentText(string const &text_, bool is_pinned_);

  static const std::int32_t ID = 274587305;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentVideo final : public PushMessageContent {
 public:
  object_ptr<video> video_;
  string caption_;
  bool is_secret_;
  bool is_pinned_;

  pushMessageContentVideo();

  pushMessageContentVideo(object_ptr<video> &&video_, string const &caption_, bool is_secret_, bool is_pinned_);

  static const std::int32_t ID = 310038831;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentVideoNote final : public PushMessageContent {
 public:
  object_ptr<videoNote> video_note_;
  bool is_pinned_;

  pushMessageContentVideoNote();

  pushMessageContentVideoNote(object_ptr<videoNote> &&video_note_, bool is_pinned_);

  static const std::int32_t ID = -1122764417;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentVoiceNote final : public PushMessageContent {
 public:
  object_ptr<voiceNote> voice_note_;
  bool is_pinned_;

  pushMessageContentVoiceNote();

  pushMessageContentVoiceNote(object_ptr<voiceNote> &&voice_note_, bool is_pinned_);

  static const std::int32_t ID = 88910987;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentBasicGroupChatCreate final : public PushMessageContent {
 public:

  pushMessageContentBasicGroupChatCreate();

  static const std::int32_t ID = -2114855172;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentChatAddMembers final : public PushMessageContent {
 public:
  string member_name_;
  bool is_current_user_;
  bool is_returned_;

  pushMessageContentChatAddMembers();

  pushMessageContentChatAddMembers(string const &member_name_, bool is_current_user_, bool is_returned_);

  static const std::int32_t ID = -1087145158;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentChatChangePhoto final : public PushMessageContent {
 public:

  pushMessageContentChatChangePhoto();

  static const std::int32_t ID = -1114222051;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentChatChangeTitle final : public PushMessageContent {
 public:
  string title_;

  pushMessageContentChatChangeTitle();

  explicit pushMessageContentChatChangeTitle(string const &title_);

  static const std::int32_t ID = -1964902749;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentChatSetTheme final : public PushMessageContent {
 public:
  string theme_name_;

  pushMessageContentChatSetTheme();

  explicit pushMessageContentChatSetTheme(string const &theme_name_);

  static const std::int32_t ID = 173882216;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentChatDeleteMember final : public PushMessageContent {
 public:
  string member_name_;
  bool is_current_user_;
  bool is_left_;

  pushMessageContentChatDeleteMember();

  pushMessageContentChatDeleteMember(string const &member_name_, bool is_current_user_, bool is_left_);

  static const std::int32_t ID = 598714783;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentChatJoinByLink final : public PushMessageContent {
 public:

  pushMessageContentChatJoinByLink();

  static const std::int32_t ID = 1553719113;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentChatJoinByRequest final : public PushMessageContent {
 public:

  pushMessageContentChatJoinByRequest();

  static const std::int32_t ID = -205823627;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentRecurringPayment final : public PushMessageContent {
 public:
  string amount_;

  pushMessageContentRecurringPayment();

  explicit pushMessageContentRecurringPayment(string const &amount_);

  static const std::int32_t ID = 1619211802;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentMessageForwards final : public PushMessageContent {
 public:
  int32 total_count_;

  pushMessageContentMessageForwards();

  explicit pushMessageContentMessageForwards(int32 total_count_);

  static const std::int32_t ID = -1913083876;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushMessageContentMediaAlbum final : public PushMessageContent {
 public:
  int32 total_count_;
  bool has_photos_;
  bool has_videos_;
  bool has_audios_;
  bool has_documents_;

  pushMessageContentMediaAlbum();

  pushMessageContentMediaAlbum(int32 total_count_, bool has_photos_, bool has_videos_, bool has_audios_, bool has_documents_);

  static const std::int32_t ID = -748426897;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pushReceiverId final : public Object {
 public:
  int64 id_;

  pushReceiverId();

  explicit pushReceiverId(int64 id_);

  static const std::int32_t ID = 371056428;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ReactionType: public Object {
 public:
};

class reactionTypeEmoji final : public ReactionType {
 public:
  string emoji_;

  reactionTypeEmoji();

  explicit reactionTypeEmoji(string const &emoji_);

  static const std::int32_t ID = -1942084920;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class reactionTypeCustomEmoji final : public ReactionType {
 public:
  int64 custom_emoji_id_;

  reactionTypeCustomEmoji();

  explicit reactionTypeCustomEmoji(int64 custom_emoji_id_);

  static const std::int32_t ID = -989117709;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class recommendedChatFilter final : public Object {
 public:
  object_ptr<chatFilter> filter_;
  string description_;

  recommendedChatFilter();

  recommendedChatFilter(object_ptr<chatFilter> &&filter_, string const &description_);

  static const std::int32_t ID = 36048610;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class recommendedChatFilters final : public Object {
 public:
  array<object_ptr<recommendedChatFilter>> chat_filters_;

  recommendedChatFilters();

  explicit recommendedChatFilters(array<object_ptr<recommendedChatFilter>> &&chat_filters_);

  static const std::int32_t ID = -263416880;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class recoveryEmailAddress final : public Object {
 public:
  string recovery_email_address_;

  recoveryEmailAddress();

  explicit recoveryEmailAddress(string const &recovery_email_address_);

  static const std::int32_t ID = 1290526187;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class remoteFile final : public Object {
 public:
  string id_;
  string unique_id_;
  bool is_uploading_active_;
  bool is_uploading_completed_;
  int53 uploaded_size_;

  remoteFile();

  remoteFile(string const &id_, string const &unique_id_, bool is_uploading_active_, bool is_uploading_completed_, int53 uploaded_size_);

  static const std::int32_t ID = 747731030;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ReplyMarkup: public Object {
 public:
};

class replyMarkupRemoveKeyboard final : public ReplyMarkup {
 public:
  bool is_personal_;

  replyMarkupRemoveKeyboard();

  explicit replyMarkupRemoveKeyboard(bool is_personal_);

  static const std::int32_t ID = -691252879;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class replyMarkupForceReply final : public ReplyMarkup {
 public:
  bool is_personal_;
  string input_field_placeholder_;

  replyMarkupForceReply();

  replyMarkupForceReply(bool is_personal_, string const &input_field_placeholder_);

  static const std::int32_t ID = 1101461919;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class replyMarkupShowKeyboard final : public ReplyMarkup {
 public:
  array<array<object_ptr<keyboardButton>>> rows_;
  bool resize_keyboard_;
  bool one_time_;
  bool is_personal_;
  string input_field_placeholder_;

  replyMarkupShowKeyboard();

  replyMarkupShowKeyboard(array<array<object_ptr<keyboardButton>>> &&rows_, bool resize_keyboard_, bool one_time_, bool is_personal_, string const &input_field_placeholder_);

  static const std::int32_t ID = -64985802;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class replyMarkupInlineKeyboard final : public ReplyMarkup {
 public:
  array<array<object_ptr<inlineKeyboardButton>>> rows_;

  replyMarkupInlineKeyboard();

  explicit replyMarkupInlineKeyboard(array<array<object_ptr<inlineKeyboardButton>>> &&rows_);

  static const std::int32_t ID = -619317658;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ResetPasswordResult: public Object {
 public:
};

class resetPasswordResultOk final : public ResetPasswordResult {
 public:

  resetPasswordResultOk();

  static const std::int32_t ID = -1397267463;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class resetPasswordResultPending final : public ResetPasswordResult {
 public:
  int32 pending_reset_date_;

  resetPasswordResultPending();

  explicit resetPasswordResultPending(int32 pending_reset_date_);

  static const std::int32_t ID = 1193925721;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class resetPasswordResultDeclined final : public ResetPasswordResult {
 public:
  int32 retry_date_;

  resetPasswordResultDeclined();

  explicit resetPasswordResultDeclined(int32 retry_date_);

  static const std::int32_t ID = -1202200373;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class RichText: public Object {
 public:
};

class richTextPlain final : public RichText {
 public:
  string text_;

  richTextPlain();

  explicit richTextPlain(string const &text_);

  static const std::int32_t ID = 482617702;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class richTextBold final : public RichText {
 public:
  object_ptr<RichText> text_;

  richTextBold();

  explicit richTextBold(object_ptr<RichText> &&text_);

  static const std::int32_t ID = 1670844268;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class richTextItalic final : public RichText {
 public:
  object_ptr<RichText> text_;

  richTextItalic();

  explicit richTextItalic(object_ptr<RichText> &&text_);

  static const std::int32_t ID = 1853354047;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class richTextUnderline final : public RichText {
 public:
  object_ptr<RichText> text_;

  richTextUnderline();

  explicit richTextUnderline(object_ptr<RichText> &&text_);

  static const std::int32_t ID = -536019572;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class richTextStrikethrough final : public RichText {
 public:
  object_ptr<RichText> text_;

  richTextStrikethrough();

  explicit richTextStrikethrough(object_ptr<RichText> &&text_);

  static const std::int32_t ID = 723413585;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class richTextFixed final : public RichText {
 public:
  object_ptr<RichText> text_;

  richTextFixed();

  explicit richTextFixed(object_ptr<RichText> &&text_);

  static const std::int32_t ID = -1271496249;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class richTextUrl final : public RichText {
 public:
  object_ptr<RichText> text_;
  string url_;
  bool is_cached_;

  richTextUrl();

  richTextUrl(object_ptr<RichText> &&text_, string const &url_, bool is_cached_);

  static const std::int32_t ID = 83939092;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class richTextEmailAddress final : public RichText {
 public:
  object_ptr<RichText> text_;
  string email_address_;

  richTextEmailAddress();

  richTextEmailAddress(object_ptr<RichText> &&text_, string const &email_address_);

  static const std::int32_t ID = 40018679;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class richTextSubscript final : public RichText {
 public:
  object_ptr<RichText> text_;

  richTextSubscript();

  explicit richTextSubscript(object_ptr<RichText> &&text_);

  static const std::int32_t ID = -868197812;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class richTextSuperscript final : public RichText {
 public:
  object_ptr<RichText> text_;

  richTextSuperscript();

  explicit richTextSuperscript(object_ptr<RichText> &&text_);

  static const std::int32_t ID = -382241437;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class richTextMarked final : public RichText {
 public:
  object_ptr<RichText> text_;

  richTextMarked();

  explicit richTextMarked(object_ptr<RichText> &&text_);

  static const std::int32_t ID = -1271999614;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class richTextPhoneNumber final : public RichText {
 public:
  object_ptr<RichText> text_;
  string phone_number_;

  richTextPhoneNumber();

  richTextPhoneNumber(object_ptr<RichText> &&text_, string const &phone_number_);

  static const std::int32_t ID = 128521539;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class richTextIcon final : public RichText {
 public:
  object_ptr<document> document_;
  int32 width_;
  int32 height_;

  richTextIcon();

  richTextIcon(object_ptr<document> &&document_, int32 width_, int32 height_);

  static const std::int32_t ID = -1480316158;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class richTextReference final : public RichText {
 public:
  object_ptr<RichText> text_;
  string anchor_name_;
  string url_;

  richTextReference();

  richTextReference(object_ptr<RichText> &&text_, string const &anchor_name_, string const &url_);

  static const std::int32_t ID = -1147530634;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class richTextAnchor final : public RichText {
 public:
  string name_;

  richTextAnchor();

  explicit richTextAnchor(string const &name_);

  static const std::int32_t ID = 1316950068;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class richTextAnchorLink final : public RichText {
 public:
  object_ptr<RichText> text_;
  string anchor_name_;
  string url_;

  richTextAnchorLink();

  richTextAnchorLink(object_ptr<RichText> &&text_, string const &anchor_name_, string const &url_);

  static const std::int32_t ID = -1541418282;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class richTexts final : public RichText {
 public:
  array<object_ptr<RichText>> texts_;

  richTexts();

  explicit richTexts(array<object_ptr<RichText>> &&texts_);

  static const std::int32_t ID = 1647457821;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class rtmpUrl final : public Object {
 public:
  string url_;
  string stream_key_;

  rtmpUrl();

  rtmpUrl(string const &url_, string const &stream_key_);

  static const std::int32_t ID = 1009302613;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class savedCredentials final : public Object {
 public:
  string id_;
  string title_;

  savedCredentials();

  savedCredentials(string const &id_, string const &title_);

  static const std::int32_t ID = -370273060;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class scopeNotificationSettings final : public Object {
 public:
  int32 mute_for_;
  int64 sound_id_;
  bool show_preview_;
  bool disable_pinned_message_notifications_;
  bool disable_mention_notifications_;

  scopeNotificationSettings();

  scopeNotificationSettings(int32 mute_for_, int64 sound_id_, bool show_preview_, bool disable_pinned_message_notifications_, bool disable_mention_notifications_);

  static const std::int32_t ID = -1383458661;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class SearchMessagesFilter: public Object {
 public:
};

class searchMessagesFilterEmpty final : public SearchMessagesFilter {
 public:

  searchMessagesFilterEmpty();

  static const std::int32_t ID = -869395657;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchMessagesFilterAnimation final : public SearchMessagesFilter {
 public:

  searchMessagesFilterAnimation();

  static const std::int32_t ID = -155713339;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchMessagesFilterAudio final : public SearchMessagesFilter {
 public:

  searchMessagesFilterAudio();

  static const std::int32_t ID = 867505275;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchMessagesFilterDocument final : public SearchMessagesFilter {
 public:

  searchMessagesFilterDocument();

  static const std::int32_t ID = 1526331215;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchMessagesFilterPhoto final : public SearchMessagesFilter {
 public:

  searchMessagesFilterPhoto();

  static const std::int32_t ID = 925932293;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchMessagesFilterVideo final : public SearchMessagesFilter {
 public:

  searchMessagesFilterVideo();

  static const std::int32_t ID = 115538222;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchMessagesFilterVoiceNote final : public SearchMessagesFilter {
 public:

  searchMessagesFilterVoiceNote();

  static const std::int32_t ID = 1841439357;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchMessagesFilterPhotoAndVideo final : public SearchMessagesFilter {
 public:

  searchMessagesFilterPhotoAndVideo();

  static const std::int32_t ID = 1352130963;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchMessagesFilterUrl final : public SearchMessagesFilter {
 public:

  searchMessagesFilterUrl();

  static const std::int32_t ID = -1828724341;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchMessagesFilterChatPhoto final : public SearchMessagesFilter {
 public:

  searchMessagesFilterChatPhoto();

  static const std::int32_t ID = -1247751329;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchMessagesFilterVideoNote final : public SearchMessagesFilter {
 public:

  searchMessagesFilterVideoNote();

  static const std::int32_t ID = 564323321;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchMessagesFilterVoiceAndVideoNote final : public SearchMessagesFilter {
 public:

  searchMessagesFilterVoiceAndVideoNote();

  static const std::int32_t ID = 664174819;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchMessagesFilterMention final : public SearchMessagesFilter {
 public:

  searchMessagesFilterMention();

  static const std::int32_t ID = 2001258652;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchMessagesFilterUnreadMention final : public SearchMessagesFilter {
 public:

  searchMessagesFilterUnreadMention();

  static const std::int32_t ID = -95769149;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchMessagesFilterUnreadReaction final : public SearchMessagesFilter {
 public:

  searchMessagesFilterUnreadReaction();

  static const std::int32_t ID = -1379651328;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchMessagesFilterFailedToSend final : public SearchMessagesFilter {
 public:

  searchMessagesFilterFailedToSend();

  static const std::int32_t ID = -596322564;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchMessagesFilterPinned final : public SearchMessagesFilter {
 public:

  searchMessagesFilterPinned();

  static const std::int32_t ID = 371805512;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class seconds final : public Object {
 public:
  double seconds_;

  seconds();

  explicit seconds(double seconds_);

  static const std::int32_t ID = 959899022;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secretChat final : public Object {
 public:
  int32 id_;
  int53 user_id_;
  object_ptr<SecretChatState> state_;
  bool is_outbound_;
  bytes key_hash_;
  int32 layer_;

  secretChat();

  secretChat(int32 id_, int53 user_id_, object_ptr<SecretChatState> &&state_, bool is_outbound_, bytes const &key_hash_, int32 layer_);

  static const std::int32_t ID = -676918325;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class SecretChatState: public Object {
 public:
};

class secretChatStatePending final : public SecretChatState {
 public:

  secretChatStatePending();

  static const std::int32_t ID = -1637050756;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secretChatStateReady final : public SecretChatState {
 public:

  secretChatStateReady();

  static const std::int32_t ID = -1611352087;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secretChatStateClosed final : public SecretChatState {
 public:

  secretChatStateClosed();

  static const std::int32_t ID = -1945106707;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sentWebAppMessage final : public Object {
 public:
  string inline_message_id_;

  sentWebAppMessage();

  explicit sentWebAppMessage(string const &inline_message_id_);

  static const std::int32_t ID = 1243934400;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class session final : public Object {
 public:
  int64 id_;
  bool is_current_;
  bool is_password_pending_;
  bool can_accept_secret_chats_;
  bool can_accept_calls_;
  object_ptr<SessionType> type_;
  int32 api_id_;
  string application_name_;
  string application_version_;
  bool is_official_application_;
  string device_model_;
  string platform_;
  string system_version_;
  int32 log_in_date_;
  int32 last_active_date_;
  string ip_;
  string country_;
  string region_;

  session();

  session(int64 id_, bool is_current_, bool is_password_pending_, bool can_accept_secret_chats_, bool can_accept_calls_, object_ptr<SessionType> &&type_, int32 api_id_, string const &application_name_, string const &application_version_, bool is_official_application_, string const &device_model_, string const &platform_, string const &system_version_, int32 log_in_date_, int32 last_active_date_, string const &ip_, string const &country_, string const &region_);

  static const std::int32_t ID = -1961323642;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class SessionType: public Object {
 public:
};

class sessionTypeAndroid final : public SessionType {
 public:

  sessionTypeAndroid();

  static const std::int32_t ID = -2071764840;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sessionTypeApple final : public SessionType {
 public:

  sessionTypeApple();

  static const std::int32_t ID = -1818635701;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sessionTypeBrave final : public SessionType {
 public:

  sessionTypeBrave();

  static const std::int32_t ID = -1216812563;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sessionTypeChrome final : public SessionType {
 public:

  sessionTypeChrome();

  static const std::int32_t ID = 1573464425;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sessionTypeEdge final : public SessionType {
 public:

  sessionTypeEdge();

  static const std::int32_t ID = -538916005;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sessionTypeFirefox final : public SessionType {
 public:

  sessionTypeFirefox();

  static const std::int32_t ID = 2122579364;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sessionTypeIpad final : public SessionType {
 public:

  sessionTypeIpad();

  static const std::int32_t ID = 1294647023;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sessionTypeIphone final : public SessionType {
 public:

  sessionTypeIphone();

  static const std::int32_t ID = 97616573;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sessionTypeLinux final : public SessionType {
 public:

  sessionTypeLinux();

  static const std::int32_t ID = -1487422871;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sessionTypeMac final : public SessionType {
 public:

  sessionTypeMac();

  static const std::int32_t ID = -612250975;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sessionTypeOpera final : public SessionType {
 public:

  sessionTypeOpera();

  static const std::int32_t ID = -1463673734;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sessionTypeSafari final : public SessionType {
 public:

  sessionTypeSafari();

  static const std::int32_t ID = 710646873;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sessionTypeUbuntu final : public SessionType {
 public:

  sessionTypeUbuntu();

  static const std::int32_t ID = 1569680069;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sessionTypeUnknown final : public SessionType {
 public:

  sessionTypeUnknown();

  static const std::int32_t ID = 233926704;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sessionTypeVivaldi final : public SessionType {
 public:

  sessionTypeVivaldi();

  static const std::int32_t ID = 1120503279;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sessionTypeWindows final : public SessionType {
 public:

  sessionTypeWindows();

  static const std::int32_t ID = -1676512600;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sessionTypeXbox final : public SessionType {
 public:

  sessionTypeXbox();

  static const std::int32_t ID = 1856216492;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sessions final : public Object {
 public:
  array<object_ptr<session>> sessions_;
  int32 inactive_session_ttl_days_;

  sessions();

  sessions(array<object_ptr<session>> &&sessions_, int32 inactive_session_ttl_days_);

  static const std::int32_t ID = 842912274;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class shippingOption final : public Object {
 public:
  string id_;
  string title_;
  array<object_ptr<labeledPricePart>> price_parts_;

  shippingOption();

  shippingOption(string const &id_, string const &title_, array<object_ptr<labeledPricePart>> &&price_parts_);

  static const std::int32_t ID = 1425690001;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class SpeechRecognitionResult: public Object {
 public:
};

class speechRecognitionResultPending final : public SpeechRecognitionResult {
 public:
  string partial_text_;

  speechRecognitionResultPending();

  explicit speechRecognitionResultPending(string const &partial_text_);

  static const std::int32_t ID = -1631810048;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class speechRecognitionResultText final : public SpeechRecognitionResult {
 public:
  string text_;

  speechRecognitionResultText();

  explicit speechRecognitionResultText(string const &text_);

  static const std::int32_t ID = -2132377123;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class speechRecognitionResultError final : public SpeechRecognitionResult {
 public:
  object_ptr<error> error_;

  speechRecognitionResultError();

  explicit speechRecognitionResultError(object_ptr<error> &&error_);

  static const std::int32_t ID = 164774908;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sponsoredMessage final : public Object {
 public:
  int53 message_id_;
  bool is_recommended_;
  int53 sponsor_chat_id_;
  object_ptr<chatInviteLinkInfo> sponsor_chat_info_;
  bool show_chat_photo_;
  object_ptr<InternalLinkType> link_;
  object_ptr<MessageContent> content_;

  sponsoredMessage();

  sponsoredMessage(int53 message_id_, bool is_recommended_, int53 sponsor_chat_id_, object_ptr<chatInviteLinkInfo> &&sponsor_chat_info_, bool show_chat_photo_, object_ptr<InternalLinkType> &&link_, object_ptr<MessageContent> &&content_);

  static const std::int32_t ID = -613319051;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sponsoredMessages final : public Object {
 public:
  array<object_ptr<sponsoredMessage>> messages_;
  int32 messages_between_;

  sponsoredMessages();

  sponsoredMessages(array<object_ptr<sponsoredMessage>> &&messages_, int32 messages_between_);

  static const std::int32_t ID = -537674389;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class StatisticalGraph: public Object {
 public:
};

class statisticalGraphData final : public StatisticalGraph {
 public:
  string json_data_;
  string zoom_token_;

  statisticalGraphData();

  statisticalGraphData(string const &json_data_, string const &zoom_token_);

  static const std::int32_t ID = -1988940244;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class statisticalGraphAsync final : public StatisticalGraph {
 public:
  string token_;

  statisticalGraphAsync();

  explicit statisticalGraphAsync(string const &token_);

  static const std::int32_t ID = 435891103;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class statisticalGraphError final : public StatisticalGraph {
 public:
  string error_message_;

  statisticalGraphError();

  explicit statisticalGraphError(string const &error_message_);

  static const std::int32_t ID = -1006788526;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class statisticalValue final : public Object {
 public:
  double value_;
  double previous_value_;
  double growth_rate_percentage_;

  statisticalValue();

  statisticalValue(double value_, double previous_value_, double growth_rate_percentage_);

  static const std::int32_t ID = 1651337846;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sticker final : public Object {
 public:
  int64 set_id_;
  int32 width_;
  int32 height_;
  string emoji_;
  object_ptr<StickerFormat> format_;
  object_ptr<StickerType> type_;
  object_ptr<maskPosition> mask_position_;
  int64 custom_emoji_id_;
  array<object_ptr<closedVectorPath>> outline_;
  object_ptr<thumbnail> thumbnail_;
  bool is_premium_;
  object_ptr<file> premium_animation_;
  object_ptr<file> sticker_;

  sticker();

  sticker(int64 set_id_, int32 width_, int32 height_, string const &emoji_, object_ptr<StickerFormat> &&format_, object_ptr<StickerType> &&type_, object_ptr<maskPosition> &&mask_position_, int64 custom_emoji_id_, array<object_ptr<closedVectorPath>> &&outline_, object_ptr<thumbnail> &&thumbnail_, bool is_premium_, object_ptr<file> &&premium_animation_, object_ptr<file> &&sticker_);

  static const std::int32_t ID = -1108220879;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class StickerFormat: public Object {
 public:
};

class stickerFormatWebp final : public StickerFormat {
 public:

  stickerFormatWebp();

  static const std::int32_t ID = -2123043040;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class stickerFormatTgs final : public StickerFormat {
 public:

  stickerFormatTgs();

  static const std::int32_t ID = 1614588662;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class stickerFormatWebm final : public StickerFormat {
 public:

  stickerFormatWebm();

  static const std::int32_t ID = -2070162097;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class stickerSet final : public Object {
 public:
  int64 id_;
  string title_;
  string name_;
  object_ptr<thumbnail> thumbnail_;
  array<object_ptr<closedVectorPath>> thumbnail_outline_;
  bool is_installed_;
  bool is_archived_;
  bool is_official_;
  object_ptr<StickerFormat> sticker_format_;
  object_ptr<StickerType> sticker_type_;
  bool is_viewed_;
  array<object_ptr<sticker>> stickers_;
  array<object_ptr<emojis>> emojis_;

  stickerSet();

  stickerSet(int64 id_, string const &title_, string const &name_, object_ptr<thumbnail> &&thumbnail_, array<object_ptr<closedVectorPath>> &&thumbnail_outline_, bool is_installed_, bool is_archived_, bool is_official_, object_ptr<StickerFormat> &&sticker_format_, object_ptr<StickerType> &&sticker_type_, bool is_viewed_, array<object_ptr<sticker>> &&stickers_, array<object_ptr<emojis>> &&emojis_);

  static const std::int32_t ID = 1899632064;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class stickerSetInfo final : public Object {
 public:
  int64 id_;
  string title_;
  string name_;
  object_ptr<thumbnail> thumbnail_;
  array<object_ptr<closedVectorPath>> thumbnail_outline_;
  bool is_installed_;
  bool is_archived_;
  bool is_official_;
  object_ptr<StickerFormat> sticker_format_;
  object_ptr<StickerType> sticker_type_;
  bool is_viewed_;
  int32 size_;
  array<object_ptr<sticker>> covers_;

  stickerSetInfo();

  stickerSetInfo(int64 id_, string const &title_, string const &name_, object_ptr<thumbnail> &&thumbnail_, array<object_ptr<closedVectorPath>> &&thumbnail_outline_, bool is_installed_, bool is_archived_, bool is_official_, object_ptr<StickerFormat> &&sticker_format_, object_ptr<StickerType> &&sticker_type_, bool is_viewed_, int32 size_, array<object_ptr<sticker>> &&covers_);

  static const std::int32_t ID = 745543121;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class stickerSets final : public Object {
 public:
  int32 total_count_;
  array<object_ptr<stickerSetInfo>> sets_;

  stickerSets();

  stickerSets(int32 total_count_, array<object_ptr<stickerSetInfo>> &&sets_);

  static const std::int32_t ID = -1883828812;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class StickerType: public Object {
 public:
};

class stickerTypeRegular final : public StickerType {
 public:

  stickerTypeRegular();

  static const std::int32_t ID = 56345973;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class stickerTypeMask final : public StickerType {
 public:

  stickerTypeMask();

  static const std::int32_t ID = -1765394796;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class stickerTypeCustomEmoji final : public StickerType {
 public:

  stickerTypeCustomEmoji();

  static const std::int32_t ID = -120752249;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class stickers final : public Object {
 public:
  array<object_ptr<sticker>> stickers_;

  stickers();

  explicit stickers(array<object_ptr<sticker>> &&stickers_);

  static const std::int32_t ID = 1974859260;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class storageStatistics final : public Object {
 public:
  int53 size_;
  int32 count_;
  array<object_ptr<storageStatisticsByChat>> by_chat_;

  storageStatistics();

  storageStatistics(int53 size_, int32 count_, array<object_ptr<storageStatisticsByChat>> &&by_chat_);

  static const std::int32_t ID = 217237013;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class storageStatisticsByChat final : public Object {
 public:
  int53 chat_id_;
  int53 size_;
  int32 count_;
  array<object_ptr<storageStatisticsByFileType>> by_file_type_;

  storageStatisticsByChat();

  storageStatisticsByChat(int53 chat_id_, int53 size_, int32 count_, array<object_ptr<storageStatisticsByFileType>> &&by_file_type_);

  static const std::int32_t ID = 635434531;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class storageStatisticsByFileType final : public Object {
 public:
  object_ptr<FileType> file_type_;
  int53 size_;
  int32 count_;

  storageStatisticsByFileType();

  storageStatisticsByFileType(object_ptr<FileType> &&file_type_, int53 size_, int32 count_);

  static const std::int32_t ID = 714012840;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class storageStatisticsFast final : public Object {
 public:
  int53 files_size_;
  int32 file_count_;
  int53 database_size_;
  int53 language_pack_database_size_;
  int53 log_size_;

  storageStatisticsFast();

  storageStatisticsFast(int53 files_size_, int32 file_count_, int53 database_size_, int53 language_pack_database_size_, int53 log_size_);

  static const std::int32_t ID = -884922271;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class StorePaymentPurpose: public Object {
 public:
};

class storePaymentPurposePremiumSubscription final : public StorePaymentPurpose {
 public:
  bool is_restore_;

  storePaymentPurposePremiumSubscription();

  explicit storePaymentPurposePremiumSubscription(bool is_restore_);

  static const std::int32_t ID = -1497906096;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class storePaymentPurposeGiftedPremium final : public StorePaymentPurpose {
 public:
  int53 user_id_;
  string currency_;
  int53 amount_;

  storePaymentPurposeGiftedPremium();

  storePaymentPurposeGiftedPremium(int53 user_id_, string const &currency_, int53 amount_);

  static const std::int32_t ID = 1916846289;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class SuggestedAction: public Object {
 public:
};

class suggestedActionEnableArchiveAndMuteNewChats final : public SuggestedAction {
 public:

  suggestedActionEnableArchiveAndMuteNewChats();

  static const std::int32_t ID = 2017586255;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class suggestedActionCheckPassword final : public SuggestedAction {
 public:

  suggestedActionCheckPassword();

  static const std::int32_t ID = 1910534839;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class suggestedActionCheckPhoneNumber final : public SuggestedAction {
 public:

  suggestedActionCheckPhoneNumber();

  static const std::int32_t ID = 648771563;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class suggestedActionViewChecksHint final : public SuggestedAction {
 public:

  suggestedActionViewChecksHint();

  static const std::int32_t ID = 891303239;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class suggestedActionConvertToBroadcastGroup final : public SuggestedAction {
 public:
  int53 supergroup_id_;

  suggestedActionConvertToBroadcastGroup();

  explicit suggestedActionConvertToBroadcastGroup(int53 supergroup_id_);

  static const std::int32_t ID = -965071304;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class suggestedActionSetPassword final : public SuggestedAction {
 public:
  int32 authorization_delay_;

  suggestedActionSetPassword();

  explicit suggestedActionSetPassword(int32 authorization_delay_);

  static const std::int32_t ID = 1863613848;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class supergroup final : public Object {
 public:
  int53 id_;
  object_ptr<usernames> usernames_;
  int32 date_;
  object_ptr<ChatMemberStatus> status_;
  int32 member_count_;
  bool has_linked_chat_;
  bool has_location_;
  bool sign_messages_;
  bool join_to_send_messages_;
  bool join_by_request_;
  bool is_slow_mode_enabled_;
  bool is_channel_;
  bool is_broadcast_group_;
  bool is_forum_;
  bool is_verified_;
  string restriction_reason_;
  bool is_scam_;
  bool is_fake_;

  supergroup();

  supergroup(int53 id_, object_ptr<usernames> &&usernames_, int32 date_, object_ptr<ChatMemberStatus> &&status_, int32 member_count_, bool has_linked_chat_, bool has_location_, bool sign_messages_, bool join_to_send_messages_, bool join_by_request_, bool is_slow_mode_enabled_, bool is_channel_, bool is_broadcast_group_, bool is_forum_, bool is_verified_, string const &restriction_reason_, bool is_scam_, bool is_fake_);

  static const std::int32_t ID = -2002240425;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class supergroupFullInfo final : public Object {
 public:
  object_ptr<chatPhoto> photo_;
  string description_;
  int32 member_count_;
  int32 administrator_count_;
  int32 restricted_count_;
  int32 banned_count_;
  int53 linked_chat_id_;
  int32 slow_mode_delay_;
  double slow_mode_delay_expires_in_;
  bool can_get_members_;
  bool can_set_username_;
  bool can_set_sticker_set_;
  bool can_set_location_;
  bool can_get_statistics_;
  bool is_all_history_available_;
  int64 sticker_set_id_;
  object_ptr<chatLocation> location_;
  object_ptr<chatInviteLink> invite_link_;
  array<object_ptr<botCommands>> bot_commands_;
  int53 upgraded_from_basic_group_id_;
  int53 upgraded_from_max_message_id_;

  supergroupFullInfo();

  supergroupFullInfo(object_ptr<chatPhoto> &&photo_, string const &description_, int32 member_count_, int32 administrator_count_, int32 restricted_count_, int32 banned_count_, int53 linked_chat_id_, int32 slow_mode_delay_, double slow_mode_delay_expires_in_, bool can_get_members_, bool can_set_username_, bool can_set_sticker_set_, bool can_set_location_, bool can_get_statistics_, bool is_all_history_available_, int64 sticker_set_id_, object_ptr<chatLocation> &&location_, object_ptr<chatInviteLink> &&invite_link_, array<object_ptr<botCommands>> &&bot_commands_, int53 upgraded_from_basic_group_id_, int53 upgraded_from_max_message_id_);

  static const std::int32_t ID = -1035719349;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class SupergroupMembersFilter: public Object {
 public:
};

class supergroupMembersFilterRecent final : public SupergroupMembersFilter {
 public:

  supergroupMembersFilterRecent();

  static const std::int32_t ID = 1178199509;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class supergroupMembersFilterContacts final : public SupergroupMembersFilter {
 public:
  string query_;

  supergroupMembersFilterContacts();

  explicit supergroupMembersFilterContacts(string const &query_);

  static const std::int32_t ID = -1282910856;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class supergroupMembersFilterAdministrators final : public SupergroupMembersFilter {
 public:

  supergroupMembersFilterAdministrators();

  static const std::int32_t ID = -2097380265;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class supergroupMembersFilterSearch final : public SupergroupMembersFilter {
 public:
  string query_;

  supergroupMembersFilterSearch();

  explicit supergroupMembersFilterSearch(string const &query_);

  static const std::int32_t ID = -1696358469;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class supergroupMembersFilterRestricted final : public SupergroupMembersFilter {
 public:
  string query_;

  supergroupMembersFilterRestricted();

  explicit supergroupMembersFilterRestricted(string const &query_);

  static const std::int32_t ID = -1107800034;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class supergroupMembersFilterBanned final : public SupergroupMembersFilter {
 public:
  string query_;

  supergroupMembersFilterBanned();

  explicit supergroupMembersFilterBanned(string const &query_);

  static const std::int32_t ID = -1210621683;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class supergroupMembersFilterMention final : public SupergroupMembersFilter {
 public:
  string query_;
  int53 message_thread_id_;

  supergroupMembersFilterMention();

  supergroupMembersFilterMention(string const &query_, int53 message_thread_id_);

  static const std::int32_t ID = 947915036;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class supergroupMembersFilterBots final : public SupergroupMembersFilter {
 public:

  supergroupMembersFilterBots();

  static const std::int32_t ID = 492138918;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class tMeUrl final : public Object {
 public:
  string url_;
  object_ptr<TMeUrlType> type_;

  tMeUrl();

  tMeUrl(string const &url_, object_ptr<TMeUrlType> &&type_);

  static const std::int32_t ID = -1140786622;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class TMeUrlType: public Object {
 public:
};

class tMeUrlTypeUser final : public TMeUrlType {
 public:
  int53 user_id_;

  tMeUrlTypeUser();

  explicit tMeUrlTypeUser(int53 user_id_);

  static const std::int32_t ID = 125336602;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class tMeUrlTypeSupergroup final : public TMeUrlType {
 public:
  int53 supergroup_id_;

  tMeUrlTypeSupergroup();

  explicit tMeUrlTypeSupergroup(int53 supergroup_id_);

  static const std::int32_t ID = -1353369944;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class tMeUrlTypeChatInvite final : public TMeUrlType {
 public:
  object_ptr<chatInviteLinkInfo> info_;

  tMeUrlTypeChatInvite();

  explicit tMeUrlTypeChatInvite(object_ptr<chatInviteLinkInfo> &&info_);

  static const std::int32_t ID = 313907785;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class tMeUrlTypeStickerSet final : public TMeUrlType {
 public:
  int64 sticker_set_id_;

  tMeUrlTypeStickerSet();

  explicit tMeUrlTypeStickerSet(int64 sticker_set_id_);

  static const std::int32_t ID = 1602473196;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class tMeUrls final : public Object {
 public:
  array<object_ptr<tMeUrl>> urls_;

  tMeUrls();

  explicit tMeUrls(array<object_ptr<tMeUrl>> &&urls_);

  static const std::int32_t ID = -1130595098;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class TargetChat: public Object {
 public:
};

class targetChatCurrent final : public TargetChat {
 public:

  targetChatCurrent();

  static const std::int32_t ID = -416689904;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class targetChatChosen final : public TargetChat {
 public:
  bool allow_user_chats_;
  bool allow_bot_chats_;
  bool allow_group_chats_;
  bool allow_channel_chats_;

  targetChatChosen();

  targetChatChosen(bool allow_user_chats_, bool allow_bot_chats_, bool allow_group_chats_, bool allow_channel_chats_);

  static const std::int32_t ID = -307442990;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class targetChatInternalLink final : public TargetChat {
 public:
  object_ptr<InternalLinkType> link_;

  targetChatInternalLink();

  explicit targetChatInternalLink(object_ptr<InternalLinkType> &&link_);

  static const std::int32_t ID = -579301408;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class temporaryPasswordState final : public Object {
 public:
  bool has_password_;
  int32 valid_for_;

  temporaryPasswordState();

  temporaryPasswordState(bool has_password_, int32 valid_for_);

  static const std::int32_t ID = 939837410;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class termsOfService final : public Object {
 public:
  object_ptr<formattedText> text_;
  int32 min_user_age_;
  bool show_popup_;

  termsOfService();

  termsOfService(object_ptr<formattedText> &&text_, int32 min_user_age_, bool show_popup_);

  static const std::int32_t ID = 739422597;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testBytes final : public Object {
 public:
  bytes value_;

  testBytes();

  explicit testBytes(bytes const &value_);

  static const std::int32_t ID = -1541225250;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testInt final : public Object {
 public:
  int32 value_;

  testInt();

  explicit testInt(int32 value_);

  static const std::int32_t ID = -574804983;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testString final : public Object {
 public:
  string value_;

  testString();

  explicit testString(string const &value_);

  static const std::int32_t ID = -27891572;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testVectorInt final : public Object {
 public:
  array<int32> value_;

  testVectorInt();

  explicit testVectorInt(array<int32> &&value_);

  static const std::int32_t ID = 593682027;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testVectorIntObject final : public Object {
 public:
  array<object_ptr<testInt>> value_;

  testVectorIntObject();

  explicit testVectorIntObject(array<object_ptr<testInt>> &&value_);

  static const std::int32_t ID = 125891546;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testVectorString final : public Object {
 public:
  array<string> value_;

  testVectorString();

  explicit testVectorString(array<string> &&value_);

  static const std::int32_t ID = 79339995;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testVectorStringObject final : public Object {
 public:
  array<object_ptr<testString>> value_;

  testVectorStringObject();

  explicit testVectorStringObject(array<object_ptr<testString>> &&value_);

  static const std::int32_t ID = 80780537;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class text final : public Object {
 public:
  string text_;

  text();

  explicit text(string const &text_);

  static const std::int32_t ID = 578181272;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntities final : public Object {
 public:
  array<object_ptr<textEntity>> entities_;

  textEntities();

  explicit textEntities(array<object_ptr<textEntity>> &&entities_);

  static const std::int32_t ID = -933199172;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntity final : public Object {
 public:
  int32 offset_;
  int32 length_;
  object_ptr<TextEntityType> type_;

  textEntity();

  textEntity(int32 offset_, int32 length_, object_ptr<TextEntityType> &&type_);

  static const std::int32_t ID = -1951688280;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class TextEntityType: public Object {
 public:
};

class textEntityTypeMention final : public TextEntityType {
 public:

  textEntityTypeMention();

  static const std::int32_t ID = 934535013;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntityTypeHashtag final : public TextEntityType {
 public:

  textEntityTypeHashtag();

  static const std::int32_t ID = -1023958307;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntityTypeCashtag final : public TextEntityType {
 public:

  textEntityTypeCashtag();

  static const std::int32_t ID = 1222915915;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntityTypeBotCommand final : public TextEntityType {
 public:

  textEntityTypeBotCommand();

  static const std::int32_t ID = -1150997581;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntityTypeUrl final : public TextEntityType {
 public:

  textEntityTypeUrl();

  static const std::int32_t ID = -1312762756;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntityTypeEmailAddress final : public TextEntityType {
 public:

  textEntityTypeEmailAddress();

  static const std::int32_t ID = 1425545249;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntityTypePhoneNumber final : public TextEntityType {
 public:

  textEntityTypePhoneNumber();

  static const std::int32_t ID = -1160140246;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntityTypeBankCardNumber final : public TextEntityType {
 public:

  textEntityTypeBankCardNumber();

  static const std::int32_t ID = 105986320;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntityTypeBold final : public TextEntityType {
 public:

  textEntityTypeBold();

  static const std::int32_t ID = -1128210000;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntityTypeItalic final : public TextEntityType {
 public:

  textEntityTypeItalic();

  static const std::int32_t ID = -118253987;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntityTypeUnderline final : public TextEntityType {
 public:

  textEntityTypeUnderline();

  static const std::int32_t ID = 792317842;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntityTypeStrikethrough final : public TextEntityType {
 public:

  textEntityTypeStrikethrough();

  static const std::int32_t ID = 961529082;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntityTypeSpoiler final : public TextEntityType {
 public:

  textEntityTypeSpoiler();

  static const std::int32_t ID = 544019899;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntityTypeCode final : public TextEntityType {
 public:

  textEntityTypeCode();

  static const std::int32_t ID = -974534326;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntityTypePre final : public TextEntityType {
 public:

  textEntityTypePre();

  static const std::int32_t ID = 1648958606;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntityTypePreCode final : public TextEntityType {
 public:
  string language_;

  textEntityTypePreCode();

  explicit textEntityTypePreCode(string const &language_);

  static const std::int32_t ID = -945325397;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntityTypeTextUrl final : public TextEntityType {
 public:
  string url_;

  textEntityTypeTextUrl();

  explicit textEntityTypeTextUrl(string const &url_);

  static const std::int32_t ID = 445719651;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntityTypeMentionName final : public TextEntityType {
 public:
  int53 user_id_;

  textEntityTypeMentionName();

  explicit textEntityTypeMentionName(int53 user_id_);

  static const std::int32_t ID = -1570974289;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntityTypeCustomEmoji final : public TextEntityType {
 public:
  int64 custom_emoji_id_;

  textEntityTypeCustomEmoji();

  explicit textEntityTypeCustomEmoji(int64 custom_emoji_id_);

  static const std::int32_t ID = 1724820677;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEntityTypeMediaTimestamp final : public TextEntityType {
 public:
  int32 media_timestamp_;

  textEntityTypeMediaTimestamp();

  explicit textEntityTypeMediaTimestamp(int32 media_timestamp_);

  static const std::int32_t ID = -1841898992;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class TextParseMode: public Object {
 public:
};

class textParseModeMarkdown final : public TextParseMode {
 public:
  int32 version_;

  textParseModeMarkdown();

  explicit textParseModeMarkdown(int32 version_);

  static const std::int32_t ID = 360073407;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textParseModeHTML final : public TextParseMode {
 public:

  textParseModeHTML();

  static const std::int32_t ID = 1660208627;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class themeParameters final : public Object {
 public:
  int32 background_color_;
  int32 secondary_background_color_;
  int32 text_color_;
  int32 hint_color_;
  int32 link_color_;
  int32 button_color_;
  int32 button_text_color_;

  themeParameters();

  themeParameters(int32 background_color_, int32 secondary_background_color_, int32 text_color_, int32 hint_color_, int32 link_color_, int32 button_color_, int32 button_text_color_);

  static const std::int32_t ID = -644809106;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class themeSettings final : public Object {
 public:
  int32 accent_color_;
  object_ptr<background> background_;
  object_ptr<BackgroundFill> outgoing_message_fill_;
  bool animate_outgoing_message_fill_;
  int32 outgoing_message_accent_color_;

  themeSettings();

  themeSettings(int32 accent_color_, object_ptr<background> &&background_, object_ptr<BackgroundFill> &&outgoing_message_fill_, bool animate_outgoing_message_fill_, int32 outgoing_message_accent_color_);

  static const std::int32_t ID = -62120942;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class thumbnail final : public Object {
 public:
  object_ptr<ThumbnailFormat> format_;
  int32 width_;
  int32 height_;
  object_ptr<file> file_;

  thumbnail();

  thumbnail(object_ptr<ThumbnailFormat> &&format_, int32 width_, int32 height_, object_ptr<file> &&file_);

  static const std::int32_t ID = 1243275371;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ThumbnailFormat: public Object {
 public:
};

class thumbnailFormatJpeg final : public ThumbnailFormat {
 public:

  thumbnailFormatJpeg();

  static const std::int32_t ID = -653503352;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class thumbnailFormatGif final : public ThumbnailFormat {
 public:

  thumbnailFormatGif();

  static const std::int32_t ID = 1252205962;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class thumbnailFormatMpeg4 final : public ThumbnailFormat {
 public:

  thumbnailFormatMpeg4();

  static const std::int32_t ID = 278616062;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class thumbnailFormatPng final : public ThumbnailFormat {
 public:

  thumbnailFormatPng();

  static const std::int32_t ID = 1577490421;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class thumbnailFormatTgs final : public ThumbnailFormat {
 public:

  thumbnailFormatTgs();

  static const std::int32_t ID = 1315522642;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class thumbnailFormatWebm final : public ThumbnailFormat {
 public:

  thumbnailFormatWebm();

  static const std::int32_t ID = -660084953;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class thumbnailFormatWebp final : public ThumbnailFormat {
 public:

  thumbnailFormatWebp();

  static const std::int32_t ID = -53588974;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class TopChatCategory: public Object {
 public:
};

class topChatCategoryUsers final : public TopChatCategory {
 public:

  topChatCategoryUsers();

  static const std::int32_t ID = 1026706816;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class topChatCategoryBots final : public TopChatCategory {
 public:

  topChatCategoryBots();

  static const std::int32_t ID = -1577129195;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class topChatCategoryGroups final : public TopChatCategory {
 public:

  topChatCategoryGroups();

  static const std::int32_t ID = 1530056846;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class topChatCategoryChannels final : public TopChatCategory {
 public:

  topChatCategoryChannels();

  static const std::int32_t ID = -500825885;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class topChatCategoryInlineBots final : public TopChatCategory {
 public:

  topChatCategoryInlineBots();

  static const std::int32_t ID = 377023356;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class topChatCategoryCalls final : public TopChatCategory {
 public:

  topChatCategoryCalls();

  static const std::int32_t ID = 356208861;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class topChatCategoryForwardChats final : public TopChatCategory {
 public:

  topChatCategoryForwardChats();

  static const std::int32_t ID = 1695922133;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class trendingStickerSets final : public Object {
 public:
  int32 total_count_;
  array<object_ptr<stickerSetInfo>> sets_;
  bool is_premium_;

  trendingStickerSets();

  trendingStickerSets(int32 total_count_, array<object_ptr<stickerSetInfo>> &&sets_, bool is_premium_);

  static const std::int32_t ID = 41028940;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class unreadReaction final : public Object {
 public:
  object_ptr<ReactionType> type_;
  object_ptr<MessageSender> sender_id_;
  bool is_big_;

  unreadReaction();

  unreadReaction(object_ptr<ReactionType> &&type_, object_ptr<MessageSender> &&sender_id_, bool is_big_);

  static const std::int32_t ID = -1940178046;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class Update: public Object {
 public:
};

class updateAuthorizationState final : public Update {
 public:
  object_ptr<AuthorizationState> authorization_state_;

  updateAuthorizationState();

  explicit updateAuthorizationState(object_ptr<AuthorizationState> &&authorization_state_);

  static const std::int32_t ID = 1622347490;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateNewMessage final : public Update {
 public:
  object_ptr<message> message_;

  updateNewMessage();

  explicit updateNewMessage(object_ptr<message> &&message_);

  static const std::int32_t ID = -563105266;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateMessageSendAcknowledged final : public Update {
 public:
  int53 chat_id_;
  int53 message_id_;

  updateMessageSendAcknowledged();

  updateMessageSendAcknowledged(int53 chat_id_, int53 message_id_);

  static const std::int32_t ID = 1302843961;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateMessageSendSucceeded final : public Update {
 public:
  object_ptr<message> message_;
  int53 old_message_id_;

  updateMessageSendSucceeded();

  updateMessageSendSucceeded(object_ptr<message> &&message_, int53 old_message_id_);

  static const std::int32_t ID = 1815715197;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateMessageSendFailed final : public Update {
 public:
  object_ptr<message> message_;
  int53 old_message_id_;
  int32 error_code_;
  string error_message_;

  updateMessageSendFailed();

  updateMessageSendFailed(object_ptr<message> &&message_, int53 old_message_id_, int32 error_code_, string const &error_message_);

  static const std::int32_t ID = -1032335779;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateMessageContent final : public Update {
 public:
  int53 chat_id_;
  int53 message_id_;
  object_ptr<MessageContent> new_content_;

  updateMessageContent();

  updateMessageContent(int53 chat_id_, int53 message_id_, object_ptr<MessageContent> &&new_content_);

  static const std::int32_t ID = 506903332;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateMessageEdited final : public Update {
 public:
  int53 chat_id_;
  int53 message_id_;
  int32 edit_date_;
  object_ptr<ReplyMarkup> reply_markup_;

  updateMessageEdited();

  updateMessageEdited(int53 chat_id_, int53 message_id_, int32 edit_date_, object_ptr<ReplyMarkup> &&reply_markup_);

  static const std::int32_t ID = -559545626;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateMessageIsPinned final : public Update {
 public:
  int53 chat_id_;
  int53 message_id_;
  bool is_pinned_;

  updateMessageIsPinned();

  updateMessageIsPinned(int53 chat_id_, int53 message_id_, bool is_pinned_);

  static const std::int32_t ID = 1102848829;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateMessageInteractionInfo final : public Update {
 public:
  int53 chat_id_;
  int53 message_id_;
  object_ptr<messageInteractionInfo> interaction_info_;

  updateMessageInteractionInfo();

  updateMessageInteractionInfo(int53 chat_id_, int53 message_id_, object_ptr<messageInteractionInfo> &&interaction_info_);

  static const std::int32_t ID = -1417659394;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateMessageContentOpened final : public Update {
 public:
  int53 chat_id_;
  int53 message_id_;

  updateMessageContentOpened();

  updateMessageContentOpened(int53 chat_id_, int53 message_id_);

  static const std::int32_t ID = -1520523131;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateMessageMentionRead final : public Update {
 public:
  int53 chat_id_;
  int53 message_id_;
  int32 unread_mention_count_;

  updateMessageMentionRead();

  updateMessageMentionRead(int53 chat_id_, int53 message_id_, int32 unread_mention_count_);

  static const std::int32_t ID = -252228282;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateMessageUnreadReactions final : public Update {
 public:
  int53 chat_id_;
  int53 message_id_;
  array<object_ptr<unreadReaction>> unread_reactions_;
  int32 unread_reaction_count_;

  updateMessageUnreadReactions();

  updateMessageUnreadReactions(int53 chat_id_, int53 message_id_, array<object_ptr<unreadReaction>> &&unread_reactions_, int32 unread_reaction_count_);

  static const std::int32_t ID = 942840008;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateMessageLiveLocationViewed final : public Update {
 public:
  int53 chat_id_;
  int53 message_id_;

  updateMessageLiveLocationViewed();

  updateMessageLiveLocationViewed(int53 chat_id_, int53 message_id_);

  static const std::int32_t ID = -1308260971;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateNewChat final : public Update {
 public:
  object_ptr<chat> chat_;

  updateNewChat();

  explicit updateNewChat(object_ptr<chat> &&chat_);

  static const std::int32_t ID = 2075757773;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatTitle final : public Update {
 public:
  int53 chat_id_;
  string title_;

  updateChatTitle();

  updateChatTitle(int53 chat_id_, string const &title_);

  static const std::int32_t ID = -175405660;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatPhoto final : public Update {
 public:
  int53 chat_id_;
  object_ptr<chatPhotoInfo> photo_;

  updateChatPhoto();

  updateChatPhoto(int53 chat_id_, object_ptr<chatPhotoInfo> &&photo_);

  static const std::int32_t ID = -324713921;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatPermissions final : public Update {
 public:
  int53 chat_id_;
  object_ptr<chatPermissions> permissions_;

  updateChatPermissions();

  updateChatPermissions(int53 chat_id_, object_ptr<chatPermissions> &&permissions_);

  static const std::int32_t ID = -1622010003;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatLastMessage final : public Update {
 public:
  int53 chat_id_;
  object_ptr<message> last_message_;
  array<object_ptr<chatPosition>> positions_;

  updateChatLastMessage();

  updateChatLastMessage(int53 chat_id_, object_ptr<message> &&last_message_, array<object_ptr<chatPosition>> &&positions_);

  static const std::int32_t ID = -923244537;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatPosition final : public Update {
 public:
  int53 chat_id_;
  object_ptr<chatPosition> position_;

  updateChatPosition();

  updateChatPosition(int53 chat_id_, object_ptr<chatPosition> &&position_);

  static const std::int32_t ID = -8979849;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatReadInbox final : public Update {
 public:
  int53 chat_id_;
  int53 last_read_inbox_message_id_;
  int32 unread_count_;

  updateChatReadInbox();

  updateChatReadInbox(int53 chat_id_, int53 last_read_inbox_message_id_, int32 unread_count_);

  static const std::int32_t ID = -797952281;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatReadOutbox final : public Update {
 public:
  int53 chat_id_;
  int53 last_read_outbox_message_id_;

  updateChatReadOutbox();

  updateChatReadOutbox(int53 chat_id_, int53 last_read_outbox_message_id_);

  static const std::int32_t ID = 708334213;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatActionBar final : public Update {
 public:
  int53 chat_id_;
  object_ptr<ChatActionBar> action_bar_;

  updateChatActionBar();

  updateChatActionBar(int53 chat_id_, object_ptr<ChatActionBar> &&action_bar_);

  static const std::int32_t ID = -643671870;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatAvailableReactions final : public Update {
 public:
  int53 chat_id_;
  object_ptr<ChatAvailableReactions> available_reactions_;

  updateChatAvailableReactions();

  updateChatAvailableReactions(int53 chat_id_, object_ptr<ChatAvailableReactions> &&available_reactions_);

  static const std::int32_t ID = -1967909895;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatDraftMessage final : public Update {
 public:
  int53 chat_id_;
  object_ptr<draftMessage> draft_message_;
  array<object_ptr<chatPosition>> positions_;

  updateChatDraftMessage();

  updateChatDraftMessage(int53 chat_id_, object_ptr<draftMessage> &&draft_message_, array<object_ptr<chatPosition>> &&positions_);

  static const std::int32_t ID = 1455190380;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatMessageSender final : public Update {
 public:
  int53 chat_id_;
  object_ptr<MessageSender> message_sender_id_;

  updateChatMessageSender();

  updateChatMessageSender(int53 chat_id_, object_ptr<MessageSender> &&message_sender_id_);

  static const std::int32_t ID = 2003849793;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatMessageTtl final : public Update {
 public:
  int53 chat_id_;
  int32 message_ttl_;

  updateChatMessageTtl();

  updateChatMessageTtl(int53 chat_id_, int32 message_ttl_);

  static const std::int32_t ID = -572479112;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatNotificationSettings final : public Update {
 public:
  int53 chat_id_;
  object_ptr<chatNotificationSettings> notification_settings_;

  updateChatNotificationSettings();

  updateChatNotificationSettings(int53 chat_id_, object_ptr<chatNotificationSettings> &&notification_settings_);

  static const std::int32_t ID = -803163050;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatPendingJoinRequests final : public Update {
 public:
  int53 chat_id_;
  object_ptr<chatJoinRequestsInfo> pending_join_requests_;

  updateChatPendingJoinRequests();

  updateChatPendingJoinRequests(int53 chat_id_, object_ptr<chatJoinRequestsInfo> &&pending_join_requests_);

  static const std::int32_t ID = 348578785;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatReplyMarkup final : public Update {
 public:
  int53 chat_id_;
  int53 reply_markup_message_id_;

  updateChatReplyMarkup();

  updateChatReplyMarkup(int53 chat_id_, int53 reply_markup_message_id_);

  static const std::int32_t ID = 1309386144;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatTheme final : public Update {
 public:
  int53 chat_id_;
  string theme_name_;

  updateChatTheme();

  updateChatTheme(int53 chat_id_, string const &theme_name_);

  static const std::int32_t ID = 838063205;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatUnreadMentionCount final : public Update {
 public:
  int53 chat_id_;
  int32 unread_mention_count_;

  updateChatUnreadMentionCount();

  updateChatUnreadMentionCount(int53 chat_id_, int32 unread_mention_count_);

  static const std::int32_t ID = -2131461348;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatUnreadReactionCount final : public Update {
 public:
  int53 chat_id_;
  int32 unread_reaction_count_;

  updateChatUnreadReactionCount();

  updateChatUnreadReactionCount(int53 chat_id_, int32 unread_reaction_count_);

  static const std::int32_t ID = -2124399395;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatVideoChat final : public Update {
 public:
  int53 chat_id_;
  object_ptr<videoChat> video_chat_;

  updateChatVideoChat();

  updateChatVideoChat(int53 chat_id_, object_ptr<videoChat> &&video_chat_);

  static const std::int32_t ID = 637226150;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatDefaultDisableNotification final : public Update {
 public:
  int53 chat_id_;
  bool default_disable_notification_;

  updateChatDefaultDisableNotification();

  updateChatDefaultDisableNotification(int53 chat_id_, bool default_disable_notification_);

  static const std::int32_t ID = 464087707;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatHasProtectedContent final : public Update {
 public:
  int53 chat_id_;
  bool has_protected_content_;

  updateChatHasProtectedContent();

  updateChatHasProtectedContent(int53 chat_id_, bool has_protected_content_);

  static const std::int32_t ID = 1800406811;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatHasScheduledMessages final : public Update {
 public:
  int53 chat_id_;
  bool has_scheduled_messages_;

  updateChatHasScheduledMessages();

  updateChatHasScheduledMessages(int53 chat_id_, bool has_scheduled_messages_);

  static const std::int32_t ID = 2064958167;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatIsBlocked final : public Update {
 public:
  int53 chat_id_;
  bool is_blocked_;

  updateChatIsBlocked();

  updateChatIsBlocked(int53 chat_id_, bool is_blocked_);

  static const std::int32_t ID = -1998946752;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatIsMarkedAsUnread final : public Update {
 public:
  int53 chat_id_;
  bool is_marked_as_unread_;

  updateChatIsMarkedAsUnread();

  updateChatIsMarkedAsUnread(int53 chat_id_, bool is_marked_as_unread_);

  static const std::int32_t ID = 1468347188;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatFilters final : public Update {
 public:
  array<object_ptr<chatFilterInfo>> chat_filters_;
  int32 main_chat_list_position_;

  updateChatFilters();

  updateChatFilters(array<object_ptr<chatFilterInfo>> &&chat_filters_, int32 main_chat_list_position_);

  static const std::int32_t ID = 81316953;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatOnlineMemberCount final : public Update {
 public:
  int53 chat_id_;
  int32 online_member_count_;

  updateChatOnlineMemberCount();

  updateChatOnlineMemberCount(int53 chat_id_, int32 online_member_count_);

  static const std::int32_t ID = 487369373;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateForumTopicInfo final : public Update {
 public:
  int53 chat_id_;
  object_ptr<forumTopicInfo> info_;

  updateForumTopicInfo();

  updateForumTopicInfo(int53 chat_id_, object_ptr<forumTopicInfo> &&info_);

  static const std::int32_t ID = 1802448073;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateScopeNotificationSettings final : public Update {
 public:
  object_ptr<NotificationSettingsScope> scope_;
  object_ptr<scopeNotificationSettings> notification_settings_;

  updateScopeNotificationSettings();

  updateScopeNotificationSettings(object_ptr<NotificationSettingsScope> &&scope_, object_ptr<scopeNotificationSettings> &&notification_settings_);

  static const std::int32_t ID = -1203975309;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateNotification final : public Update {
 public:
  int32 notification_group_id_;
  object_ptr<notification> notification_;

  updateNotification();

  updateNotification(int32 notification_group_id_, object_ptr<notification> &&notification_);

  static const std::int32_t ID = -1897496876;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateNotificationGroup final : public Update {
 public:
  int32 notification_group_id_;
  object_ptr<NotificationGroupType> type_;
  int53 chat_id_;
  int53 notification_settings_chat_id_;
  int64 notification_sound_id_;
  int32 total_count_;
  array<object_ptr<notification>> added_notifications_;
  array<int32> removed_notification_ids_;

  updateNotificationGroup();

  updateNotificationGroup(int32 notification_group_id_, object_ptr<NotificationGroupType> &&type_, int53 chat_id_, int53 notification_settings_chat_id_, int64 notification_sound_id_, int32 total_count_, array<object_ptr<notification>> &&added_notifications_, array<int32> &&removed_notification_ids_);

  static const std::int32_t ID = 1381081378;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateActiveNotifications final : public Update {
 public:
  array<object_ptr<notificationGroup>> groups_;

  updateActiveNotifications();

  explicit updateActiveNotifications(array<object_ptr<notificationGroup>> &&groups_);

  static const std::int32_t ID = -1306672221;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateHavePendingNotifications final : public Update {
 public:
  bool have_delayed_notifications_;
  bool have_unreceived_notifications_;

  updateHavePendingNotifications();

  updateHavePendingNotifications(bool have_delayed_notifications_, bool have_unreceived_notifications_);

  static const std::int32_t ID = 179233243;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateDeleteMessages final : public Update {
 public:
  int53 chat_id_;
  array<int53> message_ids_;
  bool is_permanent_;
  bool from_cache_;

  updateDeleteMessages();

  updateDeleteMessages(int53 chat_id_, array<int53> &&message_ids_, bool is_permanent_, bool from_cache_);

  static const std::int32_t ID = 1669252686;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatAction final : public Update {
 public:
  int53 chat_id_;
  int53 message_thread_id_;
  object_ptr<MessageSender> sender_id_;
  object_ptr<ChatAction> action_;

  updateChatAction();

  updateChatAction(int53 chat_id_, int53 message_thread_id_, object_ptr<MessageSender> &&sender_id_, object_ptr<ChatAction> &&action_);

  static const std::int32_t ID = -1698703832;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateUserStatus final : public Update {
 public:
  int53 user_id_;
  object_ptr<UserStatus> status_;

  updateUserStatus();

  updateUserStatus(int53 user_id_, object_ptr<UserStatus> &&status_);

  static const std::int32_t ID = 958468625;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateUser final : public Update {
 public:
  object_ptr<user> user_;

  updateUser();

  explicit updateUser(object_ptr<user> &&user_);

  static const std::int32_t ID = 1183394041;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateBasicGroup final : public Update {
 public:
  object_ptr<basicGroup> basic_group_;

  updateBasicGroup();

  explicit updateBasicGroup(object_ptr<basicGroup> &&basic_group_);

  static const std::int32_t ID = -1003239581;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateSupergroup final : public Update {
 public:
  object_ptr<supergroup> supergroup_;

  updateSupergroup();

  explicit updateSupergroup(object_ptr<supergroup> &&supergroup_);

  static const std::int32_t ID = -76782300;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateSecretChat final : public Update {
 public:
  object_ptr<secretChat> secret_chat_;

  updateSecretChat();

  explicit updateSecretChat(object_ptr<secretChat> &&secret_chat_);

  static const std::int32_t ID = -1666903253;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateUserFullInfo final : public Update {
 public:
  int53 user_id_;
  object_ptr<userFullInfo> user_full_info_;

  updateUserFullInfo();

  updateUserFullInfo(int53 user_id_, object_ptr<userFullInfo> &&user_full_info_);

  static const std::int32_t ID = -51197161;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateBasicGroupFullInfo final : public Update {
 public:
  int53 basic_group_id_;
  object_ptr<basicGroupFullInfo> basic_group_full_info_;

  updateBasicGroupFullInfo();

  updateBasicGroupFullInfo(int53 basic_group_id_, object_ptr<basicGroupFullInfo> &&basic_group_full_info_);

  static const std::int32_t ID = 1391881151;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateSupergroupFullInfo final : public Update {
 public:
  int53 supergroup_id_;
  object_ptr<supergroupFullInfo> supergroup_full_info_;

  updateSupergroupFullInfo();

  updateSupergroupFullInfo(int53 supergroup_id_, object_ptr<supergroupFullInfo> &&supergroup_full_info_);

  static const std::int32_t ID = 435539214;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateServiceNotification final : public Update {
 public:
  string type_;
  object_ptr<MessageContent> content_;

  updateServiceNotification();

  updateServiceNotification(string const &type_, object_ptr<MessageContent> &&content_);

  static const std::int32_t ID = 1318622637;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateFile final : public Update {
 public:
  object_ptr<file> file_;

  updateFile();

  explicit updateFile(object_ptr<file> &&file_);

  static const std::int32_t ID = 114132831;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateFileGenerationStart final : public Update {
 public:
  int64 generation_id_;
  string original_path_;
  string destination_path_;
  string conversion_;

  updateFileGenerationStart();

  updateFileGenerationStart(int64 generation_id_, string const &original_path_, string const &destination_path_, string const &conversion_);

  static const std::int32_t ID = 216817388;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateFileGenerationStop final : public Update {
 public:
  int64 generation_id_;

  updateFileGenerationStop();

  explicit updateFileGenerationStop(int64 generation_id_);

  static const std::int32_t ID = -1894449685;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateFileDownloads final : public Update {
 public:
  int53 total_size_;
  int32 total_count_;
  int53 downloaded_size_;

  updateFileDownloads();

  updateFileDownloads(int53 total_size_, int32 total_count_, int53 downloaded_size_);

  static const std::int32_t ID = -389213497;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateFileAddedToDownloads final : public Update {
 public:
  object_ptr<fileDownload> file_download_;
  object_ptr<downloadedFileCounts> counts_;

  updateFileAddedToDownloads();

  updateFileAddedToDownloads(object_ptr<fileDownload> &&file_download_, object_ptr<downloadedFileCounts> &&counts_);

  static const std::int32_t ID = 1609929242;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateFileDownload final : public Update {
 public:
  int32 file_id_;
  int32 complete_date_;
  bool is_paused_;
  object_ptr<downloadedFileCounts> counts_;

  updateFileDownload();

  updateFileDownload(int32 file_id_, int32 complete_date_, bool is_paused_, object_ptr<downloadedFileCounts> &&counts_);

  static const std::int32_t ID = 875529162;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateFileRemovedFromDownloads final : public Update {
 public:
  int32 file_id_;
  object_ptr<downloadedFileCounts> counts_;

  updateFileRemovedFromDownloads();

  updateFileRemovedFromDownloads(int32 file_id_, object_ptr<downloadedFileCounts> &&counts_);

  static const std::int32_t ID = 1853625576;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateCall final : public Update {
 public:
  object_ptr<call> call_;

  updateCall();

  explicit updateCall(object_ptr<call> &&call_);

  static const std::int32_t ID = 1337184477;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateGroupCall final : public Update {
 public:
  object_ptr<groupCall> group_call_;

  updateGroupCall();

  explicit updateGroupCall(object_ptr<groupCall> &&group_call_);

  static const std::int32_t ID = 808603136;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateGroupCallParticipant final : public Update {
 public:
  int32 group_call_id_;
  object_ptr<groupCallParticipant> participant_;

  updateGroupCallParticipant();

  updateGroupCallParticipant(int32 group_call_id_, object_ptr<groupCallParticipant> &&participant_);

  static const std::int32_t ID = -803128071;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateNewCallSignalingData final : public Update {
 public:
  int32 call_id_;
  bytes data_;

  updateNewCallSignalingData();

  updateNewCallSignalingData(int32 call_id_, bytes const &data_);

  static const std::int32_t ID = 583634317;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateUserPrivacySettingRules final : public Update {
 public:
  object_ptr<UserPrivacySetting> setting_;
  object_ptr<userPrivacySettingRules> rules_;

  updateUserPrivacySettingRules();

  updateUserPrivacySettingRules(object_ptr<UserPrivacySetting> &&setting_, object_ptr<userPrivacySettingRules> &&rules_);

  static const std::int32_t ID = -912960778;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateUnreadMessageCount final : public Update {
 public:
  object_ptr<ChatList> chat_list_;
  int32 unread_count_;
  int32 unread_unmuted_count_;

  updateUnreadMessageCount();

  updateUnreadMessageCount(object_ptr<ChatList> &&chat_list_, int32 unread_count_, int32 unread_unmuted_count_);

  static const std::int32_t ID = 78987721;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateUnreadChatCount final : public Update {
 public:
  object_ptr<ChatList> chat_list_;
  int32 total_count_;
  int32 unread_count_;
  int32 unread_unmuted_count_;
  int32 marked_as_unread_count_;
  int32 marked_as_unread_unmuted_count_;

  updateUnreadChatCount();

  updateUnreadChatCount(object_ptr<ChatList> &&chat_list_, int32 total_count_, int32 unread_count_, int32 unread_unmuted_count_, int32 marked_as_unread_count_, int32 marked_as_unread_unmuted_count_);

  static const std::int32_t ID = 1994494530;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateOption final : public Update {
 public:
  string name_;
  object_ptr<OptionValue> value_;

  updateOption();

  updateOption(string const &name_, object_ptr<OptionValue> &&value_);

  static const std::int32_t ID = 900822020;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateStickerSet final : public Update {
 public:
  object_ptr<stickerSet> sticker_set_;

  updateStickerSet();

  explicit updateStickerSet(object_ptr<stickerSet> &&sticker_set_);

  static const std::int32_t ID = 1879268812;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateInstalledStickerSets final : public Update {
 public:
  object_ptr<StickerType> sticker_type_;
  array<int64> sticker_set_ids_;

  updateInstalledStickerSets();

  updateInstalledStickerSets(object_ptr<StickerType> &&sticker_type_, array<int64> &&sticker_set_ids_);

  static const std::int32_t ID = -1735084182;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateTrendingStickerSets final : public Update {
 public:
  object_ptr<StickerType> sticker_type_;
  object_ptr<trendingStickerSets> sticker_sets_;

  updateTrendingStickerSets();

  updateTrendingStickerSets(object_ptr<StickerType> &&sticker_type_, object_ptr<trendingStickerSets> &&sticker_sets_);

  static const std::int32_t ID = 1266307239;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateRecentStickers final : public Update {
 public:
  bool is_attached_;
  array<int32> sticker_ids_;

  updateRecentStickers();

  updateRecentStickers(bool is_attached_, array<int32> &&sticker_ids_);

  static const std::int32_t ID = 1906403540;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateFavoriteStickers final : public Update {
 public:
  array<int32> sticker_ids_;

  updateFavoriteStickers();

  explicit updateFavoriteStickers(array<int32> &&sticker_ids_);

  static const std::int32_t ID = 1662240999;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateSavedAnimations final : public Update {
 public:
  array<int32> animation_ids_;

  updateSavedAnimations();

  explicit updateSavedAnimations(array<int32> &&animation_ids_);

  static const std::int32_t ID = 65563814;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateSavedNotificationSounds final : public Update {
 public:
  array<int64> notification_sound_ids_;

  updateSavedNotificationSounds();

  explicit updateSavedNotificationSounds(array<int64> &&notification_sound_ids_);

  static const std::int32_t ID = 1052725698;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateSelectedBackground final : public Update {
 public:
  bool for_dark_theme_;
  object_ptr<background> background_;

  updateSelectedBackground();

  updateSelectedBackground(bool for_dark_theme_, object_ptr<background> &&background_);

  static const std::int32_t ID = -1715658659;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatThemes final : public Update {
 public:
  array<object_ptr<chatTheme>> chat_themes_;

  updateChatThemes();

  explicit updateChatThemes(array<object_ptr<chatTheme>> &&chat_themes_);

  static const std::int32_t ID = -1588098376;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateLanguagePackStrings final : public Update {
 public:
  string localization_target_;
  string language_pack_id_;
  array<object_ptr<languagePackString>> strings_;

  updateLanguagePackStrings();

  updateLanguagePackStrings(string const &localization_target_, string const &language_pack_id_, array<object_ptr<languagePackString>> &&strings_);

  static const std::int32_t ID = -1056319886;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateConnectionState final : public Update {
 public:
  object_ptr<ConnectionState> state_;

  updateConnectionState();

  explicit updateConnectionState(object_ptr<ConnectionState> &&state_);

  static const std::int32_t ID = 1469292078;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateTermsOfService final : public Update {
 public:
  string terms_of_service_id_;
  object_ptr<termsOfService> terms_of_service_;

  updateTermsOfService();

  updateTermsOfService(string const &terms_of_service_id_, object_ptr<termsOfService> &&terms_of_service_);

  static const std::int32_t ID = -1304640162;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateUsersNearby final : public Update {
 public:
  array<object_ptr<chatNearby>> users_nearby_;

  updateUsersNearby();

  explicit updateUsersNearby(array<object_ptr<chatNearby>> &&users_nearby_);

  static const std::int32_t ID = -1517109163;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateAttachmentMenuBots final : public Update {
 public:
  array<object_ptr<attachmentMenuBot>> bots_;

  updateAttachmentMenuBots();

  explicit updateAttachmentMenuBots(array<object_ptr<attachmentMenuBot>> &&bots_);

  static const std::int32_t ID = 291369922;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateWebAppMessageSent final : public Update {
 public:
  int64 web_app_launch_id_;

  updateWebAppMessageSent();

  explicit updateWebAppMessageSent(int64 web_app_launch_id_);

  static const std::int32_t ID = 1480790569;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateActiveEmojiReactions final : public Update {
 public:
  array<string> emojis_;

  updateActiveEmojiReactions();

  explicit updateActiveEmojiReactions(array<string> &&emojis_);

  static const std::int32_t ID = 77556818;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateDefaultReactionType final : public Update {
 public:
  object_ptr<ReactionType> reaction_type_;

  updateDefaultReactionType();

  explicit updateDefaultReactionType(object_ptr<ReactionType> &&reaction_type_);

  static const std::int32_t ID = 1264668933;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateDiceEmojis final : public Update {
 public:
  array<string> emojis_;

  updateDiceEmojis();

  explicit updateDiceEmojis(array<string> &&emojis_);

  static const std::int32_t ID = -1069066940;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateAnimatedEmojiMessageClicked final : public Update {
 public:
  int53 chat_id_;
  int53 message_id_;
  object_ptr<sticker> sticker_;

  updateAnimatedEmojiMessageClicked();

  updateAnimatedEmojiMessageClicked(int53 chat_id_, int53 message_id_, object_ptr<sticker> &&sticker_);

  static const std::int32_t ID = -1558809595;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateAnimationSearchParameters final : public Update {
 public:
  string provider_;
  array<string> emojis_;

  updateAnimationSearchParameters();

  updateAnimationSearchParameters(string const &provider_, array<string> &&emojis_);

  static const std::int32_t ID = -1144983202;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateSuggestedActions final : public Update {
 public:
  array<object_ptr<SuggestedAction>> added_actions_;
  array<object_ptr<SuggestedAction>> removed_actions_;

  updateSuggestedActions();

  updateSuggestedActions(array<object_ptr<SuggestedAction>> &&added_actions_, array<object_ptr<SuggestedAction>> &&removed_actions_);

  static const std::int32_t ID = 1459452346;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateNewInlineQuery final : public Update {
 public:
  int64 id_;
  int53 sender_user_id_;
  object_ptr<location> user_location_;
  object_ptr<ChatType> chat_type_;
  string query_;
  string offset_;

  updateNewInlineQuery();

  updateNewInlineQuery(int64 id_, int53 sender_user_id_, object_ptr<location> &&user_location_, object_ptr<ChatType> &&chat_type_, string const &query_, string const &offset_);

  static const std::int32_t ID = 1903279924;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateNewChosenInlineResult final : public Update {
 public:
  int53 sender_user_id_;
  object_ptr<location> user_location_;
  string query_;
  string result_id_;
  string inline_message_id_;

  updateNewChosenInlineResult();

  updateNewChosenInlineResult(int53 sender_user_id_, object_ptr<location> &&user_location_, string const &query_, string const &result_id_, string const &inline_message_id_);

  static const std::int32_t ID = -884191395;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateNewCallbackQuery final : public Update {
 public:
  int64 id_;
  int53 sender_user_id_;
  int53 chat_id_;
  int53 message_id_;
  int64 chat_instance_;
  object_ptr<CallbackQueryPayload> payload_;

  updateNewCallbackQuery();

  updateNewCallbackQuery(int64 id_, int53 sender_user_id_, int53 chat_id_, int53 message_id_, int64 chat_instance_, object_ptr<CallbackQueryPayload> &&payload_);

  static const std::int32_t ID = -1989881762;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateNewInlineCallbackQuery final : public Update {
 public:
  int64 id_;
  int53 sender_user_id_;
  string inline_message_id_;
  int64 chat_instance_;
  object_ptr<CallbackQueryPayload> payload_;

  updateNewInlineCallbackQuery();

  updateNewInlineCallbackQuery(int64 id_, int53 sender_user_id_, string const &inline_message_id_, int64 chat_instance_, object_ptr<CallbackQueryPayload> &&payload_);

  static const std::int32_t ID = -319212358;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateNewShippingQuery final : public Update {
 public:
  int64 id_;
  int53 sender_user_id_;
  string invoice_payload_;
  object_ptr<address> shipping_address_;

  updateNewShippingQuery();

  updateNewShippingQuery(int64 id_, int53 sender_user_id_, string const &invoice_payload_, object_ptr<address> &&shipping_address_);

  static const std::int32_t ID = 693651058;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateNewPreCheckoutQuery final : public Update {
 public:
  int64 id_;
  int53 sender_user_id_;
  string currency_;
  int53 total_amount_;
  bytes invoice_payload_;
  string shipping_option_id_;
  object_ptr<orderInfo> order_info_;

  updateNewPreCheckoutQuery();

  updateNewPreCheckoutQuery(int64 id_, int53 sender_user_id_, string const &currency_, int53 total_amount_, bytes const &invoice_payload_, string const &shipping_option_id_, object_ptr<orderInfo> &&order_info_);

  static const std::int32_t ID = 708342217;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateNewCustomEvent final : public Update {
 public:
  string event_;

  updateNewCustomEvent();

  explicit updateNewCustomEvent(string const &event_);

  static const std::int32_t ID = 1994222092;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateNewCustomQuery final : public Update {
 public:
  int64 id_;
  string data_;
  int32 timeout_;

  updateNewCustomQuery();

  updateNewCustomQuery(int64 id_, string const &data_, int32 timeout_);

  static const std::int32_t ID = -687670874;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updatePoll final : public Update {
 public:
  object_ptr<poll> poll_;

  updatePoll();

  explicit updatePoll(object_ptr<poll> &&poll_);

  static const std::int32_t ID = -1771342902;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updatePollAnswer final : public Update {
 public:
  int64 poll_id_;
  int53 user_id_;
  array<int32> option_ids_;

  updatePollAnswer();

  updatePollAnswer(int64 poll_id_, int53 user_id_, array<int32> &&option_ids_);

  static const std::int32_t ID = -1209100651;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatMember final : public Update {
 public:
  int53 chat_id_;
  int53 actor_user_id_;
  int32 date_;
  object_ptr<chatInviteLink> invite_link_;
  object_ptr<chatMember> old_chat_member_;
  object_ptr<chatMember> new_chat_member_;

  updateChatMember();

  updateChatMember(int53 chat_id_, int53 actor_user_id_, int32 date_, object_ptr<chatInviteLink> &&invite_link_, object_ptr<chatMember> &&old_chat_member_, object_ptr<chatMember> &&new_chat_member_);

  static const std::int32_t ID = -2027989165;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateNewChatJoinRequest final : public Update {
 public:
  int53 chat_id_;
  object_ptr<chatJoinRequest> request_;
  object_ptr<chatInviteLink> invite_link_;

  updateNewChatJoinRequest();

  updateNewChatJoinRequest(int53 chat_id_, object_ptr<chatJoinRequest> &&request_, object_ptr<chatInviteLink> &&invite_link_);

  static const std::int32_t ID = 175285905;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updates final : public Object {
 public:
  array<object_ptr<Update>> updates_;

  updates();

  explicit updates(array<object_ptr<Update>> &&updates_);

  static const std::int32_t ID = 475842347;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class user final : public Object {
 public:
  int53 id_;
  string first_name_;
  string last_name_;
  object_ptr<usernames> usernames_;
  string phone_number_;
  object_ptr<UserStatus> status_;
  object_ptr<profilePhoto> profile_photo_;
  object_ptr<emojiStatus> emoji_status_;
  bool is_contact_;
  bool is_mutual_contact_;
  bool is_verified_;
  bool is_premium_;
  bool is_support_;
  string restriction_reason_;
  bool is_scam_;
  bool is_fake_;
  bool have_access_;
  object_ptr<UserType> type_;
  string language_code_;
  bool added_to_attachment_menu_;

  user();

  user(int53 id_, string const &first_name_, string const &last_name_, object_ptr<usernames> &&usernames_, string const &phone_number_, object_ptr<UserStatus> &&status_, object_ptr<profilePhoto> &&profile_photo_, object_ptr<emojiStatus> &&emoji_status_, bool is_contact_, bool is_mutual_contact_, bool is_verified_, bool is_premium_, bool is_support_, string const &restriction_reason_, bool is_scam_, bool is_fake_, bool have_access_, object_ptr<UserType> &&type_, string const &language_code_, bool added_to_attachment_menu_);

  static const std::int32_t ID = -651883724;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userFullInfo final : public Object {
 public:
  object_ptr<chatPhoto> photo_;
  bool is_blocked_;
  bool can_be_called_;
  bool supports_video_calls_;
  bool has_private_calls_;
  bool has_private_forwards_;
  bool has_restricted_voice_and_video_note_messages_;
  bool need_phone_number_privacy_exception_;
  object_ptr<formattedText> bio_;
  array<object_ptr<premiumPaymentOption>> premium_gift_options_;
  int32 group_in_common_count_;
  object_ptr<botInfo> bot_info_;

  userFullInfo();

  userFullInfo(object_ptr<chatPhoto> &&photo_, bool is_blocked_, bool can_be_called_, bool supports_video_calls_, bool has_private_calls_, bool has_private_forwards_, bool has_restricted_voice_and_video_note_messages_, bool need_phone_number_privacy_exception_, object_ptr<formattedText> &&bio_, array<object_ptr<premiumPaymentOption>> &&premium_gift_options_, int32 group_in_common_count_, object_ptr<botInfo> &&bot_info_);

  static const std::int32_t ID = -272527755;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class UserPrivacySetting: public Object {
 public:
};

class userPrivacySettingShowStatus final : public UserPrivacySetting {
 public:

  userPrivacySettingShowStatus();

  static const std::int32_t ID = 1862829310;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userPrivacySettingShowProfilePhoto final : public UserPrivacySetting {
 public:

  userPrivacySettingShowProfilePhoto();

  static const std::int32_t ID = 1408485877;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userPrivacySettingShowLinkInForwardedMessages final : public UserPrivacySetting {
 public:

  userPrivacySettingShowLinkInForwardedMessages();

  static const std::int32_t ID = 592688870;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userPrivacySettingShowPhoneNumber final : public UserPrivacySetting {
 public:

  userPrivacySettingShowPhoneNumber();

  static const std::int32_t ID = -791567831;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userPrivacySettingAllowChatInvites final : public UserPrivacySetting {
 public:

  userPrivacySettingAllowChatInvites();

  static const std::int32_t ID = 1271668007;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userPrivacySettingAllowCalls final : public UserPrivacySetting {
 public:

  userPrivacySettingAllowCalls();

  static const std::int32_t ID = -906967291;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userPrivacySettingAllowPeerToPeerCalls final : public UserPrivacySetting {
 public:

  userPrivacySettingAllowPeerToPeerCalls();

  static const std::int32_t ID = 352500032;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userPrivacySettingAllowFindingByPhoneNumber final : public UserPrivacySetting {
 public:

  userPrivacySettingAllowFindingByPhoneNumber();

  static const std::int32_t ID = -1846645423;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages final : public UserPrivacySetting {
 public:

  userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages();

  static const std::int32_t ID = 338112060;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class UserPrivacySettingRule: public Object {
 public:
};

class userPrivacySettingRuleAllowAll final : public UserPrivacySettingRule {
 public:

  userPrivacySettingRuleAllowAll();

  static const std::int32_t ID = -1967186881;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userPrivacySettingRuleAllowContacts final : public UserPrivacySettingRule {
 public:

  userPrivacySettingRuleAllowContacts();

  static const std::int32_t ID = -1892733680;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userPrivacySettingRuleAllowUsers final : public UserPrivacySettingRule {
 public:
  array<int53> user_ids_;

  userPrivacySettingRuleAllowUsers();

  explicit userPrivacySettingRuleAllowUsers(array<int53> &&user_ids_);

  static const std::int32_t ID = 1110988334;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userPrivacySettingRuleAllowChatMembers final : public UserPrivacySettingRule {
 public:
  array<int53> chat_ids_;

  userPrivacySettingRuleAllowChatMembers();

  explicit userPrivacySettingRuleAllowChatMembers(array<int53> &&chat_ids_);

  static const std::int32_t ID = -2048749863;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userPrivacySettingRuleRestrictAll final : public UserPrivacySettingRule {
 public:

  userPrivacySettingRuleRestrictAll();

  static const std::int32_t ID = -1406495408;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userPrivacySettingRuleRestrictContacts final : public UserPrivacySettingRule {
 public:

  userPrivacySettingRuleRestrictContacts();

  static const std::int32_t ID = 1008389378;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userPrivacySettingRuleRestrictUsers final : public UserPrivacySettingRule {
 public:
  array<int53> user_ids_;

  userPrivacySettingRuleRestrictUsers();

  explicit userPrivacySettingRuleRestrictUsers(array<int53> &&user_ids_);

  static const std::int32_t ID = 622796522;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userPrivacySettingRuleRestrictChatMembers final : public UserPrivacySettingRule {
 public:
  array<int53> chat_ids_;

  userPrivacySettingRuleRestrictChatMembers();

  explicit userPrivacySettingRuleRestrictChatMembers(array<int53> &&chat_ids_);

  static const std::int32_t ID = 392530897;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userPrivacySettingRules final : public Object {
 public:
  array<object_ptr<UserPrivacySettingRule>> rules_;

  userPrivacySettingRules();

  explicit userPrivacySettingRules(array<object_ptr<UserPrivacySettingRule>> &&rules_);

  static const std::int32_t ID = 322477541;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class UserStatus: public Object {
 public:
};

class userStatusEmpty final : public UserStatus {
 public:

  userStatusEmpty();

  static const std::int32_t ID = 164646985;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userStatusOnline final : public UserStatus {
 public:
  int32 expires_;

  userStatusOnline();

  explicit userStatusOnline(int32 expires_);

  static const std::int32_t ID = -1529460876;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userStatusOffline final : public UserStatus {
 public:
  int32 was_online_;

  userStatusOffline();

  explicit userStatusOffline(int32 was_online_);

  static const std::int32_t ID = -759984891;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userStatusRecently final : public UserStatus {
 public:

  userStatusRecently();

  static const std::int32_t ID = -496024847;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userStatusLastWeek final : public UserStatus {
 public:

  userStatusLastWeek();

  static const std::int32_t ID = 129960444;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userStatusLastMonth final : public UserStatus {
 public:

  userStatusLastMonth();

  static const std::int32_t ID = 2011940674;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userSupportInfo final : public Object {
 public:
  object_ptr<formattedText> message_;
  string author_;
  int32 date_;

  userSupportInfo();

  userSupportInfo(object_ptr<formattedText> &&message_, string const &author_, int32 date_);

  static const std::int32_t ID = -1257366487;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class UserType: public Object {
 public:
};

class userTypeRegular final : public UserType {
 public:

  userTypeRegular();

  static const std::int32_t ID = -598644325;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userTypeDeleted final : public UserType {
 public:

  userTypeDeleted();

  static const std::int32_t ID = -1807729372;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userTypeBot final : public UserType {
 public:
  bool can_join_groups_;
  bool can_read_all_group_messages_;
  bool is_inline_;
  string inline_query_placeholder_;
  bool need_location_;
  bool can_be_added_to_attachment_menu_;

  userTypeBot();

  userTypeBot(bool can_join_groups_, bool can_read_all_group_messages_, bool is_inline_, string const &inline_query_placeholder_, bool need_location_, bool can_be_added_to_attachment_menu_);

  static const std::int32_t ID = -970625144;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userTypeUnknown final : public UserType {
 public:

  userTypeUnknown();

  static const std::int32_t ID = -724541123;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class usernames final : public Object {
 public:
  array<string> active_usernames_;
  array<string> disabled_usernames_;
  string editable_username_;

  usernames();

  usernames(array<string> &&active_usernames_, array<string> &&disabled_usernames_, string const &editable_username_);

  static const std::int32_t ID = 799608565;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class users final : public Object {
 public:
  int32 total_count_;
  array<int53> user_ids_;

  users();

  users(int32 total_count_, array<int53> &&user_ids_);

  static const std::int32_t ID = 171203420;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class validatedOrderInfo final : public Object {
 public:
  string order_info_id_;
  array<object_ptr<shippingOption>> shipping_options_;

  validatedOrderInfo();

  validatedOrderInfo(string const &order_info_id_, array<object_ptr<shippingOption>> &&shipping_options_);

  static const std::int32_t ID = 1511451484;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class VectorPathCommand: public Object {
 public:
};

class vectorPathCommandLine final : public VectorPathCommand {
 public:
  object_ptr<point> end_point_;

  vectorPathCommandLine();

  explicit vectorPathCommandLine(object_ptr<point> &&end_point_);

  static const std::int32_t ID = -614056822;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class vectorPathCommandCubicBezierCurve final : public VectorPathCommand {
 public:
  object_ptr<point> start_control_point_;
  object_ptr<point> end_control_point_;
  object_ptr<point> end_point_;

  vectorPathCommandCubicBezierCurve();

  vectorPathCommandCubicBezierCurve(object_ptr<point> &&start_control_point_, object_ptr<point> &&end_control_point_, object_ptr<point> &&end_point_);

  static const std::int32_t ID = 1229733434;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class venue final : public Object {
 public:
  object_ptr<location> location_;
  string title_;
  string address_;
  string provider_;
  string id_;
  string type_;

  venue();

  venue(object_ptr<location> &&location_, string const &title_, string const &address_, string const &provider_, string const &id_, string const &type_);

  static const std::int32_t ID = 1070406393;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class video final : public Object {
 public:
  int32 duration_;
  int32 width_;
  int32 height_;
  string file_name_;
  string mime_type_;
  bool has_stickers_;
  bool supports_streaming_;
  object_ptr<minithumbnail> minithumbnail_;
  object_ptr<thumbnail> thumbnail_;
  object_ptr<file> video_;

  video();

  video(int32 duration_, int32 width_, int32 height_, string const &file_name_, string const &mime_type_, bool has_stickers_, bool supports_streaming_, object_ptr<minithumbnail> &&minithumbnail_, object_ptr<thumbnail> &&thumbnail_, object_ptr<file> &&video_);

  static const std::int32_t ID = 832856268;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class videoChat final : public Object {
 public:
  int32 group_call_id_;
  bool has_participants_;
  object_ptr<MessageSender> default_participant_id_;

  videoChat();

  videoChat(int32 group_call_id_, bool has_participants_, object_ptr<MessageSender> &&default_participant_id_);

  static const std::int32_t ID = -1374319320;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class videoNote final : public Object {
 public:
  int32 duration_;
  bytes waveform_;
  int32 length_;
  object_ptr<minithumbnail> minithumbnail_;
  object_ptr<thumbnail> thumbnail_;
  object_ptr<SpeechRecognitionResult> speech_recognition_result_;
  object_ptr<file> video_;

  videoNote();

  videoNote(int32 duration_, bytes const &waveform_, int32 length_, object_ptr<minithumbnail> &&minithumbnail_, object_ptr<thumbnail> &&thumbnail_, object_ptr<SpeechRecognitionResult> &&speech_recognition_result_, object_ptr<file> &&video_);

  static const std::int32_t ID = 2062096581;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class voiceNote final : public Object {
 public:
  int32 duration_;
  bytes waveform_;
  string mime_type_;
  object_ptr<SpeechRecognitionResult> speech_recognition_result_;
  object_ptr<file> voice_;

  voiceNote();

  voiceNote(int32 duration_, bytes const &waveform_, string const &mime_type_, object_ptr<SpeechRecognitionResult> &&speech_recognition_result_, object_ptr<file> &&voice_);

  static const std::int32_t ID = -1175302923;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class webAppInfo final : public Object {
 public:
  int64 launch_id_;
  string url_;

  webAppInfo();

  webAppInfo(int64 launch_id_, string const &url_);

  static const std::int32_t ID = 788378344;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class webPage final : public Object {
 public:
  string url_;
  string display_url_;
  string type_;
  string site_name_;
  string title_;
  object_ptr<formattedText> description_;
  object_ptr<photo> photo_;
  string embed_url_;
  string embed_type_;
  int32 embed_width_;
  int32 embed_height_;
  int32 duration_;
  string author_;
  object_ptr<animation> animation_;
  object_ptr<audio> audio_;
  object_ptr<document> document_;
  object_ptr<sticker> sticker_;
  object_ptr<video> video_;
  object_ptr<videoNote> video_note_;
  object_ptr<voiceNote> voice_note_;
  int32 instant_view_version_;

  webPage();

  webPage(string const &url_, string const &display_url_, string const &type_, string const &site_name_, string const &title_, object_ptr<formattedText> &&description_, object_ptr<photo> &&photo_, string const &embed_url_, string const &embed_type_, int32 embed_width_, int32 embed_height_, int32 duration_, string const &author_, object_ptr<animation> &&animation_, object_ptr<audio> &&audio_, object_ptr<document> &&document_, object_ptr<sticker> &&sticker_, object_ptr<video> &&video_, object_ptr<videoNote> &&video_note_, object_ptr<voiceNote> &&voice_note_, int32 instant_view_version_);

  static const std::int32_t ID = -577333714;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class webPageInstantView final : public Object {
 public:
  array<object_ptr<PageBlock>> page_blocks_;
  int32 view_count_;
  int32 version_;
  bool is_rtl_;
  bool is_full_;
  object_ptr<InternalLinkType> feedback_link_;

  webPageInstantView();

  webPageInstantView(array<object_ptr<PageBlock>> &&page_blocks_, int32 view_count_, int32 version_, bool is_rtl_, bool is_full_, object_ptr<InternalLinkType> &&feedback_link_);

  static const std::int32_t ID = 778202453;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class acceptCall final : public Function {
 public:
  int32 call_id_;
  object_ptr<callProtocol> protocol_;

  acceptCall();

  acceptCall(int32 call_id_, object_ptr<callProtocol> &&protocol_);

  static const std::int32_t ID = -646618416;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class acceptTermsOfService final : public Function {
 public:
  string terms_of_service_id_;

  acceptTermsOfService();

  explicit acceptTermsOfService(string const &terms_of_service_id_);

  static const std::int32_t ID = 2130576356;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class addChatMember final : public Function {
 public:
  int53 chat_id_;
  int53 user_id_;
  int32 forward_limit_;

  addChatMember();

  addChatMember(int53 chat_id_, int53 user_id_, int32 forward_limit_);

  static const std::int32_t ID = 499426810;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class addChatMembers final : public Function {
 public:
  int53 chat_id_;
  array<int53> user_ids_;

  addChatMembers();

  addChatMembers(int53 chat_id_, array<int53> &&user_ids_);

  static const std::int32_t ID = -519951226;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class addChatToList final : public Function {
 public:
  int53 chat_id_;
  object_ptr<ChatList> chat_list_;

  addChatToList();

  addChatToList(int53 chat_id_, object_ptr<ChatList> &&chat_list_);

  static const std::int32_t ID = -80523595;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class addContact final : public Function {
 public:
  object_ptr<contact> contact_;
  bool share_phone_number_;

  addContact();

  addContact(object_ptr<contact> &&contact_, bool share_phone_number_);

  static const std::int32_t ID = 1869640000;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class addCustomServerLanguagePack final : public Function {
 public:
  string language_pack_id_;

  addCustomServerLanguagePack();

  explicit addCustomServerLanguagePack(string const &language_pack_id_);

  static const std::int32_t ID = 4492771;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class addFavoriteSticker final : public Function {
 public:
  object_ptr<InputFile> sticker_;

  addFavoriteSticker();

  explicit addFavoriteSticker(object_ptr<InputFile> &&sticker_);

  static const std::int32_t ID = 324504799;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class addFileToDownloads final : public Function {
 public:
  int32 file_id_;
  int53 chat_id_;
  int53 message_id_;
  int32 priority_;

  addFileToDownloads();

  addFileToDownloads(int32 file_id_, int53 chat_id_, int53 message_id_, int32 priority_);

  static const std::int32_t ID = 867533751;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<file>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class addLocalMessage final : public Function {
 public:
  int53 chat_id_;
  object_ptr<MessageSender> sender_id_;
  int53 reply_to_message_id_;
  bool disable_notification_;
  object_ptr<InputMessageContent> input_message_content_;

  addLocalMessage();

  addLocalMessage(int53 chat_id_, object_ptr<MessageSender> &&sender_id_, int53 reply_to_message_id_, bool disable_notification_, object_ptr<InputMessageContent> &&input_message_content_);

  static const std::int32_t ID = -1998878725;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<message>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class addLogMessage final : public Function {
 public:
  int32 verbosity_level_;
  string text_;

  addLogMessage();

  addLogMessage(int32 verbosity_level_, string const &text_);

  static const std::int32_t ID = 1597427692;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class addMessageReaction final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  object_ptr<ReactionType> reaction_type_;
  bool is_big_;
  bool update_recent_reactions_;

  addMessageReaction();

  addMessageReaction(int53 chat_id_, int53 message_id_, object_ptr<ReactionType> &&reaction_type_, bool is_big_, bool update_recent_reactions_);

  static const std::int32_t ID = 1419269613;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class addNetworkStatistics final : public Function {
 public:
  object_ptr<NetworkStatisticsEntry> entry_;

  addNetworkStatistics();

  explicit addNetworkStatistics(object_ptr<NetworkStatisticsEntry> &&entry_);

  static const std::int32_t ID = 1264825305;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class addProxy final : public Function {
 public:
  string server_;
  int32 port_;
  bool enable_;
  object_ptr<ProxyType> type_;

  addProxy();

  addProxy(string const &server_, int32 port_, bool enable_, object_ptr<ProxyType> &&type_);

  static const std::int32_t ID = 331529432;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<proxy>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class addRecentSticker final : public Function {
 public:
  bool is_attached_;
  object_ptr<InputFile> sticker_;

  addRecentSticker();

  addRecentSticker(bool is_attached_, object_ptr<InputFile> &&sticker_);

  static const std::int32_t ID = -1478109026;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickers>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class addRecentlyFoundChat final : public Function {
 public:
  int53 chat_id_;

  addRecentlyFoundChat();

  explicit addRecentlyFoundChat(int53 chat_id_);

  static const std::int32_t ID = -1746396787;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class addSavedAnimation final : public Function {
 public:
  object_ptr<InputFile> animation_;

  addSavedAnimation();

  explicit addSavedAnimation(object_ptr<InputFile> &&animation_);

  static const std::int32_t ID = -1538525088;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class addSavedNotificationSound final : public Function {
 public:
  object_ptr<InputFile> sound_;

  addSavedNotificationSound();

  explicit addSavedNotificationSound(object_ptr<InputFile> &&sound_);

  static const std::int32_t ID = 1043956975;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<notificationSound>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class addStickerToSet final : public Function {
 public:
  int53 user_id_;
  string name_;
  object_ptr<inputSticker> sticker_;

  addStickerToSet();

  addStickerToSet(int53 user_id_, string const &name_, object_ptr<inputSticker> &&sticker_);

  static const std::int32_t ID = -1340783267;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickerSet>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class answerCallbackQuery final : public Function {
 public:
  int64 callback_query_id_;
  string text_;
  bool show_alert_;
  string url_;
  int32 cache_time_;

  answerCallbackQuery();

  answerCallbackQuery(int64 callback_query_id_, string const &text_, bool show_alert_, string const &url_, int32 cache_time_);

  static const std::int32_t ID = -1153028490;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class answerCustomQuery final : public Function {
 public:
  int64 custom_query_id_;
  string data_;

  answerCustomQuery();

  answerCustomQuery(int64 custom_query_id_, string const &data_);

  static const std::int32_t ID = -1293603521;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class answerInlineQuery final : public Function {
 public:
  int64 inline_query_id_;
  bool is_personal_;
  array<object_ptr<InputInlineQueryResult>> results_;
  int32 cache_time_;
  string next_offset_;
  string switch_pm_text_;
  string switch_pm_parameter_;

  answerInlineQuery();

  answerInlineQuery(int64 inline_query_id_, bool is_personal_, array<object_ptr<InputInlineQueryResult>> &&results_, int32 cache_time_, string const &next_offset_, string const &switch_pm_text_, string const &switch_pm_parameter_);

  static const std::int32_t ID = 485879477;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class answerPreCheckoutQuery final : public Function {
 public:
  int64 pre_checkout_query_id_;
  string error_message_;

  answerPreCheckoutQuery();

  answerPreCheckoutQuery(int64 pre_checkout_query_id_, string const &error_message_);

  static const std::int32_t ID = -1486789653;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class answerShippingQuery final : public Function {
 public:
  int64 shipping_query_id_;
  array<object_ptr<shippingOption>> shipping_options_;
  string error_message_;

  answerShippingQuery();

  answerShippingQuery(int64 shipping_query_id_, array<object_ptr<shippingOption>> &&shipping_options_, string const &error_message_);

  static const std::int32_t ID = -434601324;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class answerWebAppQuery final : public Function {
 public:
  string web_app_query_id_;
  object_ptr<InputInlineQueryResult> result_;

  answerWebAppQuery();

  answerWebAppQuery(string const &web_app_query_id_, object_ptr<InputInlineQueryResult> &&result_);

  static const std::int32_t ID = -1598776079;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<sentWebAppMessage>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class assignAppStoreTransaction final : public Function {
 public:
  bytes receipt_;
  object_ptr<StorePaymentPurpose> purpose_;

  assignAppStoreTransaction();

  assignAppStoreTransaction(bytes const &receipt_, object_ptr<StorePaymentPurpose> &&purpose_);

  static const std::int32_t ID = -2030892112;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class assignGooglePlayTransaction final : public Function {
 public:
  string package_name_;
  string store_product_id_;
  string purchase_token_;
  object_ptr<StorePaymentPurpose> purpose_;

  assignGooglePlayTransaction();

  assignGooglePlayTransaction(string const &package_name_, string const &store_product_id_, string const &purchase_token_, object_ptr<StorePaymentPurpose> &&purpose_);

  static const std::int32_t ID = -1992704860;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class banChatMember final : public Function {
 public:
  int53 chat_id_;
  object_ptr<MessageSender> member_id_;
  int32 banned_until_date_;
  bool revoke_messages_;

  banChatMember();

  banChatMember(int53 chat_id_, object_ptr<MessageSender> &&member_id_, int32 banned_until_date_, bool revoke_messages_);

  static const std::int32_t ID = -888111748;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class blockMessageSenderFromReplies final : public Function {
 public:
  int53 message_id_;
  bool delete_message_;
  bool delete_all_messages_;
  bool report_spam_;

  blockMessageSenderFromReplies();

  blockMessageSenderFromReplies(int53 message_id_, bool delete_message_, bool delete_all_messages_, bool report_spam_);

  static const std::int32_t ID = -1214384757;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class canPurchasePremium final : public Function {
 public:
  object_ptr<StorePaymentPurpose> purpose_;

  canPurchasePremium();

  explicit canPurchasePremium(object_ptr<StorePaymentPurpose> &&purpose_);

  static const std::int32_t ID = -371319616;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class canTransferOwnership final : public Function {
 public:

  canTransferOwnership();

  static const std::int32_t ID = 634602508;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<CanTransferOwnershipResult>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class cancelDownloadFile final : public Function {
 public:
  int32 file_id_;
  bool only_if_pending_;

  cancelDownloadFile();

  cancelDownloadFile(int32 file_id_, bool only_if_pending_);

  static const std::int32_t ID = -1954524450;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class cancelPasswordReset final : public Function {
 public:

  cancelPasswordReset();

  static const std::int32_t ID = 940733538;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class cancelPreliminaryUploadFile final : public Function {
 public:
  int32 file_id_;

  cancelPreliminaryUploadFile();

  explicit cancelPreliminaryUploadFile(int32 file_id_);

  static const std::int32_t ID = 823412414;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class changeImportedContacts final : public Function {
 public:
  array<object_ptr<contact>> contacts_;

  changeImportedContacts();

  explicit changeImportedContacts(array<object_ptr<contact>> &&contacts_);

  static const std::int32_t ID = 1968207955;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<importedContacts>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class changePhoneNumber final : public Function {
 public:
  string phone_number_;
  object_ptr<phoneNumberAuthenticationSettings> settings_;

  changePhoneNumber();

  changePhoneNumber(string const &phone_number_, object_ptr<phoneNumberAuthenticationSettings> &&settings_);

  static const std::int32_t ID = -124666973;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<authenticationCodeInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class changeStickerSet final : public Function {
 public:
  int64 set_id_;
  bool is_installed_;
  bool is_archived_;

  changeStickerSet();

  changeStickerSet(int64 set_id_, bool is_installed_, bool is_archived_);

  static const std::int32_t ID = 449357293;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkAuthenticationBotToken final : public Function {
 public:
  string token_;

  checkAuthenticationBotToken();

  explicit checkAuthenticationBotToken(string const &token_);

  static const std::int32_t ID = 639321206;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkAuthenticationCode final : public Function {
 public:
  string code_;

  checkAuthenticationCode();

  explicit checkAuthenticationCode(string const &code_);

  static const std::int32_t ID = -302103382;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkAuthenticationEmailCode final : public Function {
 public:
  object_ptr<EmailAddressAuthentication> code_;

  checkAuthenticationEmailCode();

  explicit checkAuthenticationEmailCode(object_ptr<EmailAddressAuthentication> &&code_);

  static const std::int32_t ID = -582827361;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkAuthenticationPassword final : public Function {
 public:
  string password_;

  checkAuthenticationPassword();

  explicit checkAuthenticationPassword(string const &password_);

  static const std::int32_t ID = -2025698400;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkAuthenticationPasswordRecoveryCode final : public Function {
 public:
  string recovery_code_;

  checkAuthenticationPasswordRecoveryCode();

  explicit checkAuthenticationPasswordRecoveryCode(string const &recovery_code_);

  static const std::int32_t ID = -603309083;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkChangePhoneNumberCode final : public Function {
 public:
  string code_;

  checkChangePhoneNumberCode();

  explicit checkChangePhoneNumberCode(string const &code_);

  static const std::int32_t ID = -1720278429;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkChatInviteLink final : public Function {
 public:
  string invite_link_;

  checkChatInviteLink();

  explicit checkChatInviteLink(string const &invite_link_);

  static const std::int32_t ID = -496940997;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatInviteLinkInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkChatUsername final : public Function {
 public:
  int53 chat_id_;
  string username_;

  checkChatUsername();

  checkChatUsername(int53 chat_id_, string const &username_);

  static const std::int32_t ID = -119119344;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<CheckChatUsernameResult>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkCreatedPublicChatsLimit final : public Function {
 public:
  object_ptr<PublicChatType> type_;

  checkCreatedPublicChatsLimit();

  explicit checkCreatedPublicChatsLimit(object_ptr<PublicChatType> &&type_);

  static const std::int32_t ID = -445546591;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkEmailAddressVerificationCode final : public Function {
 public:
  string code_;

  checkEmailAddressVerificationCode();

  explicit checkEmailAddressVerificationCode(string const &code_);

  static const std::int32_t ID = -426386685;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkLoginEmailAddressCode final : public Function {
 public:
  object_ptr<EmailAddressAuthentication> code_;

  checkLoginEmailAddressCode();

  explicit checkLoginEmailAddressCode(object_ptr<EmailAddressAuthentication> &&code_);

  static const std::int32_t ID = -1454244766;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkPasswordRecoveryCode final : public Function {
 public:
  string recovery_code_;

  checkPasswordRecoveryCode();

  explicit checkPasswordRecoveryCode(string const &recovery_code_);

  static const std::int32_t ID = -200794600;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkPhoneNumberConfirmationCode final : public Function {
 public:
  string code_;

  checkPhoneNumberConfirmationCode();

  explicit checkPhoneNumberConfirmationCode(string const &code_);

  static const std::int32_t ID = -1348060966;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkPhoneNumberVerificationCode final : public Function {
 public:
  string code_;

  checkPhoneNumberVerificationCode();

  explicit checkPhoneNumberVerificationCode(string const &code_);

  static const std::int32_t ID = 1497462718;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkRecoveryEmailAddressCode final : public Function {
 public:
  string code_;

  checkRecoveryEmailAddressCode();

  explicit checkRecoveryEmailAddressCode(string const &code_);

  static const std::int32_t ID = -1997039589;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<passwordState>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class checkStickerSetName final : public Function {
 public:
  string name_;

  checkStickerSetName();

  explicit checkStickerSetName(string const &name_);

  static const std::int32_t ID = -1789392642;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<CheckStickerSetNameResult>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class cleanFileName final : public Function {
 public:
  string file_name_;

  cleanFileName();

  explicit cleanFileName(string const &file_name_);

  static const std::int32_t ID = 967964667;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<text>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class clearAllDraftMessages final : public Function {
 public:
  bool exclude_secret_chats_;

  clearAllDraftMessages();

  explicit clearAllDraftMessages(bool exclude_secret_chats_);

  static const std::int32_t ID = -46369573;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class clearImportedContacts final : public Function {
 public:

  clearImportedContacts();

  static const std::int32_t ID = 869503298;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class clearRecentEmojiStatuses final : public Function {
 public:

  clearRecentEmojiStatuses();

  static const std::int32_t ID = -428749986;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class clearRecentReactions final : public Function {
 public:

  clearRecentReactions();

  static const std::int32_t ID = 1298253650;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class clearRecentStickers final : public Function {
 public:
  bool is_attached_;

  clearRecentStickers();

  explicit clearRecentStickers(bool is_attached_);

  static const std::int32_t ID = -321242684;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class clearRecentlyFoundChats final : public Function {
 public:

  clearRecentlyFoundChats();

  static const std::int32_t ID = -285582542;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class clickAnimatedEmojiMessage final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;

  clickAnimatedEmojiMessage();

  clickAnimatedEmojiMessage(int53 chat_id_, int53 message_id_);

  static const std::int32_t ID = 196179554;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<sticker>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class clickPremiumSubscriptionButton final : public Function {
 public:

  clickPremiumSubscriptionButton();

  static const std::int32_t ID = -369319162;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class close final : public Function {
 public:

  close();

  static const std::int32_t ID = -1187782273;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class closeChat final : public Function {
 public:
  int53 chat_id_;

  closeChat();

  explicit closeChat(int53 chat_id_);

  static const std::int32_t ID = 39749353;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class closeSecretChat final : public Function {
 public:
  int32 secret_chat_id_;

  closeSecretChat();

  explicit closeSecretChat(int32 secret_chat_id_);

  static const std::int32_t ID = -471006133;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class closeWebApp final : public Function {
 public:
  int64 web_app_launch_id_;

  closeWebApp();

  explicit closeWebApp(int64 web_app_launch_id_);

  static const std::int32_t ID = 1755391174;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class confirmQrCodeAuthentication final : public Function {
 public:
  string link_;

  confirmQrCodeAuthentication();

  explicit confirmQrCodeAuthentication(string const &link_);

  static const std::int32_t ID = -376199379;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<session>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class createBasicGroupChat final : public Function {
 public:
  int53 basic_group_id_;
  bool force_;

  createBasicGroupChat();

  createBasicGroupChat(int53 basic_group_id_, bool force_);

  static const std::int32_t ID = 1972024548;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chat>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class createCall final : public Function {
 public:
  int53 user_id_;
  object_ptr<callProtocol> protocol_;
  bool is_video_;

  createCall();

  createCall(int53 user_id_, object_ptr<callProtocol> &&protocol_, bool is_video_);

  static const std::int32_t ID = -1104663024;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<callId>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class createChatFilter final : public Function {
 public:
  object_ptr<chatFilter> filter_;

  createChatFilter();

  explicit createChatFilter(object_ptr<chatFilter> &&filter_);

  static const std::int32_t ID = 49065126;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatFilterInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class createChatInviteLink final : public Function {
 public:
  int53 chat_id_;
  string name_;
  int32 expiration_date_;
  int32 member_limit_;
  bool creates_join_request_;

  createChatInviteLink();

  createChatInviteLink(int53 chat_id_, string const &name_, int32 expiration_date_, int32 member_limit_, bool creates_join_request_);

  static const std::int32_t ID = 287744833;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatInviteLink>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class createForumTopic final : public Function {
 public:
  int53 chat_id_;
  string name_;
  object_ptr<forumTopicIcon> icon_;

  createForumTopic();

  createForumTopic(int53 chat_id_, string const &name_, object_ptr<forumTopicIcon> &&icon_);

  static const std::int32_t ID = -1040570140;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<forumTopicInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class createInvoiceLink final : public Function {
 public:
  object_ptr<InputMessageContent> invoice_;

  createInvoiceLink();

  explicit createInvoiceLink(object_ptr<InputMessageContent> &&invoice_);

  static const std::int32_t ID = 216787233;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<httpUrl>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class createNewBasicGroupChat final : public Function {
 public:
  array<int53> user_ids_;
  string title_;

  createNewBasicGroupChat();

  createNewBasicGroupChat(array<int53> &&user_ids_, string const &title_);

  static const std::int32_t ID = -2104438789;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chat>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class createNewSecretChat final : public Function {
 public:
  int53 user_id_;

  createNewSecretChat();

  explicit createNewSecretChat(int53 user_id_);

  static const std::int32_t ID = -620682651;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chat>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class createNewStickerSet final : public Function {
 public:
  int53 user_id_;
  string title_;
  string name_;
  object_ptr<StickerType> sticker_type_;
  array<object_ptr<inputSticker>> stickers_;
  string source_;

  createNewStickerSet();

  createNewStickerSet(int53 user_id_, string const &title_, string const &name_, object_ptr<StickerType> &&sticker_type_, array<object_ptr<inputSticker>> &&stickers_, string const &source_);

  static const std::int32_t ID = 1424129402;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickerSet>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class createNewSupergroupChat final : public Function {
 public:
  string title_;
  bool is_channel_;
  string description_;
  object_ptr<chatLocation> location_;
  bool for_import_;

  createNewSupergroupChat();

  createNewSupergroupChat(string const &title_, bool is_channel_, string const &description_, object_ptr<chatLocation> &&location_, bool for_import_);

  static const std::int32_t ID = -830232895;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chat>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class createPrivateChat final : public Function {
 public:
  int53 user_id_;
  bool force_;

  createPrivateChat();

  createPrivateChat(int53 user_id_, bool force_);

  static const std::int32_t ID = -947758327;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chat>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class createSecretChat final : public Function {
 public:
  int32 secret_chat_id_;

  createSecretChat();

  explicit createSecretChat(int32 secret_chat_id_);

  static const std::int32_t ID = 1930285615;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chat>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class createSupergroupChat final : public Function {
 public:
  int53 supergroup_id_;
  bool force_;

  createSupergroupChat();

  createSupergroupChat(int53 supergroup_id_, bool force_);

  static const std::int32_t ID = 1187475691;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chat>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class createTemporaryPassword final : public Function {
 public:
  string password_;
  int32 valid_for_;

  createTemporaryPassword();

  createTemporaryPassword(string const &password_, int32 valid_for_);

  static const std::int32_t ID = -1626509434;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<temporaryPasswordState>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class createVideoChat final : public Function {
 public:
  int53 chat_id_;
  string title_;
  int32 start_date_;
  bool is_rtmp_stream_;

  createVideoChat();

  createVideoChat(int53 chat_id_, string const &title_, int32 start_date_, bool is_rtmp_stream_);

  static const std::int32_t ID = 2124715405;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<groupCallId>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deleteAccount final : public Function {
 public:
  string reason_;
  string password_;

  deleteAccount();

  deleteAccount(string const &reason_, string const &password_);

  static const std::int32_t ID = 1395816134;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deleteAllCallMessages final : public Function {
 public:
  bool revoke_;

  deleteAllCallMessages();

  explicit deleteAllCallMessages(bool revoke_);

  static const std::int32_t ID = -1466445325;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deleteAllRevokedChatInviteLinks final : public Function {
 public:
  int53 chat_id_;
  int53 creator_user_id_;

  deleteAllRevokedChatInviteLinks();

  deleteAllRevokedChatInviteLinks(int53 chat_id_, int53 creator_user_id_);

  static const std::int32_t ID = 1112020698;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deleteChat final : public Function {
 public:
  int53 chat_id_;

  deleteChat();

  explicit deleteChat(int53 chat_id_);

  static const std::int32_t ID = -171253666;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deleteChatFilter final : public Function {
 public:
  int32 chat_filter_id_;

  deleteChatFilter();

  explicit deleteChatFilter(int32 chat_filter_id_);

  static const std::int32_t ID = -523220877;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deleteChatHistory final : public Function {
 public:
  int53 chat_id_;
  bool remove_from_chat_list_;
  bool revoke_;

  deleteChatHistory();

  deleteChatHistory(int53 chat_id_, bool remove_from_chat_list_, bool revoke_);

  static const std::int32_t ID = -1472081761;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deleteChatMessagesByDate final : public Function {
 public:
  int53 chat_id_;
  int32 min_date_;
  int32 max_date_;
  bool revoke_;

  deleteChatMessagesByDate();

  deleteChatMessagesByDate(int53 chat_id_, int32 min_date_, int32 max_date_, bool revoke_);

  static const std::int32_t ID = -1639653185;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deleteChatMessagesBySender final : public Function {
 public:
  int53 chat_id_;
  object_ptr<MessageSender> sender_id_;

  deleteChatMessagesBySender();

  deleteChatMessagesBySender(int53 chat_id_, object_ptr<MessageSender> &&sender_id_);

  static const std::int32_t ID = -1164235161;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deleteChatReplyMarkup final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;

  deleteChatReplyMarkup();

  deleteChatReplyMarkup(int53 chat_id_, int53 message_id_);

  static const std::int32_t ID = 100637531;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deleteCommands final : public Function {
 public:
  object_ptr<BotCommandScope> scope_;
  string language_code_;

  deleteCommands();

  deleteCommands(object_ptr<BotCommandScope> &&scope_, string const &language_code_);

  static const std::int32_t ID = 1002732586;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deleteFile final : public Function {
 public:
  int32 file_id_;

  deleteFile();

  explicit deleteFile(int32 file_id_);

  static const std::int32_t ID = 1807653676;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deleteForumTopic final : public Function {
 public:
  int53 chat_id_;
  int53 message_thread_id_;

  deleteForumTopic();

  deleteForumTopic(int53 chat_id_, int53 message_thread_id_);

  static const std::int32_t ID = 1864916152;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deleteLanguagePack final : public Function {
 public:
  string language_pack_id_;

  deleteLanguagePack();

  explicit deleteLanguagePack(string const &language_pack_id_);

  static const std::int32_t ID = -2108761026;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deleteMessages final : public Function {
 public:
  int53 chat_id_;
  array<int53> message_ids_;
  bool revoke_;

  deleteMessages();

  deleteMessages(int53 chat_id_, array<int53> &&message_ids_, bool revoke_);

  static const std::int32_t ID = 1130090173;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deletePassportElement final : public Function {
 public:
  object_ptr<PassportElementType> type_;

  deletePassportElement();

  explicit deletePassportElement(object_ptr<PassportElementType> &&type_);

  static const std::int32_t ID = -1719555468;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deleteProfilePhoto final : public Function {
 public:
  int64 profile_photo_id_;

  deleteProfilePhoto();

  explicit deleteProfilePhoto(int64 profile_photo_id_);

  static const std::int32_t ID = 1319794625;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deleteRevokedChatInviteLink final : public Function {
 public:
  int53 chat_id_;
  string invite_link_;

  deleteRevokedChatInviteLink();

  deleteRevokedChatInviteLink(int53 chat_id_, string const &invite_link_);

  static const std::int32_t ID = -1859711873;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deleteSavedCredentials final : public Function {
 public:

  deleteSavedCredentials();

  static const std::int32_t ID = 826300114;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class deleteSavedOrderInfo final : public Function {
 public:

  deleteSavedOrderInfo();

  static const std::int32_t ID = 1629058164;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class destroy final : public Function {
 public:

  destroy();

  static const std::int32_t ID = 685331274;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class disableAllSupergroupUsernames final : public Function {
 public:
  int53 supergroup_id_;

  disableAllSupergroupUsernames();

  explicit disableAllSupergroupUsernames(int53 supergroup_id_);

  static const std::int32_t ID = 843511216;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class disableProxy final : public Function {
 public:

  disableProxy();

  static const std::int32_t ID = -2100095102;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class discardCall final : public Function {
 public:
  int32 call_id_;
  bool is_disconnected_;
  int32 duration_;
  bool is_video_;
  int64 connection_id_;

  discardCall();

  discardCall(int32 call_id_, bool is_disconnected_, int32 duration_, bool is_video_, int64 connection_id_);

  static const std::int32_t ID = -1784044162;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class disconnectAllWebsites final : public Function {
 public:

  disconnectAllWebsites();

  static const std::int32_t ID = -1082985981;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class disconnectWebsite final : public Function {
 public:
  int64 website_id_;

  disconnectWebsite();

  explicit disconnectWebsite(int64 website_id_);

  static const std::int32_t ID = -778767395;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class downloadFile final : public Function {
 public:
  int32 file_id_;
  int32 priority_;
  int53 offset_;
  int53 limit_;
  bool synchronous_;

  downloadFile();

  downloadFile(int32 file_id_, int32 priority_, int53 offset_, int53 limit_, bool synchronous_);

  static const std::int32_t ID = 1059402292;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<file>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class editChatFilter final : public Function {
 public:
  int32 chat_filter_id_;
  object_ptr<chatFilter> filter_;

  editChatFilter();

  editChatFilter(int32 chat_filter_id_, object_ptr<chatFilter> &&filter_);

  static const std::int32_t ID = -1674793086;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatFilterInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class editChatInviteLink final : public Function {
 public:
  int53 chat_id_;
  string invite_link_;
  string name_;
  int32 expiration_date_;
  int32 member_limit_;
  bool creates_join_request_;

  editChatInviteLink();

  editChatInviteLink(int53 chat_id_, string const &invite_link_, string const &name_, int32 expiration_date_, int32 member_limit_, bool creates_join_request_);

  static const std::int32_t ID = 1320303996;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatInviteLink>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class editCustomLanguagePackInfo final : public Function {
 public:
  object_ptr<languagePackInfo> info_;

  editCustomLanguagePackInfo();

  explicit editCustomLanguagePackInfo(object_ptr<languagePackInfo> &&info_);

  static const std::int32_t ID = 1320751257;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class editForumTopic final : public Function {
 public:
  int53 chat_id_;
  int53 message_thread_id_;
  string name_;
  int64 icon_custom_emoji_id_;

  editForumTopic();

  editForumTopic(int53 chat_id_, int53 message_thread_id_, string const &name_, int64 icon_custom_emoji_id_);

  static const std::int32_t ID = -222142032;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class editInlineMessageCaption final : public Function {
 public:
  string inline_message_id_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<formattedText> caption_;

  editInlineMessageCaption();

  editInlineMessageCaption(string const &inline_message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<formattedText> &&caption_);

  static const std::int32_t ID = -760985929;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class editInlineMessageLiveLocation final : public Function {
 public:
  string inline_message_id_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<location> location_;
  int32 heading_;
  int32 proximity_alert_radius_;

  editInlineMessageLiveLocation();

  editInlineMessageLiveLocation(string const &inline_message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<location> &&location_, int32 heading_, int32 proximity_alert_radius_);

  static const std::int32_t ID = -156902912;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class editInlineMessageMedia final : public Function {
 public:
  string inline_message_id_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<InputMessageContent> input_message_content_;

  editInlineMessageMedia();

  editInlineMessageMedia(string const &inline_message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_);

  static const std::int32_t ID = 23553921;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class editInlineMessageReplyMarkup final : public Function {
 public:
  string inline_message_id_;
  object_ptr<ReplyMarkup> reply_markup_;

  editInlineMessageReplyMarkup();

  editInlineMessageReplyMarkup(string const &inline_message_id_, object_ptr<ReplyMarkup> &&reply_markup_);

  static const std::int32_t ID = -67565858;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class editInlineMessageText final : public Function {
 public:
  string inline_message_id_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<InputMessageContent> input_message_content_;

  editInlineMessageText();

  editInlineMessageText(string const &inline_message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_);

  static const std::int32_t ID = -855457307;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class editMessageCaption final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<formattedText> caption_;

  editMessageCaption();

  editMessageCaption(int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<formattedText> &&caption_);

  static const std::int32_t ID = 1154677038;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<message>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class editMessageLiveLocation final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<location> location_;
  int32 heading_;
  int32 proximity_alert_radius_;

  editMessageLiveLocation();

  editMessageLiveLocation(int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<location> &&location_, int32 heading_, int32 proximity_alert_radius_);

  static const std::int32_t ID = -14047982;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<message>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class editMessageMedia final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<InputMessageContent> input_message_content_;

  editMessageMedia();

  editMessageMedia(int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_);

  static const std::int32_t ID = -1152678125;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<message>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class editMessageReplyMarkup final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  object_ptr<ReplyMarkup> reply_markup_;

  editMessageReplyMarkup();

  editMessageReplyMarkup(int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_);

  static const std::int32_t ID = 332127881;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<message>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class editMessageSchedulingState final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  object_ptr<MessageSchedulingState> scheduling_state_;

  editMessageSchedulingState();

  editMessageSchedulingState(int53 chat_id_, int53 message_id_, object_ptr<MessageSchedulingState> &&scheduling_state_);

  static const std::int32_t ID = -1372976192;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class editMessageText final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<InputMessageContent> input_message_content_;

  editMessageText();

  editMessageText(int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_);

  static const std::int32_t ID = 196272567;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<message>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class editProxy final : public Function {
 public:
  int32 proxy_id_;
  string server_;
  int32 port_;
  bool enable_;
  object_ptr<ProxyType> type_;

  editProxy();

  editProxy(int32 proxy_id_, string const &server_, int32 port_, bool enable_, object_ptr<ProxyType> &&type_);

  static const std::int32_t ID = -1605883821;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<proxy>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class enableProxy final : public Function {
 public:
  int32 proxy_id_;

  enableProxy();

  explicit enableProxy(int32 proxy_id_);

  static const std::int32_t ID = 1494450838;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class endGroupCall final : public Function {
 public:
  int32 group_call_id_;

  endGroupCall();

  explicit endGroupCall(int32 group_call_id_);

  static const std::int32_t ID = 573131959;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class endGroupCallRecording final : public Function {
 public:
  int32 group_call_id_;

  endGroupCallRecording();

  explicit endGroupCallRecording(int32 group_call_id_);

  static const std::int32_t ID = -75799927;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class endGroupCallScreenSharing final : public Function {
 public:
  int32 group_call_id_;

  endGroupCallScreenSharing();

  explicit endGroupCallScreenSharing(int32 group_call_id_);

  static const std::int32_t ID = -2047599540;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class finishFileGeneration final : public Function {
 public:
  int64 generation_id_;
  object_ptr<error> error_;

  finishFileGeneration();

  finishFileGeneration(int64 generation_id_, object_ptr<error> &&error_);

  static const std::int32_t ID = -1055060835;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class forwardMessages final : public Function {
 public:
  int53 chat_id_;
  int53 message_thread_id_;
  int53 from_chat_id_;
  array<int53> message_ids_;
  object_ptr<messageSendOptions> options_;
  bool send_copy_;
  bool remove_caption_;
  bool only_preview_;

  forwardMessages();

  forwardMessages(int53 chat_id_, int53 message_thread_id_, int53 from_chat_id_, array<int53> &&message_ids_, object_ptr<messageSendOptions> &&options_, bool send_copy_, bool remove_caption_, bool only_preview_);

  static const std::int32_t ID = -1054441908;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getAccountTtl final : public Function {
 public:

  getAccountTtl();

  static const std::int32_t ID = -443905161;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<accountTtl>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getActiveLiveLocationMessages final : public Function {
 public:

  getActiveLiveLocationMessages();

  static const std::int32_t ID = -1425459567;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getActiveSessions final : public Function {
 public:

  getActiveSessions();

  static const std::int32_t ID = 1119710526;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<sessions>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getAllPassportElements final : public Function {
 public:
  string password_;

  getAllPassportElements();

  explicit getAllPassportElements(string const &password_);

  static const std::int32_t ID = -2038945045;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<passportElements>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getAnimatedEmoji final : public Function {
 public:
  string emoji_;

  getAnimatedEmoji();

  explicit getAnimatedEmoji(string const &emoji_);

  static const std::int32_t ID = 1065635702;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<animatedEmoji>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getApplicationConfig final : public Function {
 public:

  getApplicationConfig();

  static const std::int32_t ID = -1823144318;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<JsonValue>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getApplicationDownloadLink final : public Function {
 public:

  getApplicationDownloadLink();

  static const std::int32_t ID = 112013252;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<httpUrl>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getArchivedStickerSets final : public Function {
 public:
  object_ptr<StickerType> sticker_type_;
  int64 offset_sticker_set_id_;
  int32 limit_;

  getArchivedStickerSets();

  getArchivedStickerSets(object_ptr<StickerType> &&sticker_type_, int64 offset_sticker_set_id_, int32 limit_);

  static const std::int32_t ID = 1001931341;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickerSets>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getAttachedStickerSets final : public Function {
 public:
  int32 file_id_;

  getAttachedStickerSets();

  explicit getAttachedStickerSets(int32 file_id_);

  static const std::int32_t ID = 1302172429;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickerSets>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getAttachmentMenuBot final : public Function {
 public:
  int53 bot_user_id_;

  getAttachmentMenuBot();

  explicit getAttachmentMenuBot(int53 bot_user_id_);

  static const std::int32_t ID = 1034248699;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<attachmentMenuBot>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getAuthorizationState final : public Function {
 public:

  getAuthorizationState();

  static const std::int32_t ID = 1949154877;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<AuthorizationState>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getAutoDownloadSettingsPresets final : public Function {
 public:

  getAutoDownloadSettingsPresets();

  static const std::int32_t ID = -1721088201;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<autoDownloadSettingsPresets>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getBackgroundUrl final : public Function {
 public:
  string name_;
  object_ptr<BackgroundType> type_;

  getBackgroundUrl();

  getBackgroundUrl(string const &name_, object_ptr<BackgroundType> &&type_);

  static const std::int32_t ID = 733769682;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<httpUrl>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getBackgrounds final : public Function {
 public:
  bool for_dark_theme_;

  getBackgrounds();

  explicit getBackgrounds(bool for_dark_theme_);

  static const std::int32_t ID = 249072633;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<backgrounds>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getBankCardInfo final : public Function {
 public:
  string bank_card_number_;

  getBankCardInfo();

  explicit getBankCardInfo(string const &bank_card_number_);

  static const std::int32_t ID = -1310515792;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<bankCardInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getBasicGroup final : public Function {
 public:
  int53 basic_group_id_;

  getBasicGroup();

  explicit getBasicGroup(int53 basic_group_id_);

  static const std::int32_t ID = -1635174828;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<basicGroup>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getBasicGroupFullInfo final : public Function {
 public:
  int53 basic_group_id_;

  getBasicGroupFullInfo();

  explicit getBasicGroupFullInfo(int53 basic_group_id_);

  static const std::int32_t ID = -1822039253;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<basicGroupFullInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getBlockedMessageSenders final : public Function {
 public:
  int32 offset_;
  int32 limit_;

  getBlockedMessageSenders();

  getBlockedMessageSenders(int32 offset_, int32 limit_);

  static const std::int32_t ID = 1947079776;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messageSenders>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getCallbackQueryAnswer final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  object_ptr<CallbackQueryPayload> payload_;

  getCallbackQueryAnswer();

  getCallbackQueryAnswer(int53 chat_id_, int53 message_id_, object_ptr<CallbackQueryPayload> &&payload_);

  static const std::int32_t ID = 116357727;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<callbackQueryAnswer>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getCallbackQueryMessage final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  int64 callback_query_id_;

  getCallbackQueryMessage();

  getCallbackQueryMessage(int53 chat_id_, int53 message_id_, int64 callback_query_id_);

  static const std::int32_t ID = -1121939086;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<message>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChat final : public Function {
 public:
  int53 chat_id_;

  getChat();

  explicit getChat(int53 chat_id_);

  static const std::int32_t ID = 1866601536;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chat>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatAdministrators final : public Function {
 public:
  int53 chat_id_;

  getChatAdministrators();

  explicit getChatAdministrators(int53 chat_id_);

  static const std::int32_t ID = 1544468155;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatAdministrators>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatAvailableMessageSenders final : public Function {
 public:
  int53 chat_id_;

  getChatAvailableMessageSenders();

  explicit getChatAvailableMessageSenders(int53 chat_id_);

  static const std::int32_t ID = 1158670635;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatMessageSenders>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatEventLog final : public Function {
 public:
  int53 chat_id_;
  string query_;
  int64 from_event_id_;
  int32 limit_;
  object_ptr<chatEventLogFilters> filters_;
  array<int53> user_ids_;

  getChatEventLog();

  getChatEventLog(int53 chat_id_, string const &query_, int64 from_event_id_, int32 limit_, object_ptr<chatEventLogFilters> &&filters_, array<int53> &&user_ids_);

  static const std::int32_t ID = -1281344669;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatEvents>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatFilter final : public Function {
 public:
  int32 chat_filter_id_;

  getChatFilter();

  explicit getChatFilter(int32 chat_filter_id_);

  static const std::int32_t ID = 1826317803;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatFilter>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatFilterDefaultIconName final : public Function {
 public:
  object_ptr<chatFilter> filter_;

  getChatFilterDefaultIconName();

  explicit getChatFilterDefaultIconName(object_ptr<chatFilter> &&filter_);

  static const std::int32_t ID = -1339828680;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<text>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatHistory final : public Function {
 public:
  int53 chat_id_;
  int53 from_message_id_;
  int32 offset_;
  int32 limit_;
  bool only_local_;

  getChatHistory();

  getChatHistory(int53 chat_id_, int53 from_message_id_, int32 offset_, int32 limit_, bool only_local_);

  static const std::int32_t ID = -799960451;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatInviteLink final : public Function {
 public:
  int53 chat_id_;
  string invite_link_;

  getChatInviteLink();

  getChatInviteLink(int53 chat_id_, string const &invite_link_);

  static const std::int32_t ID = -479575555;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatInviteLink>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatInviteLinkCounts final : public Function {
 public:
  int53 chat_id_;

  getChatInviteLinkCounts();

  explicit getChatInviteLinkCounts(int53 chat_id_);

  static const std::int32_t ID = 890299025;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatInviteLinkCounts>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatInviteLinkMembers final : public Function {
 public:
  int53 chat_id_;
  string invite_link_;
  object_ptr<chatInviteLinkMember> offset_member_;
  int32 limit_;

  getChatInviteLinkMembers();

  getChatInviteLinkMembers(int53 chat_id_, string const &invite_link_, object_ptr<chatInviteLinkMember> &&offset_member_, int32 limit_);

  static const std::int32_t ID = -977921638;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatInviteLinkMembers>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatInviteLinks final : public Function {
 public:
  int53 chat_id_;
  int53 creator_user_id_;
  bool is_revoked_;
  int32 offset_date_;
  string offset_invite_link_;
  int32 limit_;

  getChatInviteLinks();

  getChatInviteLinks(int53 chat_id_, int53 creator_user_id_, bool is_revoked_, int32 offset_date_, string const &offset_invite_link_, int32 limit_);

  static const std::int32_t ID = 883252396;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatInviteLinks>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatJoinRequests final : public Function {
 public:
  int53 chat_id_;
  string invite_link_;
  string query_;
  object_ptr<chatJoinRequest> offset_request_;
  int32 limit_;

  getChatJoinRequests();

  getChatJoinRequests(int53 chat_id_, string const &invite_link_, string const &query_, object_ptr<chatJoinRequest> &&offset_request_, int32 limit_);

  static const std::int32_t ID = -388428126;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatJoinRequests>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatListsToAddChat final : public Function {
 public:
  int53 chat_id_;

  getChatListsToAddChat();

  explicit getChatListsToAddChat(int53 chat_id_);

  static const std::int32_t ID = 654956193;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatLists>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatMember final : public Function {
 public:
  int53 chat_id_;
  object_ptr<MessageSender> member_id_;

  getChatMember();

  getChatMember(int53 chat_id_, object_ptr<MessageSender> &&member_id_);

  static const std::int32_t ID = -792636814;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatMember>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatMessageByDate final : public Function {
 public:
  int53 chat_id_;
  int32 date_;

  getChatMessageByDate();

  getChatMessageByDate(int53 chat_id_, int32 date_);

  static const std::int32_t ID = 1062564150;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<message>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatMessageCalendar final : public Function {
 public:
  int53 chat_id_;
  object_ptr<SearchMessagesFilter> filter_;
  int53 from_message_id_;

  getChatMessageCalendar();

  getChatMessageCalendar(int53 chat_id_, object_ptr<SearchMessagesFilter> &&filter_, int53 from_message_id_);

  static const std::int32_t ID = -326164204;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messageCalendar>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatMessageCount final : public Function {
 public:
  int53 chat_id_;
  object_ptr<SearchMessagesFilter> filter_;
  bool return_local_;

  getChatMessageCount();

  getChatMessageCount(int53 chat_id_, object_ptr<SearchMessagesFilter> &&filter_, bool return_local_);

  static const std::int32_t ID = 205435308;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<count>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatMessagePosition final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  object_ptr<SearchMessagesFilter> filter_;
  int53 message_thread_id_;

  getChatMessagePosition();

  getChatMessagePosition(int53 chat_id_, int53 message_id_, object_ptr<SearchMessagesFilter> &&filter_, int53 message_thread_id_);

  static const std::int32_t ID = -559474920;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<count>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatNotificationSettingsExceptions final : public Function {
 public:
  object_ptr<NotificationSettingsScope> scope_;
  bool compare_sound_;

  getChatNotificationSettingsExceptions();

  getChatNotificationSettingsExceptions(object_ptr<NotificationSettingsScope> &&scope_, bool compare_sound_);

  static const std::int32_t ID = 201199121;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chats>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatPinnedMessage final : public Function {
 public:
  int53 chat_id_;

  getChatPinnedMessage();

  explicit getChatPinnedMessage(int53 chat_id_);

  static const std::int32_t ID = 359865008;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<message>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatScheduledMessages final : public Function {
 public:
  int53 chat_id_;

  getChatScheduledMessages();

  explicit getChatScheduledMessages(int53 chat_id_);

  static const std::int32_t ID = -549638149;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatSparseMessagePositions final : public Function {
 public:
  int53 chat_id_;
  object_ptr<SearchMessagesFilter> filter_;
  int53 from_message_id_;
  int32 limit_;

  getChatSparseMessagePositions();

  getChatSparseMessagePositions(int53 chat_id_, object_ptr<SearchMessagesFilter> &&filter_, int53 from_message_id_, int32 limit_);

  static const std::int32_t ID = -462227580;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messagePositions>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatSponsoredMessages final : public Function {
 public:
  int53 chat_id_;

  getChatSponsoredMessages();

  explicit getChatSponsoredMessages(int53 chat_id_);

  static const std::int32_t ID = 1353203864;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<sponsoredMessages>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChatStatistics final : public Function {
 public:
  int53 chat_id_;
  bool is_dark_;

  getChatStatistics();

  getChatStatistics(int53 chat_id_, bool is_dark_);

  static const std::int32_t ID = 327057816;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ChatStatistics>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getChats final : public Function {
 public:
  object_ptr<ChatList> chat_list_;
  int32 limit_;

  getChats();

  getChats(object_ptr<ChatList> &&chat_list_, int32 limit_);

  static const std::int32_t ID = -972768574;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chats>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getCommands final : public Function {
 public:
  object_ptr<BotCommandScope> scope_;
  string language_code_;

  getCommands();

  getCommands(object_ptr<BotCommandScope> &&scope_, string const &language_code_);

  static const std::int32_t ID = 1488621559;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<botCommands>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getConnectedWebsites final : public Function {
 public:

  getConnectedWebsites();

  static const std::int32_t ID = -170536110;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<connectedWebsites>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getContacts final : public Function {
 public:

  getContacts();

  static const std::int32_t ID = -1417722768;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<users>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getCountries final : public Function {
 public:

  getCountries();

  static const std::int32_t ID = -51902050;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<countries>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getCountryCode final : public Function {
 public:

  getCountryCode();

  static const std::int32_t ID = 1540593906;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<text>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getCreatedPublicChats final : public Function {
 public:
  object_ptr<PublicChatType> type_;

  getCreatedPublicChats();

  explicit getCreatedPublicChats(object_ptr<PublicChatType> &&type_);

  static const std::int32_t ID = 710354415;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chats>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getCurrentState final : public Function {
 public:

  getCurrentState();

  static const std::int32_t ID = -1191417719;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<updates>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getCustomEmojiReactionAnimations final : public Function {
 public:

  getCustomEmojiReactionAnimations();

  static const std::int32_t ID = 1232375250;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickers>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getCustomEmojiStickers final : public Function {
 public:
  array<int64> custom_emoji_ids_;

  getCustomEmojiStickers();

  explicit getCustomEmojiStickers(array<int64> &&custom_emoji_ids_);

  static const std::int32_t ID = -2127427955;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickers>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getDatabaseStatistics final : public Function {
 public:

  getDatabaseStatistics();

  static const std::int32_t ID = -1942760263;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<databaseStatistics>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getDeepLinkInfo final : public Function {
 public:
  string link_;

  getDeepLinkInfo();

  explicit getDeepLinkInfo(string const &link_);

  static const std::int32_t ID = 680673150;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<deepLinkInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getDefaultEmojiStatuses final : public Function {
 public:

  getDefaultEmojiStatuses();

  static const std::int32_t ID = 618946243;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<emojiStatuses>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getEmojiReaction final : public Function {
 public:
  string emoji_;

  getEmojiReaction();

  explicit getEmojiReaction(string const &emoji_);

  static const std::int32_t ID = -449572388;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<emojiReaction>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getEmojiSuggestionsUrl final : public Function {
 public:
  string language_code_;

  getEmojiSuggestionsUrl();

  explicit getEmojiSuggestionsUrl(string const &language_code_);

  static const std::int32_t ID = -1404101841;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<httpUrl>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getExternalLink final : public Function {
 public:
  string link_;
  bool allow_write_access_;

  getExternalLink();

  getExternalLink(string const &link_, bool allow_write_access_);

  static const std::int32_t ID = 1586688235;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<httpUrl>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getExternalLinkInfo final : public Function {
 public:
  string link_;

  getExternalLinkInfo();

  explicit getExternalLinkInfo(string const &link_);

  static const std::int32_t ID = 1175288383;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<LoginUrlInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getFavoriteStickers final : public Function {
 public:

  getFavoriteStickers();

  static const std::int32_t ID = -338964672;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickers>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getFile final : public Function {
 public:
  int32 file_id_;

  getFile();

  explicit getFile(int32 file_id_);

  static const std::int32_t ID = 1553923406;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<file>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getFileDownloadedPrefixSize final : public Function {
 public:
  int32 file_id_;
  int53 offset_;

  getFileDownloadedPrefixSize();

  getFileDownloadedPrefixSize(int32 file_id_, int53 offset_);

  static const std::int32_t ID = 855948589;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<fileDownloadedPrefixSize>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getFileExtension final : public Function {
 public:
  string mime_type_;

  getFileExtension();

  explicit getFileExtension(string const &mime_type_);

  static const std::int32_t ID = -106055372;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<text>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getFileMimeType final : public Function {
 public:
  string file_name_;

  getFileMimeType();

  explicit getFileMimeType(string const &file_name_);

  static const std::int32_t ID = -2073879671;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<text>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getForumTopicDefaultIcons final : public Function {
 public:

  getForumTopicDefaultIcons();

  static const std::int32_t ID = 1479898332;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickers>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getGameHighScores final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  int53 user_id_;

  getGameHighScores();

  getGameHighScores(int53 chat_id_, int53 message_id_, int53 user_id_);

  static const std::int32_t ID = 15746459;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<gameHighScores>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getGroupCall final : public Function {
 public:
  int32 group_call_id_;

  getGroupCall();

  explicit getGroupCall(int32 group_call_id_);

  static const std::int32_t ID = 1468491406;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<groupCall>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getGroupCallInviteLink final : public Function {
 public:
  int32 group_call_id_;
  bool can_self_unmute_;

  getGroupCallInviteLink();

  getGroupCallInviteLink(int32 group_call_id_, bool can_self_unmute_);

  static const std::int32_t ID = 719407396;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<httpUrl>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getGroupCallStreamSegment final : public Function {
 public:
  int32 group_call_id_;
  int53 time_offset_;
  int32 scale_;
  int32 channel_id_;
  object_ptr<GroupCallVideoQuality> video_quality_;

  getGroupCallStreamSegment();

  getGroupCallStreamSegment(int32 group_call_id_, int53 time_offset_, int32 scale_, int32 channel_id_, object_ptr<GroupCallVideoQuality> &&video_quality_);

  static const std::int32_t ID = -2077959515;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<filePart>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getGroupCallStreams final : public Function {
 public:
  int32 group_call_id_;

  getGroupCallStreams();

  explicit getGroupCallStreams(int32 group_call_id_);

  static const std::int32_t ID = -1619226268;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<groupCallStreams>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getGroupsInCommon final : public Function {
 public:
  int53 user_id_;
  int53 offset_chat_id_;
  int32 limit_;

  getGroupsInCommon();

  getGroupsInCommon(int53 user_id_, int53 offset_chat_id_, int32 limit_);

  static const std::int32_t ID = 381539178;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chats>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getImportedContactCount final : public Function {
 public:

  getImportedContactCount();

  static const std::int32_t ID = -656336346;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<count>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getInactiveSupergroupChats final : public Function {
 public:

  getInactiveSupergroupChats();

  static const std::int32_t ID = -657720907;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chats>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getInlineGameHighScores final : public Function {
 public:
  string inline_message_id_;
  int53 user_id_;

  getInlineGameHighScores();

  getInlineGameHighScores(string const &inline_message_id_, int53 user_id_);

  static const std::int32_t ID = -533107798;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<gameHighScores>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getInlineQueryResults final : public Function {
 public:
  int53 bot_user_id_;
  int53 chat_id_;
  object_ptr<location> user_location_;
  string query_;
  string offset_;

  getInlineQueryResults();

  getInlineQueryResults(int53 bot_user_id_, int53 chat_id_, object_ptr<location> &&user_location_, string const &query_, string const &offset_);

  static const std::int32_t ID = 2044524652;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<inlineQueryResults>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getInstalledStickerSets final : public Function {
 public:
  object_ptr<StickerType> sticker_type_;

  getInstalledStickerSets();

  explicit getInstalledStickerSets(object_ptr<StickerType> &&sticker_type_);

  static const std::int32_t ID = 1630467830;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickerSets>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getInternalLinkType final : public Function {
 public:
  string link_;

  getInternalLinkType();

  explicit getInternalLinkType(string const &link_);

  static const std::int32_t ID = -1948428535;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<InternalLinkType>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getJsonString final : public Function {
 public:
  object_ptr<JsonValue> json_value_;

  getJsonString();

  explicit getJsonString(object_ptr<JsonValue> &&json_value_);

  static const std::int32_t ID = 663458849;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<text>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getJsonValue final : public Function {
 public:
  string json_;

  getJsonValue();

  explicit getJsonValue(string const &json_);

  static const std::int32_t ID = -1829086715;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<JsonValue>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getLanguagePackInfo final : public Function {
 public:
  string language_pack_id_;

  getLanguagePackInfo();

  explicit getLanguagePackInfo(string const &language_pack_id_);

  static const std::int32_t ID = 2077809320;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<languagePackInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getLanguagePackString final : public Function {
 public:
  string language_pack_database_path_;
  string localization_target_;
  string language_pack_id_;
  string key_;

  getLanguagePackString();

  getLanguagePackString(string const &language_pack_database_path_, string const &localization_target_, string const &language_pack_id_, string const &key_);

  static const std::int32_t ID = 150789747;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<LanguagePackStringValue>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getLanguagePackStrings final : public Function {
 public:
  string language_pack_id_;
  array<string> keys_;

  getLanguagePackStrings();

  getLanguagePackStrings(string const &language_pack_id_, array<string> &&keys_);

  static const std::int32_t ID = 1246259088;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<languagePackStrings>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getLocalizationTargetInfo final : public Function {
 public:
  bool only_local_;

  getLocalizationTargetInfo();

  explicit getLocalizationTargetInfo(bool only_local_);

  static const std::int32_t ID = 1849499526;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<localizationTargetInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getLogStream final : public Function {
 public:

  getLogStream();

  static const std::int32_t ID = 1167608667;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<LogStream>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getLogTagVerbosityLevel final : public Function {
 public:
  string tag_;

  getLogTagVerbosityLevel();

  explicit getLogTagVerbosityLevel(string const &tag_);

  static const std::int32_t ID = 951004547;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<logVerbosityLevel>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getLogTags final : public Function {
 public:

  getLogTags();

  static const std::int32_t ID = -254449190;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<logTags>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getLogVerbosityLevel final : public Function {
 public:

  getLogVerbosityLevel();

  static const std::int32_t ID = 594057956;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<logVerbosityLevel>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getLoginUrl final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  int53 button_id_;
  bool allow_write_access_;

  getLoginUrl();

  getLoginUrl(int53 chat_id_, int53 message_id_, int53 button_id_, bool allow_write_access_);

  static const std::int32_t ID = 791844305;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<httpUrl>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getLoginUrlInfo final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  int53 button_id_;

  getLoginUrlInfo();

  getLoginUrlInfo(int53 chat_id_, int53 message_id_, int53 button_id_);

  static const std::int32_t ID = -859202125;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<LoginUrlInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getMapThumbnailFile final : public Function {
 public:
  object_ptr<location> location_;
  int32 zoom_;
  int32 width_;
  int32 height_;
  int32 scale_;
  int53 chat_id_;

  getMapThumbnailFile();

  getMapThumbnailFile(object_ptr<location> &&location_, int32 zoom_, int32 width_, int32 height_, int32 scale_, int53 chat_id_);

  static const std::int32_t ID = -152660070;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<file>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getMarkdownText final : public Function {
 public:
  object_ptr<formattedText> text_;

  getMarkdownText();

  explicit getMarkdownText(object_ptr<formattedText> &&text_);

  static const std::int32_t ID = 164524584;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<formattedText>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getMe final : public Function {
 public:

  getMe();

  static const std::int32_t ID = -191516033;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<user>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getMenuButton final : public Function {
 public:
  int53 user_id_;

  getMenuButton();

  explicit getMenuButton(int53 user_id_);

  static const std::int32_t ID = -437324736;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<botMenuButton>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getMessage final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;

  getMessage();

  getMessage(int53 chat_id_, int53 message_id_);

  static const std::int32_t ID = -1821196160;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<message>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getMessageAddedReactions final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  object_ptr<ReactionType> reaction_type_;
  string offset_;
  int32 limit_;

  getMessageAddedReactions();

  getMessageAddedReactions(int53 chat_id_, int53 message_id_, object_ptr<ReactionType> &&reaction_type_, string const &offset_, int32 limit_);

  static const std::int32_t ID = 2110172754;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<addedReactions>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getMessageAvailableReactions final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  int32 row_size_;

  getMessageAvailableReactions();

  getMessageAvailableReactions(int53 chat_id_, int53 message_id_, int32 row_size_);

  static const std::int32_t ID = 1994098354;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<availableReactions>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getMessageEmbeddingCode final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  bool for_album_;

  getMessageEmbeddingCode();

  getMessageEmbeddingCode(int53 chat_id_, int53 message_id_, bool for_album_);

  static const std::int32_t ID = 1654967561;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<text>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getMessageFileType final : public Function {
 public:
  string message_file_head_;

  getMessageFileType();

  explicit getMessageFileType(string const &message_file_head_);

  static const std::int32_t ID = -490270764;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<MessageFileType>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getMessageImportConfirmationText final : public Function {
 public:
  int53 chat_id_;

  getMessageImportConfirmationText();

  explicit getMessageImportConfirmationText(int53 chat_id_);

  static const std::int32_t ID = 390627752;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<text>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getMessageLink final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  int32 media_timestamp_;
  bool for_album_;
  bool in_message_thread_;

  getMessageLink();

  getMessageLink(int53 chat_id_, int53 message_id_, int32 media_timestamp_, bool for_album_, bool in_message_thread_);

  static const std::int32_t ID = -984158342;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messageLink>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getMessageLinkInfo final : public Function {
 public:
  string url_;

  getMessageLinkInfo();

  explicit getMessageLinkInfo(string const &url_);

  static const std::int32_t ID = -700533672;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messageLinkInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getMessageLocally final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;

  getMessageLocally();

  getMessageLocally(int53 chat_id_, int53 message_id_);

  static const std::int32_t ID = -603575444;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<message>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getMessagePublicForwards final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  string offset_;
  int32 limit_;

  getMessagePublicForwards();

  getMessagePublicForwards(int53 chat_id_, int53 message_id_, string const &offset_, int32 limit_);

  static const std::int32_t ID = 1611049289;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<foundMessages>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getMessageStatistics final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  bool is_dark_;

  getMessageStatistics();

  getMessageStatistics(int53 chat_id_, int53 message_id_, bool is_dark_);

  static const std::int32_t ID = 1270194648;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messageStatistics>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getMessageThread final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;

  getMessageThread();

  getMessageThread(int53 chat_id_, int53 message_id_);

  static const std::int32_t ID = 2062695998;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messageThreadInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getMessageThreadHistory final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  int53 from_message_id_;
  int32 offset_;
  int32 limit_;

  getMessageThreadHistory();

  getMessageThreadHistory(int53 chat_id_, int53 message_id_, int53 from_message_id_, int32 offset_, int32 limit_);

  static const std::int32_t ID = -1808411608;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getMessageViewers final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;

  getMessageViewers();

  getMessageViewers(int53 chat_id_, int53 message_id_);

  static const std::int32_t ID = -1879496099;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<users>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getMessages final : public Function {
 public:
  int53 chat_id_;
  array<int53> message_ids_;

  getMessages();

  getMessages(int53 chat_id_, array<int53> &&message_ids_);

  static const std::int32_t ID = 425299338;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getNetworkStatistics final : public Function {
 public:
  bool only_current_;

  getNetworkStatistics();

  explicit getNetworkStatistics(bool only_current_);

  static const std::int32_t ID = -986228706;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<networkStatistics>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getOption final : public Function {
 public:
  string name_;

  getOption();

  explicit getOption(string const &name_);

  static const std::int32_t ID = -1572495746;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<OptionValue>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getPassportAuthorizationForm final : public Function {
 public:
  int53 bot_user_id_;
  string scope_;
  string public_key_;
  string nonce_;

  getPassportAuthorizationForm();

  getPassportAuthorizationForm(int53 bot_user_id_, string const &scope_, string const &public_key_, string const &nonce_);

  static const std::int32_t ID = 1636107398;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<passportAuthorizationForm>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getPassportAuthorizationFormAvailableElements final : public Function {
 public:
  int32 autorization_form_id_;
  string password_;

  getPassportAuthorizationFormAvailableElements();

  getPassportAuthorizationFormAvailableElements(int32 autorization_form_id_, string const &password_);

  static const std::int32_t ID = 1738134754;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<passportElementsWithErrors>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getPassportElement final : public Function {
 public:
  object_ptr<PassportElementType> type_;
  string password_;

  getPassportElement();

  getPassportElement(object_ptr<PassportElementType> &&type_, string const &password_);

  static const std::int32_t ID = -1882398342;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<PassportElement>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getPasswordState final : public Function {
 public:

  getPasswordState();

  static const std::int32_t ID = -174752904;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<passwordState>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getPaymentForm final : public Function {
 public:
  object_ptr<InputInvoice> input_invoice_;
  object_ptr<themeParameters> theme_;

  getPaymentForm();

  getPaymentForm(object_ptr<InputInvoice> &&input_invoice_, object_ptr<themeParameters> &&theme_);

  static const std::int32_t ID = -1924172076;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<paymentForm>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getPaymentReceipt final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;

  getPaymentReceipt();

  getPaymentReceipt(int53 chat_id_, int53 message_id_);

  static const std::int32_t ID = 1013758294;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<paymentReceipt>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getPhoneNumberInfo final : public Function {
 public:
  string phone_number_prefix_;

  getPhoneNumberInfo();

  explicit getPhoneNumberInfo(string const &phone_number_prefix_);

  static const std::int32_t ID = -1608344583;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<phoneNumberInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getPhoneNumberInfoSync final : public Function {
 public:
  string language_code_;
  string phone_number_prefix_;

  getPhoneNumberInfoSync();

  getPhoneNumberInfoSync(string const &language_code_, string const &phone_number_prefix_);

  static const std::int32_t ID = 547061048;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<phoneNumberInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getPollVoters final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  int32 option_id_;
  int32 offset_;
  int32 limit_;

  getPollVoters();

  getPollVoters(int53 chat_id_, int53 message_id_, int32 option_id_, int32 offset_, int32 limit_);

  static const std::int32_t ID = 2075288734;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<users>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getPreferredCountryLanguage final : public Function {
 public:
  string country_code_;

  getPreferredCountryLanguage();

  explicit getPreferredCountryLanguage(string const &country_code_);

  static const std::int32_t ID = -933049386;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<text>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getPremiumFeatures final : public Function {
 public:
  object_ptr<PremiumSource> source_;

  getPremiumFeatures();

  explicit getPremiumFeatures(object_ptr<PremiumSource> &&source_);

  static const std::int32_t ID = -1260640695;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<premiumFeatures>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getPremiumLimit final : public Function {
 public:
  object_ptr<PremiumLimitType> limit_type_;

  getPremiumLimit();

  explicit getPremiumLimit(object_ptr<PremiumLimitType> &&limit_type_);

  static const std::int32_t ID = 1075313898;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<premiumLimit>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getPremiumState final : public Function {
 public:

  getPremiumState();

  static const std::int32_t ID = 663632610;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<premiumState>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getPremiumStickerExamples final : public Function {
 public:

  getPremiumStickerExamples();

  static const std::int32_t ID = 1399442328;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickers>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getPremiumStickers final : public Function {
 public:
  int32 limit_;

  getPremiumStickers();

  explicit getPremiumStickers(int32 limit_);

  static const std::int32_t ID = -280950192;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickers>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getProxies final : public Function {
 public:

  getProxies();

  static const std::int32_t ID = -95026381;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<proxies>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getProxyLink final : public Function {
 public:
  int32 proxy_id_;

  getProxyLink();

  explicit getProxyLink(int32 proxy_id_);

  static const std::int32_t ID = -1054495112;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<httpUrl>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getPushReceiverId final : public Function {
 public:
  string payload_;

  getPushReceiverId();

  explicit getPushReceiverId(string const &payload_);

  static const std::int32_t ID = -286505294;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<pushReceiverId>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getRecentEmojiStatuses final : public Function {
 public:

  getRecentEmojiStatuses();

  static const std::int32_t ID = -1371914967;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<emojiStatuses>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getRecentInlineBots final : public Function {
 public:

  getRecentInlineBots();

  static const std::int32_t ID = 1437823548;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<users>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getRecentStickers final : public Function {
 public:
  bool is_attached_;

  getRecentStickers();

  explicit getRecentStickers(bool is_attached_);

  static const std::int32_t ID = -579622241;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickers>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getRecentlyOpenedChats final : public Function {
 public:
  int32 limit_;

  getRecentlyOpenedChats();

  explicit getRecentlyOpenedChats(int32 limit_);

  static const std::int32_t ID = -1924156893;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chats>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getRecentlyVisitedTMeUrls final : public Function {
 public:
  string referrer_;

  getRecentlyVisitedTMeUrls();

  explicit getRecentlyVisitedTMeUrls(string const &referrer_);

  static const std::int32_t ID = 806754961;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<tMeUrls>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getRecommendedChatFilters final : public Function {
 public:

  getRecommendedChatFilters();

  static const std::int32_t ID = -779390746;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<recommendedChatFilters>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getRecoveryEmailAddress final : public Function {
 public:
  string password_;

  getRecoveryEmailAddress();

  explicit getRecoveryEmailAddress(string const &password_);

  static const std::int32_t ID = -1594770947;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<recoveryEmailAddress>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getRemoteFile final : public Function {
 public:
  string remote_file_id_;
  object_ptr<FileType> file_type_;

  getRemoteFile();

  getRemoteFile(string const &remote_file_id_, object_ptr<FileType> &&file_type_);

  static const std::int32_t ID = 2137204530;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<file>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getRepliedMessage final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;

  getRepliedMessage();

  getRepliedMessage(int53 chat_id_, int53 message_id_);

  static const std::int32_t ID = -641918531;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<message>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getSavedAnimations final : public Function {
 public:

  getSavedAnimations();

  static const std::int32_t ID = 7051032;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<animations>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getSavedNotificationSound final : public Function {
 public:
  int64 notification_sound_id_;

  getSavedNotificationSound();

  explicit getSavedNotificationSound(int64 notification_sound_id_);

  static const std::int32_t ID = 459569431;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<notificationSounds>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getSavedNotificationSounds final : public Function {
 public:

  getSavedNotificationSounds();

  static const std::int32_t ID = -1070305368;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<notificationSounds>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getSavedOrderInfo final : public Function {
 public:

  getSavedOrderInfo();

  static const std::int32_t ID = -1152016675;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<orderInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getScopeNotificationSettings final : public Function {
 public:
  object_ptr<NotificationSettingsScope> scope_;

  getScopeNotificationSettings();

  explicit getScopeNotificationSettings(object_ptr<NotificationSettingsScope> &&scope_);

  static const std::int32_t ID = -995613361;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<scopeNotificationSettings>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getSecretChat final : public Function {
 public:
  int32 secret_chat_id_;

  getSecretChat();

  explicit getSecretChat(int32 secret_chat_id_);

  static const std::int32_t ID = 40599169;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<secretChat>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getStatisticalGraph final : public Function {
 public:
  int53 chat_id_;
  string token_;
  int53 x_;

  getStatisticalGraph();

  getStatisticalGraph(int53 chat_id_, string const &token_, int53 x_);

  static const std::int32_t ID = 1100975515;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<StatisticalGraph>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getStickerEmojis final : public Function {
 public:
  object_ptr<InputFile> sticker_;

  getStickerEmojis();

  explicit getStickerEmojis(object_ptr<InputFile> &&sticker_);

  static const std::int32_t ID = -1895508665;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<emojis>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getStickerSet final : public Function {
 public:
  int64 set_id_;

  getStickerSet();

  explicit getStickerSet(int64 set_id_);

  static const std::int32_t ID = 1052318659;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickerSet>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getStickers final : public Function {
 public:
  object_ptr<StickerType> sticker_type_;
  string query_;
  int32 limit_;
  int53 chat_id_;

  getStickers();

  getStickers(object_ptr<StickerType> &&sticker_type_, string const &query_, int32 limit_, int53 chat_id_);

  static const std::int32_t ID = 1158058819;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickers>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getStorageStatistics final : public Function {
 public:
  int32 chat_limit_;

  getStorageStatistics();

  explicit getStorageStatistics(int32 chat_limit_);

  static const std::int32_t ID = -853193929;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<storageStatistics>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getStorageStatisticsFast final : public Function {
 public:

  getStorageStatisticsFast();

  static const std::int32_t ID = 61368066;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<storageStatisticsFast>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getSuggestedFileName final : public Function {
 public:
  int32 file_id_;
  string directory_;

  getSuggestedFileName();

  getSuggestedFileName(int32 file_id_, string const &directory_);

  static const std::int32_t ID = -2049399674;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<text>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getSuggestedStickerSetName final : public Function {
 public:
  string title_;

  getSuggestedStickerSetName();

  explicit getSuggestedStickerSetName(string const &title_);

  static const std::int32_t ID = -1340995520;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<text>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getSuitableDiscussionChats final : public Function {
 public:

  getSuitableDiscussionChats();

  static const std::int32_t ID = 49044982;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chats>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getSupergroup final : public Function {
 public:
  int53 supergroup_id_;

  getSupergroup();

  explicit getSupergroup(int53 supergroup_id_);

  static const std::int32_t ID = 989663458;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<supergroup>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getSupergroupFullInfo final : public Function {
 public:
  int53 supergroup_id_;

  getSupergroupFullInfo();

  explicit getSupergroupFullInfo(int53 supergroup_id_);

  static const std::int32_t ID = 1099776056;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<supergroupFullInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getSupergroupMembers final : public Function {
 public:
  int53 supergroup_id_;
  object_ptr<SupergroupMembersFilter> filter_;
  int32 offset_;
  int32 limit_;

  getSupergroupMembers();

  getSupergroupMembers(int53 supergroup_id_, object_ptr<SupergroupMembersFilter> &&filter_, int32 offset_, int32 limit_);

  static const std::int32_t ID = -570940984;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatMembers>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getSupportUser final : public Function {
 public:

  getSupportUser();

  static const std::int32_t ID = -1733497700;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<user>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getTemporaryPasswordState final : public Function {
 public:

  getTemporaryPasswordState();

  static const std::int32_t ID = -12670830;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<temporaryPasswordState>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getTextEntities final : public Function {
 public:
  string text_;

  getTextEntities();

  explicit getTextEntities(string const &text_);

  static const std::int32_t ID = -341490693;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<textEntities>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getThemeParametersJsonString final : public Function {
 public:
  object_ptr<themeParameters> theme_;

  getThemeParametersJsonString();

  explicit getThemeParametersJsonString(object_ptr<themeParameters> &&theme_);

  static const std::int32_t ID = -1850145288;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<text>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getThemedEmojiStatuses final : public Function {
 public:

  getThemedEmojiStatuses();

  static const std::int32_t ID = 1791346882;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<emojiStatuses>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getTopChats final : public Function {
 public:
  object_ptr<TopChatCategory> category_;
  int32 limit_;

  getTopChats();

  getTopChats(object_ptr<TopChatCategory> &&category_, int32 limit_);

  static const std::int32_t ID = -388410847;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chats>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getTrendingStickerSets final : public Function {
 public:
  object_ptr<StickerType> sticker_type_;
  int32 offset_;
  int32 limit_;

  getTrendingStickerSets();

  getTrendingStickerSets(object_ptr<StickerType> &&sticker_type_, int32 offset_, int32 limit_);

  static const std::int32_t ID = -531085986;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<trendingStickerSets>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getUser final : public Function {
 public:
  int53 user_id_;

  getUser();

  explicit getUser(int53 user_id_);

  static const std::int32_t ID = 1117363211;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<user>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getUserFullInfo final : public Function {
 public:
  int53 user_id_;

  getUserFullInfo();

  explicit getUserFullInfo(int53 user_id_);

  static const std::int32_t ID = -776823720;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<userFullInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getUserPrivacySettingRules final : public Function {
 public:
  object_ptr<UserPrivacySetting> setting_;

  getUserPrivacySettingRules();

  explicit getUserPrivacySettingRules(object_ptr<UserPrivacySetting> &&setting_);

  static const std::int32_t ID = -2077223311;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<userPrivacySettingRules>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getUserProfilePhotos final : public Function {
 public:
  int53 user_id_;
  int32 offset_;
  int32 limit_;

  getUserProfilePhotos();

  getUserProfilePhotos(int53 user_id_, int32 offset_, int32 limit_);

  static const std::int32_t ID = -908132798;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatPhotos>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getUserSupportInfo final : public Function {
 public:
  int53 user_id_;

  getUserSupportInfo();

  explicit getUserSupportInfo(int53 user_id_);

  static const std::int32_t ID = 1957008133;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<userSupportInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getVideoChatAvailableParticipants final : public Function {
 public:
  int53 chat_id_;

  getVideoChatAvailableParticipants();

  explicit getVideoChatAvailableParticipants(int53 chat_id_);

  static const std::int32_t ID = -1000496379;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messageSenders>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getVideoChatRtmpUrl final : public Function {
 public:
  int53 chat_id_;

  getVideoChatRtmpUrl();

  explicit getVideoChatRtmpUrl(int53 chat_id_);

  static const std::int32_t ID = 1210784543;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<rtmpUrl>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getWebAppUrl final : public Function {
 public:
  int53 bot_user_id_;
  string url_;
  object_ptr<themeParameters> theme_;
  string application_name_;

  getWebAppUrl();

  getWebAppUrl(int53 bot_user_id_, string const &url_, object_ptr<themeParameters> &&theme_, string const &application_name_);

  static const std::int32_t ID = 1690578110;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<httpUrl>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getWebPageInstantView final : public Function {
 public:
  string url_;
  bool force_full_;

  getWebPageInstantView();

  getWebPageInstantView(string const &url_, bool force_full_);

  static const std::int32_t ID = -1962649975;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<webPageInstantView>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class getWebPagePreview final : public Function {
 public:
  object_ptr<formattedText> text_;

  getWebPagePreview();

  explicit getWebPagePreview(object_ptr<formattedText> &&text_);

  static const std::int32_t ID = 573441580;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<webPage>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class hideSuggestedAction final : public Function {
 public:
  object_ptr<SuggestedAction> action_;

  hideSuggestedAction();

  explicit hideSuggestedAction(object_ptr<SuggestedAction> &&action_);

  static const std::int32_t ID = -1561384065;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class importContacts final : public Function {
 public:
  array<object_ptr<contact>> contacts_;

  importContacts();

  explicit importContacts(array<object_ptr<contact>> &&contacts_);

  static const std::int32_t ID = -215132767;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<importedContacts>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class importMessages final : public Function {
 public:
  int53 chat_id_;
  object_ptr<InputFile> message_file_;
  array<object_ptr<InputFile>> attached_files_;

  importMessages();

  importMessages(int53 chat_id_, object_ptr<InputFile> &&message_file_, array<object_ptr<InputFile>> &&attached_files_);

  static const std::int32_t ID = -1864116784;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inviteGroupCallParticipants final : public Function {
 public:
  int32 group_call_id_;
  array<int53> user_ids_;

  inviteGroupCallParticipants();

  inviteGroupCallParticipants(int32 group_call_id_, array<int53> &&user_ids_);

  static const std::int32_t ID = 1867097679;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class joinChat final : public Function {
 public:
  int53 chat_id_;

  joinChat();

  explicit joinChat(int53 chat_id_);

  static const std::int32_t ID = 326769313;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class joinChatByInviteLink final : public Function {
 public:
  string invite_link_;

  joinChatByInviteLink();

  explicit joinChatByInviteLink(string const &invite_link_);

  static const std::int32_t ID = -1049973882;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chat>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class joinGroupCall final : public Function {
 public:
  int32 group_call_id_;
  object_ptr<MessageSender> participant_id_;
  int32 audio_source_id_;
  string payload_;
  bool is_muted_;
  bool is_my_video_enabled_;
  string invite_hash_;

  joinGroupCall();

  joinGroupCall(int32 group_call_id_, object_ptr<MessageSender> &&participant_id_, int32 audio_source_id_, string const &payload_, bool is_muted_, bool is_my_video_enabled_, string const &invite_hash_);

  static const std::int32_t ID = -1043773467;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<text>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class leaveChat final : public Function {
 public:
  int53 chat_id_;

  leaveChat();

  explicit leaveChat(int53 chat_id_);

  static const std::int32_t ID = -1825080735;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class leaveGroupCall final : public Function {
 public:
  int32 group_call_id_;

  leaveGroupCall();

  explicit leaveGroupCall(int32 group_call_id_);

  static const std::int32_t ID = 980152233;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class loadChats final : public Function {
 public:
  object_ptr<ChatList> chat_list_;
  int32 limit_;

  loadChats();

  loadChats(object_ptr<ChatList> &&chat_list_, int32 limit_);

  static const std::int32_t ID = -1885635205;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class loadGroupCallParticipants final : public Function {
 public:
  int32 group_call_id_;
  int32 limit_;

  loadGroupCallParticipants();

  loadGroupCallParticipants(int32 group_call_id_, int32 limit_);

  static const std::int32_t ID = 938720974;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class logOut final : public Function {
 public:

  logOut();

  static const std::int32_t ID = -1581923301;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class openChat final : public Function {
 public:
  int53 chat_id_;

  openChat();

  explicit openChat(int53 chat_id_);

  static const std::int32_t ID = -323371509;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class openMessageContent final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;

  openMessageContent();

  openMessageContent(int53 chat_id_, int53 message_id_);

  static const std::int32_t ID = -739088005;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class openWebApp final : public Function {
 public:
  int53 chat_id_;
  int53 bot_user_id_;
  string url_;
  object_ptr<themeParameters> theme_;
  string application_name_;
  int53 message_thread_id_;
  int53 reply_to_message_id_;

  openWebApp();

  openWebApp(int53 chat_id_, int53 bot_user_id_, string const &url_, object_ptr<themeParameters> &&theme_, string const &application_name_, int53 message_thread_id_, int53 reply_to_message_id_);

  static const std::int32_t ID = -763742925;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<webAppInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class optimizeStorage final : public Function {
 public:
  int53 size_;
  int32 ttl_;
  int32 count_;
  int32 immunity_delay_;
  array<object_ptr<FileType>> file_types_;
  array<int53> chat_ids_;
  array<int53> exclude_chat_ids_;
  bool return_deleted_file_statistics_;
  int32 chat_limit_;

  optimizeStorage();

  optimizeStorage(int53 size_, int32 ttl_, int32 count_, int32 immunity_delay_, array<object_ptr<FileType>> &&file_types_, array<int53> &&chat_ids_, array<int53> &&exclude_chat_ids_, bool return_deleted_file_statistics_, int32 chat_limit_);

  static const std::int32_t ID = 853186759;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<storageStatistics>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class parseMarkdown final : public Function {
 public:
  object_ptr<formattedText> text_;

  parseMarkdown();

  explicit parseMarkdown(object_ptr<formattedText> &&text_);

  static const std::int32_t ID = 756366063;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<formattedText>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class parseTextEntities final : public Function {
 public:
  string text_;
  object_ptr<TextParseMode> parse_mode_;

  parseTextEntities();

  parseTextEntities(string const &text_, object_ptr<TextParseMode> &&parse_mode_);

  static const std::int32_t ID = -1709194593;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<formattedText>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pinChatMessage final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  bool disable_notification_;
  bool only_for_self_;

  pinChatMessage();

  pinChatMessage(int53 chat_id_, int53 message_id_, bool disable_notification_, bool only_for_self_);

  static const std::int32_t ID = 2034719663;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pingProxy final : public Function {
 public:
  int32 proxy_id_;

  pingProxy();

  explicit pingProxy(int32 proxy_id_);

  static const std::int32_t ID = -979681103;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<seconds>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class preliminaryUploadFile final : public Function {
 public:
  object_ptr<InputFile> file_;
  object_ptr<FileType> file_type_;
  int32 priority_;

  preliminaryUploadFile();

  preliminaryUploadFile(object_ptr<InputFile> &&file_, object_ptr<FileType> &&file_type_, int32 priority_);

  static const std::int32_t ID = 1894239129;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<file>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class processChatJoinRequest final : public Function {
 public:
  int53 chat_id_;
  int53 user_id_;
  bool approve_;

  processChatJoinRequest();

  processChatJoinRequest(int53 chat_id_, int53 user_id_, bool approve_);

  static const std::int32_t ID = 1004876963;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class processChatJoinRequests final : public Function {
 public:
  int53 chat_id_;
  string invite_link_;
  bool approve_;

  processChatJoinRequests();

  processChatJoinRequests(int53 chat_id_, string const &invite_link_, bool approve_);

  static const std::int32_t ID = 1048722894;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class processPushNotification final : public Function {
 public:
  string payload_;

  processPushNotification();

  explicit processPushNotification(string const &payload_);

  static const std::int32_t ID = 786679952;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class rateSpeechRecognition final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  bool is_good_;

  rateSpeechRecognition();

  rateSpeechRecognition(int53 chat_id_, int53 message_id_, bool is_good_);

  static const std::int32_t ID = -287521867;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class readAllChatMentions final : public Function {
 public:
  int53 chat_id_;

  readAllChatMentions();

  explicit readAllChatMentions(int53 chat_id_);

  static const std::int32_t ID = 1357558453;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class readAllChatReactions final : public Function {
 public:
  int53 chat_id_;

  readAllChatReactions();

  explicit readAllChatReactions(int53 chat_id_);

  static const std::int32_t ID = 1421973357;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class readAllMessageThreadMentions final : public Function {
 public:
  int53 chat_id_;
  int53 message_thread_id_;

  readAllMessageThreadMentions();

  readAllMessageThreadMentions(int53 chat_id_, int53 message_thread_id_);

  static const std::int32_t ID = 1323136341;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class readAllMessageThreadReactions final : public Function {
 public:
  int53 chat_id_;
  int53 message_thread_id_;

  readAllMessageThreadReactions();

  readAllMessageThreadReactions(int53 chat_id_, int53 message_thread_id_);

  static const std::int32_t ID = -792975554;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class readFilePart final : public Function {
 public:
  int32 file_id_;
  int53 offset_;
  int53 count_;

  readFilePart();

  readFilePart(int32 file_id_, int53 offset_, int53 count_);

  static const std::int32_t ID = 906798861;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<filePart>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class recognizeSpeech final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;

  recognizeSpeech();

  recognizeSpeech(int53 chat_id_, int53 message_id_);

  static const std::int32_t ID = 1741947577;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class recoverAuthenticationPassword final : public Function {
 public:
  string recovery_code_;
  string new_password_;
  string new_hint_;

  recoverAuthenticationPassword();

  recoverAuthenticationPassword(string const &recovery_code_, string const &new_password_, string const &new_hint_);

  static const std::int32_t ID = -131001053;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class recoverPassword final : public Function {
 public:
  string recovery_code_;
  string new_password_;
  string new_hint_;

  recoverPassword();

  recoverPassword(string const &recovery_code_, string const &new_password_, string const &new_hint_);

  static const std::int32_t ID = -1524262541;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<passwordState>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class registerDevice final : public Function {
 public:
  object_ptr<DeviceToken> device_token_;
  array<int53> other_user_ids_;

  registerDevice();

  registerDevice(object_ptr<DeviceToken> &&device_token_, array<int53> &&other_user_ids_);

  static const std::int32_t ID = 366088823;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<pushReceiverId>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class registerUser final : public Function {
 public:
  string first_name_;
  string last_name_;

  registerUser();

  registerUser(string const &first_name_, string const &last_name_);

  static const std::int32_t ID = -109994467;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class removeAllFilesFromDownloads final : public Function {
 public:
  bool only_active_;
  bool only_completed_;
  bool delete_from_cache_;

  removeAllFilesFromDownloads();

  removeAllFilesFromDownloads(bool only_active_, bool only_completed_, bool delete_from_cache_);

  static const std::int32_t ID = -1186433402;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class removeBackground final : public Function {
 public:
  int64 background_id_;

  removeBackground();

  explicit removeBackground(int64 background_id_);

  static const std::int32_t ID = -1484545642;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class removeChatActionBar final : public Function {
 public:
  int53 chat_id_;

  removeChatActionBar();

  explicit removeChatActionBar(int53 chat_id_);

  static const std::int32_t ID = -1650968070;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class removeContacts final : public Function {
 public:
  array<int53> user_ids_;

  removeContacts();

  explicit removeContacts(array<int53> &&user_ids_);

  static const std::int32_t ID = 1943858054;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class removeFavoriteSticker final : public Function {
 public:
  object_ptr<InputFile> sticker_;

  removeFavoriteSticker();

  explicit removeFavoriteSticker(object_ptr<InputFile> &&sticker_);

  static const std::int32_t ID = 1152945264;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class removeFileFromDownloads final : public Function {
 public:
  int32 file_id_;
  bool delete_from_cache_;

  removeFileFromDownloads();

  removeFileFromDownloads(int32 file_id_, bool delete_from_cache_);

  static const std::int32_t ID = 1460060142;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class removeMessageReaction final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  object_ptr<ReactionType> reaction_type_;

  removeMessageReaction();

  removeMessageReaction(int53 chat_id_, int53 message_id_, object_ptr<ReactionType> &&reaction_type_);

  static const std::int32_t ID = -1756934789;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class removeNotification final : public Function {
 public:
  int32 notification_group_id_;
  int32 notification_id_;

  removeNotification();

  removeNotification(int32 notification_group_id_, int32 notification_id_);

  static const std::int32_t ID = 862630734;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class removeNotificationGroup final : public Function {
 public:
  int32 notification_group_id_;
  int32 max_notification_id_;

  removeNotificationGroup();

  removeNotificationGroup(int32 notification_group_id_, int32 max_notification_id_);

  static const std::int32_t ID = 1713005454;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class removeProxy final : public Function {
 public:
  int32 proxy_id_;

  removeProxy();

  explicit removeProxy(int32 proxy_id_);

  static const std::int32_t ID = 1369219847;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class removeRecentHashtag final : public Function {
 public:
  string hashtag_;

  removeRecentHashtag();

  explicit removeRecentHashtag(string const &hashtag_);

  static const std::int32_t ID = -1013735260;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class removeRecentSticker final : public Function {
 public:
  bool is_attached_;
  object_ptr<InputFile> sticker_;

  removeRecentSticker();

  removeRecentSticker(bool is_attached_, object_ptr<InputFile> &&sticker_);

  static const std::int32_t ID = 1246577677;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class removeRecentlyFoundChat final : public Function {
 public:
  int53 chat_id_;

  removeRecentlyFoundChat();

  explicit removeRecentlyFoundChat(int53 chat_id_);

  static const std::int32_t ID = 717340444;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class removeSavedAnimation final : public Function {
 public:
  object_ptr<InputFile> animation_;

  removeSavedAnimation();

  explicit removeSavedAnimation(object_ptr<InputFile> &&animation_);

  static const std::int32_t ID = -495605479;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class removeSavedNotificationSound final : public Function {
 public:
  int64 notification_sound_id_;

  removeSavedNotificationSound();

  explicit removeSavedNotificationSound(int64 notification_sound_id_);

  static const std::int32_t ID = -480032946;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class removeStickerFromSet final : public Function {
 public:
  object_ptr<InputFile> sticker_;

  removeStickerFromSet();

  explicit removeStickerFromSet(object_ptr<InputFile> &&sticker_);

  static const std::int32_t ID = 1642196644;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class removeTopChat final : public Function {
 public:
  object_ptr<TopChatCategory> category_;
  int53 chat_id_;

  removeTopChat();

  removeTopChat(object_ptr<TopChatCategory> &&category_, int53 chat_id_);

  static const std::int32_t ID = -1907876267;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class reorderActiveUsernames final : public Function {
 public:
  array<string> usernames_;

  reorderActiveUsernames();

  explicit reorderActiveUsernames(array<string> &&usernames_);

  static const std::int32_t ID = -455399375;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class reorderChatFilters final : public Function {
 public:
  array<int32> chat_filter_ids_;
  int32 main_chat_list_position_;

  reorderChatFilters();

  reorderChatFilters(array<int32> &&chat_filter_ids_, int32 main_chat_list_position_);

  static const std::int32_t ID = -1805801313;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class reorderInstalledStickerSets final : public Function {
 public:
  object_ptr<StickerType> sticker_type_;
  array<int64> sticker_set_ids_;

  reorderInstalledStickerSets();

  reorderInstalledStickerSets(object_ptr<StickerType> &&sticker_type_, array<int64> &&sticker_set_ids_);

  static const std::int32_t ID = 1074928158;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class reorderSupergroupActiveUsernames final : public Function {
 public:
  int53 supergroup_id_;
  array<string> usernames_;

  reorderSupergroupActiveUsernames();

  reorderSupergroupActiveUsernames(int53 supergroup_id_, array<string> &&usernames_);

  static const std::int32_t ID = -1962466095;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class replacePrimaryChatInviteLink final : public Function {
 public:
  int53 chat_id_;

  replacePrimaryChatInviteLink();

  explicit replacePrimaryChatInviteLink(int53 chat_id_);

  static const std::int32_t ID = 1067350941;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatInviteLink>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class replaceVideoChatRtmpUrl final : public Function {
 public:
  int53 chat_id_;

  replaceVideoChatRtmpUrl();

  explicit replaceVideoChatRtmpUrl(int53 chat_id_);

  static const std::int32_t ID = 558862304;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<rtmpUrl>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class reportChat final : public Function {
 public:
  int53 chat_id_;
  array<int53> message_ids_;
  object_ptr<ChatReportReason> reason_;
  string text_;

  reportChat();

  reportChat(int53 chat_id_, array<int53> &&message_ids_, object_ptr<ChatReportReason> &&reason_, string const &text_);

  static const std::int32_t ID = -964543587;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class reportChatPhoto final : public Function {
 public:
  int53 chat_id_;
  int32 file_id_;
  object_ptr<ChatReportReason> reason_;
  string text_;

  reportChatPhoto();

  reportChatPhoto(int53 chat_id_, int32 file_id_, object_ptr<ChatReportReason> &&reason_, string const &text_);

  static const std::int32_t ID = 734652708;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class reportMessageReactions final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  object_ptr<MessageSender> sender_id_;

  reportMessageReactions();

  reportMessageReactions(int53 chat_id_, int53 message_id_, object_ptr<MessageSender> &&sender_id_);

  static const std::int32_t ID = 919111719;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class reportSupergroupSpam final : public Function {
 public:
  int53 supergroup_id_;
  array<int53> message_ids_;

  reportSupergroupSpam();

  reportSupergroupSpam(int53 supergroup_id_, array<int53> &&message_ids_);

  static const std::int32_t ID = -94825000;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class requestAuthenticationPasswordRecovery final : public Function {
 public:

  requestAuthenticationPasswordRecovery();

  static const std::int32_t ID = 1393896118;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class requestPasswordRecovery final : public Function {
 public:

  requestPasswordRecovery();

  static const std::int32_t ID = -13777582;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<emailAddressAuthenticationCodeInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class requestQrCodeAuthentication final : public Function {
 public:
  array<int53> other_user_ids_;

  requestQrCodeAuthentication();

  explicit requestQrCodeAuthentication(array<int53> &&other_user_ids_);

  static const std::int32_t ID = 1363496527;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class resendAuthenticationCode final : public Function {
 public:

  resendAuthenticationCode();

  static const std::int32_t ID = -814377191;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class resendChangePhoneNumberCode final : public Function {
 public:

  resendChangePhoneNumberCode();

  static const std::int32_t ID = -786772060;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<authenticationCodeInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class resendEmailAddressVerificationCode final : public Function {
 public:

  resendEmailAddressVerificationCode();

  static const std::int32_t ID = -1872416732;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<emailAddressAuthenticationCodeInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class resendLoginEmailAddressCode final : public Function {
 public:

  resendLoginEmailAddressCode();

  static const std::int32_t ID = 292966933;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<emailAddressAuthenticationCodeInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class resendMessages final : public Function {
 public:
  int53 chat_id_;
  array<int53> message_ids_;

  resendMessages();

  resendMessages(int53 chat_id_, array<int53> &&message_ids_);

  static const std::int32_t ID = -940655817;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class resendPhoneNumberConfirmationCode final : public Function {
 public:

  resendPhoneNumberConfirmationCode();

  static const std::int32_t ID = 2069068522;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<authenticationCodeInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class resendPhoneNumberVerificationCode final : public Function {
 public:

  resendPhoneNumberVerificationCode();

  static const std::int32_t ID = 1367629820;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<authenticationCodeInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class resendRecoveryEmailAddressCode final : public Function {
 public:

  resendRecoveryEmailAddressCode();

  static const std::int32_t ID = 433483548;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<passwordState>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class resetAllNotificationSettings final : public Function {
 public:

  resetAllNotificationSettings();

  static const std::int32_t ID = -174020359;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class resetBackgrounds final : public Function {
 public:

  resetBackgrounds();

  static const std::int32_t ID = 204852088;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class resetNetworkStatistics final : public Function {
 public:

  resetNetworkStatistics();

  static const std::int32_t ID = 1646452102;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class resetPassword final : public Function {
 public:

  resetPassword();

  static const std::int32_t ID = -593589091;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ResetPasswordResult>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class revokeChatInviteLink final : public Function {
 public:
  int53 chat_id_;
  string invite_link_;

  revokeChatInviteLink();

  revokeChatInviteLink(int53 chat_id_, string const &invite_link_);

  static const std::int32_t ID = -776514135;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatInviteLinks>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class revokeGroupCallInviteLink final : public Function {
 public:
  int32 group_call_id_;

  revokeGroupCallInviteLink();

  explicit revokeGroupCallInviteLink(int32 group_call_id_);

  static const std::int32_t ID = 501589140;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class saveApplicationLogEvent final : public Function {
 public:
  string type_;
  int53 chat_id_;
  object_ptr<JsonValue> data_;

  saveApplicationLogEvent();

  saveApplicationLogEvent(string const &type_, int53 chat_id_, object_ptr<JsonValue> &&data_);

  static const std::int32_t ID = -811154930;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchBackground final : public Function {
 public:
  string name_;

  searchBackground();

  explicit searchBackground(string const &name_);

  static const std::int32_t ID = -2130996959;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<background>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchCallMessages final : public Function {
 public:
  int53 from_message_id_;
  int32 limit_;
  bool only_missed_;

  searchCallMessages();

  searchCallMessages(int53 from_message_id_, int32 limit_, bool only_missed_);

  static const std::int32_t ID = -1077230820;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchChatMembers final : public Function {
 public:
  int53 chat_id_;
  string query_;
  int32 limit_;
  object_ptr<ChatMembersFilter> filter_;

  searchChatMembers();

  searchChatMembers(int53 chat_id_, string const &query_, int32 limit_, object_ptr<ChatMembersFilter> &&filter_);

  static const std::int32_t ID = -445823291;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatMembers>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchChatMessages final : public Function {
 public:
  int53 chat_id_;
  string query_;
  object_ptr<MessageSender> sender_id_;
  int53 from_message_id_;
  int32 offset_;
  int32 limit_;
  object_ptr<SearchMessagesFilter> filter_;
  int53 message_thread_id_;

  searchChatMessages();

  searchChatMessages(int53 chat_id_, string const &query_, object_ptr<MessageSender> &&sender_id_, int53 from_message_id_, int32 offset_, int32 limit_, object_ptr<SearchMessagesFilter> &&filter_, int53 message_thread_id_);

  static const std::int32_t ID = -162154976;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchChatRecentLocationMessages final : public Function {
 public:
  int53 chat_id_;
  int32 limit_;

  searchChatRecentLocationMessages();

  searchChatRecentLocationMessages(int53 chat_id_, int32 limit_);

  static const std::int32_t ID = 950238950;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchChats final : public Function {
 public:
  string query_;
  int32 limit_;

  searchChats();

  searchChats(string const &query_, int32 limit_);

  static const std::int32_t ID = -1879787060;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chats>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchChatsNearby final : public Function {
 public:
  object_ptr<location> location_;

  searchChatsNearby();

  explicit searchChatsNearby(object_ptr<location> &&location_);

  static const std::int32_t ID = -196753377;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatsNearby>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchChatsOnServer final : public Function {
 public:
  string query_;
  int32 limit_;

  searchChatsOnServer();

  searchChatsOnServer(string const &query_, int32 limit_);

  static const std::int32_t ID = -1158402188;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chats>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchContacts final : public Function {
 public:
  string query_;
  int32 limit_;

  searchContacts();

  searchContacts(string const &query_, int32 limit_);

  static const std::int32_t ID = -1794690715;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<users>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchEmojis final : public Function {
 public:
  string text_;
  bool exact_match_;
  array<string> input_language_codes_;

  searchEmojis();

  searchEmojis(string const &text_, bool exact_match_, array<string> &&input_language_codes_);

  static const std::int32_t ID = 398837927;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<emojis>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchFileDownloads final : public Function {
 public:
  string query_;
  bool only_active_;
  bool only_completed_;
  string offset_;
  int32 limit_;

  searchFileDownloads();

  searchFileDownloads(string const &query_, bool only_active_, bool only_completed_, string const &offset_, int32 limit_);

  static const std::int32_t ID = 706611286;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<foundFileDownloads>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchHashtags final : public Function {
 public:
  string prefix_;
  int32 limit_;

  searchHashtags();

  searchHashtags(string const &prefix_, int32 limit_);

  static const std::int32_t ID = 1043637617;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<hashtags>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchInstalledStickerSets final : public Function {
 public:
  object_ptr<StickerType> sticker_type_;
  string query_;
  int32 limit_;

  searchInstalledStickerSets();

  searchInstalledStickerSets(object_ptr<StickerType> &&sticker_type_, string const &query_, int32 limit_);

  static const std::int32_t ID = 2120122276;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickerSets>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchMessages final : public Function {
 public:
  object_ptr<ChatList> chat_list_;
  string query_;
  int32 offset_date_;
  int53 offset_chat_id_;
  int53 offset_message_id_;
  int32 limit_;
  object_ptr<SearchMessagesFilter> filter_;
  int32 min_date_;
  int32 max_date_;

  searchMessages();

  searchMessages(object_ptr<ChatList> &&chat_list_, string const &query_, int32 offset_date_, int53 offset_chat_id_, int53 offset_message_id_, int32 limit_, object_ptr<SearchMessagesFilter> &&filter_, int32 min_date_, int32 max_date_);

  static const std::int32_t ID = -225214062;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchOutgoingDocumentMessages final : public Function {
 public:
  string query_;
  int32 limit_;

  searchOutgoingDocumentMessages();

  searchOutgoingDocumentMessages(string const &query_, int32 limit_);

  static const std::int32_t ID = -1071397762;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<foundMessages>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchPublicChat final : public Function {
 public:
  string username_;

  searchPublicChat();

  explicit searchPublicChat(string const &username_);

  static const std::int32_t ID = 857135533;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chat>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchPublicChats final : public Function {
 public:
  string query_;

  searchPublicChats();

  explicit searchPublicChats(string const &query_);

  static const std::int32_t ID = 970385337;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chats>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchSecretMessages final : public Function {
 public:
  int53 chat_id_;
  string query_;
  string offset_;
  int32 limit_;
  object_ptr<SearchMessagesFilter> filter_;

  searchSecretMessages();

  searchSecretMessages(int53 chat_id_, string const &query_, string const &offset_, int32 limit_, object_ptr<SearchMessagesFilter> &&filter_);

  static const std::int32_t ID = -852865892;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<foundMessages>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchStickerSet final : public Function {
 public:
  string name_;

  searchStickerSet();

  explicit searchStickerSet(string const &name_);

  static const std::int32_t ID = 1157930222;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickerSet>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchStickerSets final : public Function {
 public:
  string query_;

  searchStickerSets();

  explicit searchStickerSets(string const &query_);

  static const std::int32_t ID = -1082314629;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickerSets>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchStickers final : public Function {
 public:
  string emoji_;
  int32 limit_;

  searchStickers();

  searchStickers(string const &emoji_, int32 limit_);

  static const std::int32_t ID = 1555771203;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickers>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchUserByPhoneNumber final : public Function {
 public:
  string phone_number_;

  searchUserByPhoneNumber();

  explicit searchUserByPhoneNumber(string const &phone_number_);

  static const std::int32_t ID = -1562236142;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<user>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendBotStartMessage final : public Function {
 public:
  int53 bot_user_id_;
  int53 chat_id_;
  string parameter_;

  sendBotStartMessage();

  sendBotStartMessage(int53 bot_user_id_, int53 chat_id_, string const &parameter_);

  static const std::int32_t ID = -1435877650;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<message>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendCallDebugInformation final : public Function {
 public:
  int32 call_id_;
  string debug_information_;

  sendCallDebugInformation();

  sendCallDebugInformation(int32 call_id_, string const &debug_information_);

  static const std::int32_t ID = 2019243839;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendCallLog final : public Function {
 public:
  int32 call_id_;
  object_ptr<InputFile> log_file_;

  sendCallLog();

  sendCallLog(int32 call_id_, object_ptr<InputFile> &&log_file_);

  static const std::int32_t ID = 1057638353;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendCallRating final : public Function {
 public:
  int32 call_id_;
  int32 rating_;
  string comment_;
  array<object_ptr<CallProblem>> problems_;

  sendCallRating();

  sendCallRating(int32 call_id_, int32 rating_, string const &comment_, array<object_ptr<CallProblem>> &&problems_);

  static const std::int32_t ID = -1402719502;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendCallSignalingData final : public Function {
 public:
  int32 call_id_;
  bytes data_;

  sendCallSignalingData();

  sendCallSignalingData(int32 call_id_, bytes const &data_);

  static const std::int32_t ID = 1412280732;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendChatAction final : public Function {
 public:
  int53 chat_id_;
  int53 message_thread_id_;
  object_ptr<ChatAction> action_;

  sendChatAction();

  sendChatAction(int53 chat_id_, int53 message_thread_id_, object_ptr<ChatAction> &&action_);

  static const std::int32_t ID = 2096947540;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendChatScreenshotTakenNotification final : public Function {
 public:
  int53 chat_id_;

  sendChatScreenshotTakenNotification();

  explicit sendChatScreenshotTakenNotification(int53 chat_id_);

  static const std::int32_t ID = 448399457;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendCustomRequest final : public Function {
 public:
  string method_;
  string parameters_;

  sendCustomRequest();

  sendCustomRequest(string const &method_, string const &parameters_);

  static const std::int32_t ID = 285045153;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<customRequestResult>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendEmailAddressVerificationCode final : public Function {
 public:
  string email_address_;

  sendEmailAddressVerificationCode();

  explicit sendEmailAddressVerificationCode(string const &email_address_);

  static const std::int32_t ID = -221621379;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<emailAddressAuthenticationCodeInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendInlineQueryResultMessage final : public Function {
 public:
  int53 chat_id_;
  int53 message_thread_id_;
  int53 reply_to_message_id_;
  object_ptr<messageSendOptions> options_;
  int64 query_id_;
  string result_id_;
  bool hide_via_bot_;

  sendInlineQueryResultMessage();

  sendInlineQueryResultMessage(int53 chat_id_, int53 message_thread_id_, int53 reply_to_message_id_, object_ptr<messageSendOptions> &&options_, int64 query_id_, string const &result_id_, bool hide_via_bot_);

  static const std::int32_t ID = -948639588;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<message>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendMessage final : public Function {
 public:
  int53 chat_id_;
  int53 message_thread_id_;
  int53 reply_to_message_id_;
  object_ptr<messageSendOptions> options_;
  object_ptr<ReplyMarkup> reply_markup_;
  object_ptr<InputMessageContent> input_message_content_;

  sendMessage();

  sendMessage(int53 chat_id_, int53 message_thread_id_, int53 reply_to_message_id_, object_ptr<messageSendOptions> &&options_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_);

  static const std::int32_t ID = 960453021;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<message>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendMessageAlbum final : public Function {
 public:
  int53 chat_id_;
  int53 message_thread_id_;
  int53 reply_to_message_id_;
  object_ptr<messageSendOptions> options_;
  array<object_ptr<InputMessageContent>> input_message_contents_;
  bool only_preview_;

  sendMessageAlbum();

  sendMessageAlbum(int53 chat_id_, int53 message_thread_id_, int53 reply_to_message_id_, object_ptr<messageSendOptions> &&options_, array<object_ptr<InputMessageContent>> &&input_message_contents_, bool only_preview_);

  static const std::int32_t ID = -1639797862;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendPassportAuthorizationForm final : public Function {
 public:
  int32 autorization_form_id_;
  array<object_ptr<PassportElementType>> types_;

  sendPassportAuthorizationForm();

  sendPassportAuthorizationForm(int32 autorization_form_id_, array<object_ptr<PassportElementType>> &&types_);

  static const std::int32_t ID = -602402218;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendPaymentForm final : public Function {
 public:
  object_ptr<InputInvoice> input_invoice_;
  int64 payment_form_id_;
  string order_info_id_;
  string shipping_option_id_;
  object_ptr<InputCredentials> credentials_;
  int53 tip_amount_;

  sendPaymentForm();

  sendPaymentForm(object_ptr<InputInvoice> &&input_invoice_, int64 payment_form_id_, string const &order_info_id_, string const &shipping_option_id_, object_ptr<InputCredentials> &&credentials_, int53 tip_amount_);

  static const std::int32_t ID = -965855094;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<paymentResult>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendPhoneNumberConfirmationCode final : public Function {
 public:
  string hash_;
  string phone_number_;
  object_ptr<phoneNumberAuthenticationSettings> settings_;

  sendPhoneNumberConfirmationCode();

  sendPhoneNumberConfirmationCode(string const &hash_, string const &phone_number_, object_ptr<phoneNumberAuthenticationSettings> &&settings_);

  static const std::int32_t ID = -1901171495;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<authenticationCodeInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendPhoneNumberVerificationCode final : public Function {
 public:
  string phone_number_;
  object_ptr<phoneNumberAuthenticationSettings> settings_;

  sendPhoneNumberVerificationCode();

  sendPhoneNumberVerificationCode(string const &phone_number_, object_ptr<phoneNumberAuthenticationSettings> &&settings_);

  static const std::int32_t ID = 2081689035;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<authenticationCodeInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendWebAppData final : public Function {
 public:
  int53 bot_user_id_;
  string button_text_;
  string data_;

  sendWebAppData();

  sendWebAppData(int53 bot_user_id_, string const &button_text_, string const &data_);

  static const std::int32_t ID = -1423978996;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setAccountTtl final : public Function {
 public:
  object_ptr<accountTtl> ttl_;

  setAccountTtl();

  explicit setAccountTtl(object_ptr<accountTtl> &&ttl_);

  static const std::int32_t ID = 701389032;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setAlarm final : public Function {
 public:
  double seconds_;

  setAlarm();

  explicit setAlarm(double seconds_);

  static const std::int32_t ID = -873497067;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setAuthenticationEmailAddress final : public Function {
 public:
  string email_address_;

  setAuthenticationEmailAddress();

  explicit setAuthenticationEmailAddress(string const &email_address_);

  static const std::int32_t ID = 1773323522;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setAuthenticationPhoneNumber final : public Function {
 public:
  string phone_number_;
  object_ptr<phoneNumberAuthenticationSettings> settings_;

  setAuthenticationPhoneNumber();

  setAuthenticationPhoneNumber(string const &phone_number_, object_ptr<phoneNumberAuthenticationSettings> &&settings_);

  static const std::int32_t ID = 868276259;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setAutoDownloadSettings final : public Function {
 public:
  object_ptr<autoDownloadSettings> settings_;
  object_ptr<NetworkType> type_;

  setAutoDownloadSettings();

  setAutoDownloadSettings(object_ptr<autoDownloadSettings> &&settings_, object_ptr<NetworkType> &&type_);

  static const std::int32_t ID = -353671948;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setBackground final : public Function {
 public:
  object_ptr<InputBackground> background_;
  object_ptr<BackgroundType> type_;
  bool for_dark_theme_;

  setBackground();

  setBackground(object_ptr<InputBackground> &&background_, object_ptr<BackgroundType> &&type_, bool for_dark_theme_);

  static const std::int32_t ID = -1035439225;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<background>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setBio final : public Function {
 public:
  string bio_;

  setBio();

  explicit setBio(string const &bio_);

  static const std::int32_t ID = -1619582124;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setBotUpdatesStatus final : public Function {
 public:
  int32 pending_update_count_;
  string error_message_;

  setBotUpdatesStatus();

  setBotUpdatesStatus(int32 pending_update_count_, string const &error_message_);

  static const std::int32_t ID = -1154926191;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setChatAvailableReactions final : public Function {
 public:
  int53 chat_id_;
  object_ptr<ChatAvailableReactions> available_reactions_;

  setChatAvailableReactions();

  setChatAvailableReactions(int53 chat_id_, object_ptr<ChatAvailableReactions> &&available_reactions_);

  static const std::int32_t ID = 267075078;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setChatClientData final : public Function {
 public:
  int53 chat_id_;
  string client_data_;

  setChatClientData();

  setChatClientData(int53 chat_id_, string const &client_data_);

  static const std::int32_t ID = -827119811;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setChatDescription final : public Function {
 public:
  int53 chat_id_;
  string description_;

  setChatDescription();

  setChatDescription(int53 chat_id_, string const &description_);

  static const std::int32_t ID = 1957213277;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setChatDiscussionGroup final : public Function {
 public:
  int53 chat_id_;
  int53 discussion_chat_id_;

  setChatDiscussionGroup();

  setChatDiscussionGroup(int53 chat_id_, int53 discussion_chat_id_);

  static const std::int32_t ID = -918801736;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setChatDraftMessage final : public Function {
 public:
  int53 chat_id_;
  int53 message_thread_id_;
  object_ptr<draftMessage> draft_message_;

  setChatDraftMessage();

  setChatDraftMessage(int53 chat_id_, int53 message_thread_id_, object_ptr<draftMessage> &&draft_message_);

  static const std::int32_t ID = 1683889946;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setChatLocation final : public Function {
 public:
  int53 chat_id_;
  object_ptr<chatLocation> location_;

  setChatLocation();

  setChatLocation(int53 chat_id_, object_ptr<chatLocation> &&location_);

  static const std::int32_t ID = -767091286;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setChatMemberStatus final : public Function {
 public:
  int53 chat_id_;
  object_ptr<MessageSender> member_id_;
  object_ptr<ChatMemberStatus> status_;

  setChatMemberStatus();

  setChatMemberStatus(int53 chat_id_, object_ptr<MessageSender> &&member_id_, object_ptr<ChatMemberStatus> &&status_);

  static const std::int32_t ID = 81794847;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setChatMessageSender final : public Function {
 public:
  int53 chat_id_;
  object_ptr<MessageSender> message_sender_id_;

  setChatMessageSender();

  setChatMessageSender(int53 chat_id_, object_ptr<MessageSender> &&message_sender_id_);

  static const std::int32_t ID = -1421513858;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setChatMessageTtl final : public Function {
 public:
  int53 chat_id_;
  int32 ttl_;

  setChatMessageTtl();

  setChatMessageTtl(int53 chat_id_, int32 ttl_);

  static const std::int32_t ID = -1578406279;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setChatNotificationSettings final : public Function {
 public:
  int53 chat_id_;
  object_ptr<chatNotificationSettings> notification_settings_;

  setChatNotificationSettings();

  setChatNotificationSettings(int53 chat_id_, object_ptr<chatNotificationSettings> &&notification_settings_);

  static const std::int32_t ID = 777199614;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setChatPermissions final : public Function {
 public:
  int53 chat_id_;
  object_ptr<chatPermissions> permissions_;

  setChatPermissions();

  setChatPermissions(int53 chat_id_, object_ptr<chatPermissions> &&permissions_);

  static const std::int32_t ID = 2138507006;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setChatPhoto final : public Function {
 public:
  int53 chat_id_;
  object_ptr<InputChatPhoto> photo_;

  setChatPhoto();

  setChatPhoto(int53 chat_id_, object_ptr<InputChatPhoto> &&photo_);

  static const std::int32_t ID = -377778941;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setChatSlowModeDelay final : public Function {
 public:
  int53 chat_id_;
  int32 slow_mode_delay_;

  setChatSlowModeDelay();

  setChatSlowModeDelay(int53 chat_id_, int32 slow_mode_delay_);

  static const std::int32_t ID = -540350914;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setChatTheme final : public Function {
 public:
  int53 chat_id_;
  string theme_name_;

  setChatTheme();

  setChatTheme(int53 chat_id_, string const &theme_name_);

  static const std::int32_t ID = -1895234925;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setChatTitle final : public Function {
 public:
  int53 chat_id_;
  string title_;

  setChatTitle();

  setChatTitle(int53 chat_id_, string const &title_);

  static const std::int32_t ID = 164282047;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setCommands final : public Function {
 public:
  object_ptr<BotCommandScope> scope_;
  string language_code_;
  array<object_ptr<botCommand>> commands_;

  setCommands();

  setCommands(object_ptr<BotCommandScope> &&scope_, string const &language_code_, array<object_ptr<botCommand>> &&commands_);

  static const std::int32_t ID = -907165606;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setCustomLanguagePack final : public Function {
 public:
  object_ptr<languagePackInfo> info_;
  array<object_ptr<languagePackString>> strings_;

  setCustomLanguagePack();

  setCustomLanguagePack(object_ptr<languagePackInfo> &&info_, array<object_ptr<languagePackString>> &&strings_);

  static const std::int32_t ID = -296742819;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setCustomLanguagePackString final : public Function {
 public:
  string language_pack_id_;
  object_ptr<languagePackString> new_string_;

  setCustomLanguagePackString();

  setCustomLanguagePackString(string const &language_pack_id_, object_ptr<languagePackString> &&new_string_);

  static const std::int32_t ID = 1316365592;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setDatabaseEncryptionKey final : public Function {
 public:
  bytes new_encryption_key_;

  setDatabaseEncryptionKey();

  explicit setDatabaseEncryptionKey(bytes const &new_encryption_key_);

  static const std::int32_t ID = -1204599371;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setDefaultChannelAdministratorRights final : public Function {
 public:
  object_ptr<chatAdministratorRights> default_channel_administrator_rights_;

  setDefaultChannelAdministratorRights();

  explicit setDefaultChannelAdministratorRights(object_ptr<chatAdministratorRights> &&default_channel_administrator_rights_);

  static const std::int32_t ID = -234004967;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setDefaultGroupAdministratorRights final : public Function {
 public:
  object_ptr<chatAdministratorRights> default_group_administrator_rights_;

  setDefaultGroupAdministratorRights();

  explicit setDefaultGroupAdministratorRights(object_ptr<chatAdministratorRights> &&default_group_administrator_rights_);

  static const std::int32_t ID = 1700231016;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setDefaultReactionType final : public Function {
 public:
  object_ptr<ReactionType> reaction_type_;

  setDefaultReactionType();

  explicit setDefaultReactionType(object_ptr<ReactionType> &&reaction_type_);

  static const std::int32_t ID = 1694730813;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setEmojiStatus final : public Function {
 public:
  object_ptr<emojiStatus> emoji_status_;
  int32 duration_;

  setEmojiStatus();

  setEmojiStatus(object_ptr<emojiStatus> &&emoji_status_, int32 duration_);

  static const std::int32_t ID = -1310680569;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setFileGenerationProgress final : public Function {
 public:
  int64 generation_id_;
  int53 expected_size_;
  int53 local_prefix_size_;

  setFileGenerationProgress();

  setFileGenerationProgress(int64 generation_id_, int53 expected_size_, int53 local_prefix_size_);

  static const std::int32_t ID = 1836403518;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setGameScore final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  bool edit_message_;
  int53 user_id_;
  int32 score_;
  bool force_;

  setGameScore();

  setGameScore(int53 chat_id_, int53 message_id_, bool edit_message_, int53 user_id_, int32 score_, bool force_);

  static const std::int32_t ID = 2127359430;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<message>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setGroupCallParticipantIsSpeaking final : public Function {
 public:
  int32 group_call_id_;
  int32 audio_source_;
  bool is_speaking_;

  setGroupCallParticipantIsSpeaking();

  setGroupCallParticipantIsSpeaking(int32 group_call_id_, int32 audio_source_, bool is_speaking_);

  static const std::int32_t ID = 927506917;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setGroupCallParticipantVolumeLevel final : public Function {
 public:
  int32 group_call_id_;
  object_ptr<MessageSender> participant_id_;
  int32 volume_level_;

  setGroupCallParticipantVolumeLevel();

  setGroupCallParticipantVolumeLevel(int32 group_call_id_, object_ptr<MessageSender> &&participant_id_, int32 volume_level_);

  static const std::int32_t ID = -1753769944;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setGroupCallTitle final : public Function {
 public:
  int32 group_call_id_;
  string title_;

  setGroupCallTitle();

  setGroupCallTitle(int32 group_call_id_, string const &title_);

  static const std::int32_t ID = -1228825139;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setInactiveSessionTtl final : public Function {
 public:
  int32 inactive_session_ttl_days_;

  setInactiveSessionTtl();

  explicit setInactiveSessionTtl(int32 inactive_session_ttl_days_);

  static const std::int32_t ID = 1570548048;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setInlineGameScore final : public Function {
 public:
  string inline_message_id_;
  bool edit_message_;
  int53 user_id_;
  int32 score_;
  bool force_;

  setInlineGameScore();

  setInlineGameScore(string const &inline_message_id_, bool edit_message_, int53 user_id_, int32 score_, bool force_);

  static const std::int32_t ID = -948871797;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setLocation final : public Function {
 public:
  object_ptr<location> location_;

  setLocation();

  explicit setLocation(object_ptr<location> &&location_);

  static const std::int32_t ID = 93926257;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setLogStream final : public Function {
 public:
  object_ptr<LogStream> log_stream_;

  setLogStream();

  explicit setLogStream(object_ptr<LogStream> &&log_stream_);

  static const std::int32_t ID = -1364199535;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setLogTagVerbosityLevel final : public Function {
 public:
  string tag_;
  int32 new_verbosity_level_;

  setLogTagVerbosityLevel();

  setLogTagVerbosityLevel(string const &tag_, int32 new_verbosity_level_);

  static const std::int32_t ID = -2095589738;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setLogVerbosityLevel final : public Function {
 public:
  int32 new_verbosity_level_;

  setLogVerbosityLevel();

  explicit setLogVerbosityLevel(int32 new_verbosity_level_);

  static const std::int32_t ID = -303429678;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setLoginEmailAddress final : public Function {
 public:
  string new_login_email_address_;

  setLoginEmailAddress();

  explicit setLoginEmailAddress(string const &new_login_email_address_);

  static const std::int32_t ID = 935019476;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<emailAddressAuthenticationCodeInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setMenuButton final : public Function {
 public:
  int53 user_id_;
  object_ptr<botMenuButton> menu_button_;

  setMenuButton();

  setMenuButton(int53 user_id_, object_ptr<botMenuButton> &&menu_button_);

  static const std::int32_t ID = -1269841599;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setName final : public Function {
 public:
  string first_name_;
  string last_name_;

  setName();

  setName(string const &first_name_, string const &last_name_);

  static const std::int32_t ID = 1711693584;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setNetworkType final : public Function {
 public:
  object_ptr<NetworkType> type_;

  setNetworkType();

  explicit setNetworkType(object_ptr<NetworkType> &&type_);

  static const std::int32_t ID = -701635234;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setOption final : public Function {
 public:
  string name_;
  object_ptr<OptionValue> value_;

  setOption();

  setOption(string const &name_, object_ptr<OptionValue> &&value_);

  static const std::int32_t ID = 2114670322;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setPassportElement final : public Function {
 public:
  object_ptr<InputPassportElement> element_;
  string password_;

  setPassportElement();

  setPassportElement(object_ptr<InputPassportElement> &&element_, string const &password_);

  static const std::int32_t ID = 2068173212;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<PassportElement>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setPassportElementErrors final : public Function {
 public:
  int53 user_id_;
  array<object_ptr<inputPassportElementError>> errors_;

  setPassportElementErrors();

  setPassportElementErrors(int53 user_id_, array<object_ptr<inputPassportElementError>> &&errors_);

  static const std::int32_t ID = -2056754881;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setPassword final : public Function {
 public:
  string old_password_;
  string new_password_;
  string new_hint_;
  bool set_recovery_email_address_;
  string new_recovery_email_address_;

  setPassword();

  setPassword(string const &old_password_, string const &new_password_, string const &new_hint_, bool set_recovery_email_address_, string const &new_recovery_email_address_);

  static const std::int32_t ID = -1193589027;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<passwordState>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setPinnedChats final : public Function {
 public:
  object_ptr<ChatList> chat_list_;
  array<int53> chat_ids_;

  setPinnedChats();

  setPinnedChats(object_ptr<ChatList> &&chat_list_, array<int53> &&chat_ids_);

  static const std::int32_t ID = -695640000;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setPollAnswer final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  array<int32> option_ids_;

  setPollAnswer();

  setPollAnswer(int53 chat_id_, int53 message_id_, array<int32> &&option_ids_);

  static const std::int32_t ID = -1399388792;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setProfilePhoto final : public Function {
 public:
  object_ptr<InputChatPhoto> photo_;

  setProfilePhoto();

  explicit setProfilePhoto(object_ptr<InputChatPhoto> &&photo_);

  static const std::int32_t ID = -2069678882;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setRecoveryEmailAddress final : public Function {
 public:
  string password_;
  string new_recovery_email_address_;

  setRecoveryEmailAddress();

  setRecoveryEmailAddress(string const &password_, string const &new_recovery_email_address_);

  static const std::int32_t ID = -1981836385;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<passwordState>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setScopeNotificationSettings final : public Function {
 public:
  object_ptr<NotificationSettingsScope> scope_;
  object_ptr<scopeNotificationSettings> notification_settings_;

  setScopeNotificationSettings();

  setScopeNotificationSettings(object_ptr<NotificationSettingsScope> &&scope_, object_ptr<scopeNotificationSettings> &&notification_settings_);

  static const std::int32_t ID = -2049984966;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setStickerPositionInSet final : public Function {
 public:
  object_ptr<InputFile> sticker_;
  int32 position_;

  setStickerPositionInSet();

  setStickerPositionInSet(object_ptr<InputFile> &&sticker_, int32 position_);

  static const std::int32_t ID = 2075281185;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setStickerSetThumbnail final : public Function {
 public:
  int53 user_id_;
  string name_;
  object_ptr<InputFile> thumbnail_;

  setStickerSetThumbnail();

  setStickerSetThumbnail(int53 user_id_, string const &name_, object_ptr<InputFile> &&thumbnail_);

  static const std::int32_t ID = 1230174856;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickerSet>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setSupergroupStickerSet final : public Function {
 public:
  int53 supergroup_id_;
  int64 sticker_set_id_;

  setSupergroupStickerSet();

  setSupergroupStickerSet(int53 supergroup_id_, int64 sticker_set_id_);

  static const std::int32_t ID = -2056344215;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setSupergroupUsername final : public Function {
 public:
  int53 supergroup_id_;
  string username_;

  setSupergroupUsername();

  setSupergroupUsername(int53 supergroup_id_, string const &username_);

  static const std::int32_t ID = 1346325252;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setTdlibParameters final : public Function {
 public:
  bool use_test_dc_;
  string database_directory_;
  string files_directory_;
  bytes database_encryption_key_;
  bool use_file_database_;
  bool use_chat_info_database_;
  bool use_message_database_;
  bool use_secret_chats_;
  int32 api_id_;
  string api_hash_;
  string system_language_code_;
  string device_model_;
  string system_version_;
  string application_version_;
  bool enable_storage_optimizer_;
  bool ignore_file_names_;

  setTdlibParameters();

  setTdlibParameters(bool use_test_dc_, string const &database_directory_, string const &files_directory_, bytes const &database_encryption_key_, bool use_file_database_, bool use_chat_info_database_, bool use_message_database_, bool use_secret_chats_, int32 api_id_, string const &api_hash_, string const &system_language_code_, string const &device_model_, string const &system_version_, string const &application_version_, bool enable_storage_optimizer_, bool ignore_file_names_);

  static const std::int32_t ID = 1384743481;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setUserPrivacySettingRules final : public Function {
 public:
  object_ptr<UserPrivacySetting> setting_;
  object_ptr<userPrivacySettingRules> rules_;

  setUserPrivacySettingRules();

  setUserPrivacySettingRules(object_ptr<UserPrivacySetting> &&setting_, object_ptr<userPrivacySettingRules> &&rules_);

  static const std::int32_t ID = -473812741;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setUserSupportInfo final : public Function {
 public:
  int53 user_id_;
  object_ptr<formattedText> message_;

  setUserSupportInfo();

  setUserSupportInfo(int53 user_id_, object_ptr<formattedText> &&message_);

  static const std::int32_t ID = -2088986621;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<userSupportInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setUsername final : public Function {
 public:
  string username_;

  setUsername();

  explicit setUsername(string const &username_);

  static const std::int32_t ID = 439901214;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class setVideoChatDefaultParticipant final : public Function {
 public:
  int53 chat_id_;
  object_ptr<MessageSender> default_participant_id_;

  setVideoChatDefaultParticipant();

  setVideoChatDefaultParticipant(int53 chat_id_, object_ptr<MessageSender> &&default_participant_id_);

  static const std::int32_t ID = -240749901;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sharePhoneNumber final : public Function {
 public:
  int53 user_id_;

  sharePhoneNumber();

  explicit sharePhoneNumber(int53 user_id_);

  static const std::int32_t ID = 1097130069;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class startGroupCallRecording final : public Function {
 public:
  int32 group_call_id_;
  string title_;
  bool record_video_;
  bool use_portrait_orientation_;

  startGroupCallRecording();

  startGroupCallRecording(int32 group_call_id_, string const &title_, bool record_video_, bool use_portrait_orientation_);

  static const std::int32_t ID = 1757774971;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class startGroupCallScreenSharing final : public Function {
 public:
  int32 group_call_id_;
  int32 audio_source_id_;
  string payload_;

  startGroupCallScreenSharing();

  startGroupCallScreenSharing(int32 group_call_id_, int32 audio_source_id_, string const &payload_);

  static const std::int32_t ID = -884068051;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<text>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class startScheduledGroupCall final : public Function {
 public:
  int32 group_call_id_;

  startScheduledGroupCall();

  explicit startScheduledGroupCall(int32 group_call_id_);

  static const std::int32_t ID = 1519938838;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class stopPoll final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;
  object_ptr<ReplyMarkup> reply_markup_;

  stopPoll();

  stopPoll(int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_);

  static const std::int32_t ID = 1659374253;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class synchronizeLanguagePack final : public Function {
 public:
  string language_pack_id_;

  synchronizeLanguagePack();

  explicit synchronizeLanguagePack(string const &language_pack_id_);

  static const std::int32_t ID = -2065307858;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class terminateAllOtherSessions final : public Function {
 public:

  terminateAllOtherSessions();

  static const std::int32_t ID = 1874485523;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class terminateSession final : public Function {
 public:
  int64 session_id_;

  terminateSession();

  explicit terminateSession(int64 session_id_);

  static const std::int32_t ID = -407385812;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testCallBytes final : public Function {
 public:
  bytes x_;

  testCallBytes();

  explicit testCallBytes(bytes const &x_);

  static const std::int32_t ID = -736011607;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<testBytes>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testCallEmpty final : public Function {
 public:

  testCallEmpty();

  static const std::int32_t ID = -627291626;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testCallString final : public Function {
 public:
  string x_;

  testCallString();

  explicit testCallString(string const &x_);

  static const std::int32_t ID = -1732818385;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<testString>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testCallVectorInt final : public Function {
 public:
  array<int32> x_;

  testCallVectorInt();

  explicit testCallVectorInt(array<int32> &&x_);

  static const std::int32_t ID = -2137277793;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<testVectorInt>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testCallVectorIntObject final : public Function {
 public:
  array<object_ptr<testInt>> x_;

  testCallVectorIntObject();

  explicit testCallVectorIntObject(array<object_ptr<testInt>> &&x_);

  static const std::int32_t ID = 1825428218;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<testVectorIntObject>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testCallVectorString final : public Function {
 public:
  array<string> x_;

  testCallVectorString();

  explicit testCallVectorString(array<string> &&x_);

  static const std::int32_t ID = -408600900;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<testVectorString>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testCallVectorStringObject final : public Function {
 public:
  array<object_ptr<testString>> x_;

  testCallVectorStringObject();

  explicit testCallVectorStringObject(array<object_ptr<testString>> &&x_);

  static const std::int32_t ID = 1527666429;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<testVectorStringObject>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testGetDifference final : public Function {
 public:

  testGetDifference();

  static const std::int32_t ID = 1747084069;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testNetwork final : public Function {
 public:

  testNetwork();

  static const std::int32_t ID = -1343998901;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testProxy final : public Function {
 public:
  string server_;
  int32 port_;
  object_ptr<ProxyType> type_;
  int32 dc_id_;
  double timeout_;

  testProxy();

  testProxy(string const &server_, int32 port_, object_ptr<ProxyType> &&type_, int32 dc_id_, double timeout_);

  static const std::int32_t ID = -1197366626;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testReturnError final : public Function {
 public:
  object_ptr<error> error_;

  testReturnError();

  explicit testReturnError(object_ptr<error> &&error_);

  static const std::int32_t ID = 455179506;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<error>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testSquareInt final : public Function {
 public:
  int32 x_;

  testSquareInt();

  explicit testSquareInt(int32 x_);

  static const std::int32_t ID = -60135024;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<testInt>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class testUseUpdate final : public Function {
 public:

  testUseUpdate();

  static const std::int32_t ID = 717094686;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Update>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleAllDownloadsArePaused final : public Function {
 public:
  bool are_paused_;

  toggleAllDownloadsArePaused();

  explicit toggleAllDownloadsArePaused(bool are_paused_);

  static const std::int32_t ID = 1251512322;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleBotIsAddedToAttachmentMenu final : public Function {
 public:
  int53 bot_user_id_;
  bool is_added_;

  toggleBotIsAddedToAttachmentMenu();

  toggleBotIsAddedToAttachmentMenu(int53 bot_user_id_, bool is_added_);

  static const std::int32_t ID = -1251515422;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleChatDefaultDisableNotification final : public Function {
 public:
  int53 chat_id_;
  bool default_disable_notification_;

  toggleChatDefaultDisableNotification();

  toggleChatDefaultDisableNotification(int53 chat_id_, bool default_disable_notification_);

  static const std::int32_t ID = 314794002;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleChatHasProtectedContent final : public Function {
 public:
  int53 chat_id_;
  bool has_protected_content_;

  toggleChatHasProtectedContent();

  toggleChatHasProtectedContent(int53 chat_id_, bool has_protected_content_);

  static const std::int32_t ID = 975231309;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleChatIsMarkedAsUnread final : public Function {
 public:
  int53 chat_id_;
  bool is_marked_as_unread_;

  toggleChatIsMarkedAsUnread();

  toggleChatIsMarkedAsUnread(int53 chat_id_, bool is_marked_as_unread_);

  static const std::int32_t ID = -986129697;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleChatIsPinned final : public Function {
 public:
  object_ptr<ChatList> chat_list_;
  int53 chat_id_;
  bool is_pinned_;

  toggleChatIsPinned();

  toggleChatIsPinned(object_ptr<ChatList> &&chat_list_, int53 chat_id_, bool is_pinned_);

  static const std::int32_t ID = -1485429186;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleDownloadIsPaused final : public Function {
 public:
  int32 file_id_;
  bool is_paused_;

  toggleDownloadIsPaused();

  toggleDownloadIsPaused(int32 file_id_, bool is_paused_);

  static const std::int32_t ID = -947493099;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleForumTopicIsClosed final : public Function {
 public:
  int53 chat_id_;
  int53 message_thread_id_;
  bool is_closed_;

  toggleForumTopicIsClosed();

  toggleForumTopicIsClosed(int53 chat_id_, int53 message_thread_id_, bool is_closed_);

  static const std::int32_t ID = -949712141;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleGroupCallEnabledStartNotification final : public Function {
 public:
  int32 group_call_id_;
  bool enabled_start_notification_;

  toggleGroupCallEnabledStartNotification();

  toggleGroupCallEnabledStartNotification(int32 group_call_id_, bool enabled_start_notification_);

  static const std::int32_t ID = 707839826;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleGroupCallIsMyVideoEnabled final : public Function {
 public:
  int32 group_call_id_;
  bool is_my_video_enabled_;

  toggleGroupCallIsMyVideoEnabled();

  toggleGroupCallIsMyVideoEnabled(int32 group_call_id_, bool is_my_video_enabled_);

  static const std::int32_t ID = -1624289030;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleGroupCallIsMyVideoPaused final : public Function {
 public:
  int32 group_call_id_;
  bool is_my_video_paused_;

  toggleGroupCallIsMyVideoPaused();

  toggleGroupCallIsMyVideoPaused(int32 group_call_id_, bool is_my_video_paused_);

  static const std::int32_t ID = -478875239;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleGroupCallMuteNewParticipants final : public Function {
 public:
  int32 group_call_id_;
  bool mute_new_participants_;

  toggleGroupCallMuteNewParticipants();

  toggleGroupCallMuteNewParticipants(int32 group_call_id_, bool mute_new_participants_);

  static const std::int32_t ID = 284082626;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleGroupCallParticipantIsHandRaised final : public Function {
 public:
  int32 group_call_id_;
  object_ptr<MessageSender> participant_id_;
  bool is_hand_raised_;

  toggleGroupCallParticipantIsHandRaised();

  toggleGroupCallParticipantIsHandRaised(int32 group_call_id_, object_ptr<MessageSender> &&participant_id_, bool is_hand_raised_);

  static const std::int32_t ID = -1896127519;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleGroupCallParticipantIsMuted final : public Function {
 public:
  int32 group_call_id_;
  object_ptr<MessageSender> participant_id_;
  bool is_muted_;

  toggleGroupCallParticipantIsMuted();

  toggleGroupCallParticipantIsMuted(int32 group_call_id_, object_ptr<MessageSender> &&participant_id_, bool is_muted_);

  static const std::int32_t ID = -1308093433;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleGroupCallScreenSharingIsPaused final : public Function {
 public:
  int32 group_call_id_;
  bool is_paused_;

  toggleGroupCallScreenSharingIsPaused();

  toggleGroupCallScreenSharingIsPaused(int32 group_call_id_, bool is_paused_);

  static const std::int32_t ID = -1602530464;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleMessageSenderIsBlocked final : public Function {
 public:
  object_ptr<MessageSender> sender_id_;
  bool is_blocked_;

  toggleMessageSenderIsBlocked();

  toggleMessageSenderIsBlocked(object_ptr<MessageSender> &&sender_id_, bool is_blocked_);

  static const std::int32_t ID = -2008084779;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleSessionCanAcceptCalls final : public Function {
 public:
  int64 session_id_;
  bool can_accept_calls_;

  toggleSessionCanAcceptCalls();

  toggleSessionCanAcceptCalls(int64 session_id_, bool can_accept_calls_);

  static const std::int32_t ID = 1819027208;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleSessionCanAcceptSecretChats final : public Function {
 public:
  int64 session_id_;
  bool can_accept_secret_chats_;

  toggleSessionCanAcceptSecretChats();

  toggleSessionCanAcceptSecretChats(int64 session_id_, bool can_accept_secret_chats_);

  static const std::int32_t ID = 1000843390;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleSupergroupIsAllHistoryAvailable final : public Function {
 public:
  int53 supergroup_id_;
  bool is_all_history_available_;

  toggleSupergroupIsAllHistoryAvailable();

  toggleSupergroupIsAllHistoryAvailable(int53 supergroup_id_, bool is_all_history_available_);

  static const std::int32_t ID = 1155110478;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleSupergroupIsBroadcastGroup final : public Function {
 public:
  int53 supergroup_id_;

  toggleSupergroupIsBroadcastGroup();

  explicit toggleSupergroupIsBroadcastGroup(int53 supergroup_id_);

  static const std::int32_t ID = 884089365;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleSupergroupIsForum final : public Function {
 public:
  int53 supergroup_id_;
  bool is_forum_;

  toggleSupergroupIsForum();

  toggleSupergroupIsForum(int53 supergroup_id_, bool is_forum_);

  static const std::int32_t ID = -1771071990;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleSupergroupJoinByRequest final : public Function {
 public:
  int53 supergroup_id_;
  bool join_by_request_;

  toggleSupergroupJoinByRequest();

  toggleSupergroupJoinByRequest(int53 supergroup_id_, bool join_by_request_);

  static const std::int32_t ID = 2111807454;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleSupergroupJoinToSendMessages final : public Function {
 public:
  int53 supergroup_id_;
  bool join_to_send_messages_;

  toggleSupergroupJoinToSendMessages();

  toggleSupergroupJoinToSendMessages(int53 supergroup_id_, bool join_to_send_messages_);

  static const std::int32_t ID = -182022642;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleSupergroupSignMessages final : public Function {
 public:
  int53 supergroup_id_;
  bool sign_messages_;

  toggleSupergroupSignMessages();

  toggleSupergroupSignMessages(int53 supergroup_id_, bool sign_messages_);

  static const std::int32_t ID = 1156568356;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleSupergroupUsernameIsActive final : public Function {
 public:
  int53 supergroup_id_;
  string username_;
  bool is_active_;

  toggleSupergroupUsernameIsActive();

  toggleSupergroupUsernameIsActive(int53 supergroup_id_, string const &username_, bool is_active_);

  static const std::int32_t ID = -1500811777;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class toggleUsernameIsActive final : public Function {
 public:
  string username_;
  bool is_active_;

  toggleUsernameIsActive();

  toggleUsernameIsActive(string const &username_, bool is_active_);

  static const std::int32_t ID = 1244098019;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class transferChatOwnership final : public Function {
 public:
  int53 chat_id_;
  int53 user_id_;
  string password_;

  transferChatOwnership();

  transferChatOwnership(int53 chat_id_, int53 user_id_, string const &password_);

  static const std::int32_t ID = 2006977043;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class translateText final : public Function {
 public:
  string text_;
  string from_language_code_;
  string to_language_code_;

  translateText();

  translateText(string const &text_, string const &from_language_code_, string const &to_language_code_);

  static const std::int32_t ID = -1619686803;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<text>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class unpinAllChatMessages final : public Function {
 public:
  int53 chat_id_;

  unpinAllChatMessages();

  explicit unpinAllChatMessages(int53 chat_id_);

  static const std::int32_t ID = -1437805385;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class unpinAllMessageThreadMessages final : public Function {
 public:
  int53 chat_id_;
  int53 message_thread_id_;

  unpinAllMessageThreadMessages();

  unpinAllMessageThreadMessages(int53 chat_id_, int53 message_thread_id_);

  static const std::int32_t ID = -1211719936;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class unpinChatMessage final : public Function {
 public:
  int53 chat_id_;
  int53 message_id_;

  unpinChatMessage();

  unpinChatMessage(int53 chat_id_, int53 message_id_);

  static const std::int32_t ID = 2065448670;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class upgradeBasicGroupChatToSupergroupChat final : public Function {
 public:
  int53 chat_id_;

  upgradeBasicGroupChatToSupergroupChat();

  explicit upgradeBasicGroupChatToSupergroupChat(int53 chat_id_);

  static const std::int32_t ID = 300488122;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chat>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class uploadStickerFile final : public Function {
 public:
  int53 user_id_;
  object_ptr<inputSticker> sticker_;

  uploadStickerFile();

  uploadStickerFile(int53 user_id_, object_ptr<inputSticker> &&sticker_);

  static const std::int32_t ID = 86279066;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<file>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class validateOrderInfo final : public Function {
 public:
  object_ptr<InputInvoice> input_invoice_;
  object_ptr<orderInfo> order_info_;
  bool allow_save_;

  validateOrderInfo();

  validateOrderInfo(object_ptr<InputInvoice> &&input_invoice_, object_ptr<orderInfo> &&order_info_, bool allow_save_);

  static const std::int32_t ID = -1248305201;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<validatedOrderInfo>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class viewMessages final : public Function {
 public:
  int53 chat_id_;
  int53 message_thread_id_;
  array<int53> message_ids_;
  bool force_read_;

  viewMessages();

  viewMessages(int53 chat_id_, int53 message_thread_id_, array<int53> &&message_ids_, bool force_read_);

  static const std::int32_t ID = -1155961496;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class viewPremiumFeature final : public Function {
 public:
  object_ptr<PremiumFeature> feature_;

  viewPremiumFeature();

  explicit viewPremiumFeature(object_ptr<PremiumFeature> &&feature_);

  static const std::int32_t ID = 192950706;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class viewTrendingStickerSets final : public Function {
 public:
  array<int64> sticker_set_ids_;

  viewTrendingStickerSets();

  explicit viewTrendingStickerSets(array<int64> &&sticker_set_ids_);

  static const std::int32_t ID = -952416520;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class writeGeneratedFilePart final : public Function {
 public:
  int64 generation_id_;
  int53 offset_;
  bytes data_;

  writeGeneratedFilePart();

  writeGeneratedFilePart(int64 generation_id_, int53 offset_, bytes const &data_);

  static const std::int32_t ID = 214474389;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ok>;

  void store(TlStorerToString &s, const char *field_name) const final;
};

}  // namespace td_api
}  // namespace td
