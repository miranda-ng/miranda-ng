#pragma once

#include "td/tl/TlObject.h"

#include "td/utils/buffer.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace td {
class TlStorerCalcLength;
class TlStorerUnsafe;
class TlStorerToString;
class TlBufferParser;

namespace telegram_api {

using int32 = std::int32_t;
using int53 = std::int64_t;
using int64 = std::int64_t;

using string = std::string;

using bytes = BufferSlice;

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

class accessPointRule;

class accountDaysTTL;

class appWebViewResultUrl;

class attachMenuBot;

class attachMenuBotIcon;

class attachMenuBotIconColor;

class AttachMenuBots;

class attachMenuBotsBot;

class AttachMenuPeerType;

class authorization;

class autoDownloadSettings;

class autoSaveException;

class autoSaveSettings;

class availableReaction;

class bankCardOpenUrl;

class BaseTheme;

class BotApp;

class botCommand;

class BotCommandScope;

class botInfo;

class BotInlineMessage;

class BotInlineResult;

class BotMenuButton;

class cdnConfig;

class cdnPublicKey;

class channelAdminLogEvent;

class ChannelAdminLogEventAction;

class channelAdminLogEventsFilter;

class ChannelLocation;

class ChannelMessagesFilter;

class ChannelParticipant;

class ChannelParticipantsFilter;

class Chat;

class chatAdminRights;

class chatAdminWithInvites;

class chatBannedRights;

class ChatFull;

class ChatInvite;

class chatInviteImporter;

class chatOnlines;

class ChatParticipant;

class ChatParticipants;

class ChatPhoto;

class ChatReactions;

class codeSettings;

class config;

class contact;

class contactStatus;

class dataJSON;

class dcOption;

class defaultHistoryTTL;

class Dialog;

class DialogFilter;

class dialogFilterSuggested;

class DialogPeer;

class Document;

class DocumentAttribute;

class DraftMessage;

class EmailVerification;

class EmailVerifyPurpose;

class emojiGroup;

class EmojiKeyword;

class emojiKeywordsDifference;

class emojiLanguage;

class EmojiList;

class EmojiStatus;

class emojiURL;

class EncryptedChat;

class EncryptedFile;

class EncryptedMessage;

class error;

class ExportedChatInvite;

class exportedChatlistInvite;

class exportedContactToken;

class exportedMessageLink;

class fileHash;

class folder;

class folderPeer;

class ForumTopic;

class game;

class GeoPoint;

class globalPrivacySettings;

class GroupCall;

class groupCallParticipant;

class groupCallParticipantVideo;

class groupCallParticipantVideoSourceGroup;

class groupCallStreamChannel;

class highScore;

class importedContact;

class inlineBotSwitchPM;

class inlineBotWebView;

class InlineQueryPeerType;

class inputAppEvent;

class InputBotApp;

class InputBotInlineMessage;

class InputBotInlineMessageID;

class InputBotInlineResult;

class InputChannel;

class InputChatPhoto;

class inputChatlistDialogFilter;

class InputCheckPasswordSRP;

class inputClientProxy;

class inputPhoneContact;

class InputDialogPeer;

class InputDocument;

class inputEncryptedChat;

class InputEncryptedFile;

class InputFile;

class InputFileLocation;

class inputFolderPeer;

class InputGame;

class InputGeoPoint;

class inputGroupCall;

class InputInvoice;

class InputMedia;

class InputMessage;

class InputNotifyPeer;

class InputPaymentCredentials;

class InputPeer;

class inputPeerNotifySettings;

class inputPhoneCall;

class InputPhoto;

class InputPrivacyKey;

class InputPrivacyRule;

class InputSecureFile;

class inputSecureValue;

class inputSingleMedia;

class InputStickerSet;

class inputStickerSetItem;

class InputStickeredMedia;

class InputStorePaymentPurpose;

class InputTheme;

class inputThemeSettings;

class InputUser;

class InputWallPaper;

class inputWebDocument;

class InputWebFileLocation;

class invoice;

class IpPort;

class jsonObjectValue;

class JSONValue;

class KeyboardButton;

class keyboardButtonRow;

class labeledPrice;

class langPackDifference;

class langPackLanguage;

class LangPackString;

class maskCoords;

class Message;

class MessageAction;

class MessageEntity;

class MessageExtendedMedia;

class messageFwdHeader;

class messageInteractionCounters;

class MessageMedia;

class messagePeerReaction;

class messageRange;

class messageReactions;

class messageReplies;

class messageReplyHeader;

class MessageUserVote;

class messageViews;

class MessagesFilter;

class nearestDc;

class NotificationSound;

class NotifyPeer;

class page;

class PageBlock;

class pageCaption;

class PageListItem;

class PageListOrderedItem;

class pageRelatedArticle;

class pageTableCell;

class pageTableRow;

class PasswordKdfAlgo;

class paymentCharge;

class paymentFormMethod;

class paymentRequestedInfo;

class paymentSavedCredentialsCard;

class Peer;

class peerBlocked;

class PeerLocated;

class peerNotifySettings;

class peerSettings;

class PhoneCall;

class PhoneCallDiscardReason;

class phoneCallProtocol;

class PhoneConnection;

class Photo;

class PhotoSize;

class poll;

class pollAnswer;

class pollAnswerVoters;

class pollResults;

class popularContact;

class postAddress;

class premiumGiftOption;

class premiumSubscriptionOption;

class PrivacyKey;

class PrivacyRule;

class Reaction;

class reactionCount;

class readParticipantDate;

class receivedNotifyMessage;

class RecentMeUrl;

class ReplyMarkup;

class ReportReason;

class RequestPeerType;

class restrictionReason;

class RichText;

class savedPhoneContact;

class searchResultsCalendarPeriod;

class searchResultPosition;

class secureCredentialsEncrypted;

class secureData;

class SecureFile;

class SecurePasswordKdfAlgo;

class SecurePlainData;

class SecureRequiredType;

class secureSecretSettings;

class secureValue;

class SecureValueError;

class secureValueHash;

class SecureValueType;

class sendAsPeer;

class SendMessageAction;

class shippingOption;

class simpleWebViewResultUrl;

class sponsoredMessage;

class statsAbsValueAndPrev;

class statsDateRangeDays;

class StatsGraph;

class statsGroupTopAdmin;

class statsGroupTopInviter;

class statsGroupTopPoster;

class statsPercentValue;

class statsURL;

class stickerKeyword;

class stickerPack;

class stickerSet;

class StickerSetCovered;

class textWithEntities;

class theme;

class themeSettings;

class topPeer;

class TopPeerCategory;

class topPeerCategoryPeers;

class Update;

class Updates;

class UrlAuthResult;

class User;

class userFull;

class UserProfilePhoto;

class UserStatus;

class username;

class VideoSize;

class WallPaper;

class wallPaperSettings;

class webAuthorization;

class WebDocument;

class WebPage;

class webPageAttributeTheme;

class webViewMessageSent;

class webViewResultUrl;

class account_authorizationForm;

class account_authorizations;

class account_autoDownloadSettings;

class account_autoSaveSettings;

class account_contentSettings;

class account_EmailVerified;

class account_EmojiStatuses;

class account_password;

class account_passwordInputSettings;

class account_passwordSettings;

class account_privacyRules;

class account_ResetPasswordResult;

class account_SavedRingtone;

class account_SavedRingtones;

class account_sentEmailCode;

class account_takeout;

class account_Themes;

class account_tmpPassword;

class account_WallPapers;

class account_webAuthorizations;

class auth_Authorization;

class auth_CodeType;

class auth_exportedAuthorization;

class auth_loggedOut;

class auth_LoginToken;

class auth_passwordRecovery;

class auth_SentCode;

class auth_SentCodeType;

class bots_botInfo;

class channels_adminLogResults;

class channels_channelParticipant;

class channels_ChannelParticipants;

class channels_sendAsPeers;

class chatlists_ChatlistInvite;

class chatlists_chatlistUpdates;

class chatlists_exportedChatlistInvite;

class chatlists_exportedInvites;

class contacts_Blocked;

class contacts_Contacts;

class contacts_found;

class contacts_importedContacts;

class contacts_resolvedPeer;

class contacts_TopPeers;

class help_AppConfig;

class help_AppUpdate;

class help_configSimple;

class help_CountriesList;

class help_country;

class help_countryCode;

class help_DeepLinkInfo;

class help_inviteText;

class help_PassportConfig;

class help_premiumPromo;

class help_PromoData;

class help_recentMeUrls;

class help_support;

class help_supportName;

class help_termsOfService;

class help_TermsOfServiceUpdate;

class help_UserInfo;

class messages_affectedFoundMessages;

class messages_affectedHistory;

class messages_affectedMessages;

class messages_AllStickers;

class messages_archivedStickers;

class messages_AvailableReactions;

class messages_botApp;

class messages_botCallbackAnswer;

class messages_botResults;

class messages_chatAdminsWithInvites;

class messages_chatFull;

class messages_chatInviteImporters;

class messages_Chats;

class messages_checkedHistoryImportPeer;

class messages_DhConfig;

class messages_Dialogs;

class messages_discussionMessage;

class messages_EmojiGroups;

class messages_ExportedChatInvite;

class messages_exportedChatInvites;

class messages_FavedStickers;

class messages_FeaturedStickers;

class messages_forumTopics;

class messages_FoundStickerSets;

class messages_highScores;

class messages_historyImport;

class messages_historyImportParsed;

class messages_inactiveChats;

class messages_messageEditData;

class messages_messageReactionsList;

class messages_messageViews;

class messages_Messages;

class messages_peerDialogs;

class messages_peerSettings;

class messages_Reactions;

class messages_RecentStickers;

class messages_SavedGifs;

class messages_searchCounter;

class messages_searchResultsCalendar;

class messages_searchResultsPositions;

class messages_SentEncryptedMessage;

class messages_SponsoredMessages;

class messages_StickerSet;

class messages_StickerSetInstallResult;

class messages_Stickers;

class messages_transcribedAudio;

class messages_translateResult;

class messages_votesList;

class payments_bankCardData;

class payments_exportedInvoice;

class payments_paymentForm;

class payments_paymentReceipt;

class payments_PaymentResult;

class payments_savedInfo;

class payments_validatedRequestedInfo;

class phone_exportedGroupCallInvite;

class phone_groupCall;

class phone_groupCallStreamChannels;

class phone_groupCallStreamRtmpUrl;

class phone_groupParticipants;

class phone_joinAsPeers;

class phone_phoneCall;

class photos_photo;

class photos_Photos;

class stats_broadcastStats;

class stats_megagroupStats;

class stats_messageStats;

class stickers_suggestedShortName;

class storage_FileType;

class updates_ChannelDifference;

class updates_Difference;

class updates_state;

class upload_CdnFile;

class upload_File;

class upload_webFile;

class users_userFull;

class Object;

class Object: public TlObject {
 public:

  static object_ptr<Object> fetch(TlBufferParser &p);
};

class Function: public TlObject {
 public:
};

class accessPointRule final : public Object {
 public:
  string phone_prefix_rules_;
  int32 dc_id_;
  array<object_ptr<IpPort>> ips_;

  static const std::int32_t ID = 1182381663;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<accessPointRule> fetch(TlBufferParser &p);

  explicit accessPointRule(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class accountDaysTTL final : public Object {
 public:
  int32 days_;

  explicit accountDaysTTL(int32 days_);

  static const std::int32_t ID = -1194283041;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<accountDaysTTL> fetch(TlBufferParser &p);

  explicit accountDaysTTL(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class appWebViewResultUrl final : public Object {
 public:
  string url_;

  static const std::int32_t ID = 1008422669;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<appWebViewResultUrl> fetch(TlBufferParser &p);

  explicit appWebViewResultUrl(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class attachMenuBot final : public Object {
 public:
  int32 flags_;
  bool inactive_;
  bool has_settings_;
  bool request_write_access_;
  int64 bot_id_;
  string short_name_;
  array<object_ptr<AttachMenuPeerType>> peer_types_;
  array<object_ptr<attachMenuBotIcon>> icons_;

  attachMenuBot();

  static const std::int32_t ID = -928371502;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<attachMenuBot> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class attachMenuBotIcon final : public Object {
 public:
  int32 flags_;
  string name_;
  object_ptr<Document> icon_;
  array<object_ptr<attachMenuBotIconColor>> colors_;

  attachMenuBotIcon();

  static const std::int32_t ID = -1297663893;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<attachMenuBotIcon> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class attachMenuBotIconColor final : public Object {
 public:
  string name_;
  int32 color_;

  static const std::int32_t ID = 1165423600;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<attachMenuBotIconColor> fetch(TlBufferParser &p);

  explicit attachMenuBotIconColor(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class AttachMenuBots: public Object {
 public:

  static object_ptr<AttachMenuBots> fetch(TlBufferParser &p);
};

class attachMenuBotsNotModified final : public AttachMenuBots {
 public:

  static const std::int32_t ID = -237467044;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<AttachMenuBots> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class attachMenuBots final : public AttachMenuBots {
 public:
  int64 hash_;
  array<object_ptr<attachMenuBot>> bots_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = 1011024320;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<AttachMenuBots> fetch(TlBufferParser &p);

  explicit attachMenuBots(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class attachMenuBotsBot final : public Object {
 public:
  object_ptr<attachMenuBot> bot_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -1816172929;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<attachMenuBotsBot> fetch(TlBufferParser &p);

  explicit attachMenuBotsBot(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class AttachMenuPeerType: public Object {
 public:

  static object_ptr<AttachMenuPeerType> fetch(TlBufferParser &p);
};

class attachMenuPeerTypeSameBotPM final : public AttachMenuPeerType {
 public:

  static const std::int32_t ID = 2104224014;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<AttachMenuPeerType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class attachMenuPeerTypeBotPM final : public AttachMenuPeerType {
 public:

  static const std::int32_t ID = -1020528102;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<AttachMenuPeerType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class attachMenuPeerTypePM final : public AttachMenuPeerType {
 public:

  static const std::int32_t ID = -247016673;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<AttachMenuPeerType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class attachMenuPeerTypeChat final : public AttachMenuPeerType {
 public:

  static const std::int32_t ID = 84480319;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<AttachMenuPeerType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class attachMenuPeerTypeBroadcast final : public AttachMenuPeerType {
 public:

  static const std::int32_t ID = 2080104188;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<AttachMenuPeerType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class authorization final : public Object {
 public:
  int32 flags_;
  bool current_;
  bool official_app_;
  bool password_pending_;
  bool encrypted_requests_disabled_;
  bool call_requests_disabled_;
  int64 hash_;
  string device_model_;
  string platform_;
  string system_version_;
  int32 api_id_;
  string app_name_;
  string app_version_;
  int32 date_created_;
  int32 date_active_;
  string ip_;
  string country_;
  string region_;

  authorization();

  static const std::int32_t ID = -1392388579;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<authorization> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class autoDownloadSettings final : public Object {
 public:
  int32 flags_;
  bool disabled_;
  bool video_preload_large_;
  bool audio_preload_next_;
  bool phonecalls_less_data_;
  int32 photo_size_max_;
  int64 video_size_max_;
  int64 file_size_max_;
  int32 video_upload_maxbitrate_;
  enum Flags : std::int32_t { DISABLED_MASK = 1, VIDEO_PRELOAD_LARGE_MASK = 2, AUDIO_PRELOAD_NEXT_MASK = 4, PHONECALLS_LESS_DATA_MASK = 8 };

  autoDownloadSettings();

  autoDownloadSettings(int32 flags_, bool disabled_, bool video_preload_large_, bool audio_preload_next_, bool phonecalls_less_data_, int32 photo_size_max_, int64 video_size_max_, int64 file_size_max_, int32 video_upload_maxbitrate_);

  static const std::int32_t ID = -1896171181;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<autoDownloadSettings> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class autoSaveException final : public Object {
 public:
  object_ptr<Peer> peer_;
  object_ptr<autoSaveSettings> settings_;

  static const std::int32_t ID = -2124403385;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<autoSaveException> fetch(TlBufferParser &p);

  explicit autoSaveException(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class autoSaveSettings final : public Object {
 public:
  int32 flags_;
  bool photos_;
  bool videos_;
  int64 video_max_size_;
  enum Flags : std::int32_t { PHOTOS_MASK = 1, VIDEOS_MASK = 2, VIDEO_MAX_SIZE_MASK = 4 };

  autoSaveSettings();

  autoSaveSettings(int32 flags_, bool photos_, bool videos_, int64 video_max_size_);

  static const std::int32_t ID = -934791986;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<autoSaveSettings> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class availableReaction final : public Object {
 public:
  int32 flags_;
  bool inactive_;
  bool premium_;
  string reaction_;
  string title_;
  object_ptr<Document> static_icon_;
  object_ptr<Document> appear_animation_;
  object_ptr<Document> select_animation_;
  object_ptr<Document> activate_animation_;
  object_ptr<Document> effect_animation_;
  object_ptr<Document> around_animation_;
  object_ptr<Document> center_icon_;

  availableReaction();

  static const std::int32_t ID = -1065882623;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<availableReaction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class bankCardOpenUrl final : public Object {
 public:
  string url_;
  string name_;

  static const std::int32_t ID = -177732982;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<bankCardOpenUrl> fetch(TlBufferParser &p);

  explicit bankCardOpenUrl(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class BaseTheme: public Object {
 public:

  static object_ptr<BaseTheme> fetch(TlBufferParser &p);
};

class baseThemeClassic final : public BaseTheme {
 public:

  static const std::int32_t ID = -1012849566;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<BaseTheme> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class baseThemeDay final : public BaseTheme {
 public:

  static const std::int32_t ID = -69724536;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<BaseTheme> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class baseThemeNight final : public BaseTheme {
 public:

  static const std::int32_t ID = -1212997976;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<BaseTheme> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class baseThemeTinted final : public BaseTheme {
 public:

  static const std::int32_t ID = 1834973166;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<BaseTheme> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class baseThemeArctic final : public BaseTheme {
 public:

  static const std::int32_t ID = 1527845466;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<BaseTheme> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class BotApp: public Object {
 public:

  static object_ptr<BotApp> fetch(TlBufferParser &p);
};

class botAppNotModified final : public BotApp {
 public:

  static const std::int32_t ID = 1571189943;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<BotApp> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botApp final : public BotApp {
 public:
  int32 flags_;
  int64 id_;
  int64 access_hash_;
  string short_name_;
  string title_;
  string description_;
  object_ptr<Photo> photo_;
  object_ptr<Document> document_;
  int64 hash_;

  botApp();

  static const std::int32_t ID = -1778593322;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<BotApp> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botCommand final : public Object {
 public:
  string command_;
  string description_;

  botCommand(string const &command_, string const &description_);

  static const std::int32_t ID = -1032140601;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<botCommand> fetch(TlBufferParser &p);

  explicit botCommand(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class BotCommandScope: public Object {
 public:
};

class botCommandScopeDefault final : public BotCommandScope {
 public:

  static const std::int32_t ID = 795652779;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botCommandScopeUsers final : public BotCommandScope {
 public:

  static const std::int32_t ID = 1011811544;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botCommandScopeChats final : public BotCommandScope {
 public:

  static const std::int32_t ID = 1877059713;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botCommandScopeChatAdmins final : public BotCommandScope {
 public:

  static const std::int32_t ID = -1180016534;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botCommandScopePeer final : public BotCommandScope {
 public:
  object_ptr<InputPeer> peer_;

  explicit botCommandScopePeer(object_ptr<InputPeer> &&peer_);

  static const std::int32_t ID = -610432643;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botCommandScopePeerAdmins final : public BotCommandScope {
 public:
  object_ptr<InputPeer> peer_;

  explicit botCommandScopePeerAdmins(object_ptr<InputPeer> &&peer_);

  static const std::int32_t ID = 1071145937;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botCommandScopePeerUser final : public BotCommandScope {
 public:
  object_ptr<InputPeer> peer_;
  object_ptr<InputUser> user_id_;

  botCommandScopePeerUser(object_ptr<InputPeer> &&peer_, object_ptr<InputUser> &&user_id_);

  static const std::int32_t ID = 169026035;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botInfo final : public Object {
 public:
  int32 flags_;
  int64 user_id_;
  string description_;
  object_ptr<Photo> description_photo_;
  object_ptr<Document> description_document_;
  array<object_ptr<botCommand>> commands_;
  object_ptr<BotMenuButton> menu_button_;
  enum Flags : std::int32_t { USER_ID_MASK = 1, DESCRIPTION_MASK = 2 };

  botInfo();

  static const std::int32_t ID = -1892676777;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<botInfo> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class BotInlineMessage: public Object {
 public:

  static object_ptr<BotInlineMessage> fetch(TlBufferParser &p);
};

class botInlineMessageMediaAuto final : public BotInlineMessage {
 public:
  int32 flags_;
  string message_;
  array<object_ptr<MessageEntity>> entities_;
  object_ptr<ReplyMarkup> reply_markup_;

  botInlineMessageMediaAuto();

  static const std::int32_t ID = 1984755728;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<BotInlineMessage> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botInlineMessageText final : public BotInlineMessage {
 public:
  int32 flags_;
  bool no_webpage_;
  string message_;
  array<object_ptr<MessageEntity>> entities_;
  object_ptr<ReplyMarkup> reply_markup_;

  botInlineMessageText();

  static const std::int32_t ID = -1937807902;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<BotInlineMessage> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botInlineMessageMediaGeo final : public BotInlineMessage {
 public:
  int32 flags_;
  object_ptr<GeoPoint> geo_;
  int32 heading_;
  int32 period_;
  int32 proximity_notification_radius_;
  object_ptr<ReplyMarkup> reply_markup_;
  enum Flags : std::int32_t { HEADING_MASK = 1, PERIOD_MASK = 2, PROXIMITY_NOTIFICATION_RADIUS_MASK = 8 };

  botInlineMessageMediaGeo();

  static const std::int32_t ID = 85477117;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<BotInlineMessage> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botInlineMessageMediaVenue final : public BotInlineMessage {
 public:
  int32 flags_;
  object_ptr<GeoPoint> geo_;
  string title_;
  string address_;
  string provider_;
  string venue_id_;
  string venue_type_;
  object_ptr<ReplyMarkup> reply_markup_;

  botInlineMessageMediaVenue();

  static const std::int32_t ID = -1970903652;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<BotInlineMessage> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botInlineMessageMediaContact final : public BotInlineMessage {
 public:
  int32 flags_;
  string phone_number_;
  string first_name_;
  string last_name_;
  string vcard_;
  object_ptr<ReplyMarkup> reply_markup_;

  botInlineMessageMediaContact();

  static const std::int32_t ID = 416402882;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<BotInlineMessage> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botInlineMessageMediaInvoice final : public BotInlineMessage {
 public:
  int32 flags_;
  bool shipping_address_requested_;
  bool test_;
  string title_;
  string description_;
  object_ptr<WebDocument> photo_;
  string currency_;
  int64 total_amount_;
  object_ptr<ReplyMarkup> reply_markup_;

  botInlineMessageMediaInvoice();

  static const std::int32_t ID = 894081801;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<BotInlineMessage> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class BotInlineResult: public Object {
 public:

  static object_ptr<BotInlineResult> fetch(TlBufferParser &p);
};

class botInlineResult final : public BotInlineResult {
 public:
  int32 flags_;
  string id_;
  string type_;
  string title_;
  string description_;
  string url_;
  object_ptr<WebDocument> thumb_;
  object_ptr<WebDocument> content_;
  object_ptr<BotInlineMessage> send_message_;
  enum Flags : std::int32_t { TITLE_MASK = 2, DESCRIPTION_MASK = 4, URL_MASK = 8 };

  botInlineResult();

  static const std::int32_t ID = 295067450;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<BotInlineResult> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botInlineMediaResult final : public BotInlineResult {
 public:
  int32 flags_;
  string id_;
  string type_;
  object_ptr<Photo> photo_;
  object_ptr<Document> document_;
  string title_;
  string description_;
  object_ptr<BotInlineMessage> send_message_;
  enum Flags : std::int32_t { TITLE_MASK = 4, DESCRIPTION_MASK = 8 };

  botInlineMediaResult();

  static const std::int32_t ID = 400266251;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<BotInlineResult> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class BotMenuButton: public Object {
 public:

  static object_ptr<BotMenuButton> fetch(TlBufferParser &p);
};

class botMenuButtonDefault final : public BotMenuButton {
 public:

  static const std::int32_t ID = 1966318984;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<BotMenuButton> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botMenuButtonCommands final : public BotMenuButton {
 public:

  static const std::int32_t ID = 1113113093;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<BotMenuButton> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class botMenuButton final : public BotMenuButton {
 public:
  string text_;
  string url_;

  botMenuButton(string const &text_, string const &url_);

  static const std::int32_t ID = -944407322;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<BotMenuButton> fetch(TlBufferParser &p);

  explicit botMenuButton(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class cdnConfig final : public Object {
 public:
  array<object_ptr<cdnPublicKey>> public_keys_;

  static const std::int32_t ID = 1462101002;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<cdnConfig> fetch(TlBufferParser &p);

  explicit cdnConfig(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class cdnPublicKey final : public Object {
 public:
  int32 dc_id_;
  string public_key_;

  static const std::int32_t ID = -914167110;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<cdnPublicKey> fetch(TlBufferParser &p);

  explicit cdnPublicKey(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEvent final : public Object {
 public:
  int64 id_;
  int32 date_;
  int64 user_id_;
  object_ptr<ChannelAdminLogEventAction> action_;

  static const std::int32_t ID = 531458253;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<channelAdminLogEvent> fetch(TlBufferParser &p);

  explicit channelAdminLogEvent(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChannelAdminLogEventAction: public Object {
 public:

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);
};

class channelAdminLogEventActionChangeTitle final : public ChannelAdminLogEventAction {
 public:
  string prev_value_;
  string new_value_;

  static const std::int32_t ID = -421545947;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionChangeTitle(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionChangeAbout final : public ChannelAdminLogEventAction {
 public:
  string prev_value_;
  string new_value_;

  static const std::int32_t ID = 1427671598;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionChangeAbout(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionChangeUsername final : public ChannelAdminLogEventAction {
 public:
  string prev_value_;
  string new_value_;

  static const std::int32_t ID = 1783299128;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionChangeUsername(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionChangePhoto final : public ChannelAdminLogEventAction {
 public:
  object_ptr<Photo> prev_photo_;
  object_ptr<Photo> new_photo_;

  static const std::int32_t ID = 1129042607;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionChangePhoto(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionToggleInvites final : public ChannelAdminLogEventAction {
 public:
  bool new_value_;

  static const std::int32_t ID = 460916654;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionToggleInvites(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionToggleSignatures final : public ChannelAdminLogEventAction {
 public:
  bool new_value_;

  static const std::int32_t ID = 648939889;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionToggleSignatures(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionUpdatePinned final : public ChannelAdminLogEventAction {
 public:
  object_ptr<Message> message_;

  static const std::int32_t ID = -370660328;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionUpdatePinned(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionEditMessage final : public ChannelAdminLogEventAction {
 public:
  object_ptr<Message> prev_message_;
  object_ptr<Message> new_message_;

  static const std::int32_t ID = 1889215493;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionEditMessage(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionDeleteMessage final : public ChannelAdminLogEventAction {
 public:
  object_ptr<Message> message_;

  static const std::int32_t ID = 1121994683;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionDeleteMessage(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionParticipantJoin final : public ChannelAdminLogEventAction {
 public:

  static const std::int32_t ID = 405815507;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionParticipantLeave final : public ChannelAdminLogEventAction {
 public:

  static const std::int32_t ID = -124291086;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionParticipantInvite final : public ChannelAdminLogEventAction {
 public:
  object_ptr<ChannelParticipant> participant_;

  static const std::int32_t ID = -484690728;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionParticipantInvite(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionParticipantToggleBan final : public ChannelAdminLogEventAction {
 public:
  object_ptr<ChannelParticipant> prev_participant_;
  object_ptr<ChannelParticipant> new_participant_;

  static const std::int32_t ID = -422036098;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionParticipantToggleBan(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionParticipantToggleAdmin final : public ChannelAdminLogEventAction {
 public:
  object_ptr<ChannelParticipant> prev_participant_;
  object_ptr<ChannelParticipant> new_participant_;

  static const std::int32_t ID = -714643696;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionParticipantToggleAdmin(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionChangeStickerSet final : public ChannelAdminLogEventAction {
 public:
  object_ptr<InputStickerSet> prev_stickerset_;
  object_ptr<InputStickerSet> new_stickerset_;

  static const std::int32_t ID = -1312568665;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionChangeStickerSet(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionTogglePreHistoryHidden final : public ChannelAdminLogEventAction {
 public:
  bool new_value_;

  static const std::int32_t ID = 1599903217;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionTogglePreHistoryHidden(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionDefaultBannedRights final : public ChannelAdminLogEventAction {
 public:
  object_ptr<chatBannedRights> prev_banned_rights_;
  object_ptr<chatBannedRights> new_banned_rights_;

  static const std::int32_t ID = 771095562;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionDefaultBannedRights(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionStopPoll final : public ChannelAdminLogEventAction {
 public:
  object_ptr<Message> message_;

  static const std::int32_t ID = -1895328189;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionStopPoll(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionChangeLinkedChat final : public ChannelAdminLogEventAction {
 public:
  int64 prev_value_;
  int64 new_value_;

  static const std::int32_t ID = 84703944;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionChangeLinkedChat(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionChangeLocation final : public ChannelAdminLogEventAction {
 public:
  object_ptr<ChannelLocation> prev_value_;
  object_ptr<ChannelLocation> new_value_;

  static const std::int32_t ID = 241923758;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionChangeLocation(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionToggleSlowMode final : public ChannelAdminLogEventAction {
 public:
  int32 prev_value_;
  int32 new_value_;

  static const std::int32_t ID = 1401984889;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionToggleSlowMode(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionStartGroupCall final : public ChannelAdminLogEventAction {
 public:
  object_ptr<inputGroupCall> call_;

  static const std::int32_t ID = 589338437;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionStartGroupCall(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionDiscardGroupCall final : public ChannelAdminLogEventAction {
 public:
  object_ptr<inputGroupCall> call_;

  static const std::int32_t ID = -610299584;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionDiscardGroupCall(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionParticipantMute final : public ChannelAdminLogEventAction {
 public:
  object_ptr<groupCallParticipant> participant_;

  static const std::int32_t ID = -115071790;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionParticipantMute(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionParticipantUnmute final : public ChannelAdminLogEventAction {
 public:
  object_ptr<groupCallParticipant> participant_;

  static const std::int32_t ID = -431740480;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionParticipantUnmute(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionToggleGroupCallSetting final : public ChannelAdminLogEventAction {
 public:
  bool join_muted_;

  static const std::int32_t ID = 1456906823;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionToggleGroupCallSetting(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionParticipantJoinByInvite final : public ChannelAdminLogEventAction {
 public:
  int32 flags_;
  bool via_chatlist_;
  object_ptr<ExportedChatInvite> invite_;

  channelAdminLogEventActionParticipantJoinByInvite();

  static const std::int32_t ID = -23084712;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionExportedInviteDelete final : public ChannelAdminLogEventAction {
 public:
  object_ptr<ExportedChatInvite> invite_;

  static const std::int32_t ID = 1515256996;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionExportedInviteDelete(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionExportedInviteRevoke final : public ChannelAdminLogEventAction {
 public:
  object_ptr<ExportedChatInvite> invite_;

  static const std::int32_t ID = 1091179342;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionExportedInviteRevoke(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionExportedInviteEdit final : public ChannelAdminLogEventAction {
 public:
  object_ptr<ExportedChatInvite> prev_invite_;
  object_ptr<ExportedChatInvite> new_invite_;

  static const std::int32_t ID = -384910503;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionExportedInviteEdit(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionParticipantVolume final : public ChannelAdminLogEventAction {
 public:
  object_ptr<groupCallParticipant> participant_;

  static const std::int32_t ID = 1048537159;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionParticipantVolume(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionChangeHistoryTTL final : public ChannelAdminLogEventAction {
 public:
  int32 prev_value_;
  int32 new_value_;

  static const std::int32_t ID = 1855199800;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionChangeHistoryTTL(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionParticipantJoinByRequest final : public ChannelAdminLogEventAction {
 public:
  object_ptr<ExportedChatInvite> invite_;
  int64 approved_by_;

  static const std::int32_t ID = -1347021750;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionParticipantJoinByRequest(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionToggleNoForwards final : public ChannelAdminLogEventAction {
 public:
  bool new_value_;

  static const std::int32_t ID = -886388890;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionToggleNoForwards(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionSendMessage final : public ChannelAdminLogEventAction {
 public:
  object_ptr<Message> message_;

  static const std::int32_t ID = 663693416;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionSendMessage(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionChangeAvailableReactions final : public ChannelAdminLogEventAction {
 public:
  object_ptr<ChatReactions> prev_value_;
  object_ptr<ChatReactions> new_value_;

  static const std::int32_t ID = -1102180616;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionChangeAvailableReactions(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionChangeUsernames final : public ChannelAdminLogEventAction {
 public:
  array<string> prev_value_;
  array<string> new_value_;

  static const std::int32_t ID = -263212119;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionChangeUsernames(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionToggleForum final : public ChannelAdminLogEventAction {
 public:
  bool new_value_;

  static const std::int32_t ID = 46949251;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionToggleForum(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionCreateTopic final : public ChannelAdminLogEventAction {
 public:
  object_ptr<ForumTopic> topic_;

  static const std::int32_t ID = 1483767080;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionCreateTopic(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionEditTopic final : public ChannelAdminLogEventAction {
 public:
  object_ptr<ForumTopic> prev_topic_;
  object_ptr<ForumTopic> new_topic_;

  static const std::int32_t ID = -261103096;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionEditTopic(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionDeleteTopic final : public ChannelAdminLogEventAction {
 public:
  object_ptr<ForumTopic> topic_;

  static const std::int32_t ID = -1374254839;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionDeleteTopic(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionPinTopic final : public ChannelAdminLogEventAction {
 public:
  int32 flags_;
  object_ptr<ForumTopic> prev_topic_;
  object_ptr<ForumTopic> new_topic_;

  channelAdminLogEventActionPinTopic();

  static const std::int32_t ID = 1569535291;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventActionToggleAntiSpam final : public ChannelAdminLogEventAction {
 public:
  bool new_value_;

  static const std::int32_t ID = 1693675004;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelAdminLogEventAction> fetch(TlBufferParser &p);

  explicit channelAdminLogEventActionToggleAntiSpam(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelAdminLogEventsFilter final : public Object {
 public:
  int32 flags_;
  bool join_;
  bool leave_;
  bool invite_;
  bool ban_;
  bool unban_;
  bool kick_;
  bool unkick_;
  bool promote_;
  bool demote_;
  bool info_;
  bool settings_;
  bool pinned_;
  bool edit_;
  bool delete_;
  bool group_call_;
  bool invites_;
  bool send_;
  bool forums_;
  enum Flags : std::int32_t { JOIN_MASK = 1, LEAVE_MASK = 2, INVITE_MASK = 4, BAN_MASK = 8, UNBAN_MASK = 16, KICK_MASK = 32, UNKICK_MASK = 64, PROMOTE_MASK = 128, DEMOTE_MASK = 256, INFO_MASK = 512, SETTINGS_MASK = 1024, PINNED_MASK = 2048, EDIT_MASK = 4096, DELETE_MASK = 8192, GROUP_CALL_MASK = 16384, INVITES_MASK = 32768, SEND_MASK = 65536, FORUMS_MASK = 131072 };

  channelAdminLogEventsFilter(int32 flags_, bool join_, bool leave_, bool invite_, bool ban_, bool unban_, bool kick_, bool unkick_, bool promote_, bool demote_, bool info_, bool settings_, bool pinned_, bool edit_, bool delete_, bool group_call_, bool invites_, bool send_, bool forums_);

  static const std::int32_t ID = -368018716;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChannelLocation: public Object {
 public:

  static object_ptr<ChannelLocation> fetch(TlBufferParser &p);
};

class channelLocationEmpty final : public ChannelLocation {
 public:

  static const std::int32_t ID = -1078612597;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelLocation> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelLocation final : public ChannelLocation {
 public:
  object_ptr<GeoPoint> geo_point_;
  string address_;

  static const std::int32_t ID = 547062491;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelLocation> fetch(TlBufferParser &p);

  explicit channelLocation(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChannelMessagesFilter: public Object {
 public:
};

class channelMessagesFilterEmpty final : public ChannelMessagesFilter {
 public:

  static const std::int32_t ID = -1798033689;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelMessagesFilter final : public ChannelMessagesFilter {
 public:
  int32 flags_;
  bool exclude_new_messages_;
  array<object_ptr<messageRange>> ranges_;
  enum Flags : std::int32_t { EXCLUDE_NEW_MESSAGES_MASK = 2 };

  channelMessagesFilter(int32 flags_, bool exclude_new_messages_, array<object_ptr<messageRange>> &&ranges_);

  static const std::int32_t ID = -847783593;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChannelParticipant: public Object {
 public:

  static object_ptr<ChannelParticipant> fetch(TlBufferParser &p);
};

class channelParticipant final : public ChannelParticipant {
 public:
  int64 user_id_;
  int32 date_;

  static const std::int32_t ID = -1072953408;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelParticipant> fetch(TlBufferParser &p);

  explicit channelParticipant(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelParticipantSelf final : public ChannelParticipant {
 public:
  int32 flags_;
  bool via_request_;
  int64 user_id_;
  int64 inviter_id_;
  int32 date_;

  channelParticipantSelf();

  static const std::int32_t ID = 900251559;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelParticipant> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelParticipantCreator final : public ChannelParticipant {
 public:
  int32 flags_;
  int64 user_id_;
  object_ptr<chatAdminRights> admin_rights_;
  string rank_;
  enum Flags : std::int32_t { RANK_MASK = 1 };

  channelParticipantCreator();

  static const std::int32_t ID = 803602899;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelParticipant> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelParticipantAdmin final : public ChannelParticipant {
 public:
  int32 flags_;
  bool can_edit_;
  bool self_;
  int64 user_id_;
  int64 inviter_id_;
  int64 promoted_by_;
  int32 date_;
  object_ptr<chatAdminRights> admin_rights_;
  string rank_;
  enum Flags : std::int32_t { INVITER_ID_MASK = 2, RANK_MASK = 4 };

  channelParticipantAdmin();

  static const std::int32_t ID = 885242707;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelParticipant> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelParticipantBanned final : public ChannelParticipant {
 public:
  int32 flags_;
  bool left_;
  object_ptr<Peer> peer_;
  int64 kicked_by_;
  int32 date_;
  object_ptr<chatBannedRights> banned_rights_;

  channelParticipantBanned();

  static const std::int32_t ID = 1844969806;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelParticipant> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelParticipantLeft final : public ChannelParticipant {
 public:
  object_ptr<Peer> peer_;

  static const std::int32_t ID = 453242886;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChannelParticipant> fetch(TlBufferParser &p);

  explicit channelParticipantLeft(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChannelParticipantsFilter: public Object {
 public:
};

class channelParticipantsRecent final : public ChannelParticipantsFilter {
 public:

  static const std::int32_t ID = -566281095;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelParticipantsAdmins final : public ChannelParticipantsFilter {
 public:

  static const std::int32_t ID = -1268741783;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelParticipantsKicked final : public ChannelParticipantsFilter {
 public:
  string q_;

  explicit channelParticipantsKicked(string const &q_);

  static const std::int32_t ID = -1548400251;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelParticipantsBots final : public ChannelParticipantsFilter {
 public:

  static const std::int32_t ID = -1328445861;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelParticipantsBanned final : public ChannelParticipantsFilter {
 public:
  string q_;

  explicit channelParticipantsBanned(string const &q_);

  static const std::int32_t ID = 338142689;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelParticipantsSearch final : public ChannelParticipantsFilter {
 public:
  string q_;

  explicit channelParticipantsSearch(string const &q_);

  static const std::int32_t ID = 106343499;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelParticipantsContacts final : public ChannelParticipantsFilter {
 public:
  string q_;

  explicit channelParticipantsContacts(string const &q_);

  static const std::int32_t ID = -1150621555;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelParticipantsMentions final : public ChannelParticipantsFilter {
 public:
  int32 flags_;
  string q_;
  int32 top_msg_id_;
  enum Flags : std::int32_t { Q_MASK = 1, TOP_MSG_ID_MASK = 2 };

  channelParticipantsMentions(int32 flags_, string const &q_, int32 top_msg_id_);

  static const std::int32_t ID = -531931925;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class Chat: public Object {
 public:

  static object_ptr<Chat> fetch(TlBufferParser &p);
};

class chatEmpty final : public Chat {
 public:
  int64 id_;

  static const std::int32_t ID = 693512293;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Chat> fetch(TlBufferParser &p);

  explicit chatEmpty(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chat final : public Chat {
 public:
  int32 flags_;
  bool creator_;
  bool left_;
  bool deactivated_;
  bool call_active_;
  bool call_not_empty_;
  bool noforwards_;
  int64 id_;
  string title_;
  object_ptr<ChatPhoto> photo_;
  int32 participants_count_;
  int32 date_;
  int32 version_;
  object_ptr<InputChannel> migrated_to_;
  object_ptr<chatAdminRights> admin_rights_;
  object_ptr<chatBannedRights> default_banned_rights_;

  chat();

  static const std::int32_t ID = 1103884886;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Chat> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatForbidden final : public Chat {
 public:
  int64 id_;
  string title_;

  static const std::int32_t ID = 1704108455;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Chat> fetch(TlBufferParser &p);

  explicit chatForbidden(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channel final : public Chat {
 public:
  int32 flags_;
  bool creator_;
  bool left_;
  bool broadcast_;
  bool verified_;
  bool megagroup_;
  bool restricted_;
  bool signatures_;
  bool min_;
  bool scam_;
  bool has_link_;
  bool has_geo_;
  bool slowmode_enabled_;
  bool call_active_;
  bool call_not_empty_;
  bool fake_;
  bool gigagroup_;
  bool noforwards_;
  bool join_to_send_;
  bool join_request_;
  bool forum_;
  int32 flags2_;
  int64 id_;
  int64 access_hash_;
  string title_;
  string username_;
  object_ptr<ChatPhoto> photo_;
  int32 date_;
  array<object_ptr<restrictionReason>> restriction_reason_;
  object_ptr<chatAdminRights> admin_rights_;
  object_ptr<chatBannedRights> banned_rights_;
  object_ptr<chatBannedRights> default_banned_rights_;
  int32 participants_count_;
  array<object_ptr<username>> usernames_;
  enum Flags : std::int32_t { ACCESS_HASH_MASK = 8192, USERNAME_MASK = 64, PARTICIPANTS_COUNT_MASK = 131072 };

  channel();

  static const std::int32_t ID = -2094689180;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Chat> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelForbidden final : public Chat {
 public:
  int32 flags_;
  bool broadcast_;
  bool megagroup_;
  int64 id_;
  int64 access_hash_;
  string title_;
  int32 until_date_;
  enum Flags : std::int32_t { UNTIL_DATE_MASK = 65536 };

  channelForbidden();

  channelForbidden(int32 flags_, bool broadcast_, bool megagroup_, int64 id_, int64 access_hash_, string const &title_, int32 until_date_);

  static const std::int32_t ID = 399807445;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Chat> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatAdminRights final : public Object {
 public:
  int32 flags_;
  bool change_info_;
  bool post_messages_;
  bool edit_messages_;
  bool delete_messages_;
  bool ban_users_;
  bool invite_users_;
  bool pin_messages_;
  bool add_admins_;
  bool anonymous_;
  bool manage_call_;
  bool other_;
  bool manage_topics_;
  enum Flags : std::int32_t { CHANGE_INFO_MASK = 1, POST_MESSAGES_MASK = 2, EDIT_MESSAGES_MASK = 4, DELETE_MESSAGES_MASK = 8, BAN_USERS_MASK = 16, INVITE_USERS_MASK = 32, PIN_MESSAGES_MASK = 128, ADD_ADMINS_MASK = 512, ANONYMOUS_MASK = 1024, MANAGE_CALL_MASK = 2048, OTHER_MASK = 4096, MANAGE_TOPICS_MASK = 8192 };

  chatAdminRights();

  chatAdminRights(int32 flags_, bool change_info_, bool post_messages_, bool edit_messages_, bool delete_messages_, bool ban_users_, bool invite_users_, bool pin_messages_, bool add_admins_, bool anonymous_, bool manage_call_, bool other_, bool manage_topics_);

  static const std::int32_t ID = 1605510357;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<chatAdminRights> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatAdminWithInvites final : public Object {
 public:
  int64 admin_id_;
  int32 invites_count_;
  int32 revoked_invites_count_;

  static const std::int32_t ID = -219353309;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<chatAdminWithInvites> fetch(TlBufferParser &p);

  explicit chatAdminWithInvites(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatBannedRights final : public Object {
 public:
  int32 flags_;
  bool view_messages_;
  bool send_messages_;
  bool send_media_;
  bool send_stickers_;
  bool send_gifs_;
  bool send_games_;
  bool send_inline_;
  bool embed_links_;
  bool send_polls_;
  bool change_info_;
  bool invite_users_;
  bool pin_messages_;
  bool manage_topics_;
  bool send_photos_;
  bool send_videos_;
  bool send_roundvideos_;
  bool send_audios_;
  bool send_voices_;
  bool send_docs_;
  bool send_plain_;
  int32 until_date_;
  enum Flags : std::int32_t { VIEW_MESSAGES_MASK = 1, SEND_MESSAGES_MASK = 2, SEND_MEDIA_MASK = 4, SEND_STICKERS_MASK = 8, SEND_GIFS_MASK = 16, SEND_GAMES_MASK = 32, SEND_INLINE_MASK = 64, EMBED_LINKS_MASK = 128, SEND_POLLS_MASK = 256, CHANGE_INFO_MASK = 1024, INVITE_USERS_MASK = 32768, PIN_MESSAGES_MASK = 131072, MANAGE_TOPICS_MASK = 262144, SEND_PHOTOS_MASK = 524288, SEND_VIDEOS_MASK = 1048576, SEND_ROUNDVIDEOS_MASK = 2097152, SEND_AUDIOS_MASK = 4194304, SEND_VOICES_MASK = 8388608, SEND_DOCS_MASK = 16777216, SEND_PLAIN_MASK = 33554432 };

  chatBannedRights();

  chatBannedRights(int32 flags_, bool view_messages_, bool send_messages_, bool send_media_, bool send_stickers_, bool send_gifs_, bool send_games_, bool send_inline_, bool embed_links_, bool send_polls_, bool change_info_, bool invite_users_, bool pin_messages_, bool manage_topics_, bool send_photos_, bool send_videos_, bool send_roundvideos_, bool send_audios_, bool send_voices_, bool send_docs_, bool send_plain_, int32 until_date_);

  static const std::int32_t ID = -1626209256;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<chatBannedRights> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChatFull: public Object {
 public:

  static object_ptr<ChatFull> fetch(TlBufferParser &p);
};

class chatFull final : public ChatFull {
 public:
  int32 flags_;
  bool can_set_username_;
  bool has_scheduled_;
  bool translations_disabled_;
  int64 id_;
  string about_;
  object_ptr<ChatParticipants> participants_;
  object_ptr<Photo> chat_photo_;
  object_ptr<peerNotifySettings> notify_settings_;
  object_ptr<ExportedChatInvite> exported_invite_;
  array<object_ptr<botInfo>> bot_info_;
  int32 pinned_msg_id_;
  int32 folder_id_;
  object_ptr<inputGroupCall> call_;
  int32 ttl_period_;
  object_ptr<Peer> groupcall_default_join_as_;
  string theme_emoticon_;
  int32 requests_pending_;
  array<int64> recent_requesters_;
  object_ptr<ChatReactions> available_reactions_;
  enum Flags : std::int32_t { PINNED_MSG_ID_MASK = 64, FOLDER_ID_MASK = 2048, TTL_PERIOD_MASK = 16384, THEME_EMOTICON_MASK = 65536, REQUESTS_PENDING_MASK = 131072 };

  chatFull();

  static const std::int32_t ID = -908914376;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChatFull> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channelFull final : public ChatFull {
 public:
  int32 flags_;
  bool can_view_participants_;
  bool can_set_username_;
  bool can_set_stickers_;
  bool hidden_prehistory_;
  bool can_set_location_;
  bool has_scheduled_;
  bool can_view_stats_;
  bool blocked_;
  int32 flags2_;
  bool can_delete_channel_;
  bool antispam_;
  bool participants_hidden_;
  bool translations_disabled_;
  int64 id_;
  string about_;
  int32 participants_count_;
  int32 admins_count_;
  int32 kicked_count_;
  int32 banned_count_;
  int32 online_count_;
  int32 read_inbox_max_id_;
  int32 read_outbox_max_id_;
  int32 unread_count_;
  object_ptr<Photo> chat_photo_;
  object_ptr<peerNotifySettings> notify_settings_;
  object_ptr<ExportedChatInvite> exported_invite_;
  array<object_ptr<botInfo>> bot_info_;
  int64 migrated_from_chat_id_;
  int32 migrated_from_max_id_;
  int32 pinned_msg_id_;
  object_ptr<stickerSet> stickerset_;
  int32 available_min_id_;
  int32 folder_id_;
  int64 linked_chat_id_;
  object_ptr<ChannelLocation> location_;
  int32 slowmode_seconds_;
  int32 slowmode_next_send_date_;
  int32 stats_dc_;
  int32 pts_;
  object_ptr<inputGroupCall> call_;
  int32 ttl_period_;
  array<string> pending_suggestions_;
  object_ptr<Peer> groupcall_default_join_as_;
  string theme_emoticon_;
  int32 requests_pending_;
  array<int64> recent_requesters_;
  object_ptr<Peer> default_send_as_;
  object_ptr<ChatReactions> available_reactions_;
  enum Flags : std::int32_t { PARTICIPANTS_COUNT_MASK = 1, ADMINS_COUNT_MASK = 2, KICKED_COUNT_MASK = 4, BANNED_COUNT_MASK = 4, ONLINE_COUNT_MASK = 8192, MIGRATED_FROM_CHAT_ID_MASK = 16, MIGRATED_FROM_MAX_ID_MASK = 16, PINNED_MSG_ID_MASK = 32, AVAILABLE_MIN_ID_MASK = 512, FOLDER_ID_MASK = 2048, LINKED_CHAT_ID_MASK = 16384, SLOWMODE_SECONDS_MASK = 131072, SLOWMODE_NEXT_SEND_DATE_MASK = 262144, STATS_DC_MASK = 4096, TTL_PERIOD_MASK = 16777216, THEME_EMOTICON_MASK = 134217728, REQUESTS_PENDING_MASK = 268435456 };

  channelFull();

  static const std::int32_t ID = -231385849;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChatFull> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChatInvite: public Object {
 public:

  static object_ptr<ChatInvite> fetch(TlBufferParser &p);
};

class chatInviteAlready final : public ChatInvite {
 public:
  object_ptr<Chat> chat_;

  static const std::int32_t ID = 1516793212;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChatInvite> fetch(TlBufferParser &p);

  explicit chatInviteAlready(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatInvite final : public ChatInvite {
 public:
  int32 flags_;
  bool channel_;
  bool broadcast_;
  bool public_;
  bool megagroup_;
  bool request_needed_;
  string title_;
  string about_;
  object_ptr<Photo> photo_;
  int32 participants_count_;
  array<object_ptr<User>> participants_;
  enum Flags : std::int32_t { ABOUT_MASK = 32 };

  chatInvite();

  static const std::int32_t ID = 806110401;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChatInvite> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatInvitePeek final : public ChatInvite {
 public:
  object_ptr<Chat> chat_;
  int32 expires_;

  static const std::int32_t ID = 1634294960;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChatInvite> fetch(TlBufferParser &p);

  explicit chatInvitePeek(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatInviteImporter final : public Object {
 public:
  int32 flags_;
  bool requested_;
  bool via_chatlist_;
  int64 user_id_;
  int32 date_;
  string about_;
  int64 approved_by_;
  enum Flags : std::int32_t { ABOUT_MASK = 4, APPROVED_BY_MASK = 2 };

  chatInviteImporter();

  static const std::int32_t ID = -1940201511;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<chatInviteImporter> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatOnlines final : public Object {
 public:
  int32 onlines_;

  static const std::int32_t ID = -264117680;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<chatOnlines> fetch(TlBufferParser &p);

  explicit chatOnlines(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChatParticipant: public Object {
 public:

  static object_ptr<ChatParticipant> fetch(TlBufferParser &p);
};

class chatParticipant final : public ChatParticipant {
 public:
  int64 user_id_;
  int64 inviter_id_;
  int32 date_;

  static const std::int32_t ID = -1070776313;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChatParticipant> fetch(TlBufferParser &p);

  explicit chatParticipant(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatParticipantCreator final : public ChatParticipant {
 public:
  int64 user_id_;

  static const std::int32_t ID = -462696732;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChatParticipant> fetch(TlBufferParser &p);

  explicit chatParticipantCreator(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatParticipantAdmin final : public ChatParticipant {
 public:
  int64 user_id_;
  int64 inviter_id_;
  int32 date_;

  static const std::int32_t ID = -1600962725;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChatParticipant> fetch(TlBufferParser &p);

  explicit chatParticipantAdmin(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChatParticipants: public Object {
 public:

  static object_ptr<ChatParticipants> fetch(TlBufferParser &p);
};

class chatParticipantsForbidden final : public ChatParticipants {
 public:
  int32 flags_;
  int64 chat_id_;
  object_ptr<ChatParticipant> self_participant_;

  chatParticipantsForbidden();

  static const std::int32_t ID = -2023500831;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChatParticipants> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatParticipants final : public ChatParticipants {
 public:
  int64 chat_id_;
  array<object_ptr<ChatParticipant>> participants_;
  int32 version_;

  static const std::int32_t ID = 1018991608;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChatParticipants> fetch(TlBufferParser &p);

  explicit chatParticipants(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChatPhoto: public Object {
 public:

  static object_ptr<ChatPhoto> fetch(TlBufferParser &p);
};

class chatPhotoEmpty final : public ChatPhoto {
 public:

  static const std::int32_t ID = 935395612;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChatPhoto> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatPhoto final : public ChatPhoto {
 public:
  int32 flags_;
  bool has_video_;
  int64 photo_id_;
  bytes stripped_thumb_;
  int32 dc_id_;
  enum Flags : std::int32_t { STRIPPED_THUMB_MASK = 2 };

  chatPhoto();

  static const std::int32_t ID = 476978193;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChatPhoto> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ChatReactions: public Object {
 public:

  static object_ptr<ChatReactions> fetch(TlBufferParser &p);
};

class chatReactionsNone final : public ChatReactions {
 public:

  static const std::int32_t ID = -352570692;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChatReactions> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatReactionsAll final : public ChatReactions {
 public:
  int32 flags_;
  bool allow_custom_;
  enum Flags : std::int32_t { ALLOW_CUSTOM_MASK = 1 };

  chatReactionsAll();

  chatReactionsAll(int32 flags_, bool allow_custom_);

  static const std::int32_t ID = 1385335754;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChatReactions> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatReactionsSome final : public ChatReactions {
 public:
  array<object_ptr<Reaction>> reactions_;

  explicit chatReactionsSome(array<object_ptr<Reaction>> &&reactions_);

  static const std::int32_t ID = 1713193015;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ChatReactions> fetch(TlBufferParser &p);

  explicit chatReactionsSome(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class codeSettings final : public Object {
 public:
  int32 flags_;
  bool allow_flashcall_;
  bool current_number_;
  bool allow_app_hash_;
  bool allow_missed_call_;
  bool allow_firebase_;
  array<bytes> logout_tokens_;
  string token_;
  bool app_sandbox_;
  enum Flags : std::int32_t { ALLOW_FLASHCALL_MASK = 1, CURRENT_NUMBER_MASK = 2, ALLOW_APP_HASH_MASK = 16, ALLOW_MISSED_CALL_MASK = 32, ALLOW_FIREBASE_MASK = 128, LOGOUT_TOKENS_MASK = 64, TOKEN_MASK = 256, APP_SANDBOX_MASK = 256 };

  codeSettings(int32 flags_, bool allow_flashcall_, bool current_number_, bool allow_app_hash_, bool allow_missed_call_, bool allow_firebase_, array<bytes> &&logout_tokens_, string const &token_, bool app_sandbox_);

  static const std::int32_t ID = -1390068360;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class config final : public Object {
 public:
  int32 flags_;
  bool default_p2p_contacts_;
  bool preload_featured_stickers_;
  bool revoke_pm_inbox_;
  bool blocked_mode_;
  bool force_try_ipv6_;
  int32 date_;
  int32 expires_;
  bool test_mode_;
  int32 this_dc_;
  array<object_ptr<dcOption>> dc_options_;
  string dc_txt_domain_name_;
  int32 chat_size_max_;
  int32 megagroup_size_max_;
  int32 forwarded_count_max_;
  int32 online_update_period_ms_;
  int32 offline_blur_timeout_ms_;
  int32 offline_idle_timeout_ms_;
  int32 online_cloud_timeout_ms_;
  int32 notify_cloud_delay_ms_;
  int32 notify_default_delay_ms_;
  int32 push_chat_period_ms_;
  int32 push_chat_limit_;
  int32 edit_time_limit_;
  int32 revoke_time_limit_;
  int32 revoke_pm_time_limit_;
  int32 rating_e_decay_;
  int32 stickers_recent_limit_;
  int32 channels_read_media_period_;
  int32 tmp_sessions_;
  int32 call_receive_timeout_ms_;
  int32 call_ring_timeout_ms_;
  int32 call_connect_timeout_ms_;
  int32 call_packet_timeout_ms_;
  string me_url_prefix_;
  string autoupdate_url_prefix_;
  string gif_search_username_;
  string venue_search_username_;
  string img_search_username_;
  string static_maps_provider_;
  int32 caption_length_max_;
  int32 message_length_max_;
  int32 webfile_dc_id_;
  string suggested_lang_code_;
  int32 lang_pack_version_;
  int32 base_lang_pack_version_;
  object_ptr<Reaction> reactions_default_;
  string autologin_token_;
  enum Flags : std::int32_t { TMP_SESSIONS_MASK = 1, AUTOUPDATE_URL_PREFIX_MASK = 128, GIF_SEARCH_USERNAME_MASK = 512, VENUE_SEARCH_USERNAME_MASK = 1024, IMG_SEARCH_USERNAME_MASK = 2048, STATIC_MAPS_PROVIDER_MASK = 4096, SUGGESTED_LANG_CODE_MASK = 4, LANG_PACK_VERSION_MASK = 4, BASE_LANG_PACK_VERSION_MASK = 4, AUTOLOGIN_TOKEN_MASK = 65536 };

  config();

  static const std::int32_t ID = -870702050;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<config> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class contact final : public Object {
 public:
  int64 user_id_;
  bool mutual_;

  static const std::int32_t ID = 341499403;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<contact> fetch(TlBufferParser &p);

  explicit contact(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class contactStatus final : public Object {
 public:
  int64 user_id_;
  object_ptr<UserStatus> status_;

  static const std::int32_t ID = 383348795;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<contactStatus> fetch(TlBufferParser &p);

  explicit contactStatus(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class dataJSON final : public Object {
 public:
  string data_;

  explicit dataJSON(string const &data_);

  static const std::int32_t ID = 2104790276;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<dataJSON> fetch(TlBufferParser &p);

  explicit dataJSON(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class dcOption final : public Object {
 public:
  int32 flags_;
  bool ipv6_;
  bool media_only_;
  bool tcpo_only_;
  bool cdn_;
  bool static_;
  bool this_port_only_;
  int32 id_;
  string ip_address_;
  int32 port_;
  bytes secret_;
  enum Flags : std::int32_t { SECRET_MASK = 1024 };

  dcOption();

  static const std::int32_t ID = 414687501;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<dcOption> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class defaultHistoryTTL final : public Object {
 public:
  int32 period_;

  static const std::int32_t ID = 1135897376;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<defaultHistoryTTL> fetch(TlBufferParser &p);

  explicit defaultHistoryTTL(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class Dialog: public Object {
 public:

  static object_ptr<Dialog> fetch(TlBufferParser &p);
};

class dialog final : public Dialog {
 public:
  int32 flags_;
  bool pinned_;
  bool unread_mark_;
  object_ptr<Peer> peer_;
  int32 top_message_;
  int32 read_inbox_max_id_;
  int32 read_outbox_max_id_;
  int32 unread_count_;
  int32 unread_mentions_count_;
  int32 unread_reactions_count_;
  object_ptr<peerNotifySettings> notify_settings_;
  int32 pts_;
  object_ptr<DraftMessage> draft_;
  int32 folder_id_;
  int32 ttl_period_;
  enum Flags : std::int32_t { PTS_MASK = 1, FOLDER_ID_MASK = 16, TTL_PERIOD_MASK = 32 };

  dialog();

  static const std::int32_t ID = -712374074;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Dialog> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class dialogFolder final : public Dialog {
 public:
  int32 flags_;
  bool pinned_;
  object_ptr<folder> folder_;
  object_ptr<Peer> peer_;
  int32 top_message_;
  int32 unread_muted_peers_count_;
  int32 unread_unmuted_peers_count_;
  int32 unread_muted_messages_count_;
  int32 unread_unmuted_messages_count_;

  dialogFolder();

  static const std::int32_t ID = 1908216652;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Dialog> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class DialogFilter: public Object {
 public:

  static object_ptr<DialogFilter> fetch(TlBufferParser &p);
};

class dialogFilter final : public DialogFilter {
 public:
  int32 flags_;
  bool contacts_;
  bool non_contacts_;
  bool groups_;
  bool broadcasts_;
  bool bots_;
  bool exclude_muted_;
  bool exclude_read_;
  bool exclude_archived_;
  int32 id_;
  string title_;
  string emoticon_;
  array<object_ptr<InputPeer>> pinned_peers_;
  array<object_ptr<InputPeer>> include_peers_;
  array<object_ptr<InputPeer>> exclude_peers_;
  enum Flags : std::int32_t { CONTACTS_MASK = 1, NON_CONTACTS_MASK = 2, GROUPS_MASK = 4, BROADCASTS_MASK = 8, BOTS_MASK = 16, EXCLUDE_MUTED_MASK = 2048, EXCLUDE_READ_MASK = 4096, EXCLUDE_ARCHIVED_MASK = 8192, EMOTICON_MASK = 33554432 };

  dialogFilter();

  dialogFilter(int32 flags_, bool contacts_, bool non_contacts_, bool groups_, bool broadcasts_, bool bots_, bool exclude_muted_, bool exclude_read_, bool exclude_archived_, int32 id_, string const &title_, string const &emoticon_, array<object_ptr<InputPeer>> &&pinned_peers_, array<object_ptr<InputPeer>> &&include_peers_, array<object_ptr<InputPeer>> &&exclude_peers_);

  static const std::int32_t ID = 1949890536;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<DialogFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class dialogFilterDefault final : public DialogFilter {
 public:

  static const std::int32_t ID = 909284270;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<DialogFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class dialogFilterChatlist final : public DialogFilter {
 public:
  int32 flags_;
  bool has_my_invites_;
  int32 id_;
  string title_;
  string emoticon_;
  array<object_ptr<InputPeer>> pinned_peers_;
  array<object_ptr<InputPeer>> include_peers_;
  enum Flags : std::int32_t { HAS_MY_INVITES_MASK = 67108864, EMOTICON_MASK = 33554432 };

  dialogFilterChatlist();

  dialogFilterChatlist(int32 flags_, bool has_my_invites_, int32 id_, string const &title_, string const &emoticon_, array<object_ptr<InputPeer>> &&pinned_peers_, array<object_ptr<InputPeer>> &&include_peers_);

  static const std::int32_t ID = -699792216;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<DialogFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class dialogFilterSuggested final : public Object {
 public:
  object_ptr<DialogFilter> filter_;
  string description_;

  static const std::int32_t ID = 2004110666;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<dialogFilterSuggested> fetch(TlBufferParser &p);

  explicit dialogFilterSuggested(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class DialogPeer: public Object {
 public:

  static object_ptr<DialogPeer> fetch(TlBufferParser &p);
};

class dialogPeer final : public DialogPeer {
 public:
  object_ptr<Peer> peer_;

  static const std::int32_t ID = -445792507;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<DialogPeer> fetch(TlBufferParser &p);

  explicit dialogPeer(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class dialogPeerFolder final : public DialogPeer {
 public:
  int32 folder_id_;

  static const std::int32_t ID = 1363483106;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<DialogPeer> fetch(TlBufferParser &p);

  explicit dialogPeerFolder(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class Document: public Object {
 public:

  static object_ptr<Document> fetch(TlBufferParser &p);
};

class documentEmpty final : public Document {
 public:
  int64 id_;

  static const std::int32_t ID = 922273905;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Document> fetch(TlBufferParser &p);

  explicit documentEmpty(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class document final : public Document {
 public:
  int32 flags_;
  int64 id_;
  int64 access_hash_;
  bytes file_reference_;
  int32 date_;
  string mime_type_;
  int64 size_;
  array<object_ptr<PhotoSize>> thumbs_;
  array<object_ptr<VideoSize>> video_thumbs_;
  int32 dc_id_;
  array<object_ptr<DocumentAttribute>> attributes_;

  document();

  document(int32 flags_, int64 id_, int64 access_hash_, bytes &&file_reference_, int32 date_, string const &mime_type_, int64 size_, array<object_ptr<PhotoSize>> &&thumbs_, array<object_ptr<VideoSize>> &&video_thumbs_, int32 dc_id_, array<object_ptr<DocumentAttribute>> &&attributes_);

  static const std::int32_t ID = -1881881384;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Document> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class DocumentAttribute: public Object {
 public:

  static object_ptr<DocumentAttribute> fetch(TlBufferParser &p);
};

class documentAttributeImageSize final : public DocumentAttribute {
 public:
  int32 w_;
  int32 h_;

  documentAttributeImageSize(int32 w_, int32 h_);

  static const std::int32_t ID = 1815593308;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<DocumentAttribute> fetch(TlBufferParser &p);

  explicit documentAttributeImageSize(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class documentAttributeAnimated final : public DocumentAttribute {
 public:

  static const std::int32_t ID = 297109817;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<DocumentAttribute> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class documentAttributeSticker final : public DocumentAttribute {
 public:
  int32 flags_;
  bool mask_;
  string alt_;
  object_ptr<InputStickerSet> stickerset_;
  object_ptr<maskCoords> mask_coords_;
  enum Flags : std::int32_t { MASK_MASK = 2, MASK_COORDS_MASK = 1 };

  documentAttributeSticker();

  documentAttributeSticker(int32 flags_, bool mask_, string const &alt_, object_ptr<InputStickerSet> &&stickerset_, object_ptr<maskCoords> &&mask_coords_);

  static const std::int32_t ID = 1662637586;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<DocumentAttribute> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class documentAttributeVideo final : public DocumentAttribute {
 public:
  int32 flags_;
  bool round_message_;
  bool supports_streaming_;
  int32 duration_;
  int32 w_;
  int32 h_;
  enum Flags : std::int32_t { ROUND_MESSAGE_MASK = 1, SUPPORTS_STREAMING_MASK = 2 };

  documentAttributeVideo();

  documentAttributeVideo(int32 flags_, bool round_message_, bool supports_streaming_, int32 duration_, int32 w_, int32 h_);

  static const std::int32_t ID = 250621158;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<DocumentAttribute> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class documentAttributeAudio final : public DocumentAttribute {
 public:
  int32 flags_;
  bool voice_;
  int32 duration_;
  string title_;
  string performer_;
  bytes waveform_;
  enum Flags : std::int32_t { VOICE_MASK = 1024, TITLE_MASK = 1, PERFORMER_MASK = 2, WAVEFORM_MASK = 4 };

  documentAttributeAudio();

  documentAttributeAudio(int32 flags_, bool voice_, int32 duration_, string const &title_, string const &performer_, bytes &&waveform_);

  static const std::int32_t ID = -1739392570;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<DocumentAttribute> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class documentAttributeFilename final : public DocumentAttribute {
 public:
  string file_name_;

  explicit documentAttributeFilename(string const &file_name_);

  static const std::int32_t ID = 358154344;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<DocumentAttribute> fetch(TlBufferParser &p);

  explicit documentAttributeFilename(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class documentAttributeHasStickers final : public DocumentAttribute {
 public:

  static const std::int32_t ID = -1744710921;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<DocumentAttribute> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class documentAttributeCustomEmoji final : public DocumentAttribute {
 public:
  int32 flags_;
  bool free_;
  bool text_color_;
  string alt_;
  object_ptr<InputStickerSet> stickerset_;
  enum Flags : std::int32_t { FREE_MASK = 1, TEXT_COLOR_MASK = 2 };

  documentAttributeCustomEmoji();

  documentAttributeCustomEmoji(int32 flags_, bool free_, bool text_color_, string const &alt_, object_ptr<InputStickerSet> &&stickerset_);

  static const std::int32_t ID = -48981863;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<DocumentAttribute> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class DraftMessage: public Object {
 public:

  static object_ptr<DraftMessage> fetch(TlBufferParser &p);
};

class draftMessageEmpty final : public DraftMessage {
 public:
  int32 flags_;
  int32 date_;
  enum Flags : std::int32_t { DATE_MASK = 1 };

  draftMessageEmpty();

  static const std::int32_t ID = 453805082;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<DraftMessage> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class draftMessage final : public DraftMessage {
 public:
  int32 flags_;
  bool no_webpage_;
  int32 reply_to_msg_id_;
  string message_;
  array<object_ptr<MessageEntity>> entities_;
  int32 date_;
  enum Flags : std::int32_t { REPLY_TO_MSG_ID_MASK = 1 };

  draftMessage();

  static const std::int32_t ID = -40996577;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<DraftMessage> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class EmailVerification: public Object {
 public:
};

class emailVerificationCode final : public EmailVerification {
 public:
  string code_;

  explicit emailVerificationCode(string const &code_);

  static const std::int32_t ID = -1842457175;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class emailVerificationGoogle final : public EmailVerification {
 public:
  string token_;

  explicit emailVerificationGoogle(string const &token_);

  static const std::int32_t ID = -611279166;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class emailVerificationApple final : public EmailVerification {
 public:
  string token_;

  explicit emailVerificationApple(string const &token_);

  static const std::int32_t ID = -1764723459;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class EmailVerifyPurpose: public Object {
 public:
};

class emailVerifyPurposeLoginSetup final : public EmailVerifyPurpose {
 public:
  string phone_number_;
  string phone_code_hash_;

  emailVerifyPurposeLoginSetup(string const &phone_number_, string const &phone_code_hash_);

  static const std::int32_t ID = 1128644211;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class emailVerifyPurposeLoginChange final : public EmailVerifyPurpose {
 public:

  static const std::int32_t ID = 1383932651;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class emailVerifyPurposePassport final : public EmailVerifyPurpose {
 public:

  static const std::int32_t ID = -1141565819;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class emojiGroup final : public Object {
 public:
  string title_;
  int64 icon_emoji_id_;
  array<string> emoticons_;

  static const std::int32_t ID = 2056961449;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<emojiGroup> fetch(TlBufferParser &p);

  explicit emojiGroup(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class EmojiKeyword: public Object {
 public:

  static object_ptr<EmojiKeyword> fetch(TlBufferParser &p);
};

class emojiKeyword final : public EmojiKeyword {
 public:
  string keyword_;
  array<string> emoticons_;

  static const std::int32_t ID = -709641735;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<EmojiKeyword> fetch(TlBufferParser &p);

  explicit emojiKeyword(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class emojiKeywordDeleted final : public EmojiKeyword {
 public:
  string keyword_;
  array<string> emoticons_;

  static const std::int32_t ID = 594408994;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<EmojiKeyword> fetch(TlBufferParser &p);

  explicit emojiKeywordDeleted(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class emojiKeywordsDifference final : public Object {
 public:
  string lang_code_;
  int32 from_version_;
  int32 version_;
  array<object_ptr<EmojiKeyword>> keywords_;

  static const std::int32_t ID = 1556570557;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<emojiKeywordsDifference> fetch(TlBufferParser &p);

  explicit emojiKeywordsDifference(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class emojiLanguage final : public Object {
 public:
  string lang_code_;

  static const std::int32_t ID = -1275374751;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<emojiLanguage> fetch(TlBufferParser &p);

  explicit emojiLanguage(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class EmojiList: public Object {
 public:

  static object_ptr<EmojiList> fetch(TlBufferParser &p);
};

class emojiListNotModified final : public EmojiList {
 public:

  static const std::int32_t ID = 1209970170;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<EmojiList> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class emojiList final : public EmojiList {
 public:
  int64 hash_;
  array<int64> document_id_;

  static const std::int32_t ID = 2048790993;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<EmojiList> fetch(TlBufferParser &p);

  explicit emojiList(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class EmojiStatus: public Object {
 public:

  static object_ptr<EmojiStatus> fetch(TlBufferParser &p);
};

class emojiStatusEmpty final : public EmojiStatus {
 public:

  static const std::int32_t ID = 769727150;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<EmojiStatus> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class emojiStatus final : public EmojiStatus {
 public:
  int64 document_id_;

  explicit emojiStatus(int64 document_id_);

  static const std::int32_t ID = -1835310691;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<EmojiStatus> fetch(TlBufferParser &p);

  explicit emojiStatus(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class emojiStatusUntil final : public EmojiStatus {
 public:
  int64 document_id_;
  int32 until_;

  emojiStatusUntil(int64 document_id_, int32 until_);

  static const std::int32_t ID = -97474361;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<EmojiStatus> fetch(TlBufferParser &p);

  explicit emojiStatusUntil(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class emojiURL final : public Object {
 public:
  string url_;

  static const std::int32_t ID = -1519029347;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<emojiURL> fetch(TlBufferParser &p);

  explicit emojiURL(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class EncryptedChat: public Object {
 public:

  static object_ptr<EncryptedChat> fetch(TlBufferParser &p);
};

class encryptedChatEmpty final : public EncryptedChat {
 public:
  int32 id_;

  static const std::int32_t ID = -1417756512;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<EncryptedChat> fetch(TlBufferParser &p);

  explicit encryptedChatEmpty(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class encryptedChatWaiting final : public EncryptedChat {
 public:
  int32 id_;
  int64 access_hash_;
  int32 date_;
  int64 admin_id_;
  int64 participant_id_;

  encryptedChatWaiting(int32 id_, int64 access_hash_, int32 date_, int64 admin_id_, int64 participant_id_);

  static const std::int32_t ID = 1722964307;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<EncryptedChat> fetch(TlBufferParser &p);

  explicit encryptedChatWaiting(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class encryptedChatRequested final : public EncryptedChat {
 public:
  int32 flags_;
  int32 folder_id_;
  int32 id_;
  int64 access_hash_;
  int32 date_;
  int64 admin_id_;
  int64 participant_id_;
  bytes g_a_;
  enum Flags : std::int32_t { FOLDER_ID_MASK = 1 };

  encryptedChatRequested();

  encryptedChatRequested(int32 flags_, int32 folder_id_, int32 id_, int64 access_hash_, int32 date_, int64 admin_id_, int64 participant_id_, bytes &&g_a_);

  static const std::int32_t ID = 1223809356;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<EncryptedChat> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class encryptedChat final : public EncryptedChat {
 public:
  int32 id_;
  int64 access_hash_;
  int32 date_;
  int64 admin_id_;
  int64 participant_id_;
  bytes g_a_or_b_;
  int64 key_fingerprint_;

  encryptedChat(int32 id_, int64 access_hash_, int32 date_, int64 admin_id_, int64 participant_id_, bytes &&g_a_or_b_, int64 key_fingerprint_);

  static const std::int32_t ID = 1643173063;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<EncryptedChat> fetch(TlBufferParser &p);

  explicit encryptedChat(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class encryptedChatDiscarded final : public EncryptedChat {
 public:
  int32 flags_;
  bool history_deleted_;
  int32 id_;

  encryptedChatDiscarded();

  static const std::int32_t ID = 505183301;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<EncryptedChat> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class EncryptedFile: public Object {
 public:

  static object_ptr<EncryptedFile> fetch(TlBufferParser &p);
};

class encryptedFileEmpty final : public EncryptedFile {
 public:

  static const std::int32_t ID = -1038136962;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<EncryptedFile> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class encryptedFile final : public EncryptedFile {
 public:
  int64 id_;
  int64 access_hash_;
  int64 size_;
  int32 dc_id_;
  int32 key_fingerprint_;

  static const std::int32_t ID = -1476358952;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<EncryptedFile> fetch(TlBufferParser &p);

  explicit encryptedFile(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class EncryptedMessage: public Object {
 public:

  static object_ptr<EncryptedMessage> fetch(TlBufferParser &p);
};

class encryptedMessage final : public EncryptedMessage {
 public:
  int64 random_id_;
  int32 chat_id_;
  int32 date_;
  bytes bytes_;
  object_ptr<EncryptedFile> file_;

  static const std::int32_t ID = -317144808;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<EncryptedMessage> fetch(TlBufferParser &p);

  explicit encryptedMessage(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class encryptedMessageService final : public EncryptedMessage {
 public:
  int64 random_id_;
  int32 chat_id_;
  int32 date_;
  bytes bytes_;

  static const std::int32_t ID = 594758406;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<EncryptedMessage> fetch(TlBufferParser &p);

  explicit encryptedMessageService(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class error final : public Object {
 public:
  int32 code_;
  string text_;

  static const std::int32_t ID = -994444869;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<error> fetch(TlBufferParser &p);

  explicit error(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ExportedChatInvite: public Object {
 public:

  static object_ptr<ExportedChatInvite> fetch(TlBufferParser &p);
};

class chatInviteExported final : public ExportedChatInvite {
 public:
  int32 flags_;
  bool revoked_;
  bool permanent_;
  bool request_needed_;
  string link_;
  int64 admin_id_;
  int32 date_;
  int32 start_date_;
  int32 expire_date_;
  int32 usage_limit_;
  int32 usage_;
  int32 requested_;
  string title_;
  enum Flags : std::int32_t { START_DATE_MASK = 16, EXPIRE_DATE_MASK = 2, USAGE_LIMIT_MASK = 4, USAGE_MASK = 8, REQUESTED_MASK = 128, TITLE_MASK = 256 };

  chatInviteExported();

  static const std::int32_t ID = 179611673;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ExportedChatInvite> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatInvitePublicJoinRequests final : public ExportedChatInvite {
 public:

  static const std::int32_t ID = -317687113;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ExportedChatInvite> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class exportedChatlistInvite final : public Object {
 public:
  int32 flags_;
  string title_;
  string url_;
  array<object_ptr<Peer>> peers_;

  exportedChatlistInvite();

  static const std::int32_t ID = 206668204;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<exportedChatlistInvite> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class exportedContactToken final : public Object {
 public:
  string url_;
  int32 expires_;

  static const std::int32_t ID = 1103040667;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<exportedContactToken> fetch(TlBufferParser &p);

  explicit exportedContactToken(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class exportedMessageLink final : public Object {
 public:
  string link_;
  string html_;

  static const std::int32_t ID = 1571494644;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<exportedMessageLink> fetch(TlBufferParser &p);

  explicit exportedMessageLink(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class fileHash final : public Object {
 public:
  int64 offset_;
  int32 limit_;
  bytes hash_;

  static const std::int32_t ID = -207944868;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<fileHash> fetch(TlBufferParser &p);

  explicit fileHash(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class folder final : public Object {
 public:
  int32 flags_;
  bool autofill_new_broadcasts_;
  bool autofill_public_groups_;
  bool autofill_new_correspondents_;
  int32 id_;
  string title_;
  object_ptr<ChatPhoto> photo_;

  folder();

  static const std::int32_t ID = -11252123;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<folder> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class folderPeer final : public Object {
 public:
  object_ptr<Peer> peer_;
  int32 folder_id_;

  static const std::int32_t ID = -373643672;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<folderPeer> fetch(TlBufferParser &p);

  explicit folderPeer(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ForumTopic: public Object {
 public:

  static object_ptr<ForumTopic> fetch(TlBufferParser &p);
};

class forumTopicDeleted final : public ForumTopic {
 public:
  int32 id_;

  static const std::int32_t ID = 37687451;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ForumTopic> fetch(TlBufferParser &p);

  explicit forumTopicDeleted(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class forumTopic final : public ForumTopic {
 public:
  int32 flags_;
  bool my_;
  bool closed_;
  bool pinned_;
  bool short_;
  bool hidden_;
  int32 id_;
  int32 date_;
  string title_;
  int32 icon_color_;
  int64 icon_emoji_id_;
  int32 top_message_;
  int32 read_inbox_max_id_;
  int32 read_outbox_max_id_;
  int32 unread_count_;
  int32 unread_mentions_count_;
  int32 unread_reactions_count_;
  object_ptr<Peer> from_id_;
  object_ptr<peerNotifySettings> notify_settings_;
  object_ptr<DraftMessage> draft_;
  enum Flags : std::int32_t { ICON_EMOJI_ID_MASK = 1 };

  forumTopic();

  static const std::int32_t ID = 1903173033;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ForumTopic> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class game final : public Object {
 public:
  int32 flags_;
  int64 id_;
  int64 access_hash_;
  string short_name_;
  string title_;
  string description_;
  object_ptr<Photo> photo_;
  object_ptr<Document> document_;

  game();

  static const std::int32_t ID = -1107729093;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<game> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class GeoPoint: public Object {
 public:

  static object_ptr<GeoPoint> fetch(TlBufferParser &p);
};

class geoPointEmpty final : public GeoPoint {
 public:

  static const std::int32_t ID = 286776671;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<GeoPoint> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class geoPoint final : public GeoPoint {
 public:
  int32 flags_;
  double long_;
  double lat_;
  int64 access_hash_;
  int32 accuracy_radius_;
  enum Flags : std::int32_t { ACCURACY_RADIUS_MASK = 1 };

  geoPoint();

  static const std::int32_t ID = -1297942941;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<GeoPoint> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class globalPrivacySettings final : public Object {
 public:
  int32 flags_;
  bool archive_and_mute_new_noncontact_peers_;
  enum Flags : std::int32_t { ARCHIVE_AND_MUTE_NEW_NONCONTACT_PEERS_MASK = 1 };

  globalPrivacySettings();

  globalPrivacySettings(int32 flags_, bool archive_and_mute_new_noncontact_peers_);

  static const std::int32_t ID = -1096616924;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<globalPrivacySettings> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class GroupCall: public Object {
 public:

  static object_ptr<GroupCall> fetch(TlBufferParser &p);
};

class groupCallDiscarded final : public GroupCall {
 public:
  int64 id_;
  int64 access_hash_;
  int32 duration_;

  static const std::int32_t ID = 2004925620;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<GroupCall> fetch(TlBufferParser &p);

  explicit groupCallDiscarded(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class groupCall final : public GroupCall {
 public:
  int32 flags_;
  bool join_muted_;
  bool can_change_join_muted_;
  bool join_date_asc_;
  bool schedule_start_subscribed_;
  bool can_start_video_;
  bool record_video_active_;
  bool rtmp_stream_;
  bool listeners_hidden_;
  int64 id_;
  int64 access_hash_;
  int32 participants_count_;
  string title_;
  int32 stream_dc_id_;
  int32 record_start_date_;
  int32 schedule_date_;
  int32 unmuted_video_count_;
  int32 unmuted_video_limit_;
  int32 version_;
  enum Flags : std::int32_t { TITLE_MASK = 8, STREAM_DC_ID_MASK = 16, RECORD_START_DATE_MASK = 32, SCHEDULE_DATE_MASK = 128, UNMUTED_VIDEO_COUNT_MASK = 1024 };

  groupCall();

  static const std::int32_t ID = -711498484;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<GroupCall> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class groupCallParticipant final : public Object {
 public:
  int32 flags_;
  bool muted_;
  bool left_;
  bool can_self_unmute_;
  bool just_joined_;
  bool versioned_;
  bool min_;
  bool muted_by_you_;
  bool volume_by_admin_;
  bool self_;
  bool video_joined_;
  object_ptr<Peer> peer_;
  int32 date_;
  int32 active_date_;
  int32 source_;
  int32 volume_;
  string about_;
  int64 raise_hand_rating_;
  object_ptr<groupCallParticipantVideo> video_;
  object_ptr<groupCallParticipantVideo> presentation_;
  enum Flags : std::int32_t { ACTIVE_DATE_MASK = 8, VOLUME_MASK = 128, ABOUT_MASK = 2048, RAISE_HAND_RATING_MASK = 8192 };

  groupCallParticipant();

  static const std::int32_t ID = -341428482;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<groupCallParticipant> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class groupCallParticipantVideo final : public Object {
 public:
  int32 flags_;
  bool paused_;
  string endpoint_;
  array<object_ptr<groupCallParticipantVideoSourceGroup>> source_groups_;
  int32 audio_source_;
  enum Flags : std::int32_t { AUDIO_SOURCE_MASK = 2 };

  groupCallParticipantVideo();

  static const std::int32_t ID = 1735736008;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<groupCallParticipantVideo> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class groupCallParticipantVideoSourceGroup final : public Object {
 public:
  string semantics_;
  array<int32> sources_;

  static const std::int32_t ID = -592373577;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<groupCallParticipantVideoSourceGroup> fetch(TlBufferParser &p);

  explicit groupCallParticipantVideoSourceGroup(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class groupCallStreamChannel final : public Object {
 public:
  int32 channel_;
  int32 scale_;
  int64 last_timestamp_ms_;

  static const std::int32_t ID = -2132064081;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<groupCallStreamChannel> fetch(TlBufferParser &p);

  explicit groupCallStreamChannel(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class highScore final : public Object {
 public:
  int32 pos_;
  int64 user_id_;
  int32 score_;

  static const std::int32_t ID = 1940093419;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<highScore> fetch(TlBufferParser &p);

  explicit highScore(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class importedContact final : public Object {
 public:
  int64 user_id_;
  int64 client_id_;

  static const std::int32_t ID = -1052885936;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<importedContact> fetch(TlBufferParser &p);

  explicit importedContact(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineBotSwitchPM final : public Object {
 public:
  string text_;
  string start_param_;

  inlineBotSwitchPM(string const &text_, string const &start_param_);

  static const std::int32_t ID = 1008755359;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<inlineBotSwitchPM> fetch(TlBufferParser &p);

  explicit inlineBotSwitchPM(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineBotWebView final : public Object {
 public:
  string text_;
  string url_;

  inlineBotWebView(string const &text_, string const &url_);

  static const std::int32_t ID = -1250781739;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<inlineBotWebView> fetch(TlBufferParser &p);

  explicit inlineBotWebView(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InlineQueryPeerType: public Object {
 public:

  static object_ptr<InlineQueryPeerType> fetch(TlBufferParser &p);
};

class inlineQueryPeerTypeSameBotPM final : public InlineQueryPeerType {
 public:

  static const std::int32_t ID = 813821341;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InlineQueryPeerType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineQueryPeerTypePM final : public InlineQueryPeerType {
 public:

  static const std::int32_t ID = -2093215828;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InlineQueryPeerType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineQueryPeerTypeChat final : public InlineQueryPeerType {
 public:

  static const std::int32_t ID = -681130742;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InlineQueryPeerType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineQueryPeerTypeMegagroup final : public InlineQueryPeerType {
 public:

  static const std::int32_t ID = 1589952067;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InlineQueryPeerType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineQueryPeerTypeBroadcast final : public InlineQueryPeerType {
 public:

  static const std::int32_t ID = 1664413338;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InlineQueryPeerType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inlineQueryPeerTypeBotPM final : public InlineQueryPeerType {
 public:

  static const std::int32_t ID = 238759180;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InlineQueryPeerType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputAppEvent final : public Object {
 public:
  double time_;
  string type_;
  int64 peer_;
  object_ptr<JSONValue> data_;

  inputAppEvent(double time_, string const &type_, int64 peer_, object_ptr<JSONValue> &&data_);

  static const std::int32_t ID = 488313413;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<inputAppEvent> fetch(TlBufferParser &p);

  explicit inputAppEvent(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputBotApp: public Object {
 public:
};

class inputBotAppID final : public InputBotApp {
 public:
  int64 id_;
  int64 access_hash_;

  inputBotAppID(int64 id_, int64 access_hash_);

  static const std::int32_t ID = -1457472134;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputBotAppShortName final : public InputBotApp {
 public:
  object_ptr<InputUser> bot_id_;
  string short_name_;

  inputBotAppShortName(object_ptr<InputUser> &&bot_id_, string const &short_name_);

  static const std::int32_t ID = -1869872121;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputBotInlineMessage: public Object {
 public:
};

class inputBotInlineMessageMediaAuto final : public InputBotInlineMessage {
 public:
  int32 flags_;
  string message_;
  array<object_ptr<MessageEntity>> entities_;
  object_ptr<ReplyMarkup> reply_markup_;
  enum Flags : std::int32_t { ENTITIES_MASK = 2, REPLY_MARKUP_MASK = 4 };

  inputBotInlineMessageMediaAuto(int32 flags_, string const &message_, array<object_ptr<MessageEntity>> &&entities_, object_ptr<ReplyMarkup> &&reply_markup_);

  static const std::int32_t ID = 864077702;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputBotInlineMessageText final : public InputBotInlineMessage {
 public:
  int32 flags_;
  bool no_webpage_;
  string message_;
  array<object_ptr<MessageEntity>> entities_;
  object_ptr<ReplyMarkup> reply_markup_;
  enum Flags : std::int32_t { NO_WEBPAGE_MASK = 1, ENTITIES_MASK = 2, REPLY_MARKUP_MASK = 4 };

  inputBotInlineMessageText(int32 flags_, bool no_webpage_, string const &message_, array<object_ptr<MessageEntity>> &&entities_, object_ptr<ReplyMarkup> &&reply_markup_);

  static const std::int32_t ID = 1036876423;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputBotInlineMessageMediaGeo final : public InputBotInlineMessage {
 public:
  int32 flags_;
  object_ptr<InputGeoPoint> geo_point_;
  int32 heading_;
  int32 period_;
  int32 proximity_notification_radius_;
  object_ptr<ReplyMarkup> reply_markup_;
  enum Flags : std::int32_t { HEADING_MASK = 1, PERIOD_MASK = 2, PROXIMITY_NOTIFICATION_RADIUS_MASK = 8, REPLY_MARKUP_MASK = 4 };

  inputBotInlineMessageMediaGeo(int32 flags_, object_ptr<InputGeoPoint> &&geo_point_, int32 heading_, int32 period_, int32 proximity_notification_radius_, object_ptr<ReplyMarkup> &&reply_markup_);

  static const std::int32_t ID = -1768777083;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputBotInlineMessageMediaVenue final : public InputBotInlineMessage {
 public:
  int32 flags_;
  object_ptr<InputGeoPoint> geo_point_;
  string title_;
  string address_;
  string provider_;
  string venue_id_;
  string venue_type_;
  object_ptr<ReplyMarkup> reply_markup_;
  enum Flags : std::int32_t { REPLY_MARKUP_MASK = 4 };

  inputBotInlineMessageMediaVenue(int32 flags_, object_ptr<InputGeoPoint> &&geo_point_, string const &title_, string const &address_, string const &provider_, string const &venue_id_, string const &venue_type_, object_ptr<ReplyMarkup> &&reply_markup_);

  static const std::int32_t ID = 1098628881;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputBotInlineMessageMediaContact final : public InputBotInlineMessage {
 public:
  int32 flags_;
  string phone_number_;
  string first_name_;
  string last_name_;
  string vcard_;
  object_ptr<ReplyMarkup> reply_markup_;
  enum Flags : std::int32_t { REPLY_MARKUP_MASK = 4 };

  inputBotInlineMessageMediaContact(int32 flags_, string const &phone_number_, string const &first_name_, string const &last_name_, string const &vcard_, object_ptr<ReplyMarkup> &&reply_markup_);

  static const std::int32_t ID = -1494368259;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputBotInlineMessageGame final : public InputBotInlineMessage {
 public:
  int32 flags_;
  object_ptr<ReplyMarkup> reply_markup_;
  enum Flags : std::int32_t { REPLY_MARKUP_MASK = 4 };

  inputBotInlineMessageGame(int32 flags_, object_ptr<ReplyMarkup> &&reply_markup_);

  static const std::int32_t ID = 1262639204;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputBotInlineMessageMediaInvoice final : public InputBotInlineMessage {
 public:
  int32 flags_;
  string title_;
  string description_;
  object_ptr<inputWebDocument> photo_;
  object_ptr<invoice> invoice_;
  bytes payload_;
  string provider_;
  object_ptr<dataJSON> provider_data_;
  object_ptr<ReplyMarkup> reply_markup_;
  enum Flags : std::int32_t { PHOTO_MASK = 1, REPLY_MARKUP_MASK = 4 };

  inputBotInlineMessageMediaInvoice(int32 flags_, string const &title_, string const &description_, object_ptr<inputWebDocument> &&photo_, object_ptr<invoice> &&invoice_, bytes &&payload_, string const &provider_, object_ptr<dataJSON> &&provider_data_, object_ptr<ReplyMarkup> &&reply_markup_);

  static const std::int32_t ID = -672693723;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputBotInlineMessageID: public Object {
 public:

  static object_ptr<InputBotInlineMessageID> fetch(TlBufferParser &p);
};

class inputBotInlineMessageID final : public InputBotInlineMessageID {
 public:
  int32 dc_id_;
  int64 id_;
  int64 access_hash_;

  inputBotInlineMessageID(int32 dc_id_, int64 id_, int64 access_hash_);

  static const std::int32_t ID = -1995686519;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputBotInlineMessageID> fetch(TlBufferParser &p);

  explicit inputBotInlineMessageID(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputBotInlineMessageID64 final : public InputBotInlineMessageID {
 public:
  int32 dc_id_;
  int64 owner_id_;
  int32 id_;
  int64 access_hash_;

  inputBotInlineMessageID64(int32 dc_id_, int64 owner_id_, int32 id_, int64 access_hash_);

  static const std::int32_t ID = -1227287081;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputBotInlineMessageID> fetch(TlBufferParser &p);

  explicit inputBotInlineMessageID64(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputBotInlineResult: public Object {
 public:
};

class inputBotInlineResult final : public InputBotInlineResult {
 public:
  int32 flags_;
  string id_;
  string type_;
  string title_;
  string description_;
  string url_;
  object_ptr<inputWebDocument> thumb_;
  object_ptr<inputWebDocument> content_;
  object_ptr<InputBotInlineMessage> send_message_;
  enum Flags : std::int32_t { TITLE_MASK = 2, DESCRIPTION_MASK = 4, URL_MASK = 8, THUMB_MASK = 16, CONTENT_MASK = 32 };

  inputBotInlineResult(int32 flags_, string const &id_, string const &type_, string const &title_, string const &description_, string const &url_, object_ptr<inputWebDocument> &&thumb_, object_ptr<inputWebDocument> &&content_, object_ptr<InputBotInlineMessage> &&send_message_);

  static const std::int32_t ID = -2000710887;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputBotInlineResultPhoto final : public InputBotInlineResult {
 public:
  string id_;
  string type_;
  object_ptr<InputPhoto> photo_;
  object_ptr<InputBotInlineMessage> send_message_;

  inputBotInlineResultPhoto(string const &id_, string const &type_, object_ptr<InputPhoto> &&photo_, object_ptr<InputBotInlineMessage> &&send_message_);

  static const std::int32_t ID = -1462213465;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputBotInlineResultDocument final : public InputBotInlineResult {
 public:
  int32 flags_;
  string id_;
  string type_;
  string title_;
  string description_;
  object_ptr<InputDocument> document_;
  object_ptr<InputBotInlineMessage> send_message_;
  enum Flags : std::int32_t { TITLE_MASK = 2, DESCRIPTION_MASK = 4 };

  inputBotInlineResultDocument(int32 flags_, string const &id_, string const &type_, string const &title_, string const &description_, object_ptr<InputDocument> &&document_, object_ptr<InputBotInlineMessage> &&send_message_);

  static const std::int32_t ID = -459324;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputBotInlineResultGame final : public InputBotInlineResult {
 public:
  string id_;
  string short_name_;
  object_ptr<InputBotInlineMessage> send_message_;

  inputBotInlineResultGame(string const &id_, string const &short_name_, object_ptr<InputBotInlineMessage> &&send_message_);

  static const std::int32_t ID = 1336154098;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputChannel: public Object {
 public:

  static object_ptr<InputChannel> fetch(TlBufferParser &p);
};

class inputChannelEmpty final : public InputChannel {
 public:

  static const std::int32_t ID = -292807034;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputChannel> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputChannel final : public InputChannel {
 public:
  int64 channel_id_;
  int64 access_hash_;

  inputChannel(int64 channel_id_, int64 access_hash_);

  static const std::int32_t ID = -212145112;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputChannel> fetch(TlBufferParser &p);

  explicit inputChannel(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputChannelFromMessage final : public InputChannel {
 public:
  object_ptr<InputPeer> peer_;
  int32 msg_id_;
  int64 channel_id_;

  inputChannelFromMessage(object_ptr<InputPeer> &&peer_, int32 msg_id_, int64 channel_id_);

  static const std::int32_t ID = 1536380829;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputChannel> fetch(TlBufferParser &p);

  explicit inputChannelFromMessage(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputChatPhoto: public Object {
 public:
};

class inputChatPhotoEmpty final : public InputChatPhoto {
 public:

  static const std::int32_t ID = 480546647;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputChatUploadedPhoto final : public InputChatPhoto {
 public:
  int32 flags_;
  object_ptr<InputFile> file_;
  object_ptr<InputFile> video_;
  double video_start_ts_;
  object_ptr<VideoSize> video_emoji_markup_;
  enum Flags : std::int32_t { FILE_MASK = 1, VIDEO_MASK = 2, VIDEO_START_TS_MASK = 4, VIDEO_EMOJI_MARKUP_MASK = 8 };

  inputChatUploadedPhoto(int32 flags_, object_ptr<InputFile> &&file_, object_ptr<InputFile> &&video_, double video_start_ts_, object_ptr<VideoSize> &&video_emoji_markup_);

  static const std::int32_t ID = -1110593856;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputChatPhoto final : public InputChatPhoto {
 public:
  object_ptr<InputPhoto> id_;

  explicit inputChatPhoto(object_ptr<InputPhoto> &&id_);

  static const std::int32_t ID = -1991004873;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputChatlistDialogFilter final : public Object {
 public:
  int32 filter_id_;

  explicit inputChatlistDialogFilter(int32 filter_id_);

  static const std::int32_t ID = -203367885;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputCheckPasswordSRP: public Object {
 public:
};

class inputCheckPasswordEmpty final : public InputCheckPasswordSRP {
 public:

  static const std::int32_t ID = -1736378792;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputCheckPasswordSRP final : public InputCheckPasswordSRP {
 public:
  int64 srp_id_;
  bytes A_;
  bytes M1_;

  inputCheckPasswordSRP(int64 srp_id_, bytes &&A_, bytes &&M1_);

  static const std::int32_t ID = -763367294;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputClientProxy final : public Object {
 public:
  string address_;
  int32 port_;

  static const std::int32_t ID = 1968737087;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPhoneContact final : public Object {
 public:
  int64 client_id_;
  string phone_;
  string first_name_;
  string last_name_;

  inputPhoneContact(int64 client_id_, string const &phone_, string const &first_name_, string const &last_name_);

  static const std::int32_t ID = -208488460;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputDialogPeer: public Object {
 public:
};

class inputDialogPeer final : public InputDialogPeer {
 public:
  object_ptr<InputPeer> peer_;

  explicit inputDialogPeer(object_ptr<InputPeer> &&peer_);

  static const std::int32_t ID = -55902537;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputDialogPeerFolder final : public InputDialogPeer {
 public:
  int32 folder_id_;

  explicit inputDialogPeerFolder(int32 folder_id_);

  static const std::int32_t ID = 1684014375;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputDocument: public Object {
 public:
};

class inputDocumentEmpty final : public InputDocument {
 public:

  static const std::int32_t ID = 1928391342;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputDocument final : public InputDocument {
 public:
  int64 id_;
  int64 access_hash_;
  bytes file_reference_;

  inputDocument(int64 id_, int64 access_hash_, bytes &&file_reference_);

  static const std::int32_t ID = 448771445;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputEncryptedChat final : public Object {
 public:
  int32 chat_id_;
  int64 access_hash_;

  inputEncryptedChat(int32 chat_id_, int64 access_hash_);

  static const std::int32_t ID = -247351839;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputEncryptedFile: public Object {
 public:
};

class inputEncryptedFileEmpty final : public InputEncryptedFile {
 public:

  static const std::int32_t ID = 406307684;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputEncryptedFileUploaded final : public InputEncryptedFile {
 public:
  int64 id_;
  int32 parts_;
  string md5_checksum_;
  int32 key_fingerprint_;

  inputEncryptedFileUploaded(int64 id_, int32 parts_, string const &md5_checksum_, int32 key_fingerprint_);

  static const std::int32_t ID = 1690108678;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputEncryptedFile final : public InputEncryptedFile {
 public:
  int64 id_;
  int64 access_hash_;

  inputEncryptedFile(int64 id_, int64 access_hash_);

  static const std::int32_t ID = 1511503333;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputEncryptedFileBigUploaded final : public InputEncryptedFile {
 public:
  int64 id_;
  int32 parts_;
  int32 key_fingerprint_;

  inputEncryptedFileBigUploaded(int64 id_, int32 parts_, int32 key_fingerprint_);

  static const std::int32_t ID = 767652808;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputFile: public Object {
 public:
};

class inputFile final : public InputFile {
 public:
  int64 id_;
  int32 parts_;
  string name_;
  string md5_checksum_;

  inputFile(int64 id_, int32 parts_, string const &name_, string const &md5_checksum_);

  static const std::int32_t ID = -181407105;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputFileBig final : public InputFile {
 public:
  int64 id_;
  int32 parts_;
  string name_;

  inputFileBig(int64 id_, int32 parts_, string const &name_);

  static const std::int32_t ID = -95482955;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputFileLocation: public Object {
 public:
};

class inputPeerPhotoFileLocationLegacy final : public InputFileLocation {
 public:
  int32 flags_;
  bool big_;
  object_ptr<InputPeer> peer_;
  int64 volume_id_;
  int32 local_id_;
  enum Flags : std::int32_t { BIG_MASK = 1 };

  inputPeerPhotoFileLocationLegacy(int32 flags_, bool big_, object_ptr<InputPeer> &&peer_, int64 volume_id_, int32 local_id_);

  static const std::int32_t ID = 668375447;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputStickerSetThumbLegacy final : public InputFileLocation {
 public:
  object_ptr<InputStickerSet> stickerset_;
  int64 volume_id_;
  int32 local_id_;

  inputStickerSetThumbLegacy(object_ptr<InputStickerSet> &&stickerset_, int64 volume_id_, int32 local_id_);

  static const std::int32_t ID = 230353641;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputFileLocation final : public InputFileLocation {
 public:
  int64 volume_id_;
  int32 local_id_;
  int64 secret_;
  bytes file_reference_;

  inputFileLocation(int64 volume_id_, int32 local_id_, int64 secret_, bytes &&file_reference_);

  static const std::int32_t ID = -539317279;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputEncryptedFileLocation final : public InputFileLocation {
 public:
  int64 id_;
  int64 access_hash_;

  inputEncryptedFileLocation(int64 id_, int64 access_hash_);

  static const std::int32_t ID = -182231723;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputDocumentFileLocation final : public InputFileLocation {
 public:
  int64 id_;
  int64 access_hash_;
  bytes file_reference_;
  string thumb_size_;

  inputDocumentFileLocation(int64 id_, int64 access_hash_, bytes &&file_reference_, string const &thumb_size_);

  static const std::int32_t ID = -1160743548;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputSecureFileLocation final : public InputFileLocation {
 public:
  int64 id_;
  int64 access_hash_;

  inputSecureFileLocation(int64 id_, int64 access_hash_);

  static const std::int32_t ID = -876089816;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputTakeoutFileLocation final : public InputFileLocation {
 public:

  static const std::int32_t ID = 700340377;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPhotoFileLocation final : public InputFileLocation {
 public:
  int64 id_;
  int64 access_hash_;
  bytes file_reference_;
  string thumb_size_;

  inputPhotoFileLocation(int64 id_, int64 access_hash_, bytes &&file_reference_, string const &thumb_size_);

  static const std::int32_t ID = 1075322878;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPhotoLegacyFileLocation final : public InputFileLocation {
 public:
  int64 id_;
  int64 access_hash_;
  bytes file_reference_;
  int64 volume_id_;
  int32 local_id_;
  int64 secret_;

  inputPhotoLegacyFileLocation(int64 id_, int64 access_hash_, bytes &&file_reference_, int64 volume_id_, int32 local_id_, int64 secret_);

  static const std::int32_t ID = -667654413;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPeerPhotoFileLocation final : public InputFileLocation {
 public:
  int32 flags_;
  bool big_;
  object_ptr<InputPeer> peer_;
  int64 photo_id_;
  enum Flags : std::int32_t { BIG_MASK = 1 };

  inputPeerPhotoFileLocation(int32 flags_, bool big_, object_ptr<InputPeer> &&peer_, int64 photo_id_);

  static const std::int32_t ID = 925204121;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputStickerSetThumb final : public InputFileLocation {
 public:
  object_ptr<InputStickerSet> stickerset_;
  int32 thumb_version_;

  inputStickerSetThumb(object_ptr<InputStickerSet> &&stickerset_, int32 thumb_version_);

  static const std::int32_t ID = -1652231205;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputGroupCallStream final : public InputFileLocation {
 public:
  int32 flags_;
  object_ptr<inputGroupCall> call_;
  int64 time_ms_;
  int32 scale_;
  int32 video_channel_;
  int32 video_quality_;
  enum Flags : std::int32_t { VIDEO_CHANNEL_MASK = 1, VIDEO_QUALITY_MASK = 1 };

  inputGroupCallStream(int32 flags_, object_ptr<inputGroupCall> &&call_, int64 time_ms_, int32 scale_, int32 video_channel_, int32 video_quality_);

  static const std::int32_t ID = 93890858;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputFolderPeer final : public Object {
 public:
  object_ptr<InputPeer> peer_;
  int32 folder_id_;

  inputFolderPeer(object_ptr<InputPeer> &&peer_, int32 folder_id_);

  static const std::int32_t ID = -70073706;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputGame: public Object {
 public:
};

class inputGameID final : public InputGame {
 public:
  int64 id_;
  int64 access_hash_;

  inputGameID(int64 id_, int64 access_hash_);

  static const std::int32_t ID = 53231223;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputGameShortName final : public InputGame {
 public:
  object_ptr<InputUser> bot_id_;
  string short_name_;

  inputGameShortName(object_ptr<InputUser> &&bot_id_, string const &short_name_);

  static const std::int32_t ID = -1020139510;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputGeoPoint: public Object {
 public:
};

class inputGeoPointEmpty final : public InputGeoPoint {
 public:

  static const std::int32_t ID = -457104426;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputGeoPoint final : public InputGeoPoint {
 public:
  int32 flags_;
  double lat_;
  double long_;
  int32 accuracy_radius_;
  enum Flags : std::int32_t { ACCURACY_RADIUS_MASK = 1 };

  inputGeoPoint(int32 flags_, double lat_, double long_, int32 accuracy_radius_);

  static const std::int32_t ID = 1210199983;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputGroupCall final : public Object {
 public:
  int64 id_;
  int64 access_hash_;

  inputGroupCall(int64 id_, int64 access_hash_);

  static const std::int32_t ID = -659913713;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<inputGroupCall> fetch(TlBufferParser &p);

  explicit inputGroupCall(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputInvoice: public Object {
 public:
};

class inputInvoiceMessage final : public InputInvoice {
 public:
  object_ptr<InputPeer> peer_;
  int32 msg_id_;

  inputInvoiceMessage(object_ptr<InputPeer> &&peer_, int32 msg_id_);

  static const std::int32_t ID = -977967015;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputInvoiceSlug final : public InputInvoice {
 public:
  string slug_;

  explicit inputInvoiceSlug(string const &slug_);

  static const std::int32_t ID = -1020867857;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputMedia: public Object {
 public:
};

class inputMediaEmpty final : public InputMedia {
 public:

  static const std::int32_t ID = -1771768449;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMediaUploadedPhoto final : public InputMedia {
 public:
  int32 flags_;
  bool spoiler_;
  object_ptr<InputFile> file_;
  array<object_ptr<InputDocument>> stickers_;
  int32 ttl_seconds_;
  enum Flags : std::int32_t { SPOILER_MASK = 4, STICKERS_MASK = 1, TTL_SECONDS_MASK = 2 };

  inputMediaUploadedPhoto(int32 flags_, bool spoiler_, object_ptr<InputFile> &&file_, array<object_ptr<InputDocument>> &&stickers_, int32 ttl_seconds_);

  static const std::int32_t ID = 505969924;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMediaPhoto final : public InputMedia {
 public:
  int32 flags_;
  bool spoiler_;
  object_ptr<InputPhoto> id_;
  int32 ttl_seconds_;
  enum Flags : std::int32_t { SPOILER_MASK = 2, TTL_SECONDS_MASK = 1 };

  inputMediaPhoto(int32 flags_, bool spoiler_, object_ptr<InputPhoto> &&id_, int32 ttl_seconds_);

  static const std::int32_t ID = -1279654347;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMediaGeoPoint final : public InputMedia {
 public:
  object_ptr<InputGeoPoint> geo_point_;

  explicit inputMediaGeoPoint(object_ptr<InputGeoPoint> &&geo_point_);

  static const std::int32_t ID = -104578748;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMediaContact final : public InputMedia {
 public:
  string phone_number_;
  string first_name_;
  string last_name_;
  string vcard_;

  inputMediaContact(string const &phone_number_, string const &first_name_, string const &last_name_, string const &vcard_);

  static const std::int32_t ID = -122978821;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMediaUploadedDocument final : public InputMedia {
 public:
  int32 flags_;
  bool nosound_video_;
  bool force_file_;
  bool spoiler_;
  object_ptr<InputFile> file_;
  object_ptr<InputFile> thumb_;
  string mime_type_;
  array<object_ptr<DocumentAttribute>> attributes_;
  array<object_ptr<InputDocument>> stickers_;
  int32 ttl_seconds_;
  enum Flags : std::int32_t { NOSOUND_VIDEO_MASK = 8, FORCE_FILE_MASK = 16, SPOILER_MASK = 32, THUMB_MASK = 4, STICKERS_MASK = 1, TTL_SECONDS_MASK = 2 };

  inputMediaUploadedDocument(int32 flags_, bool nosound_video_, bool force_file_, bool spoiler_, object_ptr<InputFile> &&file_, object_ptr<InputFile> &&thumb_, string const &mime_type_, array<object_ptr<DocumentAttribute>> &&attributes_, array<object_ptr<InputDocument>> &&stickers_, int32 ttl_seconds_);

  static const std::int32_t ID = 1530447553;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMediaDocument final : public InputMedia {
 public:
  int32 flags_;
  bool spoiler_;
  object_ptr<InputDocument> id_;
  int32 ttl_seconds_;
  string query_;
  enum Flags : std::int32_t { SPOILER_MASK = 4, TTL_SECONDS_MASK = 1, QUERY_MASK = 2 };

  inputMediaDocument(int32 flags_, bool spoiler_, object_ptr<InputDocument> &&id_, int32 ttl_seconds_, string const &query_);

  static const std::int32_t ID = 860303448;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMediaVenue final : public InputMedia {
 public:
  object_ptr<InputGeoPoint> geo_point_;
  string title_;
  string address_;
  string provider_;
  string venue_id_;
  string venue_type_;

  inputMediaVenue(object_ptr<InputGeoPoint> &&geo_point_, string const &title_, string const &address_, string const &provider_, string const &venue_id_, string const &venue_type_);

  static const std::int32_t ID = -1052959727;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMediaPhotoExternal final : public InputMedia {
 public:
  int32 flags_;
  bool spoiler_;
  string url_;
  int32 ttl_seconds_;
  enum Flags : std::int32_t { SPOILER_MASK = 2, TTL_SECONDS_MASK = 1 };

  inputMediaPhotoExternal(int32 flags_, bool spoiler_, string const &url_, int32 ttl_seconds_);

  static const std::int32_t ID = -440664550;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMediaDocumentExternal final : public InputMedia {
 public:
  int32 flags_;
  bool spoiler_;
  string url_;
  int32 ttl_seconds_;
  enum Flags : std::int32_t { SPOILER_MASK = 2, TTL_SECONDS_MASK = 1 };

  inputMediaDocumentExternal(int32 flags_, bool spoiler_, string const &url_, int32 ttl_seconds_);

  static const std::int32_t ID = -78455655;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMediaGame final : public InputMedia {
 public:
  object_ptr<InputGame> id_;

  explicit inputMediaGame(object_ptr<InputGame> &&id_);

  static const std::int32_t ID = -750828557;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMediaInvoice final : public InputMedia {
 public:
  int32 flags_;
  string title_;
  string description_;
  object_ptr<inputWebDocument> photo_;
  object_ptr<invoice> invoice_;
  bytes payload_;
  string provider_;
  object_ptr<dataJSON> provider_data_;
  string start_param_;
  object_ptr<InputMedia> extended_media_;
  enum Flags : std::int32_t { PHOTO_MASK = 1, START_PARAM_MASK = 2, EXTENDED_MEDIA_MASK = 4 };

  inputMediaInvoice(int32 flags_, string const &title_, string const &description_, object_ptr<inputWebDocument> &&photo_, object_ptr<invoice> &&invoice_, bytes &&payload_, string const &provider_, object_ptr<dataJSON> &&provider_data_, string const &start_param_, object_ptr<InputMedia> &&extended_media_);

  static const std::int32_t ID = -1900697899;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMediaGeoLive final : public InputMedia {
 public:
  int32 flags_;
  bool stopped_;
  object_ptr<InputGeoPoint> geo_point_;
  int32 heading_;
  int32 period_;
  int32 proximity_notification_radius_;
  enum Flags : std::int32_t { STOPPED_MASK = 1, HEADING_MASK = 4, PERIOD_MASK = 2, PROXIMITY_NOTIFICATION_RADIUS_MASK = 8 };

  inputMediaGeoLive(int32 flags_, bool stopped_, object_ptr<InputGeoPoint> &&geo_point_, int32 heading_, int32 period_, int32 proximity_notification_radius_);

  static const std::int32_t ID = -1759532989;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMediaPoll final : public InputMedia {
 public:
  int32 flags_;
  object_ptr<poll> poll_;
  array<bytes> correct_answers_;
  string solution_;
  array<object_ptr<MessageEntity>> solution_entities_;
  enum Flags : std::int32_t { CORRECT_ANSWERS_MASK = 1, SOLUTION_MASK = 2, SOLUTION_ENTITIES_MASK = 2 };

  inputMediaPoll(int32 flags_, object_ptr<poll> &&poll_, array<bytes> &&correct_answers_, string const &solution_, array<object_ptr<MessageEntity>> &&solution_entities_);

  static const std::int32_t ID = 261416433;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMediaDice final : public InputMedia {
 public:
  string emoticon_;

  explicit inputMediaDice(string const &emoticon_);

  static const std::int32_t ID = -428884101;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputMessage: public Object {
 public:
};

class inputMessageID final : public InputMessage {
 public:
  int32 id_;

  explicit inputMessageID(int32 id_);

  static const std::int32_t ID = -1502174430;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessageReplyTo final : public InputMessage {
 public:
  int32 id_;

  explicit inputMessageReplyTo(int32 id_);

  static const std::int32_t ID = -1160215659;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessagePinned final : public InputMessage {
 public:

  static const std::int32_t ID = -2037963464;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessageCallbackQuery final : public InputMessage {
 public:
  int32 id_;
  int64 query_id_;

  inputMessageCallbackQuery(int32 id_, int64 query_id_);

  static const std::int32_t ID = -1392895362;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputNotifyPeer: public Object {
 public:
};

class inputNotifyPeer final : public InputNotifyPeer {
 public:
  object_ptr<InputPeer> peer_;

  explicit inputNotifyPeer(object_ptr<InputPeer> &&peer_);

  static const std::int32_t ID = -1195615476;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputNotifyUsers final : public InputNotifyPeer {
 public:

  static const std::int32_t ID = 423314455;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputNotifyChats final : public InputNotifyPeer {
 public:

  static const std::int32_t ID = 1251338318;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputNotifyBroadcasts final : public InputNotifyPeer {
 public:

  static const std::int32_t ID = -1311015810;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputNotifyForumTopic final : public InputNotifyPeer {
 public:
  object_ptr<InputPeer> peer_;
  int32 top_msg_id_;

  inputNotifyForumTopic(object_ptr<InputPeer> &&peer_, int32 top_msg_id_);

  static const std::int32_t ID = 1548122514;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputPaymentCredentials: public Object {
 public:
};

class inputPaymentCredentialsSaved final : public InputPaymentCredentials {
 public:
  string id_;
  bytes tmp_password_;

  inputPaymentCredentialsSaved(string const &id_, bytes &&tmp_password_);

  static const std::int32_t ID = -1056001329;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPaymentCredentials final : public InputPaymentCredentials {
 public:
  int32 flags_;
  bool save_;
  object_ptr<dataJSON> data_;
  enum Flags : std::int32_t { SAVE_MASK = 1 };

  inputPaymentCredentials(int32 flags_, bool save_, object_ptr<dataJSON> &&data_);

  static const std::int32_t ID = 873977640;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPaymentCredentialsApplePay final : public InputPaymentCredentials {
 public:
  object_ptr<dataJSON> payment_data_;

  explicit inputPaymentCredentialsApplePay(object_ptr<dataJSON> &&payment_data_);

  static const std::int32_t ID = 178373535;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPaymentCredentialsGooglePay final : public InputPaymentCredentials {
 public:
  object_ptr<dataJSON> payment_token_;

  explicit inputPaymentCredentialsGooglePay(object_ptr<dataJSON> &&payment_token_);

  static const std::int32_t ID = -1966921727;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputPeer: public Object {
 public:

  static object_ptr<InputPeer> fetch(TlBufferParser &p);
};

class inputPeerEmpty final : public InputPeer {
 public:

  static const std::int32_t ID = 2134579434;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputPeer> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPeerSelf final : public InputPeer {
 public:

  static const std::int32_t ID = 2107670217;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputPeer> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPeerChat final : public InputPeer {
 public:
  int64 chat_id_;

  explicit inputPeerChat(int64 chat_id_);

  static const std::int32_t ID = 900291769;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputPeer> fetch(TlBufferParser &p);

  explicit inputPeerChat(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPeerUser final : public InputPeer {
 public:
  int64 user_id_;
  int64 access_hash_;

  inputPeerUser(int64 user_id_, int64 access_hash_);

  static const std::int32_t ID = -571955892;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputPeer> fetch(TlBufferParser &p);

  explicit inputPeerUser(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPeerChannel final : public InputPeer {
 public:
  int64 channel_id_;
  int64 access_hash_;

  inputPeerChannel(int64 channel_id_, int64 access_hash_);

  static const std::int32_t ID = 666680316;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputPeer> fetch(TlBufferParser &p);

  explicit inputPeerChannel(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPeerUserFromMessage final : public InputPeer {
 public:
  object_ptr<InputPeer> peer_;
  int32 msg_id_;
  int64 user_id_;

  inputPeerUserFromMessage(object_ptr<InputPeer> &&peer_, int32 msg_id_, int64 user_id_);

  static const std::int32_t ID = -1468331492;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputPeer> fetch(TlBufferParser &p);

  explicit inputPeerUserFromMessage(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPeerChannelFromMessage final : public InputPeer {
 public:
  object_ptr<InputPeer> peer_;
  int32 msg_id_;
  int64 channel_id_;

  inputPeerChannelFromMessage(object_ptr<InputPeer> &&peer_, int32 msg_id_, int64 channel_id_);

  static const std::int32_t ID = -1121318848;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputPeer> fetch(TlBufferParser &p);

  explicit inputPeerChannelFromMessage(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPeerNotifySettings final : public Object {
 public:
  int32 flags_;
  bool show_previews_;
  bool silent_;
  int32 mute_until_;
  object_ptr<NotificationSound> sound_;
  enum Flags : std::int32_t { SHOW_PREVIEWS_MASK = 1, SILENT_MASK = 2, MUTE_UNTIL_MASK = 4, SOUND_MASK = 8 };

  inputPeerNotifySettings(int32 flags_, bool show_previews_, bool silent_, int32 mute_until_, object_ptr<NotificationSound> &&sound_);

  static const std::int32_t ID = -551616469;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPhoneCall final : public Object {
 public:
  int64 id_;
  int64 access_hash_;

  inputPhoneCall(int64 id_, int64 access_hash_);

  static const std::int32_t ID = 506920429;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputPhoto: public Object {
 public:
};

class inputPhotoEmpty final : public InputPhoto {
 public:

  static const std::int32_t ID = 483901197;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPhoto final : public InputPhoto {
 public:
  int64 id_;
  int64 access_hash_;
  bytes file_reference_;

  inputPhoto(int64 id_, int64 access_hash_, bytes &&file_reference_);

  static const std::int32_t ID = 1001634122;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputPrivacyKey: public Object {
 public:
};

class inputPrivacyKeyStatusTimestamp final : public InputPrivacyKey {
 public:

  static const std::int32_t ID = 1335282456;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPrivacyKeyChatInvite final : public InputPrivacyKey {
 public:

  static const std::int32_t ID = -1107622874;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPrivacyKeyPhoneCall final : public InputPrivacyKey {
 public:

  static const std::int32_t ID = -88417185;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPrivacyKeyPhoneP2P final : public InputPrivacyKey {
 public:

  static const std::int32_t ID = -610373422;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPrivacyKeyForwards final : public InputPrivacyKey {
 public:

  static const std::int32_t ID = -1529000952;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPrivacyKeyProfilePhoto final : public InputPrivacyKey {
 public:

  static const std::int32_t ID = 1461304012;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPrivacyKeyPhoneNumber final : public InputPrivacyKey {
 public:

  static const std::int32_t ID = 55761658;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPrivacyKeyAddedByPhone final : public InputPrivacyKey {
 public:

  static const std::int32_t ID = -786326563;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPrivacyKeyVoiceMessages final : public InputPrivacyKey {
 public:

  static const std::int32_t ID = -1360618136;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputPrivacyRule: public Object {
 public:
};

class inputPrivacyValueAllowContacts final : public InputPrivacyRule {
 public:

  static const std::int32_t ID = 218751099;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPrivacyValueAllowAll final : public InputPrivacyRule {
 public:

  static const std::int32_t ID = 407582158;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPrivacyValueAllowUsers final : public InputPrivacyRule {
 public:
  array<object_ptr<InputUser>> users_;

  explicit inputPrivacyValueAllowUsers(array<object_ptr<InputUser>> &&users_);

  static const std::int32_t ID = 320652927;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPrivacyValueDisallowContacts final : public InputPrivacyRule {
 public:

  static const std::int32_t ID = 195371015;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPrivacyValueDisallowAll final : public InputPrivacyRule {
 public:

  static const std::int32_t ID = -697604407;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPrivacyValueDisallowUsers final : public InputPrivacyRule {
 public:
  array<object_ptr<InputUser>> users_;

  explicit inputPrivacyValueDisallowUsers(array<object_ptr<InputUser>> &&users_);

  static const std::int32_t ID = -1877932953;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPrivacyValueAllowChatParticipants final : public InputPrivacyRule {
 public:
  array<int64> chats_;

  explicit inputPrivacyValueAllowChatParticipants(array<int64> &&chats_);

  static const std::int32_t ID = -2079962673;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputPrivacyValueDisallowChatParticipants final : public InputPrivacyRule {
 public:
  array<int64> chats_;

  explicit inputPrivacyValueDisallowChatParticipants(array<int64> &&chats_);

  static const std::int32_t ID = -380694650;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputSecureFile: public Object {
 public:
};

class inputSecureFileUploaded final : public InputSecureFile {
 public:
  int64 id_;
  int32 parts_;
  string md5_checksum_;
  bytes file_hash_;
  bytes secret_;

  inputSecureFileUploaded(int64 id_, int32 parts_, string const &md5_checksum_, bytes &&file_hash_, bytes &&secret_);

  static const std::int32_t ID = 859091184;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputSecureFile final : public InputSecureFile {
 public:
  int64 id_;
  int64 access_hash_;

  inputSecureFile(int64 id_, int64 access_hash_);

  static const std::int32_t ID = 1399317950;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputSecureValue final : public Object {
 public:
  int32 flags_;
  object_ptr<SecureValueType> type_;
  object_ptr<secureData> data_;
  object_ptr<InputSecureFile> front_side_;
  object_ptr<InputSecureFile> reverse_side_;
  object_ptr<InputSecureFile> selfie_;
  array<object_ptr<InputSecureFile>> translation_;
  array<object_ptr<InputSecureFile>> files_;
  object_ptr<SecurePlainData> plain_data_;
  enum Flags : std::int32_t { DATA_MASK = 1, FRONT_SIDE_MASK = 2, REVERSE_SIDE_MASK = 4, SELFIE_MASK = 8, TRANSLATION_MASK = 64, FILES_MASK = 16, PLAIN_DATA_MASK = 32 };

  inputSecureValue(int32 flags_, object_ptr<SecureValueType> &&type_, object_ptr<secureData> &&data_, object_ptr<InputSecureFile> &&front_side_, object_ptr<InputSecureFile> &&reverse_side_, object_ptr<InputSecureFile> &&selfie_, array<object_ptr<InputSecureFile>> &&translation_, array<object_ptr<InputSecureFile>> &&files_, object_ptr<SecurePlainData> &&plain_data_);

  static const std::int32_t ID = -618540889;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputSingleMedia final : public Object {
 public:
  int32 flags_;
  object_ptr<InputMedia> media_;
  int64 random_id_;
  string message_;
  array<object_ptr<MessageEntity>> entities_;
  enum Flags : std::int32_t { ENTITIES_MASK = 1 };

  inputSingleMedia(int32 flags_, object_ptr<InputMedia> &&media_, int64 random_id_, string const &message_, array<object_ptr<MessageEntity>> &&entities_);

  static const std::int32_t ID = 482797855;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputStickerSet: public Object {
 public:

  static object_ptr<InputStickerSet> fetch(TlBufferParser &p);
};

class inputStickerSetEmpty final : public InputStickerSet {
 public:

  static const std::int32_t ID = -4838507;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputStickerSet> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputStickerSetID final : public InputStickerSet {
 public:
  int64 id_;
  int64 access_hash_;

  inputStickerSetID(int64 id_, int64 access_hash_);

  static const std::int32_t ID = -1645763991;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputStickerSet> fetch(TlBufferParser &p);

  explicit inputStickerSetID(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputStickerSetShortName final : public InputStickerSet {
 public:
  string short_name_;

  explicit inputStickerSetShortName(string const &short_name_);

  static const std::int32_t ID = -2044933984;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputStickerSet> fetch(TlBufferParser &p);

  explicit inputStickerSetShortName(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputStickerSetAnimatedEmoji final : public InputStickerSet {
 public:

  static const std::int32_t ID = 42402760;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputStickerSet> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputStickerSetDice final : public InputStickerSet {
 public:
  string emoticon_;

  explicit inputStickerSetDice(string const &emoticon_);

  static const std::int32_t ID = -427863538;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputStickerSet> fetch(TlBufferParser &p);

  explicit inputStickerSetDice(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputStickerSetAnimatedEmojiAnimations final : public InputStickerSet {
 public:

  static const std::int32_t ID = 215889721;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputStickerSet> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputStickerSetPremiumGifts final : public InputStickerSet {
 public:

  static const std::int32_t ID = -930399486;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputStickerSet> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputStickerSetEmojiGenericAnimations final : public InputStickerSet {
 public:

  static const std::int32_t ID = 80008398;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputStickerSet> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputStickerSetEmojiDefaultStatuses final : public InputStickerSet {
 public:

  static const std::int32_t ID = 701560302;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputStickerSet> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputStickerSetEmojiDefaultTopicIcons final : public InputStickerSet {
 public:

  static const std::int32_t ID = 1153562857;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputStickerSet> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputStickerSetItem final : public Object {
 public:
  int32 flags_;
  object_ptr<InputDocument> document_;
  string emoji_;
  object_ptr<maskCoords> mask_coords_;
  string keywords_;
  enum Flags : std::int32_t { MASK_COORDS_MASK = 1, KEYWORDS_MASK = 2 };

  inputStickerSetItem(int32 flags_, object_ptr<InputDocument> &&document_, string const &emoji_, object_ptr<maskCoords> &&mask_coords_, string const &keywords_);

  static const std::int32_t ID = 853188252;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputStickeredMedia: public Object {
 public:
};

class inputStickeredMediaPhoto final : public InputStickeredMedia {
 public:
  object_ptr<InputPhoto> id_;

  explicit inputStickeredMediaPhoto(object_ptr<InputPhoto> &&id_);

  static const std::int32_t ID = 1251549527;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputStickeredMediaDocument final : public InputStickeredMedia {
 public:
  object_ptr<InputDocument> id_;

  explicit inputStickeredMediaDocument(object_ptr<InputDocument> &&id_);

  static const std::int32_t ID = 70813275;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputStorePaymentPurpose: public Object {
 public:
};

class inputStorePaymentPremiumSubscription final : public InputStorePaymentPurpose {
 public:
  int32 flags_;
  bool restore_;
  bool upgrade_;
  enum Flags : std::int32_t { RESTORE_MASK = 1, UPGRADE_MASK = 2 };

  inputStorePaymentPremiumSubscription(int32 flags_, bool restore_, bool upgrade_);

  static const std::int32_t ID = -1502273946;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputStorePaymentGiftPremium final : public InputStorePaymentPurpose {
 public:
  object_ptr<InputUser> user_id_;
  string currency_;
  int64 amount_;

  inputStorePaymentGiftPremium(object_ptr<InputUser> &&user_id_, string const &currency_, int64 amount_);

  static const std::int32_t ID = 1634697192;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputTheme: public Object {
 public:
};

class inputTheme final : public InputTheme {
 public:
  int64 id_;
  int64 access_hash_;

  inputTheme(int64 id_, int64 access_hash_);

  static const std::int32_t ID = 1012306921;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputThemeSlug final : public InputTheme {
 public:
  string slug_;

  explicit inputThemeSlug(string const &slug_);

  static const std::int32_t ID = -175567375;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputThemeSettings final : public Object {
 public:
  int32 flags_;
  bool message_colors_animated_;
  object_ptr<BaseTheme> base_theme_;
  int32 accent_color_;
  int32 outbox_accent_color_;
  array<int32> message_colors_;
  object_ptr<InputWallPaper> wallpaper_;
  object_ptr<wallPaperSettings> wallpaper_settings_;
  enum Flags : std::int32_t { MESSAGE_COLORS_ANIMATED_MASK = 4, OUTBOX_ACCENT_COLOR_MASK = 8, MESSAGE_COLORS_MASK = 1, WALLPAPER_MASK = 2, WALLPAPER_SETTINGS_MASK = 2 };

  inputThemeSettings(int32 flags_, bool message_colors_animated_, object_ptr<BaseTheme> &&base_theme_, int32 accent_color_, int32 outbox_accent_color_, array<int32> &&message_colors_, object_ptr<InputWallPaper> &&wallpaper_, object_ptr<wallPaperSettings> &&wallpaper_settings_);

  static const std::int32_t ID = -1881255857;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputUser: public Object {
 public:

  static object_ptr<InputUser> fetch(TlBufferParser &p);
};

class inputUserEmpty final : public InputUser {
 public:

  static const std::int32_t ID = -1182234929;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputUser> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputUserSelf final : public InputUser {
 public:

  static const std::int32_t ID = -138301121;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputUser> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputUser final : public InputUser {
 public:
  int64 user_id_;
  int64 access_hash_;

  inputUser(int64 user_id_, int64 access_hash_);

  static const std::int32_t ID = -233744186;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputUser> fetch(TlBufferParser &p);

  explicit inputUser(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputUserFromMessage final : public InputUser {
 public:
  object_ptr<InputPeer> peer_;
  int32 msg_id_;
  int64 user_id_;

  inputUserFromMessage(object_ptr<InputPeer> &&peer_, int32 msg_id_, int64 user_id_);

  static const std::int32_t ID = 497305826;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<InputUser> fetch(TlBufferParser &p);

  explicit inputUserFromMessage(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputWallPaper: public Object {
 public:
};

class inputWallPaper final : public InputWallPaper {
 public:
  int64 id_;
  int64 access_hash_;

  inputWallPaper(int64 id_, int64 access_hash_);

  static const std::int32_t ID = -433014407;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputWallPaperSlug final : public InputWallPaper {
 public:
  string slug_;

  explicit inputWallPaperSlug(string const &slug_);

  static const std::int32_t ID = 1913199744;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputWallPaperNoFile final : public InputWallPaper {
 public:
  int64 id_;

  explicit inputWallPaperNoFile(int64 id_);

  static const std::int32_t ID = -1770371538;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputWebDocument final : public Object {
 public:
  string url_;
  int32 size_;
  string mime_type_;
  array<object_ptr<DocumentAttribute>> attributes_;

  inputWebDocument(string const &url_, int32 size_, string const &mime_type_, array<object_ptr<DocumentAttribute>> &&attributes_);

  static const std::int32_t ID = -1678949555;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class InputWebFileLocation: public Object {
 public:
};

class inputWebFileLocation final : public InputWebFileLocation {
 public:
  string url_;
  int64 access_hash_;

  inputWebFileLocation(string const &url_, int64 access_hash_);

  static const std::int32_t ID = -1036396922;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputWebFileGeoPointLocation final : public InputWebFileLocation {
 public:
  object_ptr<InputGeoPoint> geo_point_;
  int64 access_hash_;
  int32 w_;
  int32 h_;
  int32 zoom_;
  int32 scale_;

  inputWebFileGeoPointLocation(object_ptr<InputGeoPoint> &&geo_point_, int64 access_hash_, int32 w_, int32 h_, int32 zoom_, int32 scale_);

  static const std::int32_t ID = -1625153079;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputWebFileAudioAlbumThumbLocation final : public InputWebFileLocation {
 public:
  int32 flags_;
  bool small_;
  object_ptr<InputDocument> document_;
  string title_;
  string performer_;
  enum Flags : std::int32_t { SMALL_MASK = 4, DOCUMENT_MASK = 1, TITLE_MASK = 2, PERFORMER_MASK = 2 };

  inputWebFileAudioAlbumThumbLocation(int32 flags_, bool small_, object_ptr<InputDocument> &&document_, string const &title_, string const &performer_);

  static const std::int32_t ID = -193992412;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class invoice final : public Object {
 public:
  int32 flags_;
  bool test_;
  bool name_requested_;
  bool phone_requested_;
  bool email_requested_;
  bool shipping_address_requested_;
  bool flexible_;
  bool phone_to_provider_;
  bool email_to_provider_;
  bool recurring_;
  string currency_;
  array<object_ptr<labeledPrice>> prices_;
  int64 max_tip_amount_;
  array<int64> suggested_tip_amounts_;
  string recurring_terms_url_;
  enum Flags : std::int32_t { TEST_MASK = 1, NAME_REQUESTED_MASK = 2, PHONE_REQUESTED_MASK = 4, EMAIL_REQUESTED_MASK = 8, SHIPPING_ADDRESS_REQUESTED_MASK = 16, FLEXIBLE_MASK = 32, PHONE_TO_PROVIDER_MASK = 64, EMAIL_TO_PROVIDER_MASK = 128, RECURRING_MASK = 512, MAX_TIP_AMOUNT_MASK = 256, SUGGESTED_TIP_AMOUNTS_MASK = 256, RECURRING_TERMS_URL_MASK = 512 };

  invoice();

  invoice(int32 flags_, bool test_, bool name_requested_, bool phone_requested_, bool email_requested_, bool shipping_address_requested_, bool flexible_, bool phone_to_provider_, bool email_to_provider_, bool recurring_, string const &currency_, array<object_ptr<labeledPrice>> &&prices_, int64 max_tip_amount_, array<int64> &&suggested_tip_amounts_, string const &recurring_terms_url_);

  static const std::int32_t ID = 1048946971;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<invoice> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class IpPort: public Object {
 public:

  static object_ptr<IpPort> fetch(TlBufferParser &p);
};

class ipPort final : public IpPort {
 public:
  int32 ipv4_;
  int32 port_;

  static const std::int32_t ID = -734810765;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<IpPort> fetch(TlBufferParser &p);

  explicit ipPort(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ipPortSecret final : public IpPort {
 public:
  int32 ipv4_;
  int32 port_;
  bytes secret_;

  static const std::int32_t ID = 932718150;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<IpPort> fetch(TlBufferParser &p);

  explicit ipPortSecret(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class jsonObjectValue final : public Object {
 public:
  string key_;
  object_ptr<JSONValue> value_;

  jsonObjectValue(string const &key_, object_ptr<JSONValue> &&value_);

  static const std::int32_t ID = -1059185703;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<jsonObjectValue> fetch(TlBufferParser &p);

  explicit jsonObjectValue(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class JSONValue: public Object {
 public:

  static object_ptr<JSONValue> fetch(TlBufferParser &p);
};

class jsonNull final : public JSONValue {
 public:

  static const std::int32_t ID = 1064139624;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<JSONValue> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class jsonBool final : public JSONValue {
 public:
  bool value_;

  explicit jsonBool(bool value_);

  static const std::int32_t ID = -952869270;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<JSONValue> fetch(TlBufferParser &p);

  explicit jsonBool(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class jsonNumber final : public JSONValue {
 public:
  double value_;

  explicit jsonNumber(double value_);

  static const std::int32_t ID = 736157604;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<JSONValue> fetch(TlBufferParser &p);

  explicit jsonNumber(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class jsonString final : public JSONValue {
 public:
  string value_;

  explicit jsonString(string const &value_);

  static const std::int32_t ID = -1222740358;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<JSONValue> fetch(TlBufferParser &p);

  explicit jsonString(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class jsonArray final : public JSONValue {
 public:
  array<object_ptr<JSONValue>> value_;

  explicit jsonArray(array<object_ptr<JSONValue>> &&value_);

  static const std::int32_t ID = -146520221;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<JSONValue> fetch(TlBufferParser &p);

  explicit jsonArray(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class jsonObject final : public JSONValue {
 public:
  array<object_ptr<jsonObjectValue>> value_;

  explicit jsonObject(array<object_ptr<jsonObjectValue>> &&value_);

  static const std::int32_t ID = -1715350371;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<JSONValue> fetch(TlBufferParser &p);

  explicit jsonObject(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class KeyboardButton: public Object {
 public:

  static object_ptr<KeyboardButton> fetch(TlBufferParser &p);
};

class keyboardButton final : public KeyboardButton {
 public:
  string text_;

  explicit keyboardButton(string const &text_);

  static const std::int32_t ID = -1560655744;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<KeyboardButton> fetch(TlBufferParser &p);

  explicit keyboardButton(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class keyboardButtonUrl final : public KeyboardButton {
 public:
  string text_;
  string url_;

  keyboardButtonUrl(string const &text_, string const &url_);

  static const std::int32_t ID = 629866245;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<KeyboardButton> fetch(TlBufferParser &p);

  explicit keyboardButtonUrl(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class keyboardButtonCallback final : public KeyboardButton {
 public:
  int32 flags_;
  bool requires_password_;
  string text_;
  bytes data_;
  enum Flags : std::int32_t { REQUIRES_PASSWORD_MASK = 1 };

  keyboardButtonCallback();

  keyboardButtonCallback(int32 flags_, bool requires_password_, string const &text_, bytes &&data_);

  static const std::int32_t ID = 901503851;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<KeyboardButton> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class keyboardButtonRequestPhone final : public KeyboardButton {
 public:
  string text_;

  explicit keyboardButtonRequestPhone(string const &text_);

  static const std::int32_t ID = -1318425559;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<KeyboardButton> fetch(TlBufferParser &p);

  explicit keyboardButtonRequestPhone(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class keyboardButtonRequestGeoLocation final : public KeyboardButton {
 public:
  string text_;

  explicit keyboardButtonRequestGeoLocation(string const &text_);

  static const std::int32_t ID = -59151553;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<KeyboardButton> fetch(TlBufferParser &p);

  explicit keyboardButtonRequestGeoLocation(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class keyboardButtonSwitchInline final : public KeyboardButton {
 public:
  int32 flags_;
  bool same_peer_;
  string text_;
  string query_;
  array<object_ptr<InlineQueryPeerType>> peer_types_;
  enum Flags : std::int32_t { SAME_PEER_MASK = 1, PEER_TYPES_MASK = 2 };

  keyboardButtonSwitchInline();

  keyboardButtonSwitchInline(int32 flags_, bool same_peer_, string const &text_, string const &query_, array<object_ptr<InlineQueryPeerType>> &&peer_types_);

  static const std::int32_t ID = -1816527947;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<KeyboardButton> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class keyboardButtonGame final : public KeyboardButton {
 public:
  string text_;

  explicit keyboardButtonGame(string const &text_);

  static const std::int32_t ID = 1358175439;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<KeyboardButton> fetch(TlBufferParser &p);

  explicit keyboardButtonGame(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class keyboardButtonBuy final : public KeyboardButton {
 public:
  string text_;

  explicit keyboardButtonBuy(string const &text_);

  static const std::int32_t ID = -1344716869;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<KeyboardButton> fetch(TlBufferParser &p);

  explicit keyboardButtonBuy(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class keyboardButtonUrlAuth final : public KeyboardButton {
 public:
  int32 flags_;
  string text_;
  string fwd_text_;
  string url_;
  int32 button_id_;
  enum Flags : std::int32_t { FWD_TEXT_MASK = 1 };

  keyboardButtonUrlAuth();

  keyboardButtonUrlAuth(int32 flags_, string const &text_, string const &fwd_text_, string const &url_, int32 button_id_);

  static const std::int32_t ID = 280464681;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<KeyboardButton> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputKeyboardButtonUrlAuth final : public KeyboardButton {
 public:
  int32 flags_;
  bool request_write_access_;
  string text_;
  string fwd_text_;
  string url_;
  object_ptr<InputUser> bot_;
  enum Flags : std::int32_t { REQUEST_WRITE_ACCESS_MASK = 1, FWD_TEXT_MASK = 2 };

  inputKeyboardButtonUrlAuth();

  inputKeyboardButtonUrlAuth(int32 flags_, bool request_write_access_, string const &text_, string const &fwd_text_, string const &url_, object_ptr<InputUser> &&bot_);

  static const std::int32_t ID = -802258988;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<KeyboardButton> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class keyboardButtonRequestPoll final : public KeyboardButton {
 public:
  int32 flags_;
  bool quiz_;
  string text_;
  enum Flags : std::int32_t { QUIZ_MASK = 1 };

  keyboardButtonRequestPoll();

  keyboardButtonRequestPoll(int32 flags_, bool quiz_, string const &text_);

  static const std::int32_t ID = -1144565411;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<KeyboardButton> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputKeyboardButtonUserProfile final : public KeyboardButton {
 public:
  string text_;
  object_ptr<InputUser> user_id_;

  inputKeyboardButtonUserProfile(string const &text_, object_ptr<InputUser> &&user_id_);

  static const std::int32_t ID = -376962181;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<KeyboardButton> fetch(TlBufferParser &p);

  explicit inputKeyboardButtonUserProfile(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class keyboardButtonUserProfile final : public KeyboardButton {
 public:
  string text_;
  int64 user_id_;

  keyboardButtonUserProfile(string const &text_, int64 user_id_);

  static const std::int32_t ID = 814112961;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<KeyboardButton> fetch(TlBufferParser &p);

  explicit keyboardButtonUserProfile(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class keyboardButtonWebView final : public KeyboardButton {
 public:
  string text_;
  string url_;

  keyboardButtonWebView(string const &text_, string const &url_);

  static const std::int32_t ID = 326529584;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<KeyboardButton> fetch(TlBufferParser &p);

  explicit keyboardButtonWebView(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class keyboardButtonSimpleWebView final : public KeyboardButton {
 public:
  string text_;
  string url_;

  keyboardButtonSimpleWebView(string const &text_, string const &url_);

  static const std::int32_t ID = -1598009252;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<KeyboardButton> fetch(TlBufferParser &p);

  explicit keyboardButtonSimpleWebView(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class keyboardButtonRequestPeer final : public KeyboardButton {
 public:
  string text_;
  int32 button_id_;
  object_ptr<RequestPeerType> peer_type_;

  keyboardButtonRequestPeer(string const &text_, int32 button_id_, object_ptr<RequestPeerType> &&peer_type_);

  static const std::int32_t ID = 218842764;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<KeyboardButton> fetch(TlBufferParser &p);

  explicit keyboardButtonRequestPeer(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class keyboardButtonRow final : public Object {
 public:
  array<object_ptr<KeyboardButton>> buttons_;

  explicit keyboardButtonRow(array<object_ptr<KeyboardButton>> &&buttons_);

  static const std::int32_t ID = 2002815875;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<keyboardButtonRow> fetch(TlBufferParser &p);

  explicit keyboardButtonRow(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class labeledPrice final : public Object {
 public:
  string label_;
  int64 amount_;

  labeledPrice(string const &label_, int64 amount_);

  static const std::int32_t ID = -886477832;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<labeledPrice> fetch(TlBufferParser &p);

  explicit labeledPrice(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class langPackDifference final : public Object {
 public:
  string lang_code_;
  int32 from_version_;
  int32 version_;
  array<object_ptr<LangPackString>> strings_;

  static const std::int32_t ID = -209337866;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<langPackDifference> fetch(TlBufferParser &p);

  explicit langPackDifference(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class langPackLanguage final : public Object {
 public:
  int32 flags_;
  bool official_;
  bool rtl_;
  bool beta_;
  string name_;
  string native_name_;
  string lang_code_;
  string base_lang_code_;
  string plural_code_;
  int32 strings_count_;
  int32 translated_count_;
  string translations_url_;
  enum Flags : std::int32_t { BASE_LANG_CODE_MASK = 2 };

  langPackLanguage();

  static const std::int32_t ID = -288727837;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<langPackLanguage> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class LangPackString: public Object {
 public:

  static object_ptr<LangPackString> fetch(TlBufferParser &p);
};

class langPackString final : public LangPackString {
 public:
  string key_;
  string value_;

  langPackString(string const &key_, string const &value_);

  static const std::int32_t ID = -892239370;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<LangPackString> fetch(TlBufferParser &p);

  explicit langPackString(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class langPackStringPluralized final : public LangPackString {
 public:
  int32 flags_;
  string key_;
  string zero_value_;
  string one_value_;
  string two_value_;
  string few_value_;
  string many_value_;
  string other_value_;
  enum Flags : std::int32_t { ZERO_VALUE_MASK = 1, ONE_VALUE_MASK = 2, TWO_VALUE_MASK = 4, FEW_VALUE_MASK = 8, MANY_VALUE_MASK = 16 };

  langPackStringPluralized();

  langPackStringPluralized(int32 flags_, string const &key_, string const &zero_value_, string const &one_value_, string const &two_value_, string const &few_value_, string const &many_value_, string const &other_value_);

  static const std::int32_t ID = 1816636575;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<LangPackString> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class langPackStringDeleted final : public LangPackString {
 public:
  string key_;

  explicit langPackStringDeleted(string const &key_);

  static const std::int32_t ID = 695856818;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<LangPackString> fetch(TlBufferParser &p);

  explicit langPackStringDeleted(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class maskCoords final : public Object {
 public:
  int32 n_;
  double x_;
  double y_;
  double zoom_;

  maskCoords(int32 n_, double x_, double y_, double zoom_);

  static const std::int32_t ID = -1361650766;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<maskCoords> fetch(TlBufferParser &p);

  explicit maskCoords(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class Message: public Object {
 public:

  static object_ptr<Message> fetch(TlBufferParser &p);
};

class messageEmpty final : public Message {
 public:
  int32 flags_;
  int32 id_;
  object_ptr<Peer> peer_id_;

  messageEmpty();

  static const std::int32_t ID = -1868117372;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Message> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class message final : public Message {
 public:
  int32 flags_;
  bool out_;
  bool mentioned_;
  bool media_unread_;
  bool silent_;
  bool post_;
  bool from_scheduled_;
  bool legacy_;
  bool edit_hide_;
  bool pinned_;
  bool noforwards_;
  int32 id_;
  object_ptr<Peer> from_id_;
  object_ptr<Peer> peer_id_;
  object_ptr<messageFwdHeader> fwd_from_;
  int64 via_bot_id_;
  object_ptr<messageReplyHeader> reply_to_;
  int32 date_;
  string message_;
  object_ptr<MessageMedia> media_;
  object_ptr<ReplyMarkup> reply_markup_;
  array<object_ptr<MessageEntity>> entities_;
  int32 views_;
  int32 forwards_;
  object_ptr<messageReplies> replies_;
  int32 edit_date_;
  string post_author_;
  int64 grouped_id_;
  object_ptr<messageReactions> reactions_;
  array<object_ptr<restrictionReason>> restriction_reason_;
  int32 ttl_period_;
  enum Flags : std::int32_t { VIA_BOT_ID_MASK = 2048, VIEWS_MASK = 1024, FORWARDS_MASK = 1024, EDIT_DATE_MASK = 32768, POST_AUTHOR_MASK = 65536, GROUPED_ID_MASK = 131072, TTL_PERIOD_MASK = 33554432 };

  message();

  message(int32 flags_, bool out_, bool mentioned_, bool media_unread_, bool silent_, bool post_, bool from_scheduled_, bool legacy_, bool edit_hide_, bool pinned_, bool noforwards_, int32 id_, object_ptr<Peer> &&from_id_, object_ptr<Peer> &&peer_id_, object_ptr<messageFwdHeader> &&fwd_from_, int64 via_bot_id_, object_ptr<messageReplyHeader> &&reply_to_, int32 date_, string const &message_, object_ptr<MessageMedia> &&media_, object_ptr<ReplyMarkup> &&reply_markup_, array<object_ptr<MessageEntity>> &&entities_, int32 views_, int32 forwards_, object_ptr<messageReplies> &&replies_, int32 edit_date_, string const &post_author_, int64 grouped_id_, object_ptr<messageReactions> &&reactions_, array<object_ptr<restrictionReason>> &&restriction_reason_, int32 ttl_period_);

  static const std::int32_t ID = 940666592;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Message> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageService final : public Message {
 public:
  int32 flags_;
  bool out_;
  bool mentioned_;
  bool media_unread_;
  bool silent_;
  bool post_;
  bool legacy_;
  int32 id_;
  object_ptr<Peer> from_id_;
  object_ptr<Peer> peer_id_;
  object_ptr<messageReplyHeader> reply_to_;
  int32 date_;
  object_ptr<MessageAction> action_;
  int32 ttl_period_;
  enum Flags : std::int32_t { TTL_PERIOD_MASK = 33554432 };

  messageService();

  static const std::int32_t ID = 721967202;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Message> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class MessageAction: public Object {
 public:

  static object_ptr<MessageAction> fetch(TlBufferParser &p);
};

class messageActionEmpty final : public MessageAction {
 public:

  static const std::int32_t ID = -1230047312;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionChatCreate final : public MessageAction {
 public:
  string title_;
  array<int64> users_;

  static const std::int32_t ID = -1119368275;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionChatCreate(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionChatEditTitle final : public MessageAction {
 public:
  string title_;

  static const std::int32_t ID = -1247687078;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionChatEditTitle(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionChatEditPhoto final : public MessageAction {
 public:
  object_ptr<Photo> photo_;

  static const std::int32_t ID = 2144015272;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionChatEditPhoto(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionChatDeletePhoto final : public MessageAction {
 public:

  static const std::int32_t ID = -1780220945;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionChatAddUser final : public MessageAction {
 public:
  array<int64> users_;

  static const std::int32_t ID = 365886720;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionChatAddUser(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionChatDeleteUser final : public MessageAction {
 public:
  int64 user_id_;

  static const std::int32_t ID = -1539362612;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionChatDeleteUser(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionChatJoinedByLink final : public MessageAction {
 public:
  int64 inviter_id_;

  static const std::int32_t ID = 51520707;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionChatJoinedByLink(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionChannelCreate final : public MessageAction {
 public:
  string title_;

  static const std::int32_t ID = -1781355374;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionChannelCreate(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionChatMigrateTo final : public MessageAction {
 public:
  int64 channel_id_;

  static const std::int32_t ID = -519864430;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionChatMigrateTo(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionChannelMigrateFrom final : public MessageAction {
 public:
  string title_;
  int64 chat_id_;

  static const std::int32_t ID = -365344535;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionChannelMigrateFrom(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionPinMessage final : public MessageAction {
 public:

  static const std::int32_t ID = -1799538451;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionHistoryClear final : public MessageAction {
 public:

  static const std::int32_t ID = -1615153660;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionGameScore final : public MessageAction {
 public:
  int64 game_id_;
  int32 score_;

  static const std::int32_t ID = -1834538890;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionGameScore(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionPaymentSentMe final : public MessageAction {
 public:
  int32 flags_;
  bool recurring_init_;
  bool recurring_used_;
  string currency_;
  int64 total_amount_;
  bytes payload_;
  object_ptr<paymentRequestedInfo> info_;
  string shipping_option_id_;
  object_ptr<paymentCharge> charge_;
  enum Flags : std::int32_t { SHIPPING_OPTION_ID_MASK = 2 };

  messageActionPaymentSentMe();

  static const std::int32_t ID = -1892568281;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionPaymentSent final : public MessageAction {
 public:
  int32 flags_;
  bool recurring_init_;
  bool recurring_used_;
  string currency_;
  int64 total_amount_;
  string invoice_slug_;
  enum Flags : std::int32_t { INVOICE_SLUG_MASK = 1 };

  messageActionPaymentSent();

  static const std::int32_t ID = -1776926890;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionPhoneCall final : public MessageAction {
 public:
  int32 flags_;
  bool video_;
  int64 call_id_;
  object_ptr<PhoneCallDiscardReason> reason_;
  int32 duration_;
  enum Flags : std::int32_t { DURATION_MASK = 2 };

  messageActionPhoneCall();

  static const std::int32_t ID = -2132731265;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionScreenshotTaken final : public MessageAction {
 public:

  static const std::int32_t ID = 1200788123;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionCustomAction final : public MessageAction {
 public:
  string message_;

  static const std::int32_t ID = -85549226;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionCustomAction(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionBotAllowed final : public MessageAction {
 public:
  int32 flags_;
  bool attach_menu_;
  string domain_;
  object_ptr<BotApp> app_;
  enum Flags : std::int32_t { DOMAIN_MASK = 1 };

  messageActionBotAllowed();

  static const std::int32_t ID = -988359047;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionSecureValuesSentMe final : public MessageAction {
 public:
  array<object_ptr<secureValue>> values_;
  object_ptr<secureCredentialsEncrypted> credentials_;

  static const std::int32_t ID = 455635795;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionSecureValuesSentMe(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionSecureValuesSent final : public MessageAction {
 public:
  array<object_ptr<SecureValueType>> types_;

  static const std::int32_t ID = -648257196;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionSecureValuesSent(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionContactSignUp final : public MessageAction {
 public:

  static const std::int32_t ID = -202219658;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionGeoProximityReached final : public MessageAction {
 public:
  object_ptr<Peer> from_id_;
  object_ptr<Peer> to_id_;
  int32 distance_;

  static const std::int32_t ID = -1730095465;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionGeoProximityReached(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionGroupCall final : public MessageAction {
 public:
  int32 flags_;
  object_ptr<inputGroupCall> call_;
  int32 duration_;
  enum Flags : std::int32_t { DURATION_MASK = 1 };

  messageActionGroupCall();

  static const std::int32_t ID = 2047704898;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionInviteToGroupCall final : public MessageAction {
 public:
  object_ptr<inputGroupCall> call_;
  array<int64> users_;

  static const std::int32_t ID = 1345295095;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionInviteToGroupCall(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionSetMessagesTTL final : public MessageAction {
 public:
  int32 flags_;
  int32 period_;
  int64 auto_setting_from_;
  enum Flags : std::int32_t { AUTO_SETTING_FROM_MASK = 1 };

  messageActionSetMessagesTTL();

  static const std::int32_t ID = 1007897979;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionGroupCallScheduled final : public MessageAction {
 public:
  object_ptr<inputGroupCall> call_;
  int32 schedule_date_;

  static const std::int32_t ID = -1281329567;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionGroupCallScheduled(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionSetChatTheme final : public MessageAction {
 public:
  string emoticon_;

  static const std::int32_t ID = -1434950843;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionSetChatTheme(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionChatJoinedByRequest final : public MessageAction {
 public:

  static const std::int32_t ID = -339958837;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionWebViewDataSentMe final : public MessageAction {
 public:
  string text_;
  string data_;

  static const std::int32_t ID = 1205698681;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionWebViewDataSentMe(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionWebViewDataSent final : public MessageAction {
 public:
  string text_;

  static const std::int32_t ID = -1262252875;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionWebViewDataSent(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionGiftPremium final : public MessageAction {
 public:
  int32 flags_;
  string currency_;
  int64 amount_;
  int32 months_;
  string crypto_currency_;
  int64 crypto_amount_;
  enum Flags : std::int32_t { CRYPTO_CURRENCY_MASK = 1, CRYPTO_AMOUNT_MASK = 1 };

  messageActionGiftPremium();

  static const std::int32_t ID = -935499028;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionTopicCreate final : public MessageAction {
 public:
  int32 flags_;
  string title_;
  int32 icon_color_;
  int64 icon_emoji_id_;
  enum Flags : std::int32_t { ICON_EMOJI_ID_MASK = 1 };

  messageActionTopicCreate();

  static const std::int32_t ID = 228168278;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionTopicEdit final : public MessageAction {
 public:
  int32 flags_;
  string title_;
  int64 icon_emoji_id_;
  bool closed_;
  bool hidden_;
  enum Flags : std::int32_t { TITLE_MASK = 1, ICON_EMOJI_ID_MASK = 2, CLOSED_MASK = 4, HIDDEN_MASK = 8 };

  messageActionTopicEdit();

  static const std::int32_t ID = -1064024032;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionSuggestProfilePhoto final : public MessageAction {
 public:
  object_ptr<Photo> photo_;

  static const std::int32_t ID = 1474192222;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionSuggestProfilePhoto(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionRequestedPeer final : public MessageAction {
 public:
  int32 button_id_;
  object_ptr<Peer> peer_;

  static const std::int32_t ID = -25742243;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionRequestedPeer(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionSetChatWallPaper final : public MessageAction {
 public:
  object_ptr<WallPaper> wallpaper_;

  static const std::int32_t ID = -1136350937;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionSetChatWallPaper(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageActionSetSameChatWallPaper final : public MessageAction {
 public:
  object_ptr<WallPaper> wallpaper_;

  static const std::int32_t ID = -1065845395;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageAction> fetch(TlBufferParser &p);

  explicit messageActionSetSameChatWallPaper(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class MessageEntity: public Object {
 public:

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);
};

class messageEntityUnknown final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;

  messageEntityUnknown(int32 offset_, int32 length_);

  static const std::int32_t ID = -1148011883;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntityUnknown(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageEntityMention final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;

  messageEntityMention(int32 offset_, int32 length_);

  static const std::int32_t ID = -100378723;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntityMention(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageEntityHashtag final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;

  messageEntityHashtag(int32 offset_, int32 length_);

  static const std::int32_t ID = 1868782349;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntityHashtag(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageEntityBotCommand final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;

  messageEntityBotCommand(int32 offset_, int32 length_);

  static const std::int32_t ID = 1827637959;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntityBotCommand(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageEntityUrl final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;

  messageEntityUrl(int32 offset_, int32 length_);

  static const std::int32_t ID = 1859134776;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntityUrl(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageEntityEmail final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;

  messageEntityEmail(int32 offset_, int32 length_);

  static const std::int32_t ID = 1692693954;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntityEmail(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageEntityBold final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;

  messageEntityBold(int32 offset_, int32 length_);

  static const std::int32_t ID = -1117713463;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntityBold(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageEntityItalic final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;

  messageEntityItalic(int32 offset_, int32 length_);

  static const std::int32_t ID = -2106619040;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntityItalic(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageEntityCode final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;

  messageEntityCode(int32 offset_, int32 length_);

  static const std::int32_t ID = 681706865;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntityCode(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageEntityPre final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;
  string language_;

  messageEntityPre(int32 offset_, int32 length_, string const &language_);

  static const std::int32_t ID = 1938967520;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntityPre(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageEntityTextUrl final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;
  string url_;

  messageEntityTextUrl(int32 offset_, int32 length_, string const &url_);

  static const std::int32_t ID = 1990644519;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntityTextUrl(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageEntityMentionName final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;
  int64 user_id_;

  messageEntityMentionName(int32 offset_, int32 length_, int64 user_id_);

  static const std::int32_t ID = -595914432;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntityMentionName(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessageEntityMentionName final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;
  object_ptr<InputUser> user_id_;

  inputMessageEntityMentionName(int32 offset_, int32 length_, object_ptr<InputUser> &&user_id_);

  static const std::int32_t ID = 546203849;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit inputMessageEntityMentionName(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageEntityPhone final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;

  messageEntityPhone(int32 offset_, int32 length_);

  static const std::int32_t ID = -1687559349;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntityPhone(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageEntityCashtag final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;

  messageEntityCashtag(int32 offset_, int32 length_);

  static const std::int32_t ID = 1280209983;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntityCashtag(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageEntityUnderline final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;

  messageEntityUnderline(int32 offset_, int32 length_);

  static const std::int32_t ID = -1672577397;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntityUnderline(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageEntityStrike final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;

  messageEntityStrike(int32 offset_, int32 length_);

  static const std::int32_t ID = -1090087980;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntityStrike(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageEntityBlockquote final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;

  messageEntityBlockquote(int32 offset_, int32 length_);

  static const std::int32_t ID = 34469328;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntityBlockquote(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageEntityBankCard final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;

  messageEntityBankCard(int32 offset_, int32 length_);

  static const std::int32_t ID = 1981704948;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntityBankCard(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageEntitySpoiler final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;

  messageEntitySpoiler(int32 offset_, int32 length_);

  static const std::int32_t ID = 852137487;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntitySpoiler(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageEntityCustomEmoji final : public MessageEntity {
 public:
  int32 offset_;
  int32 length_;
  int64 document_id_;

  messageEntityCustomEmoji(int32 offset_, int32 length_, int64 document_id_);

  static const std::int32_t ID = -925956616;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageEntity> fetch(TlBufferParser &p);

  explicit messageEntityCustomEmoji(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class MessageExtendedMedia: public Object {
 public:

  static object_ptr<MessageExtendedMedia> fetch(TlBufferParser &p);
};

class messageExtendedMediaPreview final : public MessageExtendedMedia {
 public:
  int32 flags_;
  int32 w_;
  int32 h_;
  object_ptr<PhotoSize> thumb_;
  int32 video_duration_;
  enum Flags : std::int32_t { W_MASK = 1, H_MASK = 1, VIDEO_DURATION_MASK = 4 };

  messageExtendedMediaPreview();

  static const std::int32_t ID = -1386050360;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageExtendedMedia> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageExtendedMedia final : public MessageExtendedMedia {
 public:
  object_ptr<MessageMedia> media_;

  static const std::int32_t ID = -297296796;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageExtendedMedia> fetch(TlBufferParser &p);

  explicit messageExtendedMedia(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageFwdHeader final : public Object {
 public:
  int32 flags_;
  bool imported_;
  object_ptr<Peer> from_id_;
  string from_name_;
  int32 date_;
  int32 channel_post_;
  string post_author_;
  object_ptr<Peer> saved_from_peer_;
  int32 saved_from_msg_id_;
  string psa_type_;
  enum Flags : std::int32_t { FROM_NAME_MASK = 32, CHANNEL_POST_MASK = 4, POST_AUTHOR_MASK = 8, SAVED_FROM_MSG_ID_MASK = 16, PSA_TYPE_MASK = 64 };

  messageFwdHeader();

  static const std::int32_t ID = 1601666510;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messageFwdHeader> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageInteractionCounters final : public Object {
 public:
  int32 msg_id_;
  int32 views_;
  int32 forwards_;

  static const std::int32_t ID = -1387279939;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messageInteractionCounters> fetch(TlBufferParser &p);

  explicit messageInteractionCounters(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class MessageMedia: public Object {
 public:

  static object_ptr<MessageMedia> fetch(TlBufferParser &p);
};

class messageMediaEmpty final : public MessageMedia {
 public:

  static const std::int32_t ID = 1038967584;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageMedia> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageMediaPhoto final : public MessageMedia {
 public:
  int32 flags_;
  bool spoiler_;
  object_ptr<Photo> photo_;
  int32 ttl_seconds_;
  enum Flags : std::int32_t { TTL_SECONDS_MASK = 4 };

  messageMediaPhoto();

  static const std::int32_t ID = 1766936791;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageMedia> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageMediaGeo final : public MessageMedia {
 public:
  object_ptr<GeoPoint> geo_;

  static const std::int32_t ID = 1457575028;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageMedia> fetch(TlBufferParser &p);

  explicit messageMediaGeo(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageMediaContact final : public MessageMedia {
 public:
  string phone_number_;
  string first_name_;
  string last_name_;
  string vcard_;
  int64 user_id_;

  static const std::int32_t ID = 1882335561;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageMedia> fetch(TlBufferParser &p);

  explicit messageMediaContact(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageMediaUnsupported final : public MessageMedia {
 public:

  static const std::int32_t ID = -1618676578;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageMedia> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageMediaDocument final : public MessageMedia {
 public:
  int32 flags_;
  bool nopremium_;
  bool spoiler_;
  object_ptr<Document> document_;
  int32 ttl_seconds_;
  enum Flags : std::int32_t { TTL_SECONDS_MASK = 4 };

  messageMediaDocument();

  static const std::int32_t ID = -1666158377;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageMedia> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageMediaWebPage final : public MessageMedia {
 public:
  object_ptr<WebPage> webpage_;

  static const std::int32_t ID = -1557277184;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageMedia> fetch(TlBufferParser &p);

  explicit messageMediaWebPage(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageMediaVenue final : public MessageMedia {
 public:
  object_ptr<GeoPoint> geo_;
  string title_;
  string address_;
  string provider_;
  string venue_id_;
  string venue_type_;

  static const std::int32_t ID = 784356159;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageMedia> fetch(TlBufferParser &p);

  explicit messageMediaVenue(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageMediaGame final : public MessageMedia {
 public:
  object_ptr<game> game_;

  static const std::int32_t ID = -38694904;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageMedia> fetch(TlBufferParser &p);

  explicit messageMediaGame(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageMediaInvoice final : public MessageMedia {
 public:
  int32 flags_;
  bool shipping_address_requested_;
  bool test_;
  string title_;
  string description_;
  object_ptr<WebDocument> photo_;
  int32 receipt_msg_id_;
  string currency_;
  int64 total_amount_;
  string start_param_;
  object_ptr<MessageExtendedMedia> extended_media_;
  enum Flags : std::int32_t { RECEIPT_MSG_ID_MASK = 4 };

  messageMediaInvoice();

  static const std::int32_t ID = -156940077;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageMedia> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageMediaGeoLive final : public MessageMedia {
 public:
  int32 flags_;
  object_ptr<GeoPoint> geo_;
  int32 heading_;
  int32 period_;
  int32 proximity_notification_radius_;
  enum Flags : std::int32_t { HEADING_MASK = 1, PROXIMITY_NOTIFICATION_RADIUS_MASK = 2 };

  messageMediaGeoLive();

  static const std::int32_t ID = -1186937242;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageMedia> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageMediaPoll final : public MessageMedia {
 public:
  object_ptr<poll> poll_;
  object_ptr<pollResults> results_;

  static const std::int32_t ID = 1272375192;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageMedia> fetch(TlBufferParser &p);

  explicit messageMediaPoll(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageMediaDice final : public MessageMedia {
 public:
  int32 value_;
  string emoticon_;

  static const std::int32_t ID = 1065280907;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageMedia> fetch(TlBufferParser &p);

  explicit messageMediaDice(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messagePeerReaction final : public Object {
 public:
  int32 flags_;
  bool big_;
  bool unread_;
  object_ptr<Peer> peer_id_;
  int32 date_;
  object_ptr<Reaction> reaction_;

  messagePeerReaction();

  static const std::int32_t ID = -1938180548;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messagePeerReaction> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageRange final : public Object {
 public:
  int32 min_id_;
  int32 max_id_;

  messageRange(int32 min_id_, int32 max_id_);

  static const std::int32_t ID = 182649427;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messageRange> fetch(TlBufferParser &p);

  explicit messageRange(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageReactions final : public Object {
 public:
  int32 flags_;
  bool min_;
  bool can_see_list_;
  array<object_ptr<reactionCount>> results_;
  array<object_ptr<messagePeerReaction>> recent_reactions_;

  messageReactions();

  static const std::int32_t ID = 1328256121;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messageReactions> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageReplies final : public Object {
 public:
  int32 flags_;
  bool comments_;
  int32 replies_;
  int32 replies_pts_;
  array<object_ptr<Peer>> recent_repliers_;
  int64 channel_id_;
  int32 max_id_;
  int32 read_max_id_;
  enum Flags : std::int32_t { CHANNEL_ID_MASK = 1, MAX_ID_MASK = 4, READ_MAX_ID_MASK = 8 };

  messageReplies();

  static const std::int32_t ID = -2083123262;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messageReplies> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageReplyHeader final : public Object {
 public:
  int32 flags_;
  bool reply_to_scheduled_;
  bool forum_topic_;
  int32 reply_to_msg_id_;
  object_ptr<Peer> reply_to_peer_id_;
  int32 reply_to_top_id_;
  enum Flags : std::int32_t { REPLY_TO_TOP_ID_MASK = 2 };

  messageReplyHeader();

  static const std::int32_t ID = -1495959709;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messageReplyHeader> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class MessageUserVote: public Object {
 public:

  static object_ptr<MessageUserVote> fetch(TlBufferParser &p);
};

class messageUserVote final : public MessageUserVote {
 public:
  int64 user_id_;
  bytes option_;
  int32 date_;

  static const std::int32_t ID = 886196148;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageUserVote> fetch(TlBufferParser &p);

  explicit messageUserVote(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageUserVoteInputOption final : public MessageUserVote {
 public:
  int64 user_id_;
  int32 date_;

  static const std::int32_t ID = 1017491692;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageUserVote> fetch(TlBufferParser &p);

  explicit messageUserVoteInputOption(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageUserVoteMultiple final : public MessageUserVote {
 public:
  int64 user_id_;
  array<bytes> options_;
  int32 date_;

  static const std::int32_t ID = -1973033641;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessageUserVote> fetch(TlBufferParser &p);

  explicit messageUserVoteMultiple(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messageViews final : public Object {
 public:
  int32 flags_;
  int32 views_;
  int32 forwards_;
  object_ptr<messageReplies> replies_;
  enum Flags : std::int32_t { VIEWS_MASK = 1, FORWARDS_MASK = 2 };

  messageViews();

  static const std::int32_t ID = 1163625789;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messageViews> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class MessagesFilter: public Object {
 public:

  static object_ptr<MessagesFilter> fetch(TlBufferParser &p);
};

class inputMessagesFilterEmpty final : public MessagesFilter {
 public:

  static const std::int32_t ID = 1474492012;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessagesFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessagesFilterPhotos final : public MessagesFilter {
 public:

  static const std::int32_t ID = -1777752804;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessagesFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessagesFilterVideo final : public MessagesFilter {
 public:

  static const std::int32_t ID = -1614803355;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessagesFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessagesFilterPhotoVideo final : public MessagesFilter {
 public:

  static const std::int32_t ID = 1458172132;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessagesFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessagesFilterDocument final : public MessagesFilter {
 public:

  static const std::int32_t ID = -1629621880;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessagesFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessagesFilterUrl final : public MessagesFilter {
 public:

  static const std::int32_t ID = 2129714567;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessagesFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessagesFilterGif final : public MessagesFilter {
 public:

  static const std::int32_t ID = -3644025;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessagesFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessagesFilterVoice final : public MessagesFilter {
 public:

  static const std::int32_t ID = 1358283666;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessagesFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessagesFilterMusic final : public MessagesFilter {
 public:

  static const std::int32_t ID = 928101534;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessagesFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessagesFilterChatPhotos final : public MessagesFilter {
 public:

  static const std::int32_t ID = 975236280;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessagesFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessagesFilterPhoneCalls final : public MessagesFilter {
 public:
  int32 flags_;
  bool missed_;
  enum Flags : std::int32_t { MISSED_MASK = 1 };

  inputMessagesFilterPhoneCalls();

  inputMessagesFilterPhoneCalls(int32 flags_, bool missed_);

  static const std::int32_t ID = -2134272152;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessagesFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessagesFilterRoundVoice final : public MessagesFilter {
 public:

  static const std::int32_t ID = 2054952868;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessagesFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessagesFilterRoundVideo final : public MessagesFilter {
 public:

  static const std::int32_t ID = -1253451181;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessagesFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessagesFilterMyMentions final : public MessagesFilter {
 public:

  static const std::int32_t ID = -1040652646;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessagesFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessagesFilterGeo final : public MessagesFilter {
 public:

  static const std::int32_t ID = -419271411;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessagesFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessagesFilterContacts final : public MessagesFilter {
 public:

  static const std::int32_t ID = -530392189;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessagesFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputMessagesFilterPinned final : public MessagesFilter {
 public:

  static const std::int32_t ID = 464520273;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<MessagesFilter> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class nearestDc final : public Object {
 public:
  string country_;
  int32 this_dc_;
  int32 nearest_dc_;

  static const std::int32_t ID = -1910892683;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<nearestDc> fetch(TlBufferParser &p);

  explicit nearestDc(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class NotificationSound: public Object {
 public:

  static object_ptr<NotificationSound> fetch(TlBufferParser &p);
};

class notificationSoundDefault final : public NotificationSound {
 public:

  static const std::int32_t ID = -1746354498;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<NotificationSound> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class notificationSoundNone final : public NotificationSound {
 public:

  static const std::int32_t ID = 1863070943;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<NotificationSound> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class notificationSoundLocal final : public NotificationSound {
 public:
  string title_;
  string data_;

  notificationSoundLocal(string const &title_, string const &data_);

  static const std::int32_t ID = -2096391452;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<NotificationSound> fetch(TlBufferParser &p);

  explicit notificationSoundLocal(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class notificationSoundRingtone final : public NotificationSound {
 public:
  int64 id_;

  explicit notificationSoundRingtone(int64 id_);

  static const std::int32_t ID = -9666487;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<NotificationSound> fetch(TlBufferParser &p);

  explicit notificationSoundRingtone(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class NotifyPeer: public Object {
 public:

  static object_ptr<NotifyPeer> fetch(TlBufferParser &p);
};

class notifyPeer final : public NotifyPeer {
 public:
  object_ptr<Peer> peer_;

  static const std::int32_t ID = -1613493288;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<NotifyPeer> fetch(TlBufferParser &p);

  explicit notifyPeer(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class notifyUsers final : public NotifyPeer {
 public:

  static const std::int32_t ID = -1261946036;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<NotifyPeer> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class notifyChats final : public NotifyPeer {
 public:

  static const std::int32_t ID = -1073230141;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<NotifyPeer> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class notifyBroadcasts final : public NotifyPeer {
 public:

  static const std::int32_t ID = -703403793;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<NotifyPeer> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class notifyForumTopic final : public NotifyPeer {
 public:
  object_ptr<Peer> peer_;
  int32 top_msg_id_;

  static const std::int32_t ID = 577659656;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<NotifyPeer> fetch(TlBufferParser &p);

  explicit notifyForumTopic(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class page final : public Object {
 public:
  int32 flags_;
  bool part_;
  bool rtl_;
  bool v2_;
  string url_;
  array<object_ptr<PageBlock>> blocks_;
  array<object_ptr<Photo>> photos_;
  array<object_ptr<Document>> documents_;
  int32 views_;
  enum Flags : std::int32_t { VIEWS_MASK = 8 };

  page();

  static const std::int32_t ID = -1738178803;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<page> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PageBlock: public Object {
 public:

  static object_ptr<PageBlock> fetch(TlBufferParser &p);
};

class pageBlockUnsupported final : public PageBlock {
 public:

  static const std::int32_t ID = 324435594;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockTitle final : public PageBlock {
 public:
  object_ptr<RichText> text_;

  static const std::int32_t ID = 1890305021;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockTitle(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockSubtitle final : public PageBlock {
 public:
  object_ptr<RichText> text_;

  static const std::int32_t ID = -1879401953;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockSubtitle(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockAuthorDate final : public PageBlock {
 public:
  object_ptr<RichText> author_;
  int32 published_date_;

  static const std::int32_t ID = -1162877472;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockAuthorDate(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockHeader final : public PageBlock {
 public:
  object_ptr<RichText> text_;

  static const std::int32_t ID = -1076861716;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockHeader(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockSubheader final : public PageBlock {
 public:
  object_ptr<RichText> text_;

  static const std::int32_t ID = -248793375;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockSubheader(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockParagraph final : public PageBlock {
 public:
  object_ptr<RichText> text_;

  static const std::int32_t ID = 1182402406;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockParagraph(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockPreformatted final : public PageBlock {
 public:
  object_ptr<RichText> text_;
  string language_;

  static const std::int32_t ID = -1066346178;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockPreformatted(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockFooter final : public PageBlock {
 public:
  object_ptr<RichText> text_;

  static const std::int32_t ID = 1216809369;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockFooter(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockDivider final : public PageBlock {
 public:

  static const std::int32_t ID = -618614392;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockAnchor final : public PageBlock {
 public:
  string name_;

  static const std::int32_t ID = -837994576;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockAnchor(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockList final : public PageBlock {
 public:
  array<object_ptr<PageListItem>> items_;

  static const std::int32_t ID = -454524911;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockList(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockBlockquote final : public PageBlock {
 public:
  object_ptr<RichText> text_;
  object_ptr<RichText> caption_;

  static const std::int32_t ID = 641563686;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockBlockquote(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockPullquote final : public PageBlock {
 public:
  object_ptr<RichText> text_;
  object_ptr<RichText> caption_;

  static const std::int32_t ID = 1329878739;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockPullquote(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockPhoto final : public PageBlock {
 public:
  int32 flags_;
  int64 photo_id_;
  object_ptr<pageCaption> caption_;
  string url_;
  int64 webpage_id_;
  enum Flags : std::int32_t { URL_MASK = 1, WEBPAGE_ID_MASK = 1 };

  pageBlockPhoto();

  static const std::int32_t ID = 391759200;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockVideo final : public PageBlock {
 public:
  int32 flags_;
  bool autoplay_;
  bool loop_;
  int64 video_id_;
  object_ptr<pageCaption> caption_;

  pageBlockVideo();

  static const std::int32_t ID = 2089805750;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockCover final : public PageBlock {
 public:
  object_ptr<PageBlock> cover_;

  static const std::int32_t ID = 972174080;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockCover(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockEmbed final : public PageBlock {
 public:
  int32 flags_;
  bool full_width_;
  bool allow_scrolling_;
  string url_;
  string html_;
  int64 poster_photo_id_;
  int32 w_;
  int32 h_;
  object_ptr<pageCaption> caption_;
  enum Flags : std::int32_t { URL_MASK = 2, HTML_MASK = 4, POSTER_PHOTO_ID_MASK = 16, W_MASK = 32, H_MASK = 32 };

  pageBlockEmbed();

  static const std::int32_t ID = -1468953147;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockEmbedPost final : public PageBlock {
 public:
  string url_;
  int64 webpage_id_;
  int64 author_photo_id_;
  string author_;
  int32 date_;
  array<object_ptr<PageBlock>> blocks_;
  object_ptr<pageCaption> caption_;

  static const std::int32_t ID = -229005301;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockEmbedPost(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockCollage final : public PageBlock {
 public:
  array<object_ptr<PageBlock>> items_;
  object_ptr<pageCaption> caption_;

  static const std::int32_t ID = 1705048653;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockCollage(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockSlideshow final : public PageBlock {
 public:
  array<object_ptr<PageBlock>> items_;
  object_ptr<pageCaption> caption_;

  static const std::int32_t ID = 52401552;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockSlideshow(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockChannel final : public PageBlock {
 public:
  object_ptr<Chat> channel_;

  static const std::int32_t ID = -283684427;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockChannel(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockAudio final : public PageBlock {
 public:
  int64 audio_id_;
  object_ptr<pageCaption> caption_;

  static const std::int32_t ID = -2143067670;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockAudio(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockKicker final : public PageBlock {
 public:
  object_ptr<RichText> text_;

  static const std::int32_t ID = 504660880;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockKicker(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockTable final : public PageBlock {
 public:
  int32 flags_;
  bool bordered_;
  bool striped_;
  object_ptr<RichText> title_;
  array<object_ptr<pageTableRow>> rows_;

  pageBlockTable();

  static const std::int32_t ID = -1085412734;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockOrderedList final : public PageBlock {
 public:
  array<object_ptr<PageListOrderedItem>> items_;

  static const std::int32_t ID = -1702174239;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockOrderedList(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockDetails final : public PageBlock {
 public:
  int32 flags_;
  bool open_;
  array<object_ptr<PageBlock>> blocks_;
  object_ptr<RichText> title_;

  pageBlockDetails();

  static const std::int32_t ID = 1987480557;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockRelatedArticles final : public PageBlock {
 public:
  object_ptr<RichText> title_;
  array<object_ptr<pageRelatedArticle>> articles_;

  static const std::int32_t ID = 370236054;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockRelatedArticles(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageBlockMap final : public PageBlock {
 public:
  object_ptr<GeoPoint> geo_;
  int32 zoom_;
  int32 w_;
  int32 h_;
  object_ptr<pageCaption> caption_;

  static const std::int32_t ID = -1538310410;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageBlock> fetch(TlBufferParser &p);

  explicit pageBlockMap(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageCaption final : public Object {
 public:
  object_ptr<RichText> text_;
  object_ptr<RichText> credit_;

  static const std::int32_t ID = 1869903447;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<pageCaption> fetch(TlBufferParser &p);

  explicit pageCaption(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PageListItem: public Object {
 public:

  static object_ptr<PageListItem> fetch(TlBufferParser &p);
};

class pageListItemText final : public PageListItem {
 public:
  object_ptr<RichText> text_;

  static const std::int32_t ID = -1188055347;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageListItem> fetch(TlBufferParser &p);

  explicit pageListItemText(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageListItemBlocks final : public PageListItem {
 public:
  array<object_ptr<PageBlock>> blocks_;

  static const std::int32_t ID = 635466748;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageListItem> fetch(TlBufferParser &p);

  explicit pageListItemBlocks(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PageListOrderedItem: public Object {
 public:

  static object_ptr<PageListOrderedItem> fetch(TlBufferParser &p);
};

class pageListOrderedItemText final : public PageListOrderedItem {
 public:
  string num_;
  object_ptr<RichText> text_;

  static const std::int32_t ID = 1577484359;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageListOrderedItem> fetch(TlBufferParser &p);

  explicit pageListOrderedItemText(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageListOrderedItemBlocks final : public PageListOrderedItem {
 public:
  string num_;
  array<object_ptr<PageBlock>> blocks_;

  static const std::int32_t ID = -1730311882;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PageListOrderedItem> fetch(TlBufferParser &p);

  explicit pageListOrderedItemBlocks(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageRelatedArticle final : public Object {
 public:
  int32 flags_;
  string url_;
  int64 webpage_id_;
  string title_;
  string description_;
  int64 photo_id_;
  string author_;
  int32 published_date_;
  enum Flags : std::int32_t { TITLE_MASK = 1, DESCRIPTION_MASK = 2, PHOTO_ID_MASK = 4, AUTHOR_MASK = 8, PUBLISHED_DATE_MASK = 16 };

  pageRelatedArticle();

  static const std::int32_t ID = -1282352120;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<pageRelatedArticle> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageTableCell final : public Object {
 public:
  int32 flags_;
  bool header_;
  bool align_center_;
  bool align_right_;
  bool valign_middle_;
  bool valign_bottom_;
  object_ptr<RichText> text_;
  int32 colspan_;
  int32 rowspan_;
  enum Flags : std::int32_t { COLSPAN_MASK = 2, ROWSPAN_MASK = 4 };

  pageTableCell();

  static const std::int32_t ID = 878078826;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<pageTableCell> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pageTableRow final : public Object {
 public:
  array<object_ptr<pageTableCell>> cells_;

  static const std::int32_t ID = -524237339;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<pageTableRow> fetch(TlBufferParser &p);

  explicit pageTableRow(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PasswordKdfAlgo: public Object {
 public:

  static object_ptr<PasswordKdfAlgo> fetch(TlBufferParser &p);
};

class passwordKdfAlgoUnknown final : public PasswordKdfAlgo {
 public:

  static const std::int32_t ID = -732254058;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PasswordKdfAlgo> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow final : public PasswordKdfAlgo {
 public:
  bytes salt1_;
  bytes salt2_;
  int32 g_;
  bytes p_;

  passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow(bytes &&salt1_, bytes &&salt2_, int32 g_, bytes &&p_);

  static const std::int32_t ID = 982592842;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PasswordKdfAlgo> fetch(TlBufferParser &p);

  explicit passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class paymentCharge final : public Object {
 public:
  string id_;
  string provider_charge_id_;

  static const std::int32_t ID = -368917890;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<paymentCharge> fetch(TlBufferParser &p);

  explicit paymentCharge(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class paymentFormMethod final : public Object {
 public:
  string url_;
  string title_;

  static const std::int32_t ID = -1996951013;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<paymentFormMethod> fetch(TlBufferParser &p);

  explicit paymentFormMethod(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class paymentRequestedInfo final : public Object {
 public:
  int32 flags_;
  string name_;
  string phone_;
  string email_;
  object_ptr<postAddress> shipping_address_;
  enum Flags : std::int32_t { NAME_MASK = 1, PHONE_MASK = 2, EMAIL_MASK = 4, SHIPPING_ADDRESS_MASK = 8 };

  paymentRequestedInfo();

  paymentRequestedInfo(int32 flags_, string const &name_, string const &phone_, string const &email_, object_ptr<postAddress> &&shipping_address_);

  static const std::int32_t ID = -1868808300;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<paymentRequestedInfo> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class paymentSavedCredentialsCard final : public Object {
 public:
  string id_;
  string title_;

  static const std::int32_t ID = -842892769;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<paymentSavedCredentialsCard> fetch(TlBufferParser &p);

  explicit paymentSavedCredentialsCard(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class Peer: public Object {
 public:

  static object_ptr<Peer> fetch(TlBufferParser &p);
};

class peerUser final : public Peer {
 public:
  int64 user_id_;

  explicit peerUser(int64 user_id_);

  static const std::int32_t ID = 1498486562;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Peer> fetch(TlBufferParser &p);

  explicit peerUser(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class peerChat final : public Peer {
 public:
  int64 chat_id_;

  explicit peerChat(int64 chat_id_);

  static const std::int32_t ID = 918946202;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Peer> fetch(TlBufferParser &p);

  explicit peerChat(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class peerChannel final : public Peer {
 public:
  int64 channel_id_;

  static const std::int32_t ID = -1566230754;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Peer> fetch(TlBufferParser &p);

  explicit peerChannel(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class peerBlocked final : public Object {
 public:
  object_ptr<Peer> peer_id_;
  int32 date_;

  static const std::int32_t ID = -386039788;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<peerBlocked> fetch(TlBufferParser &p);

  explicit peerBlocked(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PeerLocated: public Object {
 public:

  static object_ptr<PeerLocated> fetch(TlBufferParser &p);
};

class peerLocated final : public PeerLocated {
 public:
  object_ptr<Peer> peer_;
  int32 expires_;
  int32 distance_;

  static const std::int32_t ID = -901375139;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PeerLocated> fetch(TlBufferParser &p);

  explicit peerLocated(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class peerSelfLocated final : public PeerLocated {
 public:
  int32 expires_;

  static const std::int32_t ID = -118740917;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PeerLocated> fetch(TlBufferParser &p);

  explicit peerSelfLocated(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class peerNotifySettings final : public Object {
 public:
  int32 flags_;
  bool show_previews_;
  bool silent_;
  int32 mute_until_;
  object_ptr<NotificationSound> ios_sound_;
  object_ptr<NotificationSound> android_sound_;
  object_ptr<NotificationSound> other_sound_;
  enum Flags : std::int32_t { SHOW_PREVIEWS_MASK = 1, SILENT_MASK = 2, MUTE_UNTIL_MASK = 4 };

  peerNotifySettings();

  static const std::int32_t ID = -1472527322;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<peerNotifySettings> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class peerSettings final : public Object {
 public:
  int32 flags_;
  bool report_spam_;
  bool add_contact_;
  bool block_contact_;
  bool share_contact_;
  bool need_contacts_exception_;
  bool report_geo_;
  bool autoarchived_;
  bool invite_members_;
  bool request_chat_broadcast_;
  int32 geo_distance_;
  string request_chat_title_;
  int32 request_chat_date_;
  enum Flags : std::int32_t { GEO_DISTANCE_MASK = 64, REQUEST_CHAT_TITLE_MASK = 512, REQUEST_CHAT_DATE_MASK = 512 };

  peerSettings();

  static const std::int32_t ID = -1525149427;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<peerSettings> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PhoneCall: public Object {
 public:

  static object_ptr<PhoneCall> fetch(TlBufferParser &p);
};

class phoneCallEmpty final : public PhoneCall {
 public:
  int64 id_;

  static const std::int32_t ID = 1399245077;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PhoneCall> fetch(TlBufferParser &p);

  explicit phoneCallEmpty(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class phoneCallWaiting final : public PhoneCall {
 public:
  int32 flags_;
  bool video_;
  int64 id_;
  int64 access_hash_;
  int32 date_;
  int64 admin_id_;
  int64 participant_id_;
  object_ptr<phoneCallProtocol> protocol_;
  int32 receive_date_;
  enum Flags : std::int32_t { RECEIVE_DATE_MASK = 1 };

  phoneCallWaiting();

  static const std::int32_t ID = -987599081;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PhoneCall> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class phoneCallRequested final : public PhoneCall {
 public:
  int32 flags_;
  bool video_;
  int64 id_;
  int64 access_hash_;
  int32 date_;
  int64 admin_id_;
  int64 participant_id_;
  bytes g_a_hash_;
  object_ptr<phoneCallProtocol> protocol_;

  phoneCallRequested();

  static const std::int32_t ID = 347139340;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PhoneCall> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class phoneCallAccepted final : public PhoneCall {
 public:
  int32 flags_;
  bool video_;
  int64 id_;
  int64 access_hash_;
  int32 date_;
  int64 admin_id_;
  int64 participant_id_;
  bytes g_b_;
  object_ptr<phoneCallProtocol> protocol_;

  phoneCallAccepted();

  static const std::int32_t ID = 912311057;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PhoneCall> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class phoneCall final : public PhoneCall {
 public:
  int32 flags_;
  bool p2p_allowed_;
  bool video_;
  int64 id_;
  int64 access_hash_;
  int32 date_;
  int64 admin_id_;
  int64 participant_id_;
  bytes g_a_or_b_;
  int64 key_fingerprint_;
  object_ptr<phoneCallProtocol> protocol_;
  array<object_ptr<PhoneConnection>> connections_;
  int32 start_date_;

  phoneCall();

  static const std::int32_t ID = -1770029977;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PhoneCall> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class phoneCallDiscarded final : public PhoneCall {
 public:
  int32 flags_;
  bool need_rating_;
  bool need_debug_;
  bool video_;
  int64 id_;
  object_ptr<PhoneCallDiscardReason> reason_;
  int32 duration_;
  enum Flags : std::int32_t { DURATION_MASK = 2 };

  phoneCallDiscarded();

  static const std::int32_t ID = 1355435489;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PhoneCall> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PhoneCallDiscardReason: public Object {
 public:

  static object_ptr<PhoneCallDiscardReason> fetch(TlBufferParser &p);
};

class phoneCallDiscardReasonMissed final : public PhoneCallDiscardReason {
 public:

  static const std::int32_t ID = -2048646399;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PhoneCallDiscardReason> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class phoneCallDiscardReasonDisconnect final : public PhoneCallDiscardReason {
 public:

  static const std::int32_t ID = -527056480;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PhoneCallDiscardReason> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class phoneCallDiscardReasonHangup final : public PhoneCallDiscardReason {
 public:

  static const std::int32_t ID = 1471006352;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PhoneCallDiscardReason> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class phoneCallDiscardReasonBusy final : public PhoneCallDiscardReason {
 public:

  static const std::int32_t ID = -84416311;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PhoneCallDiscardReason> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class phoneCallProtocol final : public Object {
 public:
  int32 flags_;
  bool udp_p2p_;
  bool udp_reflector_;
  int32 min_layer_;
  int32 max_layer_;
  array<string> library_versions_;
  enum Flags : std::int32_t { UDP_P2P_MASK = 1, UDP_REFLECTOR_MASK = 2 };

  phoneCallProtocol();

  phoneCallProtocol(int32 flags_, bool udp_p2p_, bool udp_reflector_, int32 min_layer_, int32 max_layer_, array<string> &&library_versions_);

  static const std::int32_t ID = -58224696;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<phoneCallProtocol> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PhoneConnection: public Object {
 public:

  static object_ptr<PhoneConnection> fetch(TlBufferParser &p);
};

class phoneConnection final : public PhoneConnection {
 public:
  int32 flags_;
  bool tcp_;
  int64 id_;
  string ip_;
  string ipv6_;
  int32 port_;
  bytes peer_tag_;

  phoneConnection();

  static const std::int32_t ID = -1665063993;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PhoneConnection> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class phoneConnectionWebrtc final : public PhoneConnection {
 public:
  int32 flags_;
  bool turn_;
  bool stun_;
  int64 id_;
  string ip_;
  string ipv6_;
  int32 port_;
  string username_;
  string password_;

  phoneConnectionWebrtc();

  static const std::int32_t ID = 1667228533;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PhoneConnection> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class Photo: public Object {
 public:

  static object_ptr<Photo> fetch(TlBufferParser &p);
};

class photoEmpty final : public Photo {
 public:
  int64 id_;

  static const std::int32_t ID = 590459437;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Photo> fetch(TlBufferParser &p);

  explicit photoEmpty(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class photo final : public Photo {
 public:
  int32 flags_;
  bool has_stickers_;
  int64 id_;
  int64 access_hash_;
  bytes file_reference_;
  int32 date_;
  array<object_ptr<PhotoSize>> sizes_;
  array<object_ptr<VideoSize>> video_sizes_;
  int32 dc_id_;

  photo();

  static const std::int32_t ID = -82216347;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Photo> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PhotoSize: public Object {
 public:

  static object_ptr<PhotoSize> fetch(TlBufferParser &p);
};

class photoSizeEmpty final : public PhotoSize {
 public:
  string type_;

  explicit photoSizeEmpty(string const &type_);

  static const std::int32_t ID = 236446268;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PhotoSize> fetch(TlBufferParser &p);

  explicit photoSizeEmpty(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class photoSize final : public PhotoSize {
 public:
  string type_;
  int32 w_;
  int32 h_;
  int32 size_;

  photoSize(string const &type_, int32 w_, int32 h_, int32 size_);

  static const std::int32_t ID = 1976012384;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PhotoSize> fetch(TlBufferParser &p);

  explicit photoSize(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class photoCachedSize final : public PhotoSize {
 public:
  string type_;
  int32 w_;
  int32 h_;
  bytes bytes_;

  photoCachedSize(string const &type_, int32 w_, int32 h_, bytes &&bytes_);

  static const std::int32_t ID = 35527382;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PhotoSize> fetch(TlBufferParser &p);

  explicit photoCachedSize(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class photoStrippedSize final : public PhotoSize {
 public:
  string type_;
  bytes bytes_;

  static const std::int32_t ID = -525288402;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PhotoSize> fetch(TlBufferParser &p);

  explicit photoStrippedSize(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class photoSizeProgressive final : public PhotoSize {
 public:
  string type_;
  int32 w_;
  int32 h_;
  array<int32> sizes_;

  static const std::int32_t ID = -96535659;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PhotoSize> fetch(TlBufferParser &p);

  explicit photoSizeProgressive(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class photoPathSize final : public PhotoSize {
 public:
  string type_;
  bytes bytes_;

  static const std::int32_t ID = -668906175;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PhotoSize> fetch(TlBufferParser &p);

  explicit photoPathSize(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class poll final : public Object {
 public:
  int64 id_;
  int32 flags_;
  bool closed_;
  bool public_voters_;
  bool multiple_choice_;
  bool quiz_;
  string question_;
  array<object_ptr<pollAnswer>> answers_;
  int32 close_period_;
  int32 close_date_;
  enum Flags : std::int32_t { CLOSED_MASK = 1, PUBLIC_VOTERS_MASK = 2, MULTIPLE_CHOICE_MASK = 4, QUIZ_MASK = 8, CLOSE_PERIOD_MASK = 16, CLOSE_DATE_MASK = 32 };

  poll();

  poll(int64 id_, int32 flags_, bool closed_, bool public_voters_, bool multiple_choice_, bool quiz_, string const &question_, array<object_ptr<pollAnswer>> &&answers_, int32 close_period_, int32 close_date_);

  static const std::int32_t ID = -2032041631;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<poll> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pollAnswer final : public Object {
 public:
  string text_;
  bytes option_;

  pollAnswer(string const &text_, bytes &&option_);

  static const std::int32_t ID = 1823064809;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<pollAnswer> fetch(TlBufferParser &p);

  explicit pollAnswer(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pollAnswerVoters final : public Object {
 public:
  int32 flags_;
  bool chosen_;
  bool correct_;
  bytes option_;
  int32 voters_;

  pollAnswerVoters();

  static const std::int32_t ID = 997055186;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<pollAnswerVoters> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class pollResults final : public Object {
 public:
  int32 flags_;
  bool min_;
  array<object_ptr<pollAnswerVoters>> results_;
  int32 total_voters_;
  array<int64> recent_voters_;
  string solution_;
  array<object_ptr<MessageEntity>> solution_entities_;
  enum Flags : std::int32_t { TOTAL_VOTERS_MASK = 4, SOLUTION_MASK = 16 };

  pollResults();

  static const std::int32_t ID = -591909213;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<pollResults> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class popularContact final : public Object {
 public:
  int64 client_id_;
  int32 importers_;

  static const std::int32_t ID = 1558266229;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<popularContact> fetch(TlBufferParser &p);

  explicit popularContact(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class postAddress final : public Object {
 public:
  string street_line1_;
  string street_line2_;
  string city_;
  string state_;
  string country_iso2_;
  string post_code_;

  postAddress(string const &street_line1_, string const &street_line2_, string const &city_, string const &state_, string const &country_iso2_, string const &post_code_);

  static const std::int32_t ID = 512535275;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<postAddress> fetch(TlBufferParser &p);

  explicit postAddress(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumGiftOption final : public Object {
 public:
  int32 flags_;
  int32 months_;
  string currency_;
  int64 amount_;
  string bot_url_;
  string store_product_;
  enum Flags : std::int32_t { STORE_PRODUCT_MASK = 1 };

  premiumGiftOption();

  static const std::int32_t ID = 1958953753;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<premiumGiftOption> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class premiumSubscriptionOption final : public Object {
 public:
  int32 flags_;
  bool current_;
  bool can_purchase_upgrade_;
  string transaction_;
  int32 months_;
  string currency_;
  int64 amount_;
  string bot_url_;
  string store_product_;
  enum Flags : std::int32_t { TRANSACTION_MASK = 8, STORE_PRODUCT_MASK = 1 };

  premiumSubscriptionOption();

  static const std::int32_t ID = 1596792306;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<premiumSubscriptionOption> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PrivacyKey: public Object {
 public:

  static object_ptr<PrivacyKey> fetch(TlBufferParser &p);
};

class privacyKeyStatusTimestamp final : public PrivacyKey {
 public:

  static const std::int32_t ID = -1137792208;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PrivacyKey> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class privacyKeyChatInvite final : public PrivacyKey {
 public:

  static const std::int32_t ID = 1343122938;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PrivacyKey> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class privacyKeyPhoneCall final : public PrivacyKey {
 public:

  static const std::int32_t ID = 1030105979;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PrivacyKey> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class privacyKeyPhoneP2P final : public PrivacyKey {
 public:

  static const std::int32_t ID = 961092808;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PrivacyKey> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class privacyKeyForwards final : public PrivacyKey {
 public:

  static const std::int32_t ID = 1777096355;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PrivacyKey> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class privacyKeyProfilePhoto final : public PrivacyKey {
 public:

  static const std::int32_t ID = -1777000467;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PrivacyKey> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class privacyKeyPhoneNumber final : public PrivacyKey {
 public:

  static const std::int32_t ID = -778378131;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PrivacyKey> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class privacyKeyAddedByPhone final : public PrivacyKey {
 public:

  static const std::int32_t ID = 1124062251;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PrivacyKey> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class privacyKeyVoiceMessages final : public PrivacyKey {
 public:

  static const std::int32_t ID = 110621716;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PrivacyKey> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class PrivacyRule: public Object {
 public:

  static object_ptr<PrivacyRule> fetch(TlBufferParser &p);
};

class privacyValueAllowContacts final : public PrivacyRule {
 public:

  static const std::int32_t ID = -123988;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PrivacyRule> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class privacyValueAllowAll final : public PrivacyRule {
 public:

  static const std::int32_t ID = 1698855810;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PrivacyRule> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class privacyValueAllowUsers final : public PrivacyRule {
 public:
  array<int64> users_;

  static const std::int32_t ID = -1198497870;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PrivacyRule> fetch(TlBufferParser &p);

  explicit privacyValueAllowUsers(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class privacyValueDisallowContacts final : public PrivacyRule {
 public:

  static const std::int32_t ID = -125240806;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PrivacyRule> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class privacyValueDisallowAll final : public PrivacyRule {
 public:

  static const std::int32_t ID = -1955338397;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PrivacyRule> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class privacyValueDisallowUsers final : public PrivacyRule {
 public:
  array<int64> users_;

  static const std::int32_t ID = -463335103;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PrivacyRule> fetch(TlBufferParser &p);

  explicit privacyValueDisallowUsers(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class privacyValueAllowChatParticipants final : public PrivacyRule {
 public:
  array<int64> chats_;

  static const std::int32_t ID = 1796427406;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PrivacyRule> fetch(TlBufferParser &p);

  explicit privacyValueAllowChatParticipants(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class privacyValueDisallowChatParticipants final : public PrivacyRule {
 public:
  array<int64> chats_;

  static const std::int32_t ID = 1103656293;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<PrivacyRule> fetch(TlBufferParser &p);

  explicit privacyValueDisallowChatParticipants(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class Reaction: public Object {
 public:

  static object_ptr<Reaction> fetch(TlBufferParser &p);
};

class reactionEmpty final : public Reaction {
 public:

  static const std::int32_t ID = 2046153753;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Reaction> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class reactionEmoji final : public Reaction {
 public:
  string emoticon_;

  explicit reactionEmoji(string const &emoticon_);

  static const std::int32_t ID = 455247544;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Reaction> fetch(TlBufferParser &p);

  explicit reactionEmoji(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class reactionCustomEmoji final : public Reaction {
 public:
  int64 document_id_;

  explicit reactionCustomEmoji(int64 document_id_);

  static const std::int32_t ID = -1992950669;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Reaction> fetch(TlBufferParser &p);

  explicit reactionCustomEmoji(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class reactionCount final : public Object {
 public:
  int32 flags_;
  int32 chosen_order_;
  object_ptr<Reaction> reaction_;
  int32 count_;
  enum Flags : std::int32_t { CHOSEN_ORDER_MASK = 1 };

  reactionCount();

  static const std::int32_t ID = -1546531968;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<reactionCount> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class readParticipantDate final : public Object {
 public:
  int64 user_id_;
  int32 date_;

  static const std::int32_t ID = 1246753138;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<readParticipantDate> fetch(TlBufferParser &p);

  explicit readParticipantDate(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class receivedNotifyMessage final : public Object {
 public:
  int32 id_;
  int32 flags_;

  static const std::int32_t ID = -1551583367;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<receivedNotifyMessage> fetch(TlBufferParser &p);

  explicit receivedNotifyMessage(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class RecentMeUrl: public Object {
 public:

  static object_ptr<RecentMeUrl> fetch(TlBufferParser &p);
};

class recentMeUrlUnknown final : public RecentMeUrl {
 public:
  string url_;

  static const std::int32_t ID = 1189204285;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RecentMeUrl> fetch(TlBufferParser &p);

  explicit recentMeUrlUnknown(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class recentMeUrlUser final : public RecentMeUrl {
 public:
  string url_;
  int64 user_id_;

  static const std::int32_t ID = -1188296222;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RecentMeUrl> fetch(TlBufferParser &p);

  explicit recentMeUrlUser(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class recentMeUrlChat final : public RecentMeUrl {
 public:
  string url_;
  int64 chat_id_;

  static const std::int32_t ID = -1294306862;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RecentMeUrl> fetch(TlBufferParser &p);

  explicit recentMeUrlChat(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class recentMeUrlChatInvite final : public RecentMeUrl {
 public:
  string url_;
  object_ptr<ChatInvite> chat_invite_;

  static const std::int32_t ID = -347535331;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RecentMeUrl> fetch(TlBufferParser &p);

  explicit recentMeUrlChatInvite(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class recentMeUrlStickerSet final : public RecentMeUrl {
 public:
  string url_;
  object_ptr<StickerSetCovered> set_;

  static const std::int32_t ID = -1140172836;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RecentMeUrl> fetch(TlBufferParser &p);

  explicit recentMeUrlStickerSet(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ReplyMarkup: public Object {
 public:

  static object_ptr<ReplyMarkup> fetch(TlBufferParser &p);
};

class replyKeyboardHide final : public ReplyMarkup {
 public:
  int32 flags_;
  bool selective_;
  enum Flags : std::int32_t { SELECTIVE_MASK = 4 };

  replyKeyboardHide();

  replyKeyboardHide(int32 flags_, bool selective_);

  static const std::int32_t ID = -1606526075;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ReplyMarkup> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class replyKeyboardForceReply final : public ReplyMarkup {
 public:
  int32 flags_;
  bool single_use_;
  bool selective_;
  string placeholder_;
  enum Flags : std::int32_t { SINGLE_USE_MASK = 2, SELECTIVE_MASK = 4, PLACEHOLDER_MASK = 8 };

  replyKeyboardForceReply();

  replyKeyboardForceReply(int32 flags_, bool single_use_, bool selective_, string const &placeholder_);

  static const std::int32_t ID = -2035021048;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ReplyMarkup> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class replyKeyboardMarkup final : public ReplyMarkup {
 public:
  int32 flags_;
  bool resize_;
  bool single_use_;
  bool selective_;
  bool persistent_;
  array<object_ptr<keyboardButtonRow>> rows_;
  string placeholder_;
  enum Flags : std::int32_t { RESIZE_MASK = 1, SINGLE_USE_MASK = 2, SELECTIVE_MASK = 4, PERSISTENT_MASK = 16, PLACEHOLDER_MASK = 8 };

  replyKeyboardMarkup();

  replyKeyboardMarkup(int32 flags_, bool resize_, bool single_use_, bool selective_, bool persistent_, array<object_ptr<keyboardButtonRow>> &&rows_, string const &placeholder_);

  static const std::int32_t ID = -2049074735;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ReplyMarkup> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class replyInlineMarkup final : public ReplyMarkup {
 public:
  array<object_ptr<keyboardButtonRow>> rows_;

  explicit replyInlineMarkup(array<object_ptr<keyboardButtonRow>> &&rows_);

  static const std::int32_t ID = 1218642516;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<ReplyMarkup> fetch(TlBufferParser &p);

  explicit replyInlineMarkup(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class ReportReason: public Object {
 public:
};

class inputReportReasonSpam final : public ReportReason {
 public:

  static const std::int32_t ID = 1490799288;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputReportReasonViolence final : public ReportReason {
 public:

  static const std::int32_t ID = 505595789;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputReportReasonPornography final : public ReportReason {
 public:

  static const std::int32_t ID = 777640226;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputReportReasonChildAbuse final : public ReportReason {
 public:

  static const std::int32_t ID = -1376497949;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputReportReasonOther final : public ReportReason {
 public:

  static const std::int32_t ID = -1041980751;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputReportReasonCopyright final : public ReportReason {
 public:

  static const std::int32_t ID = -1685456582;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputReportReasonGeoIrrelevant final : public ReportReason {
 public:

  static const std::int32_t ID = -606798099;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputReportReasonFake final : public ReportReason {
 public:

  static const std::int32_t ID = -170010905;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputReportReasonIllegalDrugs final : public ReportReason {
 public:

  static const std::int32_t ID = 177124030;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class inputReportReasonPersonalDetails final : public ReportReason {
 public:

  static const std::int32_t ID = -1631091139;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class RequestPeerType: public Object {
 public:

  static object_ptr<RequestPeerType> fetch(TlBufferParser &p);
};

class requestPeerTypeUser final : public RequestPeerType {
 public:
  int32 flags_;
  bool bot_;
  bool premium_;
  enum Flags : std::int32_t { BOT_MASK = 1, PREMIUM_MASK = 2 };

  requestPeerTypeUser();

  requestPeerTypeUser(int32 flags_, bool bot_, bool premium_);

  static const std::int32_t ID = 1597737472;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RequestPeerType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class requestPeerTypeChat final : public RequestPeerType {
 public:
  int32 flags_;
  bool creator_;
  bool bot_participant_;
  bool has_username_;
  bool forum_;
  object_ptr<chatAdminRights> user_admin_rights_;
  object_ptr<chatAdminRights> bot_admin_rights_;
  enum Flags : std::int32_t { CREATOR_MASK = 1, BOT_PARTICIPANT_MASK = 32, HAS_USERNAME_MASK = 8, FORUM_MASK = 16, USER_ADMIN_RIGHTS_MASK = 2, BOT_ADMIN_RIGHTS_MASK = 4 };

  requestPeerTypeChat();

  requestPeerTypeChat(int32 flags_, bool creator_, bool bot_participant_, bool has_username_, bool forum_, object_ptr<chatAdminRights> &&user_admin_rights_, object_ptr<chatAdminRights> &&bot_admin_rights_);

  static const std::int32_t ID = -906990053;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RequestPeerType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class requestPeerTypeBroadcast final : public RequestPeerType {
 public:
  int32 flags_;
  bool creator_;
  bool has_username_;
  object_ptr<chatAdminRights> user_admin_rights_;
  object_ptr<chatAdminRights> bot_admin_rights_;
  enum Flags : std::int32_t { CREATOR_MASK = 1, HAS_USERNAME_MASK = 8, USER_ADMIN_RIGHTS_MASK = 2, BOT_ADMIN_RIGHTS_MASK = 4 };

  requestPeerTypeBroadcast();

  requestPeerTypeBroadcast(int32 flags_, bool creator_, bool has_username_, object_ptr<chatAdminRights> &&user_admin_rights_, object_ptr<chatAdminRights> &&bot_admin_rights_);

  static const std::int32_t ID = 865857388;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RequestPeerType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class restrictionReason final : public Object {
 public:
  string platform_;
  string reason_;
  string text_;

  static const std::int32_t ID = -797791052;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<restrictionReason> fetch(TlBufferParser &p);

  explicit restrictionReason(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class RichText: public Object {
 public:

  static object_ptr<RichText> fetch(TlBufferParser &p);
};

class textEmpty final : public RichText {
 public:

  static const std::int32_t ID = -599948721;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RichText> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textPlain final : public RichText {
 public:
  string text_;

  static const std::int32_t ID = 1950782688;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RichText> fetch(TlBufferParser &p);

  explicit textPlain(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textBold final : public RichText {
 public:
  object_ptr<RichText> text_;

  static const std::int32_t ID = 1730456516;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RichText> fetch(TlBufferParser &p);

  explicit textBold(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textItalic final : public RichText {
 public:
  object_ptr<RichText> text_;

  static const std::int32_t ID = -653089380;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RichText> fetch(TlBufferParser &p);

  explicit textItalic(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textUnderline final : public RichText {
 public:
  object_ptr<RichText> text_;

  static const std::int32_t ID = -1054465340;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RichText> fetch(TlBufferParser &p);

  explicit textUnderline(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textStrike final : public RichText {
 public:
  object_ptr<RichText> text_;

  static const std::int32_t ID = -1678197867;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RichText> fetch(TlBufferParser &p);

  explicit textStrike(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textFixed final : public RichText {
 public:
  object_ptr<RichText> text_;

  static const std::int32_t ID = 1816074681;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RichText> fetch(TlBufferParser &p);

  explicit textFixed(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textUrl final : public RichText {
 public:
  object_ptr<RichText> text_;
  string url_;
  int64 webpage_id_;

  static const std::int32_t ID = 1009288385;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RichText> fetch(TlBufferParser &p);

  explicit textUrl(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textEmail final : public RichText {
 public:
  object_ptr<RichText> text_;
  string email_;

  static const std::int32_t ID = -564523562;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RichText> fetch(TlBufferParser &p);

  explicit textEmail(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textConcat final : public RichText {
 public:
  array<object_ptr<RichText>> texts_;

  static const std::int32_t ID = 2120376535;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RichText> fetch(TlBufferParser &p);

  explicit textConcat(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textSubscript final : public RichText {
 public:
  object_ptr<RichText> text_;

  static const std::int32_t ID = -311786236;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RichText> fetch(TlBufferParser &p);

  explicit textSubscript(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textSuperscript final : public RichText {
 public:
  object_ptr<RichText> text_;

  static const std::int32_t ID = -939827711;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RichText> fetch(TlBufferParser &p);

  explicit textSuperscript(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textMarked final : public RichText {
 public:
  object_ptr<RichText> text_;

  static const std::int32_t ID = 55281185;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RichText> fetch(TlBufferParser &p);

  explicit textMarked(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textPhone final : public RichText {
 public:
  object_ptr<RichText> text_;
  string phone_;

  static const std::int32_t ID = 483104362;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RichText> fetch(TlBufferParser &p);

  explicit textPhone(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textImage final : public RichText {
 public:
  int64 document_id_;
  int32 w_;
  int32 h_;

  static const std::int32_t ID = 136105807;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RichText> fetch(TlBufferParser &p);

  explicit textImage(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textAnchor final : public RichText {
 public:
  object_ptr<RichText> text_;
  string name_;

  static const std::int32_t ID = 894777186;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<RichText> fetch(TlBufferParser &p);

  explicit textAnchor(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class savedPhoneContact final : public Object {
 public:
  string phone_;
  string first_name_;
  string last_name_;
  int32 date_;

  static const std::int32_t ID = 289586518;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<savedPhoneContact> fetch(TlBufferParser &p);

  explicit savedPhoneContact(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchResultsCalendarPeriod final : public Object {
 public:
  int32 date_;
  int32 min_msg_id_;
  int32 max_msg_id_;
  int32 count_;

  static const std::int32_t ID = -911191137;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<searchResultsCalendarPeriod> fetch(TlBufferParser &p);

  explicit searchResultsCalendarPeriod(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class searchResultPosition final : public Object {
 public:
  int32 msg_id_;
  int32 date_;
  int32 offset_;

  static const std::int32_t ID = 2137295719;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<searchResultPosition> fetch(TlBufferParser &p);

  explicit searchResultPosition(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureCredentialsEncrypted final : public Object {
 public:
  bytes data_;
  bytes hash_;
  bytes secret_;

  secureCredentialsEncrypted(bytes &&data_, bytes &&hash_, bytes &&secret_);

  static const std::int32_t ID = 871426631;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<secureCredentialsEncrypted> fetch(TlBufferParser &p);

  explicit secureCredentialsEncrypted(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureData final : public Object {
 public:
  bytes data_;
  bytes data_hash_;
  bytes secret_;

  secureData(bytes &&data_, bytes &&data_hash_, bytes &&secret_);

  static const std::int32_t ID = -1964327229;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<secureData> fetch(TlBufferParser &p);

  explicit secureData(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class SecureFile: public Object {
 public:

  static object_ptr<SecureFile> fetch(TlBufferParser &p);
};

class secureFileEmpty final : public SecureFile {
 public:

  static const std::int32_t ID = 1679398724;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureFile> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureFile final : public SecureFile {
 public:
  int64 id_;
  int64 access_hash_;
  int64 size_;
  int32 dc_id_;
  int32 date_;
  bytes file_hash_;
  bytes secret_;

  static const std::int32_t ID = 2097791614;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureFile> fetch(TlBufferParser &p);

  explicit secureFile(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class SecurePasswordKdfAlgo: public Object {
 public:

  static object_ptr<SecurePasswordKdfAlgo> fetch(TlBufferParser &p);
};

class securePasswordKdfAlgoUnknown final : public SecurePasswordKdfAlgo {
 public:

  static const std::int32_t ID = 4883767;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecurePasswordKdfAlgo> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class securePasswordKdfAlgoPBKDF2HMACSHA512iter100000 final : public SecurePasswordKdfAlgo {
 public:
  bytes salt_;

  explicit securePasswordKdfAlgoPBKDF2HMACSHA512iter100000(bytes &&salt_);

  static const std::int32_t ID = -1141711456;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecurePasswordKdfAlgo> fetch(TlBufferParser &p);

  explicit securePasswordKdfAlgoPBKDF2HMACSHA512iter100000(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class securePasswordKdfAlgoSHA512 final : public SecurePasswordKdfAlgo {
 public:
  bytes salt_;

  explicit securePasswordKdfAlgoSHA512(bytes &&salt_);

  static const std::int32_t ID = -2042159726;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecurePasswordKdfAlgo> fetch(TlBufferParser &p);

  explicit securePasswordKdfAlgoSHA512(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class SecurePlainData: public Object {
 public:

  static object_ptr<SecurePlainData> fetch(TlBufferParser &p);
};

class securePlainPhone final : public SecurePlainData {
 public:
  string phone_;

  explicit securePlainPhone(string const &phone_);

  static const std::int32_t ID = 2103482845;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecurePlainData> fetch(TlBufferParser &p);

  explicit securePlainPhone(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class securePlainEmail final : public SecurePlainData {
 public:
  string email_;

  explicit securePlainEmail(string const &email_);

  static const std::int32_t ID = 569137759;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecurePlainData> fetch(TlBufferParser &p);

  explicit securePlainEmail(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class SecureRequiredType: public Object {
 public:

  static object_ptr<SecureRequiredType> fetch(TlBufferParser &p);
};

class secureRequiredType final : public SecureRequiredType {
 public:
  int32 flags_;
  bool native_names_;
  bool selfie_required_;
  bool translation_required_;
  object_ptr<SecureValueType> type_;

  secureRequiredType();

  static const std::int32_t ID = -2103600678;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureRequiredType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureRequiredTypeOneOf final : public SecureRequiredType {
 public:
  array<object_ptr<SecureRequiredType>> types_;

  static const std::int32_t ID = 41187252;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureRequiredType> fetch(TlBufferParser &p);

  explicit secureRequiredTypeOneOf(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureSecretSettings final : public Object {
 public:
  object_ptr<SecurePasswordKdfAlgo> secure_algo_;
  bytes secure_secret_;
  int64 secure_secret_id_;

  secureSecretSettings(object_ptr<SecurePasswordKdfAlgo> &&secure_algo_, bytes &&secure_secret_, int64 secure_secret_id_);

  static const std::int32_t ID = 354925740;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<secureSecretSettings> fetch(TlBufferParser &p);

  explicit secureSecretSettings(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValue final : public Object {
 public:
  int32 flags_;
  object_ptr<SecureValueType> type_;
  object_ptr<secureData> data_;
  object_ptr<SecureFile> front_side_;
  object_ptr<SecureFile> reverse_side_;
  object_ptr<SecureFile> selfie_;
  array<object_ptr<SecureFile>> translation_;
  array<object_ptr<SecureFile>> files_;
  object_ptr<SecurePlainData> plain_data_;
  bytes hash_;

  secureValue();

  static const std::int32_t ID = 411017418;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<secureValue> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class SecureValueError: public Object {
 public:

  static object_ptr<SecureValueError> fetch(TlBufferParser &p);
};

class secureValueErrorData final : public SecureValueError {
 public:
  object_ptr<SecureValueType> type_;
  bytes data_hash_;
  string field_;
  string text_;

  secureValueErrorData(object_ptr<SecureValueType> &&type_, bytes &&data_hash_, string const &field_, string const &text_);

  static const std::int32_t ID = -391902247;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueError> fetch(TlBufferParser &p);

  explicit secureValueErrorData(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueErrorFrontSide final : public SecureValueError {
 public:
  object_ptr<SecureValueType> type_;
  bytes file_hash_;
  string text_;

  secureValueErrorFrontSide(object_ptr<SecureValueType> &&type_, bytes &&file_hash_, string const &text_);

  static const std::int32_t ID = 12467706;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueError> fetch(TlBufferParser &p);

  explicit secureValueErrorFrontSide(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueErrorReverseSide final : public SecureValueError {
 public:
  object_ptr<SecureValueType> type_;
  bytes file_hash_;
  string text_;

  secureValueErrorReverseSide(object_ptr<SecureValueType> &&type_, bytes &&file_hash_, string const &text_);

  static const std::int32_t ID = -2037765467;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueError> fetch(TlBufferParser &p);

  explicit secureValueErrorReverseSide(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueErrorSelfie final : public SecureValueError {
 public:
  object_ptr<SecureValueType> type_;
  bytes file_hash_;
  string text_;

  secureValueErrorSelfie(object_ptr<SecureValueType> &&type_, bytes &&file_hash_, string const &text_);

  static const std::int32_t ID = -449327402;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueError> fetch(TlBufferParser &p);

  explicit secureValueErrorSelfie(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueErrorFile final : public SecureValueError {
 public:
  object_ptr<SecureValueType> type_;
  bytes file_hash_;
  string text_;

  secureValueErrorFile(object_ptr<SecureValueType> &&type_, bytes &&file_hash_, string const &text_);

  static const std::int32_t ID = 2054162547;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueError> fetch(TlBufferParser &p);

  explicit secureValueErrorFile(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueErrorFiles final : public SecureValueError {
 public:
  object_ptr<SecureValueType> type_;
  array<bytes> file_hash_;
  string text_;

  secureValueErrorFiles(object_ptr<SecureValueType> &&type_, array<bytes> &&file_hash_, string const &text_);

  static const std::int32_t ID = 1717706985;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueError> fetch(TlBufferParser &p);

  explicit secureValueErrorFiles(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueError final : public SecureValueError {
 public:
  object_ptr<SecureValueType> type_;
  bytes hash_;
  string text_;

  secureValueError(object_ptr<SecureValueType> &&type_, bytes &&hash_, string const &text_);

  static const std::int32_t ID = -2036501105;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueError> fetch(TlBufferParser &p);

  explicit secureValueError(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueErrorTranslationFile final : public SecureValueError {
 public:
  object_ptr<SecureValueType> type_;
  bytes file_hash_;
  string text_;

  secureValueErrorTranslationFile(object_ptr<SecureValueType> &&type_, bytes &&file_hash_, string const &text_);

  static const std::int32_t ID = -1592506512;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueError> fetch(TlBufferParser &p);

  explicit secureValueErrorTranslationFile(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueErrorTranslationFiles final : public SecureValueError {
 public:
  object_ptr<SecureValueType> type_;
  array<bytes> file_hash_;
  string text_;

  secureValueErrorTranslationFiles(object_ptr<SecureValueType> &&type_, array<bytes> &&file_hash_, string const &text_);

  static const std::int32_t ID = 878931416;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueError> fetch(TlBufferParser &p);

  explicit secureValueErrorTranslationFiles(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueHash final : public Object {
 public:
  object_ptr<SecureValueType> type_;
  bytes hash_;

  secureValueHash(object_ptr<SecureValueType> &&type_, bytes &&hash_);

  static const std::int32_t ID = -316748368;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class SecureValueType: public Object {
 public:

  static object_ptr<SecureValueType> fetch(TlBufferParser &p);
};

class secureValueTypePersonalDetails final : public SecureValueType {
 public:

  static const std::int32_t ID = -1658158621;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueTypePassport final : public SecureValueType {
 public:

  static const std::int32_t ID = 1034709504;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueTypeDriverLicense final : public SecureValueType {
 public:

  static const std::int32_t ID = 115615172;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueTypeIdentityCard final : public SecureValueType {
 public:

  static const std::int32_t ID = -1596951477;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueTypeInternalPassport final : public SecureValueType {
 public:

  static const std::int32_t ID = -1717268701;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueTypeAddress final : public SecureValueType {
 public:

  static const std::int32_t ID = -874308058;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueTypeUtilityBill final : public SecureValueType {
 public:

  static const std::int32_t ID = -63531698;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueTypeBankStatement final : public SecureValueType {
 public:

  static const std::int32_t ID = -1995211763;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueTypeRentalAgreement final : public SecureValueType {
 public:

  static const std::int32_t ID = -1954007928;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueTypePassportRegistration final : public SecureValueType {
 public:

  static const std::int32_t ID = -1713143702;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueTypeTemporaryRegistration final : public SecureValueType {
 public:

  static const std::int32_t ID = -368907213;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueTypePhone final : public SecureValueType {
 public:

  static const std::int32_t ID = -1289704741;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class secureValueTypeEmail final : public SecureValueType {
 public:

  static const std::int32_t ID = -1908627474;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SecureValueType> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendAsPeer final : public Object {
 public:
  int32 flags_;
  bool premium_required_;
  object_ptr<Peer> peer_;

  sendAsPeer();

  static const std::int32_t ID = -1206095820;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<sendAsPeer> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class SendMessageAction: public Object {
 public:

  static object_ptr<SendMessageAction> fetch(TlBufferParser &p);
};

class sendMessageTypingAction final : public SendMessageAction {
 public:

  static const std::int32_t ID = 381645902;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SendMessageAction> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendMessageCancelAction final : public SendMessageAction {
 public:

  static const std::int32_t ID = -44119819;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SendMessageAction> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendMessageRecordVideoAction final : public SendMessageAction {
 public:

  static const std::int32_t ID = -1584933265;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SendMessageAction> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendMessageUploadVideoAction final : public SendMessageAction {
 public:
  int32 progress_;

  explicit sendMessageUploadVideoAction(int32 progress_);

  static const std::int32_t ID = -378127636;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SendMessageAction> fetch(TlBufferParser &p);

  explicit sendMessageUploadVideoAction(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendMessageRecordAudioAction final : public SendMessageAction {
 public:

  static const std::int32_t ID = -718310409;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SendMessageAction> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendMessageUploadAudioAction final : public SendMessageAction {
 public:
  int32 progress_;

  explicit sendMessageUploadAudioAction(int32 progress_);

  static const std::int32_t ID = -212740181;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SendMessageAction> fetch(TlBufferParser &p);

  explicit sendMessageUploadAudioAction(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendMessageUploadPhotoAction final : public SendMessageAction {
 public:
  int32 progress_;

  explicit sendMessageUploadPhotoAction(int32 progress_);

  static const std::int32_t ID = -774682074;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SendMessageAction> fetch(TlBufferParser &p);

  explicit sendMessageUploadPhotoAction(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendMessageUploadDocumentAction final : public SendMessageAction {
 public:
  int32 progress_;

  explicit sendMessageUploadDocumentAction(int32 progress_);

  static const std::int32_t ID = -1441998364;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SendMessageAction> fetch(TlBufferParser &p);

  explicit sendMessageUploadDocumentAction(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendMessageGeoLocationAction final : public SendMessageAction {
 public:

  static const std::int32_t ID = 393186209;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SendMessageAction> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendMessageChooseContactAction final : public SendMessageAction {
 public:

  static const std::int32_t ID = 1653390447;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SendMessageAction> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendMessageGamePlayAction final : public SendMessageAction {
 public:

  static const std::int32_t ID = -580219064;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SendMessageAction> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendMessageRecordRoundAction final : public SendMessageAction {
 public:

  static const std::int32_t ID = -1997373508;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SendMessageAction> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendMessageUploadRoundAction final : public SendMessageAction {
 public:
  int32 progress_;

  explicit sendMessageUploadRoundAction(int32 progress_);

  static const std::int32_t ID = 608050278;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SendMessageAction> fetch(TlBufferParser &p);

  explicit sendMessageUploadRoundAction(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class speakingInGroupCallAction final : public SendMessageAction {
 public:

  static const std::int32_t ID = -651419003;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SendMessageAction> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendMessageHistoryImportAction final : public SendMessageAction {
 public:
  int32 progress_;

  explicit sendMessageHistoryImportAction(int32 progress_);

  static const std::int32_t ID = -606432698;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SendMessageAction> fetch(TlBufferParser &p);

  explicit sendMessageHistoryImportAction(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendMessageChooseStickerAction final : public SendMessageAction {
 public:

  static const std::int32_t ID = -1336228175;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SendMessageAction> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendMessageEmojiInteraction final : public SendMessageAction {
 public:
  string emoticon_;
  int32 msg_id_;
  object_ptr<dataJSON> interaction_;

  sendMessageEmojiInteraction(string const &emoticon_, int32 msg_id_, object_ptr<dataJSON> &&interaction_);

  static const std::int32_t ID = 630664139;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SendMessageAction> fetch(TlBufferParser &p);

  explicit sendMessageEmojiInteraction(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sendMessageEmojiInteractionSeen final : public SendMessageAction {
 public:
  string emoticon_;

  explicit sendMessageEmojiInteractionSeen(string const &emoticon_);

  static const std::int32_t ID = -1234857938;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<SendMessageAction> fetch(TlBufferParser &p);

  explicit sendMessageEmojiInteractionSeen(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class shippingOption final : public Object {
 public:
  string id_;
  string title_;
  array<object_ptr<labeledPrice>> prices_;

  shippingOption(string const &id_, string const &title_, array<object_ptr<labeledPrice>> &&prices_);

  static const std::int32_t ID = -1239335713;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<shippingOption> fetch(TlBufferParser &p);

  explicit shippingOption(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class simpleWebViewResultUrl final : public Object {
 public:
  string url_;

  static const std::int32_t ID = -2010155333;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<simpleWebViewResultUrl> fetch(TlBufferParser &p);

  explicit simpleWebViewResultUrl(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class sponsoredMessage final : public Object {
 public:
  int32 flags_;
  bool recommended_;
  bool show_peer_photo_;
  bytes random_id_;
  object_ptr<Peer> from_id_;
  object_ptr<ChatInvite> chat_invite_;
  string chat_invite_hash_;
  int32 channel_post_;
  string start_param_;
  string message_;
  array<object_ptr<MessageEntity>> entities_;
  string sponsor_info_;
  string additional_info_;
  enum Flags : std::int32_t { CHAT_INVITE_HASH_MASK = 16, CHANNEL_POST_MASK = 4, START_PARAM_MASK = 1, SPONSOR_INFO_MASK = 128, ADDITIONAL_INFO_MASK = 256 };

  sponsoredMessage();

  static const std::int32_t ID = -64636888;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<sponsoredMessage> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class statsAbsValueAndPrev final : public Object {
 public:
  double current_;
  double previous_;

  static const std::int32_t ID = -884757282;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<statsAbsValueAndPrev> fetch(TlBufferParser &p);

  explicit statsAbsValueAndPrev(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class statsDateRangeDays final : public Object {
 public:
  int32 min_date_;
  int32 max_date_;

  static const std::int32_t ID = -1237848657;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<statsDateRangeDays> fetch(TlBufferParser &p);

  explicit statsDateRangeDays(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class StatsGraph: public Object {
 public:

  static object_ptr<StatsGraph> fetch(TlBufferParser &p);
};

class statsGraphAsync final : public StatsGraph {
 public:
  string token_;

  static const std::int32_t ID = 1244130093;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<StatsGraph> fetch(TlBufferParser &p);

  explicit statsGraphAsync(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class statsGraphError final : public StatsGraph {
 public:
  string error_;

  static const std::int32_t ID = -1092839390;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<StatsGraph> fetch(TlBufferParser &p);

  explicit statsGraphError(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class statsGraph final : public StatsGraph {
 public:
  int32 flags_;
  object_ptr<dataJSON> json_;
  string zoom_token_;
  enum Flags : std::int32_t { ZOOM_TOKEN_MASK = 1 };

  statsGraph();

  static const std::int32_t ID = -1901828938;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<StatsGraph> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class statsGroupTopAdmin final : public Object {
 public:
  int64 user_id_;
  int32 deleted_;
  int32 kicked_;
  int32 banned_;

  static const std::int32_t ID = -682079097;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<statsGroupTopAdmin> fetch(TlBufferParser &p);

  explicit statsGroupTopAdmin(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class statsGroupTopInviter final : public Object {
 public:
  int64 user_id_;
  int32 invitations_;

  static const std::int32_t ID = 1398765469;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<statsGroupTopInviter> fetch(TlBufferParser &p);

  explicit statsGroupTopInviter(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class statsGroupTopPoster final : public Object {
 public:
  int64 user_id_;
  int32 messages_;
  int32 avg_chars_;

  static const std::int32_t ID = -1660637285;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<statsGroupTopPoster> fetch(TlBufferParser &p);

  explicit statsGroupTopPoster(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class statsPercentValue final : public Object {
 public:
  double part_;
  double total_;

  static const std::int32_t ID = -875679776;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<statsPercentValue> fetch(TlBufferParser &p);

  explicit statsPercentValue(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class statsURL final : public Object {
 public:
  string url_;

  static const std::int32_t ID = 1202287072;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerToString &s, const char *field_name) const final;
};

class stickerKeyword final : public Object {
 public:
  int64 document_id_;
  array<string> keyword_;

  static const std::int32_t ID = -50416996;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<stickerKeyword> fetch(TlBufferParser &p);

  explicit stickerKeyword(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class stickerPack final : public Object {
 public:
  string emoticon_;
  array<int64> documents_;

  static const std::int32_t ID = 313694676;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<stickerPack> fetch(TlBufferParser &p);

  explicit stickerPack(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class stickerSet final : public Object {
 public:
  int32 flags_;
  bool archived_;
  bool official_;
  bool masks_;
  bool animated_;
  bool videos_;
  bool emojis_;
  int32 installed_date_;
  int64 id_;
  int64 access_hash_;
  string title_;
  string short_name_;
  array<object_ptr<PhotoSize>> thumbs_;
  int32 thumb_dc_id_;
  int32 thumb_version_;
  int64 thumb_document_id_;
  int32 count_;
  int32 hash_;
  enum Flags : std::int32_t { INSTALLED_DATE_MASK = 1, THUMB_DC_ID_MASK = 16, THUMB_VERSION_MASK = 16, THUMB_DOCUMENT_ID_MASK = 256 };

  stickerSet();

  static const std::int32_t ID = 768691932;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<stickerSet> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class StickerSetCovered: public Object {
 public:

  static object_ptr<StickerSetCovered> fetch(TlBufferParser &p);
};

class stickerSetCovered final : public StickerSetCovered {
 public:
  object_ptr<stickerSet> set_;
  object_ptr<Document> cover_;

  static const std::int32_t ID = 1678812626;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<StickerSetCovered> fetch(TlBufferParser &p);

  explicit stickerSetCovered(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class stickerSetMultiCovered final : public StickerSetCovered {
 public:
  object_ptr<stickerSet> set_;
  array<object_ptr<Document>> covers_;

  static const std::int32_t ID = 872932635;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<StickerSetCovered> fetch(TlBufferParser &p);

  explicit stickerSetMultiCovered(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class stickerSetFullCovered final : public StickerSetCovered {
 public:
  object_ptr<stickerSet> set_;
  array<object_ptr<stickerPack>> packs_;
  array<object_ptr<stickerKeyword>> keywords_;
  array<object_ptr<Document>> documents_;

  static const std::int32_t ID = 1087454222;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<StickerSetCovered> fetch(TlBufferParser &p);

  explicit stickerSetFullCovered(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class stickerSetNoCovered final : public StickerSetCovered {
 public:
  object_ptr<stickerSet> set_;

  static const std::int32_t ID = 2008112412;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<StickerSetCovered> fetch(TlBufferParser &p);

  explicit stickerSetNoCovered(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class textWithEntities final : public Object {
 public:
  string text_;
  array<object_ptr<MessageEntity>> entities_;

  textWithEntities(string const &text_, array<object_ptr<MessageEntity>> &&entities_);

  static const std::int32_t ID = 1964978502;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<textWithEntities> fetch(TlBufferParser &p);

  explicit textWithEntities(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class theme final : public Object {
 public:
  int32 flags_;
  bool creator_;
  bool default_;
  bool for_chat_;
  int64 id_;
  int64 access_hash_;
  string slug_;
  string title_;
  object_ptr<Document> document_;
  array<object_ptr<themeSettings>> settings_;
  string emoticon_;
  int32 installs_count_;
  enum Flags : std::int32_t { EMOTICON_MASK = 64, INSTALLS_COUNT_MASK = 16 };

  theme();

  static const std::int32_t ID = -1609668650;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<theme> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class themeSettings final : public Object {
 public:
  int32 flags_;
  bool message_colors_animated_;
  object_ptr<BaseTheme> base_theme_;
  int32 accent_color_;
  int32 outbox_accent_color_;
  array<int32> message_colors_;
  object_ptr<WallPaper> wallpaper_;
  enum Flags : std::int32_t { OUTBOX_ACCENT_COLOR_MASK = 8 };

  themeSettings();

  static const std::int32_t ID = -94849324;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<themeSettings> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class topPeer final : public Object {
 public:
  object_ptr<Peer> peer_;
  double rating_;

  static const std::int32_t ID = -305282981;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<topPeer> fetch(TlBufferParser &p);

  explicit topPeer(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class TopPeerCategory: public Object {
 public:

  static object_ptr<TopPeerCategory> fetch(TlBufferParser &p);
};

class topPeerCategoryBotsPM final : public TopPeerCategory {
 public:

  static const std::int32_t ID = -1419371685;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<TopPeerCategory> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class topPeerCategoryBotsInline final : public TopPeerCategory {
 public:

  static const std::int32_t ID = 344356834;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<TopPeerCategory> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class topPeerCategoryCorrespondents final : public TopPeerCategory {
 public:

  static const std::int32_t ID = 104314861;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<TopPeerCategory> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class topPeerCategoryGroups final : public TopPeerCategory {
 public:

  static const std::int32_t ID = -1122524854;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<TopPeerCategory> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class topPeerCategoryChannels final : public TopPeerCategory {
 public:

  static const std::int32_t ID = 371037736;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<TopPeerCategory> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class topPeerCategoryPhoneCalls final : public TopPeerCategory {
 public:

  static const std::int32_t ID = 511092620;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<TopPeerCategory> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class topPeerCategoryForwardUsers final : public TopPeerCategory {
 public:

  static const std::int32_t ID = -1472172887;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<TopPeerCategory> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class topPeerCategoryForwardChats final : public TopPeerCategory {
 public:

  static const std::int32_t ID = -68239120;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<TopPeerCategory> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class topPeerCategoryPeers final : public Object {
 public:
  object_ptr<TopPeerCategory> category_;
  int32 count_;
  array<object_ptr<topPeer>> peers_;

  static const std::int32_t ID = -75283823;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<topPeerCategoryPeers> fetch(TlBufferParser &p);

  explicit topPeerCategoryPeers(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class Update: public Object {
 public:

  static object_ptr<Update> fetch(TlBufferParser &p);
};

class updateNewMessage final : public Update {
 public:
  object_ptr<Message> message_;
  int32 pts_;
  int32 pts_count_;

  updateNewMessage(object_ptr<Message> &&message_, int32 pts_, int32 pts_count_);

  static const std::int32_t ID = 522914557;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateNewMessage(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateMessageID final : public Update {
 public:
  int32 id_;
  int64 random_id_;

  static const std::int32_t ID = 1318109142;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateMessageID(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateDeleteMessages final : public Update {
 public:
  array<int32> messages_;
  int32 pts_;
  int32 pts_count_;

  updateDeleteMessages(array<int32> &&messages_, int32 pts_, int32 pts_count_);

  static const std::int32_t ID = -1576161051;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateDeleteMessages(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateUserTyping final : public Update {
 public:
  int64 user_id_;
  object_ptr<SendMessageAction> action_;

  static const std::int32_t ID = -1071741569;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateUserTyping(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatUserTyping final : public Update {
 public:
  int64 chat_id_;
  object_ptr<Peer> from_id_;
  object_ptr<SendMessageAction> action_;

  static const std::int32_t ID = -2092401936;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateChatUserTyping(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatParticipants final : public Update {
 public:
  object_ptr<ChatParticipants> participants_;

  static const std::int32_t ID = 125178264;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateChatParticipants(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateUserStatus final : public Update {
 public:
  int64 user_id_;
  object_ptr<UserStatus> status_;

  static const std::int32_t ID = -440534818;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateUserStatus(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateUserName final : public Update {
 public:
  int64 user_id_;
  string first_name_;
  string last_name_;
  array<object_ptr<username>> usernames_;

  static const std::int32_t ID = -1484486364;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateUserName(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateNewEncryptedMessage final : public Update {
 public:
  object_ptr<EncryptedMessage> message_;
  int32 qts_;

  static const std::int32_t ID = 314359194;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateNewEncryptedMessage(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateEncryptedChatTyping final : public Update {
 public:
  int32 chat_id_;

  static const std::int32_t ID = 386986326;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateEncryptedChatTyping(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateEncryption final : public Update {
 public:
  object_ptr<EncryptedChat> chat_;
  int32 date_;

  static const std::int32_t ID = -1264392051;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateEncryption(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateEncryptedMessagesRead final : public Update {
 public:
  int32 chat_id_;
  int32 max_date_;
  int32 date_;

  static const std::int32_t ID = 956179895;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateEncryptedMessagesRead(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatParticipantAdd final : public Update {
 public:
  int64 chat_id_;
  int64 user_id_;
  int64 inviter_id_;
  int32 date_;
  int32 version_;

  static const std::int32_t ID = 1037718609;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateChatParticipantAdd(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatParticipantDelete final : public Update {
 public:
  int64 chat_id_;
  int64 user_id_;
  int32 version_;

  static const std::int32_t ID = -483443337;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateChatParticipantDelete(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateDcOptions final : public Update {
 public:
  array<object_ptr<dcOption>> dc_options_;

  static const std::int32_t ID = -1906403213;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateDcOptions(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateNotifySettings final : public Update {
 public:
  object_ptr<NotifyPeer> peer_;
  object_ptr<peerNotifySettings> notify_settings_;

  static const std::int32_t ID = -1094555409;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateNotifySettings(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateServiceNotification final : public Update {
 public:
  int32 flags_;
  bool popup_;
  int32 inbox_date_;
  string type_;
  string message_;
  object_ptr<MessageMedia> media_;
  array<object_ptr<MessageEntity>> entities_;
  enum Flags : std::int32_t { INBOX_DATE_MASK = 2 };

  updateServiceNotification();

  updateServiceNotification(int32 flags_, bool popup_, int32 inbox_date_, string const &type_, string const &message_, object_ptr<MessageMedia> &&media_, array<object_ptr<MessageEntity>> &&entities_);

  static const std::int32_t ID = -337352679;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updatePrivacy final : public Update {
 public:
  object_ptr<PrivacyKey> key_;
  array<object_ptr<PrivacyRule>> rules_;

  static const std::int32_t ID = -298113238;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updatePrivacy(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateUserPhone final : public Update {
 public:
  int64 user_id_;
  string phone_;

  static const std::int32_t ID = 88680979;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateUserPhone(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateReadHistoryInbox final : public Update {
 public:
  int32 flags_;
  int32 folder_id_;
  object_ptr<Peer> peer_;
  int32 max_id_;
  int32 still_unread_count_;
  int32 pts_;
  int32 pts_count_;
  enum Flags : std::int32_t { FOLDER_ID_MASK = 1 };

  updateReadHistoryInbox();

  static const std::int32_t ID = -1667805217;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateReadHistoryOutbox final : public Update {
 public:
  object_ptr<Peer> peer_;
  int32 max_id_;
  int32 pts_;
  int32 pts_count_;

  static const std::int32_t ID = 791617983;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateReadHistoryOutbox(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateWebPage final : public Update {
 public:
  object_ptr<WebPage> webpage_;
  int32 pts_;
  int32 pts_count_;

  static const std::int32_t ID = 2139689491;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateWebPage(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateReadMessagesContents final : public Update {
 public:
  array<int32> messages_;
  int32 pts_;
  int32 pts_count_;

  static const std::int32_t ID = 1757493555;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateReadMessagesContents(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChannelTooLong final : public Update {
 public:
  int32 flags_;
  int64 channel_id_;
  int32 pts_;
  enum Flags : std::int32_t { PTS_MASK = 1 };

  updateChannelTooLong();

  updateChannelTooLong(int32 flags_, int64 channel_id_, int32 pts_);

  static const std::int32_t ID = 277713951;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChannel final : public Update {
 public:
  int64 channel_id_;

  static const std::int32_t ID = 1666927625;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateChannel(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateNewChannelMessage final : public Update {
 public:
  object_ptr<Message> message_;
  int32 pts_;
  int32 pts_count_;

  static const std::int32_t ID = 1656358105;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateNewChannelMessage(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateReadChannelInbox final : public Update {
 public:
  int32 flags_;
  int32 folder_id_;
  int64 channel_id_;
  int32 max_id_;
  int32 still_unread_count_;
  int32 pts_;
  enum Flags : std::int32_t { FOLDER_ID_MASK = 1 };

  updateReadChannelInbox();

  static const std::int32_t ID = -1842450928;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateDeleteChannelMessages final : public Update {
 public:
  int64 channel_id_;
  array<int32> messages_;
  int32 pts_;
  int32 pts_count_;

  static const std::int32_t ID = -1020437742;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateDeleteChannelMessages(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChannelMessageViews final : public Update {
 public:
  int64 channel_id_;
  int32 id_;
  int32 views_;

  static const std::int32_t ID = -232346616;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateChannelMessageViews(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatParticipantAdmin final : public Update {
 public:
  int64 chat_id_;
  int64 user_id_;
  bool is_admin_;
  int32 version_;

  static const std::int32_t ID = -674602590;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateChatParticipantAdmin(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateNewStickerSet final : public Update {
 public:
  object_ptr<messages_StickerSet> stickerset_;

  static const std::int32_t ID = 1753886890;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateNewStickerSet(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateStickerSetsOrder final : public Update {
 public:
  int32 flags_;
  bool masks_;
  bool emojis_;
  array<int64> order_;

  updateStickerSetsOrder();

  static const std::int32_t ID = 196268545;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateStickerSets final : public Update {
 public:
  int32 flags_;
  bool masks_;
  bool emojis_;

  updateStickerSets();

  static const std::int32_t ID = 834816008;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateSavedGifs final : public Update {
 public:

  static const std::int32_t ID = -1821035490;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateBotInlineQuery final : public Update {
 public:
  int32 flags_;
  int64 query_id_;
  int64 user_id_;
  string query_;
  object_ptr<GeoPoint> geo_;
  object_ptr<InlineQueryPeerType> peer_type_;
  string offset_;

  updateBotInlineQuery();

  static const std::int32_t ID = 1232025500;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateBotInlineSend final : public Update {
 public:
  int32 flags_;
  int64 user_id_;
  string query_;
  object_ptr<GeoPoint> geo_;
  string id_;
  object_ptr<InputBotInlineMessageID> msg_id_;

  updateBotInlineSend();

  static const std::int32_t ID = 317794823;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateEditChannelMessage final : public Update {
 public:
  object_ptr<Message> message_;
  int32 pts_;
  int32 pts_count_;

  updateEditChannelMessage(object_ptr<Message> &&message_, int32 pts_, int32 pts_count_);

  static const std::int32_t ID = 457133559;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateEditChannelMessage(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateBotCallbackQuery final : public Update {
 public:
  int32 flags_;
  int64 query_id_;
  int64 user_id_;
  object_ptr<Peer> peer_;
  int32 msg_id_;
  int64 chat_instance_;
  bytes data_;
  string game_short_name_;
  enum Flags : std::int32_t { DATA_MASK = 1, GAME_SHORT_NAME_MASK = 2 };

  updateBotCallbackQuery();

  static const std::int32_t ID = -1177566067;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateEditMessage final : public Update {
 public:
  object_ptr<Message> message_;
  int32 pts_;
  int32 pts_count_;

  static const std::int32_t ID = -469536605;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateEditMessage(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateInlineBotCallbackQuery final : public Update {
 public:
  int32 flags_;
  int64 query_id_;
  int64 user_id_;
  object_ptr<InputBotInlineMessageID> msg_id_;
  int64 chat_instance_;
  bytes data_;
  string game_short_name_;
  enum Flags : std::int32_t { DATA_MASK = 1, GAME_SHORT_NAME_MASK = 2 };

  updateInlineBotCallbackQuery();

  static const std::int32_t ID = 1763610706;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateReadChannelOutbox final : public Update {
 public:
  int64 channel_id_;
  int32 max_id_;

  static const std::int32_t ID = -1218471511;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateReadChannelOutbox(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateDraftMessage final : public Update {
 public:
  int32 flags_;
  object_ptr<Peer> peer_;
  int32 top_msg_id_;
  object_ptr<DraftMessage> draft_;
  enum Flags : std::int32_t { TOP_MSG_ID_MASK = 1 };

  updateDraftMessage();

  static const std::int32_t ID = 457829485;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateReadFeaturedStickers final : public Update {
 public:

  static const std::int32_t ID = 1461528386;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateRecentStickers final : public Update {
 public:

  static const std::int32_t ID = -1706939360;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateConfig final : public Update {
 public:

  static const std::int32_t ID = -1574314746;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updatePtsChanged final : public Update {
 public:

  static const std::int32_t ID = 861169551;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChannelWebPage final : public Update {
 public:
  int64 channel_id_;
  object_ptr<WebPage> webpage_;
  int32 pts_;
  int32 pts_count_;

  static const std::int32_t ID = 791390623;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateChannelWebPage(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateDialogPinned final : public Update {
 public:
  int32 flags_;
  bool pinned_;
  int32 folder_id_;
  object_ptr<DialogPeer> peer_;
  enum Flags : std::int32_t { FOLDER_ID_MASK = 2 };

  updateDialogPinned();

  static const std::int32_t ID = 1852826908;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updatePinnedDialogs final : public Update {
 public:
  int32 flags_;
  int32 folder_id_;
  array<object_ptr<DialogPeer>> order_;
  enum Flags : std::int32_t { FOLDER_ID_MASK = 2 };

  updatePinnedDialogs();

  static const std::int32_t ID = -99664734;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateBotWebhookJSON final : public Update {
 public:
  object_ptr<dataJSON> data_;

  static const std::int32_t ID = -2095595325;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateBotWebhookJSON(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateBotWebhookJSONQuery final : public Update {
 public:
  int64 query_id_;
  object_ptr<dataJSON> data_;
  int32 timeout_;

  static const std::int32_t ID = -1684914010;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateBotWebhookJSONQuery(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateBotShippingQuery final : public Update {
 public:
  int64 query_id_;
  int64 user_id_;
  bytes payload_;
  object_ptr<postAddress> shipping_address_;

  static const std::int32_t ID = -1246823043;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateBotShippingQuery(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateBotPrecheckoutQuery final : public Update {
 public:
  int32 flags_;
  int64 query_id_;
  int64 user_id_;
  bytes payload_;
  object_ptr<paymentRequestedInfo> info_;
  string shipping_option_id_;
  string currency_;
  int64 total_amount_;
  enum Flags : std::int32_t { SHIPPING_OPTION_ID_MASK = 2 };

  updateBotPrecheckoutQuery();

  static const std::int32_t ID = -1934976362;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updatePhoneCall final : public Update {
 public:
  object_ptr<PhoneCall> phone_call_;

  static const std::int32_t ID = -1425052898;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updatePhoneCall(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateLangPackTooLong final : public Update {
 public:
  string lang_code_;

  static const std::int32_t ID = 1180041828;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateLangPackTooLong(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateLangPack final : public Update {
 public:
  object_ptr<langPackDifference> difference_;

  static const std::int32_t ID = 1442983757;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateLangPack(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateFavedStickers final : public Update {
 public:

  static const std::int32_t ID = -451831443;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChannelReadMessagesContents final : public Update {
 public:
  int32 flags_;
  int64 channel_id_;
  int32 top_msg_id_;
  array<int32> messages_;
  enum Flags : std::int32_t { TOP_MSG_ID_MASK = 1 };

  updateChannelReadMessagesContents();

  static const std::int32_t ID = -366410403;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateContactsReset final : public Update {
 public:

  static const std::int32_t ID = 1887741886;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChannelAvailableMessages final : public Update {
 public:
  int64 channel_id_;
  int32 available_min_id_;

  static const std::int32_t ID = -1304443240;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateChannelAvailableMessages(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateDialogUnreadMark final : public Update {
 public:
  int32 flags_;
  bool unread_;
  object_ptr<DialogPeer> peer_;

  updateDialogUnreadMark();

  static const std::int32_t ID = -513517117;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateMessagePoll final : public Update {
 public:
  int32 flags_;
  int64 poll_id_;
  object_ptr<poll> poll_;
  object_ptr<pollResults> results_;

  updateMessagePoll();

  static const std::int32_t ID = -1398708869;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatDefaultBannedRights final : public Update {
 public:
  object_ptr<Peer> peer_;
  object_ptr<chatBannedRights> default_banned_rights_;
  int32 version_;

  static const std::int32_t ID = 1421875280;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateChatDefaultBannedRights(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateFolderPeers final : public Update {
 public:
  array<object_ptr<folderPeer>> folder_peers_;
  int32 pts_;
  int32 pts_count_;

  static const std::int32_t ID = 422972864;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateFolderPeers(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updatePeerSettings final : public Update {
 public:
  object_ptr<Peer> peer_;
  object_ptr<peerSettings> settings_;

  static const std::int32_t ID = 1786671974;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updatePeerSettings(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updatePeerLocated final : public Update {
 public:
  array<object_ptr<PeerLocated>> peers_;

  static const std::int32_t ID = -1263546448;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updatePeerLocated(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateNewScheduledMessage final : public Update {
 public:
  object_ptr<Message> message_;

  static const std::int32_t ID = 967122427;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateNewScheduledMessage(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateDeleteScheduledMessages final : public Update {
 public:
  object_ptr<Peer> peer_;
  array<int32> messages_;

  static const std::int32_t ID = -1870238482;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateDeleteScheduledMessages(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateTheme final : public Update {
 public:
  object_ptr<theme> theme_;

  static const std::int32_t ID = -2112423005;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateTheme(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateGeoLiveViewed final : public Update {
 public:
  object_ptr<Peer> peer_;
  int32 msg_id_;

  static const std::int32_t ID = -2027964103;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateGeoLiveViewed(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateLoginToken final : public Update {
 public:

  static const std::int32_t ID = 1448076945;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateMessagePollVote final : public Update {
 public:
  int64 poll_id_;
  int64 user_id_;
  array<bytes> options_;
  int32 qts_;

  static const std::int32_t ID = 274961865;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateMessagePollVote(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateDialogFilter final : public Update {
 public:
  int32 flags_;
  int32 id_;
  object_ptr<DialogFilter> filter_;

  updateDialogFilter();

  static const std::int32_t ID = 654302845;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateDialogFilterOrder final : public Update {
 public:
  array<int32> order_;

  static const std::int32_t ID = -1512627963;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateDialogFilterOrder(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateDialogFilters final : public Update {
 public:

  static const std::int32_t ID = 889491791;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updatePhoneCallSignalingData final : public Update {
 public:
  int64 phone_call_id_;
  bytes data_;

  static const std::int32_t ID = 643940105;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updatePhoneCallSignalingData(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChannelMessageForwards final : public Update {
 public:
  int64 channel_id_;
  int32 id_;
  int32 forwards_;

  static const std::int32_t ID = -761649164;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateChannelMessageForwards(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateReadChannelDiscussionInbox final : public Update {
 public:
  int32 flags_;
  int64 channel_id_;
  int32 top_msg_id_;
  int32 read_max_id_;
  int64 broadcast_id_;
  int32 broadcast_post_;
  enum Flags : std::int32_t { BROADCAST_ID_MASK = 1, BROADCAST_POST_MASK = 1 };

  updateReadChannelDiscussionInbox();

  static const std::int32_t ID = -693004986;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateReadChannelDiscussionOutbox final : public Update {
 public:
  int64 channel_id_;
  int32 top_msg_id_;
  int32 read_max_id_;

  static const std::int32_t ID = 1767677564;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateReadChannelDiscussionOutbox(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updatePeerBlocked final : public Update {
 public:
  object_ptr<Peer> peer_id_;
  bool blocked_;

  static const std::int32_t ID = 610945826;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updatePeerBlocked(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChannelUserTyping final : public Update {
 public:
  int32 flags_;
  int64 channel_id_;
  int32 top_msg_id_;
  object_ptr<Peer> from_id_;
  object_ptr<SendMessageAction> action_;
  enum Flags : std::int32_t { TOP_MSG_ID_MASK = 1 };

  updateChannelUserTyping();

  static const std::int32_t ID = -1937192669;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updatePinnedMessages final : public Update {
 public:
  int32 flags_;
  bool pinned_;
  object_ptr<Peer> peer_;
  array<int32> messages_;
  int32 pts_;
  int32 pts_count_;

  updatePinnedMessages();

  static const std::int32_t ID = -309990731;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updatePinnedChannelMessages final : public Update {
 public:
  int32 flags_;
  bool pinned_;
  int64 channel_id_;
  array<int32> messages_;
  int32 pts_;
  int32 pts_count_;

  updatePinnedChannelMessages();

  static const std::int32_t ID = 1538885128;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChat final : public Update {
 public:
  int64 chat_id_;

  static const std::int32_t ID = -124097970;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateChat(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateGroupCallParticipants final : public Update {
 public:
  object_ptr<inputGroupCall> call_;
  array<object_ptr<groupCallParticipant>> participants_;
  int32 version_;

  static const std::int32_t ID = -219423922;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateGroupCallParticipants(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateGroupCall final : public Update {
 public:
  int64 chat_id_;
  object_ptr<GroupCall> call_;

  static const std::int32_t ID = 347227392;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateGroupCall(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updatePeerHistoryTTL final : public Update {
 public:
  int32 flags_;
  object_ptr<Peer> peer_;
  int32 ttl_period_;
  enum Flags : std::int32_t { TTL_PERIOD_MASK = 1 };

  updatePeerHistoryTTL();

  static const std::int32_t ID = -1147422299;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChatParticipant final : public Update {
 public:
  int32 flags_;
  int64 chat_id_;
  int32 date_;
  int64 actor_id_;
  int64 user_id_;
  object_ptr<ChatParticipant> prev_participant_;
  object_ptr<ChatParticipant> new_participant_;
  object_ptr<ExportedChatInvite> invite_;
  int32 qts_;

  updateChatParticipant();

  static const std::int32_t ID = -796432838;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChannelParticipant final : public Update {
 public:
  int32 flags_;
  bool via_chatlist_;
  int64 channel_id_;
  int32 date_;
  int64 actor_id_;
  int64 user_id_;
  object_ptr<ChannelParticipant> prev_participant_;
  object_ptr<ChannelParticipant> new_participant_;
  object_ptr<ExportedChatInvite> invite_;
  int32 qts_;

  updateChannelParticipant();

  static const std::int32_t ID = -1738720581;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateBotStopped final : public Update {
 public:
  int64 user_id_;
  int32 date_;
  bool stopped_;
  int32 qts_;

  static const std::int32_t ID = -997782967;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateBotStopped(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateGroupCallConnection final : public Update {
 public:
  int32 flags_;
  bool presentation_;
  object_ptr<dataJSON> params_;

  updateGroupCallConnection();

  static const std::int32_t ID = 192428418;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateBotCommands final : public Update {
 public:
  object_ptr<Peer> peer_;
  int64 bot_id_;
  array<object_ptr<botCommand>> commands_;

  static const std::int32_t ID = 1299263278;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateBotCommands(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updatePendingJoinRequests final : public Update {
 public:
  object_ptr<Peer> peer_;
  int32 requests_pending_;
  array<int64> recent_requesters_;

  static const std::int32_t ID = 1885586395;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updatePendingJoinRequests(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateBotChatInviteRequester final : public Update {
 public:
  object_ptr<Peer> peer_;
  int32 date_;
  int64 user_id_;
  string about_;
  object_ptr<ExportedChatInvite> invite_;
  int32 qts_;

  static const std::int32_t ID = 299870598;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateBotChatInviteRequester(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateMessageReactions final : public Update {
 public:
  int32 flags_;
  object_ptr<Peer> peer_;
  int32 msg_id_;
  int32 top_msg_id_;
  object_ptr<messageReactions> reactions_;
  enum Flags : std::int32_t { TOP_MSG_ID_MASK = 1 };

  updateMessageReactions();

  static const std::int32_t ID = 1578843320;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateAttachMenuBots final : public Update {
 public:

  static const std::int32_t ID = 397910539;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateWebViewResultSent final : public Update {
 public:
  int64 query_id_;

  static const std::int32_t ID = 361936797;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateWebViewResultSent(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateBotMenuButton final : public Update {
 public:
  int64 bot_id_;
  object_ptr<BotMenuButton> button_;

  static const std::int32_t ID = 347625491;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateBotMenuButton(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateSavedRingtones final : public Update {
 public:

  static const std::int32_t ID = 1960361625;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateTranscribedAudio final : public Update {
 public:
  int32 flags_;
  bool pending_;
  object_ptr<Peer> peer_;
  int32 msg_id_;
  int64 transcription_id_;
  string text_;

  updateTranscribedAudio();

  static const std::int32_t ID = 8703322;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateReadFeaturedEmojiStickers final : public Update {
 public:

  static const std::int32_t ID = -78886548;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateUserEmojiStatus final : public Update {
 public:
  int64 user_id_;
  object_ptr<EmojiStatus> emoji_status_;

  static const std::int32_t ID = 674706841;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateUserEmojiStatus(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateRecentEmojiStatuses final : public Update {
 public:

  static const std::int32_t ID = 821314523;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateRecentReactions final : public Update {
 public:

  static const std::int32_t ID = 1870160884;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateMoveStickerSetToTop final : public Update {
 public:
  int32 flags_;
  bool masks_;
  bool emojis_;
  int64 stickerset_;

  updateMoveStickerSetToTop();

  static const std::int32_t ID = -2030252155;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateMessageExtendedMedia final : public Update {
 public:
  object_ptr<Peer> peer_;
  int32 msg_id_;
  object_ptr<MessageExtendedMedia> extended_media_;

  static const std::int32_t ID = 1517529484;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateMessageExtendedMedia(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChannelPinnedTopic final : public Update {
 public:
  int32 flags_;
  bool pinned_;
  int64 channel_id_;
  int32 topic_id_;

  updateChannelPinnedTopic();

  static const std::int32_t ID = 422509539;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateChannelPinnedTopics final : public Update {
 public:
  int32 flags_;
  int64 channel_id_;
  array<int32> order_;

  updateChannelPinnedTopics();

  static const std::int32_t ID = -31881726;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateUser final : public Update {
 public:
  int64 user_id_;

  static const std::int32_t ID = 542282808;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateUser(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateAutoSaveSettings final : public Update {
 public:

  static const std::int32_t ID = -335171433;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateGroupInvitePrivacyForbidden final : public Update {
 public:
  int64 user_id_;

  static const std::int32_t ID = -856651050;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Update> fetch(TlBufferParser &p);

  explicit updateGroupInvitePrivacyForbidden(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class Updates: public Object {
 public:

  static object_ptr<Updates> fetch(TlBufferParser &p);
};

class updatesTooLong final : public Updates {
 public:

  static const std::int32_t ID = -484987010;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Updates> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateShortMessage final : public Updates {
 public:
  int32 flags_;
  bool out_;
  bool mentioned_;
  bool media_unread_;
  bool silent_;
  int32 id_;
  int64 user_id_;
  string message_;
  int32 pts_;
  int32 pts_count_;
  int32 date_;
  object_ptr<messageFwdHeader> fwd_from_;
  int64 via_bot_id_;
  object_ptr<messageReplyHeader> reply_to_;
  array<object_ptr<MessageEntity>> entities_;
  int32 ttl_period_;
  enum Flags : std::int32_t { VIA_BOT_ID_MASK = 2048, TTL_PERIOD_MASK = 33554432 };

  updateShortMessage();

  static const std::int32_t ID = 826001400;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Updates> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateShortChatMessage final : public Updates {
 public:
  int32 flags_;
  bool out_;
  bool mentioned_;
  bool media_unread_;
  bool silent_;
  int32 id_;
  int64 from_id_;
  int64 chat_id_;
  string message_;
  int32 pts_;
  int32 pts_count_;
  int32 date_;
  object_ptr<messageFwdHeader> fwd_from_;
  int64 via_bot_id_;
  object_ptr<messageReplyHeader> reply_to_;
  array<object_ptr<MessageEntity>> entities_;
  int32 ttl_period_;
  enum Flags : std::int32_t { VIA_BOT_ID_MASK = 2048, TTL_PERIOD_MASK = 33554432 };

  updateShortChatMessage();

  static const std::int32_t ID = 1299050149;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Updates> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateShort final : public Updates {
 public:
  object_ptr<Update> update_;
  int32 date_;

  static const std::int32_t ID = 2027216577;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Updates> fetch(TlBufferParser &p);

  explicit updateShort(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updatesCombined final : public Updates {
 public:
  array<object_ptr<Update>> updates_;
  array<object_ptr<User>> users_;
  array<object_ptr<Chat>> chats_;
  int32 date_;
  int32 seq_start_;
  int32 seq_;

  static const std::int32_t ID = 1918567619;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Updates> fetch(TlBufferParser &p);

  explicit updatesCombined(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updates final : public Updates {
 public:
  array<object_ptr<Update>> updates_;
  array<object_ptr<User>> users_;
  array<object_ptr<Chat>> chats_;
  int32 date_;
  int32 seq_;

  updates();

  static const std::int32_t ID = 1957577280;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Updates> fetch(TlBufferParser &p);

  explicit updates(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updateShortSentMessage final : public Updates {
 public:
  int32 flags_;
  bool out_;
  int32 id_;
  int32 pts_;
  int32 pts_count_;
  int32 date_;
  object_ptr<MessageMedia> media_;
  array<object_ptr<MessageEntity>> entities_;
  int32 ttl_period_;
  enum Flags : std::int32_t { TTL_PERIOD_MASK = 33554432 };

  updateShortSentMessage();

  static const std::int32_t ID = -1877614335;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<Updates> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class UrlAuthResult: public Object {
 public:

  static object_ptr<UrlAuthResult> fetch(TlBufferParser &p);
};

class urlAuthResultRequest final : public UrlAuthResult {
 public:
  int32 flags_;
  bool request_write_access_;
  object_ptr<User> bot_;
  string domain_;

  urlAuthResultRequest();

  static const std::int32_t ID = -1831650802;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<UrlAuthResult> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class urlAuthResultAccepted final : public UrlAuthResult {
 public:
  string url_;

  static const std::int32_t ID = -1886646706;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<UrlAuthResult> fetch(TlBufferParser &p);

  explicit urlAuthResultAccepted(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class urlAuthResultDefault final : public UrlAuthResult {
 public:

  static const std::int32_t ID = -1445536993;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<UrlAuthResult> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class User: public Object {
 public:

  static object_ptr<User> fetch(TlBufferParser &p);
};

class userEmpty final : public User {
 public:
  int64 id_;

  static const std::int32_t ID = -742634630;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<User> fetch(TlBufferParser &p);

  explicit userEmpty(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class user final : public User {
 public:
  int32 flags_;
  bool self_;
  bool contact_;
  bool mutual_contact_;
  bool deleted_;
  bool bot_;
  bool bot_chat_history_;
  bool bot_nochats_;
  bool verified_;
  bool restricted_;
  bool min_;
  bool bot_inline_geo_;
  bool support_;
  bool scam_;
  bool apply_min_photo_;
  bool fake_;
  bool bot_attach_menu_;
  bool premium_;
  bool attach_menu_enabled_;
  int32 flags2_;
  bool bot_can_edit_;
  int64 id_;
  int64 access_hash_;
  string first_name_;
  string last_name_;
  string username_;
  string phone_;
  object_ptr<UserProfilePhoto> photo_;
  object_ptr<UserStatus> status_;
  int32 bot_info_version_;
  array<object_ptr<restrictionReason>> restriction_reason_;
  string bot_inline_placeholder_;
  string lang_code_;
  object_ptr<EmojiStatus> emoji_status_;
  array<object_ptr<username>> usernames_;
  enum Flags : std::int32_t { ACCESS_HASH_MASK = 1, FIRST_NAME_MASK = 2, LAST_NAME_MASK = 4, USERNAME_MASK = 8, PHONE_MASK = 16, BOT_INFO_VERSION_MASK = 16384, BOT_INLINE_PLACEHOLDER_MASK = 524288, LANG_CODE_MASK = 4194304 };

  user();

  user(int32 flags_, bool self_, bool contact_, bool mutual_contact_, bool deleted_, bool bot_, bool bot_chat_history_, bool bot_nochats_, bool verified_, bool restricted_, bool min_, bool bot_inline_geo_, bool support_, bool scam_, bool apply_min_photo_, bool fake_, bool bot_attach_menu_, bool premium_, bool attach_menu_enabled_, int32 flags2_, bool bot_can_edit_, int64 id_, int64 access_hash_, string const &first_name_, string const &last_name_, string const &username_, string const &phone_, object_ptr<UserProfilePhoto> &&photo_, object_ptr<UserStatus> &&status_, int32 bot_info_version_, array<object_ptr<restrictionReason>> &&restriction_reason_, string const &bot_inline_placeholder_, string const &lang_code_, object_ptr<EmojiStatus> &&emoji_status_, array<object_ptr<username>> &&usernames_);

  static const std::int32_t ID = -1885878744;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<User> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userFull final : public Object {
 public:
  int32 flags_;
  bool blocked_;
  bool phone_calls_available_;
  bool phone_calls_private_;
  bool can_pin_message_;
  bool has_scheduled_;
  bool video_calls_available_;
  bool voice_messages_forbidden_;
  bool translations_disabled_;
  int64 id_;
  string about_;
  object_ptr<peerSettings> settings_;
  object_ptr<Photo> personal_photo_;
  object_ptr<Photo> profile_photo_;
  object_ptr<Photo> fallback_photo_;
  object_ptr<peerNotifySettings> notify_settings_;
  object_ptr<botInfo> bot_info_;
  int32 pinned_msg_id_;
  int32 common_chats_count_;
  int32 folder_id_;
  int32 ttl_period_;
  string theme_emoticon_;
  string private_forward_name_;
  object_ptr<chatAdminRights> bot_group_admin_rights_;
  object_ptr<chatAdminRights> bot_broadcast_admin_rights_;
  array<object_ptr<premiumGiftOption>> premium_gifts_;
  object_ptr<WallPaper> wallpaper_;
  enum Flags : std::int32_t { ABOUT_MASK = 2, PINNED_MSG_ID_MASK = 64, FOLDER_ID_MASK = 2048, TTL_PERIOD_MASK = 16384, THEME_EMOTICON_MASK = 32768, PRIVATE_FORWARD_NAME_MASK = 65536 };

  userFull();

  static const std::int32_t ID = -1813324973;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<userFull> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class UserProfilePhoto: public Object {
 public:

  static object_ptr<UserProfilePhoto> fetch(TlBufferParser &p);
};

class userProfilePhotoEmpty final : public UserProfilePhoto {
 public:

  static const std::int32_t ID = 1326562017;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<UserProfilePhoto> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userProfilePhoto final : public UserProfilePhoto {
 public:
  int32 flags_;
  bool has_video_;
  bool personal_;
  int64 photo_id_;
  bytes stripped_thumb_;
  int32 dc_id_;
  enum Flags : std::int32_t { STRIPPED_THUMB_MASK = 2 };

  userProfilePhoto();

  userProfilePhoto(int32 flags_, bool has_video_, bool personal_, int64 photo_id_, bytes &&stripped_thumb_, int32 dc_id_);

  static const std::int32_t ID = -2100168954;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<UserProfilePhoto> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class UserStatus: public Object {
 public:

  static object_ptr<UserStatus> fetch(TlBufferParser &p);
};

class userStatusEmpty final : public UserStatus {
 public:

  static const std::int32_t ID = 164646985;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<UserStatus> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userStatusOnline final : public UserStatus {
 public:
  int32 expires_;

  static const std::int32_t ID = -306628279;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<UserStatus> fetch(TlBufferParser &p);

  explicit userStatusOnline(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userStatusOffline final : public UserStatus {
 public:
  int32 was_online_;

  static const std::int32_t ID = 9203775;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<UserStatus> fetch(TlBufferParser &p);

  explicit userStatusOffline(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userStatusRecently final : public UserStatus {
 public:

  static const std::int32_t ID = -496024847;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<UserStatus> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userStatusLastWeek final : public UserStatus {
 public:

  static const std::int32_t ID = 129960444;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<UserStatus> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class userStatusLastMonth final : public UserStatus {
 public:

  static const std::int32_t ID = 2011940674;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<UserStatus> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class username final : public Object {
 public:
  int32 flags_;
  bool editable_;
  bool active_;
  string username_;

  username();

  static const std::int32_t ID = -1274595769;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<username> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class VideoSize: public Object {
 public:

  static object_ptr<VideoSize> fetch(TlBufferParser &p);
};

class videoSize final : public VideoSize {
 public:
  int32 flags_;
  string type_;
  int32 w_;
  int32 h_;
  int32 size_;
  double video_start_ts_;
  enum Flags : std::int32_t { VIDEO_START_TS_MASK = 1 };

  videoSize();

  videoSize(int32 flags_, string const &type_, int32 w_, int32 h_, int32 size_, double video_start_ts_);

  static const std::int32_t ID = -567037804;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<VideoSize> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class videoSizeEmojiMarkup final : public VideoSize {
 public:
  int64 emoji_id_;
  array<int32> background_colors_;

  videoSizeEmojiMarkup(int64 emoji_id_, array<int32> &&background_colors_);

  static const std::int32_t ID = -128171716;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<VideoSize> fetch(TlBufferParser &p);

  explicit videoSizeEmojiMarkup(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class videoSizeStickerMarkup final : public VideoSize {
 public:
  object_ptr<InputStickerSet> stickerset_;
  int64 sticker_id_;
  array<int32> background_colors_;

  videoSizeStickerMarkup(object_ptr<InputStickerSet> &&stickerset_, int64 sticker_id_, array<int32> &&background_colors_);

  static const std::int32_t ID = 228623102;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<VideoSize> fetch(TlBufferParser &p);

  explicit videoSizeStickerMarkup(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class WallPaper: public Object {
 public:

  static object_ptr<WallPaper> fetch(TlBufferParser &p);
};

class wallPaper final : public WallPaper {
 public:
  int64 id_;
  int32 flags_;
  bool creator_;
  bool default_;
  bool pattern_;
  bool dark_;
  int64 access_hash_;
  string slug_;
  object_ptr<Document> document_;
  object_ptr<wallPaperSettings> settings_;

  wallPaper();

  static const std::int32_t ID = -1539849235;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<WallPaper> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class wallPaperNoFile final : public WallPaper {
 public:
  int64 id_;
  int32 flags_;
  bool default_;
  bool dark_;
  object_ptr<wallPaperSettings> settings_;

  wallPaperNoFile();

  static const std::int32_t ID = -528465642;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<WallPaper> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class wallPaperSettings final : public Object {
 public:
  int32 flags_;
  bool blur_;
  bool motion_;
  int32 background_color_;
  int32 second_background_color_;
  int32 third_background_color_;
  int32 fourth_background_color_;
  int32 intensity_;
  int32 rotation_;
  enum Flags : std::int32_t { BLUR_MASK = 2, MOTION_MASK = 4, BACKGROUND_COLOR_MASK = 1, SECOND_BACKGROUND_COLOR_MASK = 16, THIRD_BACKGROUND_COLOR_MASK = 32, FOURTH_BACKGROUND_COLOR_MASK = 64, INTENSITY_MASK = 8, ROTATION_MASK = 16 };

  wallPaperSettings();

  wallPaperSettings(int32 flags_, bool blur_, bool motion_, int32 background_color_, int32 second_background_color_, int32 third_background_color_, int32 fourth_background_color_, int32 intensity_, int32 rotation_);

  static const std::int32_t ID = 499236004;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<wallPaperSettings> fetch(TlBufferParser &p);

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class webAuthorization final : public Object {
 public:
  int64 hash_;
  int64 bot_id_;
  string domain_;
  string browser_;
  string platform_;
  int32 date_created_;
  int32 date_active_;
  string ip_;
  string region_;

  static const std::int32_t ID = -1493633966;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<webAuthorization> fetch(TlBufferParser &p);

  explicit webAuthorization(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class WebDocument: public Object {
 public:

  static object_ptr<WebDocument> fetch(TlBufferParser &p);
};

class webDocument final : public WebDocument {
 public:
  string url_;
  int64 access_hash_;
  int32 size_;
  string mime_type_;
  array<object_ptr<DocumentAttribute>> attributes_;

  static const std::int32_t ID = 475467473;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<WebDocument> fetch(TlBufferParser &p);

  explicit webDocument(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class webDocumentNoProxy final : public WebDocument {
 public:
  string url_;
  int32 size_;
  string mime_type_;
  array<object_ptr<DocumentAttribute>> attributes_;

  static const std::int32_t ID = -104284986;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<WebDocument> fetch(TlBufferParser &p);

  explicit webDocumentNoProxy(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class WebPage: public Object {
 public:

  static object_ptr<WebPage> fetch(TlBufferParser &p);
};

class webPageEmpty final : public WebPage {
 public:
  int64 id_;

  static const std::int32_t ID = -350980120;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<WebPage> fetch(TlBufferParser &p);

  explicit webPageEmpty(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class webPagePending final : public WebPage {
 public:
  int64 id_;
  int32 date_;

  static const std::int32_t ID = -981018084;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<WebPage> fetch(TlBufferParser &p);

  explicit webPagePending(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class webPage final : public WebPage {
 public:
  int32 flags_;
  int64 id_;
  string url_;
  string display_url_;
  int32 hash_;
  string type_;
  string site_name_;
  string title_;
  string description_;
  object_ptr<Photo> photo_;
  string embed_url_;
  string embed_type_;
  int32 embed_width_;
  int32 embed_height_;
  int32 duration_;
  string author_;
  object_ptr<Document> document_;
  object_ptr<page> cached_page_;
  array<object_ptr<webPageAttributeTheme>> attributes_;
  enum Flags : std::int32_t { TYPE_MASK = 1, SITE_NAME_MASK = 2, TITLE_MASK = 4, DESCRIPTION_MASK = 8, EMBED_URL_MASK = 32, EMBED_TYPE_MASK = 32, EMBED_WIDTH_MASK = 64, EMBED_HEIGHT_MASK = 64, DURATION_MASK = 128, AUTHOR_MASK = 256 };

  webPage();

  static const std::int32_t ID = -392411726;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<WebPage> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class webPageNotModified final : public WebPage {
 public:
  int32 flags_;
  int32 cached_page_views_;
  enum Flags : std::int32_t { CACHED_PAGE_VIEWS_MASK = 1 };

  webPageNotModified();

  static const std::int32_t ID = 1930545681;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<WebPage> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class webPageAttributeTheme final : public Object {
 public:
  int32 flags_;
  array<object_ptr<Document>> documents_;
  object_ptr<themeSettings> settings_;

  webPageAttributeTheme();

  static const std::int32_t ID = 1421174295;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<webPageAttributeTheme> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class webViewMessageSent final : public Object {
 public:
  int32 flags_;
  object_ptr<InputBotInlineMessageID> msg_id_;

  webViewMessageSent();

  static const std::int32_t ID = 211046684;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<webViewMessageSent> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class webViewResultUrl final : public Object {
 public:
  int64 query_id_;
  string url_;

  static const std::int32_t ID = 202659196;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<webViewResultUrl> fetch(TlBufferParser &p);

  explicit webViewResultUrl(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_authorizationForm final : public Object {
 public:
  int32 flags_;
  array<object_ptr<SecureRequiredType>> required_types_;
  array<object_ptr<secureValue>> values_;
  array<object_ptr<SecureValueError>> errors_;
  array<object_ptr<User>> users_;
  string privacy_policy_url_;
  enum Flags : std::int32_t { PRIVACY_POLICY_URL_MASK = 1 };

  account_authorizationForm();

  static const std::int32_t ID = -1389486888;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_authorizationForm> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_authorizations final : public Object {
 public:
  int32 authorization_ttl_days_;
  array<object_ptr<authorization>> authorizations_;

  static const std::int32_t ID = 1275039392;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_authorizations> fetch(TlBufferParser &p);

  explicit account_authorizations(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_autoDownloadSettings final : public Object {
 public:
  object_ptr<autoDownloadSettings> low_;
  object_ptr<autoDownloadSettings> medium_;
  object_ptr<autoDownloadSettings> high_;

  static const std::int32_t ID = 1674235686;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_autoDownloadSettings> fetch(TlBufferParser &p);

  explicit account_autoDownloadSettings(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_autoSaveSettings final : public Object {
 public:
  object_ptr<autoSaveSettings> users_settings_;
  object_ptr<autoSaveSettings> chats_settings_;
  object_ptr<autoSaveSettings> broadcasts_settings_;
  array<object_ptr<autoSaveException>> exceptions_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = 1279133341;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_autoSaveSettings> fetch(TlBufferParser &p);

  explicit account_autoSaveSettings(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_contentSettings final : public Object {
 public:
  int32 flags_;
  bool sensitive_enabled_;
  bool sensitive_can_change_;

  account_contentSettings();

  static const std::int32_t ID = 1474462241;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_contentSettings> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_EmailVerified: public Object {
 public:

  static object_ptr<account_EmailVerified> fetch(TlBufferParser &p);
};

class account_emailVerified final : public account_EmailVerified {
 public:
  string email_;

  static const std::int32_t ID = 731303195;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_EmailVerified> fetch(TlBufferParser &p);

  explicit account_emailVerified(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_emailVerifiedLogin final : public account_EmailVerified {
 public:
  string email_;
  object_ptr<auth_SentCode> sent_code_;

  static const std::int32_t ID = -507835039;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_EmailVerified> fetch(TlBufferParser &p);

  explicit account_emailVerifiedLogin(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_EmojiStatuses: public Object {
 public:

  static object_ptr<account_EmojiStatuses> fetch(TlBufferParser &p);
};

class account_emojiStatusesNotModified final : public account_EmojiStatuses {
 public:

  static const std::int32_t ID = -796072379;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_EmojiStatuses> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_emojiStatuses final : public account_EmojiStatuses {
 public:
  int64 hash_;
  array<object_ptr<EmojiStatus>> statuses_;

  static const std::int32_t ID = -1866176559;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_EmojiStatuses> fetch(TlBufferParser &p);

  explicit account_emojiStatuses(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_password final : public Object {
 public:
  int32 flags_;
  bool has_recovery_;
  bool has_secure_values_;
  bool has_password_;
  object_ptr<PasswordKdfAlgo> current_algo_;
  bytes srp_B_;
  int64 srp_id_;
  string hint_;
  string email_unconfirmed_pattern_;
  object_ptr<PasswordKdfAlgo> new_algo_;
  object_ptr<SecurePasswordKdfAlgo> new_secure_algo_;
  bytes secure_random_;
  int32 pending_reset_date_;
  string login_email_pattern_;
  enum Flags : std::int32_t { SRP_B_MASK = 4, SRP_ID_MASK = 4, HINT_MASK = 8, EMAIL_UNCONFIRMED_PATTERN_MASK = 16, PENDING_RESET_DATE_MASK = 32, LOGIN_EMAIL_PATTERN_MASK = 64 };

  account_password();

  static const std::int32_t ID = -1787080453;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_password> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_passwordInputSettings final : public Object {
 public:
  int32 flags_;
  object_ptr<PasswordKdfAlgo> new_algo_;
  bytes new_password_hash_;
  string hint_;
  string email_;
  object_ptr<secureSecretSettings> new_secure_settings_;
  enum Flags : std::int32_t { NEW_ALGO_MASK = 1, NEW_PASSWORD_HASH_MASK = 1, HINT_MASK = 1, EMAIL_MASK = 2, NEW_SECURE_SETTINGS_MASK = 4 };

  account_passwordInputSettings(int32 flags_, object_ptr<PasswordKdfAlgo> &&new_algo_, bytes &&new_password_hash_, string const &hint_, string const &email_, object_ptr<secureSecretSettings> &&new_secure_settings_);

  static const std::int32_t ID = -1036572727;
  std::int32_t get_id() const final {
    return ID;
  }

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_passwordSettings final : public Object {
 public:
  int32 flags_;
  string email_;
  object_ptr<secureSecretSettings> secure_settings_;
  enum Flags : std::int32_t { EMAIL_MASK = 1 };

  account_passwordSettings();

  static const std::int32_t ID = -1705233435;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_passwordSettings> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_privacyRules final : public Object {
 public:
  array<object_ptr<PrivacyRule>> rules_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = 1352683077;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_privacyRules> fetch(TlBufferParser &p);

  explicit account_privacyRules(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_ResetPasswordResult: public Object {
 public:

  static object_ptr<account_ResetPasswordResult> fetch(TlBufferParser &p);
};

class account_resetPasswordFailedWait final : public account_ResetPasswordResult {
 public:
  int32 retry_date_;

  static const std::int32_t ID = -478701471;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_ResetPasswordResult> fetch(TlBufferParser &p);

  explicit account_resetPasswordFailedWait(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_resetPasswordRequestedWait final : public account_ResetPasswordResult {
 public:
  int32 until_date_;

  static const std::int32_t ID = -370148227;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_ResetPasswordResult> fetch(TlBufferParser &p);

  explicit account_resetPasswordRequestedWait(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_resetPasswordOk final : public account_ResetPasswordResult {
 public:

  static const std::int32_t ID = -383330754;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_ResetPasswordResult> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_SavedRingtone: public Object {
 public:

  static object_ptr<account_SavedRingtone> fetch(TlBufferParser &p);
};

class account_savedRingtone final : public account_SavedRingtone {
 public:

  static const std::int32_t ID = -1222230163;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_SavedRingtone> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_savedRingtoneConverted final : public account_SavedRingtone {
 public:
  object_ptr<Document> document_;

  static const std::int32_t ID = 523271863;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_SavedRingtone> fetch(TlBufferParser &p);

  explicit account_savedRingtoneConverted(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_SavedRingtones: public Object {
 public:

  static object_ptr<account_SavedRingtones> fetch(TlBufferParser &p);
};

class account_savedRingtonesNotModified final : public account_SavedRingtones {
 public:

  static const std::int32_t ID = -67704655;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_SavedRingtones> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_savedRingtones final : public account_SavedRingtones {
 public:
  int64 hash_;
  array<object_ptr<Document>> ringtones_;

  static const std::int32_t ID = -1041683259;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_SavedRingtones> fetch(TlBufferParser &p);

  explicit account_savedRingtones(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_sentEmailCode final : public Object {
 public:
  string email_pattern_;
  int32 length_;

  static const std::int32_t ID = -2128640689;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_sentEmailCode> fetch(TlBufferParser &p);

  explicit account_sentEmailCode(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_takeout final : public Object {
 public:
  int64 id_;

  static const std::int32_t ID = 1304052993;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_takeout> fetch(TlBufferParser &p);

  explicit account_takeout(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_Themes: public Object {
 public:

  static object_ptr<account_Themes> fetch(TlBufferParser &p);
};

class account_themesNotModified final : public account_Themes {
 public:

  static const std::int32_t ID = -199313886;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_Themes> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_themes final : public account_Themes {
 public:
  int64 hash_;
  array<object_ptr<theme>> themes_;

  static const std::int32_t ID = -1707242387;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_Themes> fetch(TlBufferParser &p);

  explicit account_themes(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_tmpPassword final : public Object {
 public:
  bytes tmp_password_;
  int32 valid_until_;

  static const std::int32_t ID = -614138572;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_tmpPassword> fetch(TlBufferParser &p);

  explicit account_tmpPassword(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_WallPapers: public Object {
 public:

  static object_ptr<account_WallPapers> fetch(TlBufferParser &p);
};

class account_wallPapersNotModified final : public account_WallPapers {
 public:

  static const std::int32_t ID = 471437699;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_WallPapers> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_wallPapers final : public account_WallPapers {
 public:
  int64 hash_;
  array<object_ptr<WallPaper>> wallpapers_;

  static const std::int32_t ID = -842824308;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_WallPapers> fetch(TlBufferParser &p);

  explicit account_wallPapers(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_webAuthorizations final : public Object {
 public:
  array<object_ptr<webAuthorization>> authorizations_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -313079300;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<account_webAuthorizations> fetch(TlBufferParser &p);

  explicit account_webAuthorizations(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_Authorization: public Object {
 public:

  static object_ptr<auth_Authorization> fetch(TlBufferParser &p);
};

class auth_authorization final : public auth_Authorization {
 public:
  int32 flags_;
  bool setup_password_required_;
  int32 otherwise_relogin_days_;
  int32 tmp_sessions_;
  bytes future_auth_token_;
  object_ptr<User> user_;
  enum Flags : std::int32_t { OTHERWISE_RELOGIN_DAYS_MASK = 2, TMP_SESSIONS_MASK = 1, FUTURE_AUTH_TOKEN_MASK = 4 };

  auth_authorization();

  static const std::int32_t ID = 782418132;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_Authorization> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_authorizationSignUpRequired final : public auth_Authorization {
 public:
  int32 flags_;
  object_ptr<help_termsOfService> terms_of_service_;

  auth_authorizationSignUpRequired();

  static const std::int32_t ID = 1148485274;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_Authorization> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_CodeType: public Object {
 public:

  static object_ptr<auth_CodeType> fetch(TlBufferParser &p);
};

class auth_codeTypeSms final : public auth_CodeType {
 public:

  static const std::int32_t ID = 1923290508;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_CodeType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_codeTypeCall final : public auth_CodeType {
 public:

  static const std::int32_t ID = 1948046307;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_CodeType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_codeTypeFlashCall final : public auth_CodeType {
 public:

  static const std::int32_t ID = 577556219;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_CodeType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_codeTypeMissedCall final : public auth_CodeType {
 public:

  static const std::int32_t ID = -702884114;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_CodeType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_codeTypeFragmentSms final : public auth_CodeType {
 public:

  static const std::int32_t ID = 116234636;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_CodeType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_exportedAuthorization final : public Object {
 public:
  int64 id_;
  bytes bytes_;

  static const std::int32_t ID = -1271602504;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_exportedAuthorization> fetch(TlBufferParser &p);

  explicit auth_exportedAuthorization(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_loggedOut final : public Object {
 public:
  int32 flags_;
  bytes future_auth_token_;
  enum Flags : std::int32_t { FUTURE_AUTH_TOKEN_MASK = 1 };

  auth_loggedOut();

  static const std::int32_t ID = -1012759713;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_loggedOut> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_LoginToken: public Object {
 public:

  static object_ptr<auth_LoginToken> fetch(TlBufferParser &p);
};

class auth_loginToken final : public auth_LoginToken {
 public:
  int32 expires_;
  bytes token_;

  static const std::int32_t ID = 1654593920;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_LoginToken> fetch(TlBufferParser &p);

  explicit auth_loginToken(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_loginTokenMigrateTo final : public auth_LoginToken {
 public:
  int32 dc_id_;
  bytes token_;

  static const std::int32_t ID = 110008598;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_LoginToken> fetch(TlBufferParser &p);

  explicit auth_loginTokenMigrateTo(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_loginTokenSuccess final : public auth_LoginToken {
 public:
  object_ptr<auth_Authorization> authorization_;

  static const std::int32_t ID = 957176926;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_LoginToken> fetch(TlBufferParser &p);

  explicit auth_loginTokenSuccess(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_passwordRecovery final : public Object {
 public:
  string email_pattern_;

  static const std::int32_t ID = 326715557;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_passwordRecovery> fetch(TlBufferParser &p);

  explicit auth_passwordRecovery(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_SentCode: public Object {
 public:

  static object_ptr<auth_SentCode> fetch(TlBufferParser &p);
};

class auth_sentCode final : public auth_SentCode {
 public:
  int32 flags_;
  object_ptr<auth_SentCodeType> type_;
  string phone_code_hash_;
  object_ptr<auth_CodeType> next_type_;
  int32 timeout_;
  enum Flags : std::int32_t { TIMEOUT_MASK = 4 };

  auth_sentCode();

  static const std::int32_t ID = 1577067778;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_SentCode> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_sentCodeSuccess final : public auth_SentCode {
 public:
  object_ptr<auth_Authorization> authorization_;

  static const std::int32_t ID = 596704836;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_SentCode> fetch(TlBufferParser &p);

  explicit auth_sentCodeSuccess(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_SentCodeType: public Object {
 public:

  static object_ptr<auth_SentCodeType> fetch(TlBufferParser &p);
};

class auth_sentCodeTypeApp final : public auth_SentCodeType {
 public:
  int32 length_;

  static const std::int32_t ID = 1035688326;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_SentCodeType> fetch(TlBufferParser &p);

  explicit auth_sentCodeTypeApp(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_sentCodeTypeSms final : public auth_SentCodeType {
 public:
  int32 length_;

  static const std::int32_t ID = -1073693790;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_SentCodeType> fetch(TlBufferParser &p);

  explicit auth_sentCodeTypeSms(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_sentCodeTypeCall final : public auth_SentCodeType {
 public:
  int32 length_;

  static const std::int32_t ID = 1398007207;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_SentCodeType> fetch(TlBufferParser &p);

  explicit auth_sentCodeTypeCall(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_sentCodeTypeFlashCall final : public auth_SentCodeType {
 public:
  string pattern_;

  static const std::int32_t ID = -1425815847;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_SentCodeType> fetch(TlBufferParser &p);

  explicit auth_sentCodeTypeFlashCall(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_sentCodeTypeMissedCall final : public auth_SentCodeType {
 public:
  string prefix_;
  int32 length_;

  static const std::int32_t ID = -2113903484;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_SentCodeType> fetch(TlBufferParser &p);

  explicit auth_sentCodeTypeMissedCall(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_sentCodeTypeEmailCode final : public auth_SentCodeType {
 public:
  int32 flags_;
  bool apple_signin_allowed_;
  bool google_signin_allowed_;
  string email_pattern_;
  int32 length_;
  int32 reset_available_period_;
  int32 reset_pending_date_;
  enum Flags : std::int32_t { RESET_AVAILABLE_PERIOD_MASK = 8, RESET_PENDING_DATE_MASK = 16 };

  auth_sentCodeTypeEmailCode();

  static const std::int32_t ID = -196020837;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_SentCodeType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_sentCodeTypeSetUpEmailRequired final : public auth_SentCodeType {
 public:
  int32 flags_;
  bool apple_signin_allowed_;
  bool google_signin_allowed_;

  auth_sentCodeTypeSetUpEmailRequired();

  static const std::int32_t ID = -1521934870;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_SentCodeType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_sentCodeTypeFragmentSms final : public auth_SentCodeType {
 public:
  string url_;
  int32 length_;

  static const std::int32_t ID = -648651719;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_SentCodeType> fetch(TlBufferParser &p);

  explicit auth_sentCodeTypeFragmentSms(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class auth_sentCodeTypeFirebaseSms final : public auth_SentCodeType {
 public:
  int32 flags_;
  bytes nonce_;
  string receipt_;
  int32 push_timeout_;
  int32 length_;
  enum Flags : std::int32_t { NONCE_MASK = 1, RECEIPT_MASK = 2, PUSH_TIMEOUT_MASK = 2 };

  auth_sentCodeTypeFirebaseSms();

  static const std::int32_t ID = -444918734;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<auth_SentCodeType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class bots_botInfo final : public Object {
 public:
  string name_;
  string about_;
  string description_;

  static const std::int32_t ID = -391678544;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<bots_botInfo> fetch(TlBufferParser &p);

  explicit bots_botInfo(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channels_adminLogResults final : public Object {
 public:
  array<object_ptr<channelAdminLogEvent>> events_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -309659827;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<channels_adminLogResults> fetch(TlBufferParser &p);

  explicit channels_adminLogResults(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channels_channelParticipant final : public Object {
 public:
  object_ptr<ChannelParticipant> participant_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -541588713;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<channels_channelParticipant> fetch(TlBufferParser &p);

  explicit channels_channelParticipant(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channels_ChannelParticipants: public Object {
 public:

  static object_ptr<channels_ChannelParticipants> fetch(TlBufferParser &p);
};

class channels_channelParticipants final : public channels_ChannelParticipants {
 public:
  int32 count_;
  array<object_ptr<ChannelParticipant>> participants_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -1699676497;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<channels_ChannelParticipants> fetch(TlBufferParser &p);

  explicit channels_channelParticipants(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channels_channelParticipantsNotModified final : public channels_ChannelParticipants {
 public:

  static const std::int32_t ID = -266911767;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<channels_ChannelParticipants> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class channels_sendAsPeers final : public Object {
 public:
  array<object_ptr<sendAsPeer>> peers_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -191450938;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<channels_sendAsPeers> fetch(TlBufferParser &p);

  explicit channels_sendAsPeers(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatlists_ChatlistInvite: public Object {
 public:

  static object_ptr<chatlists_ChatlistInvite> fetch(TlBufferParser &p);
};

class chatlists_chatlistInviteAlready final : public chatlists_ChatlistInvite {
 public:
  int32 filter_id_;
  array<object_ptr<Peer>> missing_peers_;
  array<object_ptr<Peer>> already_peers_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -91752871;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<chatlists_ChatlistInvite> fetch(TlBufferParser &p);

  explicit chatlists_chatlistInviteAlready(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatlists_chatlistInvite final : public chatlists_ChatlistInvite {
 public:
  int32 flags_;
  string title_;
  string emoticon_;
  array<object_ptr<Peer>> peers_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;
  enum Flags : std::int32_t { EMOTICON_MASK = 1 };

  chatlists_chatlistInvite();

  static const std::int32_t ID = 500007837;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<chatlists_ChatlistInvite> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatlists_chatlistUpdates final : public Object {
 public:
  array<object_ptr<Peer>> missing_peers_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -1816295539;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<chatlists_chatlistUpdates> fetch(TlBufferParser &p);

  explicit chatlists_chatlistUpdates(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatlists_exportedChatlistInvite final : public Object {
 public:
  object_ptr<DialogFilter> filter_;
  object_ptr<exportedChatlistInvite> invite_;

  static const std::int32_t ID = 283567014;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<chatlists_exportedChatlistInvite> fetch(TlBufferParser &p);

  explicit chatlists_exportedChatlistInvite(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class chatlists_exportedInvites final : public Object {
 public:
  array<object_ptr<exportedChatlistInvite>> invites_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = 279670215;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<chatlists_exportedInvites> fetch(TlBufferParser &p);

  explicit chatlists_exportedInvites(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class contacts_Blocked: public Object {
 public:

  static object_ptr<contacts_Blocked> fetch(TlBufferParser &p);
};

class contacts_blocked final : public contacts_Blocked {
 public:
  array<object_ptr<peerBlocked>> blocked_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = 182326673;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<contacts_Blocked> fetch(TlBufferParser &p);

  explicit contacts_blocked(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class contacts_blockedSlice final : public contacts_Blocked {
 public:
  int32 count_;
  array<object_ptr<peerBlocked>> blocked_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -513392236;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<contacts_Blocked> fetch(TlBufferParser &p);

  explicit contacts_blockedSlice(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class contacts_Contacts: public Object {
 public:

  static object_ptr<contacts_Contacts> fetch(TlBufferParser &p);
};

class contacts_contactsNotModified final : public contacts_Contacts {
 public:

  static const std::int32_t ID = -1219778094;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<contacts_Contacts> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class contacts_contacts final : public contacts_Contacts {
 public:
  array<object_ptr<contact>> contacts_;
  int32 saved_count_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -353862078;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<contacts_Contacts> fetch(TlBufferParser &p);

  explicit contacts_contacts(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class contacts_found final : public Object {
 public:
  array<object_ptr<Peer>> my_results_;
  array<object_ptr<Peer>> results_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -1290580579;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<contacts_found> fetch(TlBufferParser &p);

  explicit contacts_found(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class contacts_importedContacts final : public Object {
 public:
  array<object_ptr<importedContact>> imported_;
  array<object_ptr<popularContact>> popular_invites_;
  array<int64> retry_contacts_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = 2010127419;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<contacts_importedContacts> fetch(TlBufferParser &p);

  explicit contacts_importedContacts(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class contacts_resolvedPeer final : public Object {
 public:
  object_ptr<Peer> peer_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = 2131196633;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<contacts_resolvedPeer> fetch(TlBufferParser &p);

  explicit contacts_resolvedPeer(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class contacts_TopPeers: public Object {
 public:

  static object_ptr<contacts_TopPeers> fetch(TlBufferParser &p);
};

class contacts_topPeersNotModified final : public contacts_TopPeers {
 public:

  static const std::int32_t ID = -567906571;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<contacts_TopPeers> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class contacts_topPeers final : public contacts_TopPeers {
 public:
  array<object_ptr<topPeerCategoryPeers>> categories_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = 1891070632;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<contacts_TopPeers> fetch(TlBufferParser &p);

  explicit contacts_topPeers(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class contacts_topPeersDisabled final : public contacts_TopPeers {
 public:

  static const std::int32_t ID = -1255369827;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<contacts_TopPeers> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_AppConfig: public Object {
 public:

  static object_ptr<help_AppConfig> fetch(TlBufferParser &p);
};

class help_appConfigNotModified final : public help_AppConfig {
 public:

  static const std::int32_t ID = 2094949405;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_AppConfig> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_appConfig final : public help_AppConfig {
 public:
  int32 hash_;
  object_ptr<JSONValue> config_;

  static const std::int32_t ID = -585598930;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_AppConfig> fetch(TlBufferParser &p);

  explicit help_appConfig(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_AppUpdate: public Object {
 public:

  static object_ptr<help_AppUpdate> fetch(TlBufferParser &p);
};

class help_appUpdate final : public help_AppUpdate {
 public:
  int32 flags_;
  bool can_not_skip_;
  int32 id_;
  string version_;
  string text_;
  array<object_ptr<MessageEntity>> entities_;
  object_ptr<Document> document_;
  string url_;
  object_ptr<Document> sticker_;
  enum Flags : std::int32_t { URL_MASK = 4 };

  help_appUpdate();

  static const std::int32_t ID = -860107216;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_AppUpdate> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_noAppUpdate final : public help_AppUpdate {
 public:

  static const std::int32_t ID = -1000708810;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_AppUpdate> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_configSimple final : public Object {
 public:
  int32 date_;
  int32 expires_;
  array<object_ptr<accessPointRule>> rules_;

  static const std::int32_t ID = 1515793004;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_configSimple> fetch(TlBufferParser &p);

  explicit help_configSimple(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_CountriesList: public Object {
 public:

  static object_ptr<help_CountriesList> fetch(TlBufferParser &p);
};

class help_countriesListNotModified final : public help_CountriesList {
 public:

  static const std::int32_t ID = -1815339214;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_CountriesList> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_countriesList final : public help_CountriesList {
 public:
  array<object_ptr<help_country>> countries_;
  int32 hash_;

  static const std::int32_t ID = -2016381538;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_CountriesList> fetch(TlBufferParser &p);

  explicit help_countriesList(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_country final : public Object {
 public:
  int32 flags_;
  bool hidden_;
  string iso2_;
  string default_name_;
  string name_;
  array<object_ptr<help_countryCode>> country_codes_;
  enum Flags : std::int32_t { NAME_MASK = 2 };

  help_country();

  static const std::int32_t ID = -1014526429;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_country> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_countryCode final : public Object {
 public:
  int32 flags_;
  string country_code_;
  array<string> prefixes_;
  array<string> patterns_;

  help_countryCode();

  static const std::int32_t ID = 1107543535;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_countryCode> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_DeepLinkInfo: public Object {
 public:

  static object_ptr<help_DeepLinkInfo> fetch(TlBufferParser &p);
};

class help_deepLinkInfoEmpty final : public help_DeepLinkInfo {
 public:

  static const std::int32_t ID = 1722786150;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_DeepLinkInfo> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_deepLinkInfo final : public help_DeepLinkInfo {
 public:
  int32 flags_;
  bool update_app_;
  string message_;
  array<object_ptr<MessageEntity>> entities_;

  help_deepLinkInfo();

  static const std::int32_t ID = 1783556146;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_DeepLinkInfo> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_inviteText final : public Object {
 public:
  string message_;

  static const std::int32_t ID = 415997816;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_inviteText> fetch(TlBufferParser &p);

  explicit help_inviteText(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_PassportConfig: public Object {
 public:

  static object_ptr<help_PassportConfig> fetch(TlBufferParser &p);
};

class help_passportConfigNotModified final : public help_PassportConfig {
 public:

  static const std::int32_t ID = -1078332329;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_PassportConfig> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_passportConfig final : public help_PassportConfig {
 public:
  int32 hash_;
  object_ptr<dataJSON> countries_langs_;

  static const std::int32_t ID = -1600596305;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_PassportConfig> fetch(TlBufferParser &p);

  explicit help_passportConfig(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_premiumPromo final : public Object {
 public:
  string status_text_;
  array<object_ptr<MessageEntity>> status_entities_;
  array<string> video_sections_;
  array<object_ptr<Document>> videos_;
  array<object_ptr<premiumSubscriptionOption>> period_options_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = 1395946908;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_premiumPromo> fetch(TlBufferParser &p);

  explicit help_premiumPromo(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_PromoData: public Object {
 public:

  static object_ptr<help_PromoData> fetch(TlBufferParser &p);
};

class help_promoDataEmpty final : public help_PromoData {
 public:
  int32 expires_;

  static const std::int32_t ID = -1728664459;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_PromoData> fetch(TlBufferParser &p);

  explicit help_promoDataEmpty(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_promoData final : public help_PromoData {
 public:
  int32 flags_;
  bool proxy_;
  int32 expires_;
  object_ptr<Peer> peer_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;
  string psa_type_;
  string psa_message_;
  enum Flags : std::int32_t { PSA_TYPE_MASK = 2, PSA_MESSAGE_MASK = 4 };

  help_promoData();

  static const std::int32_t ID = -1942390465;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_PromoData> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_recentMeUrls final : public Object {
 public:
  array<object_ptr<RecentMeUrl>> urls_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = 235081943;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_recentMeUrls> fetch(TlBufferParser &p);

  explicit help_recentMeUrls(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_support final : public Object {
 public:
  string phone_number_;
  object_ptr<User> user_;

  static const std::int32_t ID = 398898678;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_support> fetch(TlBufferParser &p);

  explicit help_support(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_supportName final : public Object {
 public:
  string name_;

  static const std::int32_t ID = -1945767479;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_supportName> fetch(TlBufferParser &p);

  explicit help_supportName(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_termsOfService final : public Object {
 public:
  int32 flags_;
  bool popup_;
  object_ptr<dataJSON> id_;
  string text_;
  array<object_ptr<MessageEntity>> entities_;
  int32 min_age_confirm_;
  enum Flags : std::int32_t { MIN_AGE_CONFIRM_MASK = 2 };

  help_termsOfService();

  static const std::int32_t ID = 2013922064;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_termsOfService> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_TermsOfServiceUpdate: public Object {
 public:

  static object_ptr<help_TermsOfServiceUpdate> fetch(TlBufferParser &p);
};

class help_termsOfServiceUpdateEmpty final : public help_TermsOfServiceUpdate {
 public:
  int32 expires_;

  static const std::int32_t ID = -483352705;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_TermsOfServiceUpdate> fetch(TlBufferParser &p);

  explicit help_termsOfServiceUpdateEmpty(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_termsOfServiceUpdate final : public help_TermsOfServiceUpdate {
 public:
  int32 expires_;
  object_ptr<help_termsOfService> terms_of_service_;

  static const std::int32_t ID = 686618977;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_TermsOfServiceUpdate> fetch(TlBufferParser &p);

  explicit help_termsOfServiceUpdate(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_UserInfo: public Object {
 public:

  static object_ptr<help_UserInfo> fetch(TlBufferParser &p);
};

class help_userInfoEmpty final : public help_UserInfo {
 public:

  static const std::int32_t ID = -206688531;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_UserInfo> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class help_userInfo final : public help_UserInfo {
 public:
  string message_;
  array<object_ptr<MessageEntity>> entities_;
  string author_;
  int32 date_;

  static const std::int32_t ID = 32192344;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<help_UserInfo> fetch(TlBufferParser &p);

  explicit help_userInfo(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_affectedFoundMessages final : public Object {
 public:
  int32 pts_;
  int32 pts_count_;
  int32 offset_;
  array<int32> messages_;

  static const std::int32_t ID = -275956116;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_affectedFoundMessages> fetch(TlBufferParser &p);

  explicit messages_affectedFoundMessages(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_affectedHistory final : public Object {
 public:
  int32 pts_;
  int32 pts_count_;
  int32 offset_;

  static const std::int32_t ID = -1269012015;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_affectedHistory> fetch(TlBufferParser &p);

  explicit messages_affectedHistory(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_affectedMessages final : public Object {
 public:
  int32 pts_;
  int32 pts_count_;

  static const std::int32_t ID = -2066640507;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_affectedMessages> fetch(TlBufferParser &p);

  explicit messages_affectedMessages(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_AllStickers: public Object {
 public:

  static object_ptr<messages_AllStickers> fetch(TlBufferParser &p);
};

class messages_allStickersNotModified final : public messages_AllStickers {
 public:

  static const std::int32_t ID = -395967805;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_AllStickers> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_allStickers final : public messages_AllStickers {
 public:
  int64 hash_;
  array<object_ptr<stickerSet>> sets_;

  static const std::int32_t ID = -843329861;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_AllStickers> fetch(TlBufferParser &p);

  explicit messages_allStickers(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_archivedStickers final : public Object {
 public:
  int32 count_;
  array<object_ptr<StickerSetCovered>> sets_;

  static const std::int32_t ID = 1338747336;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_archivedStickers> fetch(TlBufferParser &p);

  explicit messages_archivedStickers(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_AvailableReactions: public Object {
 public:

  static object_ptr<messages_AvailableReactions> fetch(TlBufferParser &p);
};

class messages_availableReactionsNotModified final : public messages_AvailableReactions {
 public:

  static const std::int32_t ID = -1626924713;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_AvailableReactions> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_availableReactions final : public messages_AvailableReactions {
 public:
  int32 hash_;
  array<object_ptr<availableReaction>> reactions_;

  static const std::int32_t ID = 1989032621;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_AvailableReactions> fetch(TlBufferParser &p);

  explicit messages_availableReactions(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_botApp final : public Object {
 public:
  int32 flags_;
  bool inactive_;
  bool request_write_access_;
  object_ptr<BotApp> app_;

  messages_botApp();

  static const std::int32_t ID = -347034123;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_botApp> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_botCallbackAnswer final : public Object {
 public:
  int32 flags_;
  bool alert_;
  bool has_url_;
  bool native_ui_;
  string message_;
  string url_;
  int32 cache_time_;
  enum Flags : std::int32_t { MESSAGE_MASK = 1, URL_MASK = 4 };

  messages_botCallbackAnswer();

  static const std::int32_t ID = 911761060;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_botCallbackAnswer> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_botResults final : public Object {
 public:
  int32 flags_;
  bool gallery_;
  int64 query_id_;
  string next_offset_;
  object_ptr<inlineBotSwitchPM> switch_pm_;
  object_ptr<inlineBotWebView> switch_webview_;
  array<object_ptr<BotInlineResult>> results_;
  int32 cache_time_;
  array<object_ptr<User>> users_;
  enum Flags : std::int32_t { NEXT_OFFSET_MASK = 2 };

  messages_botResults();

  static const std::int32_t ID = -534646026;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_botResults> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_chatAdminsWithInvites final : public Object {
 public:
  array<object_ptr<chatAdminWithInvites>> admins_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -1231326505;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_chatAdminsWithInvites> fetch(TlBufferParser &p);

  explicit messages_chatAdminsWithInvites(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_chatFull final : public Object {
 public:
  object_ptr<ChatFull> full_chat_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -438840932;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_chatFull> fetch(TlBufferParser &p);

  explicit messages_chatFull(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_chatInviteImporters final : public Object {
 public:
  int32 count_;
  array<object_ptr<chatInviteImporter>> importers_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -2118733814;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_chatInviteImporters> fetch(TlBufferParser &p);

  explicit messages_chatInviteImporters(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_Chats: public Object {
 public:

  static object_ptr<messages_Chats> fetch(TlBufferParser &p);
};

class messages_chats final : public messages_Chats {
 public:
  array<object_ptr<Chat>> chats_;

  static const std::int32_t ID = 1694474197;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_Chats> fetch(TlBufferParser &p);

  explicit messages_chats(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_chatsSlice final : public messages_Chats {
 public:
  int32 count_;
  array<object_ptr<Chat>> chats_;

  static const std::int32_t ID = -1663561404;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_Chats> fetch(TlBufferParser &p);

  explicit messages_chatsSlice(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_checkedHistoryImportPeer final : public Object {
 public:
  string confirm_text_;

  static const std::int32_t ID = -1571952873;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_checkedHistoryImportPeer> fetch(TlBufferParser &p);

  explicit messages_checkedHistoryImportPeer(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_DhConfig: public Object {
 public:

  static object_ptr<messages_DhConfig> fetch(TlBufferParser &p);
};

class messages_dhConfigNotModified final : public messages_DhConfig {
 public:
  bytes random_;

  static const std::int32_t ID = -1058912715;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_DhConfig> fetch(TlBufferParser &p);

  explicit messages_dhConfigNotModified(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_dhConfig final : public messages_DhConfig {
 public:
  int32 g_;
  bytes p_;
  int32 version_;
  bytes random_;

  static const std::int32_t ID = 740433629;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_DhConfig> fetch(TlBufferParser &p);

  explicit messages_dhConfig(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_Dialogs: public Object {
 public:

  static object_ptr<messages_Dialogs> fetch(TlBufferParser &p);
};

class messages_dialogs final : public messages_Dialogs {
 public:
  array<object_ptr<Dialog>> dialogs_;
  array<object_ptr<Message>> messages_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = 364538944;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_Dialogs> fetch(TlBufferParser &p);

  explicit messages_dialogs(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_dialogsSlice final : public messages_Dialogs {
 public:
  int32 count_;
  array<object_ptr<Dialog>> dialogs_;
  array<object_ptr<Message>> messages_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = 1910543603;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_Dialogs> fetch(TlBufferParser &p);

  explicit messages_dialogsSlice(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_dialogsNotModified final : public messages_Dialogs {
 public:
  int32 count_;

  static const std::int32_t ID = -253500010;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_Dialogs> fetch(TlBufferParser &p);

  explicit messages_dialogsNotModified(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_discussionMessage final : public Object {
 public:
  int32 flags_;
  array<object_ptr<Message>> messages_;
  int32 max_id_;
  int32 read_inbox_max_id_;
  int32 read_outbox_max_id_;
  int32 unread_count_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;
  enum Flags : std::int32_t { MAX_ID_MASK = 1, READ_INBOX_MAX_ID_MASK = 2, READ_OUTBOX_MAX_ID_MASK = 4 };

  messages_discussionMessage();

  static const std::int32_t ID = -1506535550;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_discussionMessage> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_EmojiGroups: public Object {
 public:

  static object_ptr<messages_EmojiGroups> fetch(TlBufferParser &p);
};

class messages_emojiGroupsNotModified final : public messages_EmojiGroups {
 public:

  static const std::int32_t ID = 1874111879;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_EmojiGroups> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_emojiGroups final : public messages_EmojiGroups {
 public:
  int32 hash_;
  array<object_ptr<emojiGroup>> groups_;

  static const std::int32_t ID = -2011186869;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_EmojiGroups> fetch(TlBufferParser &p);

  explicit messages_emojiGroups(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_ExportedChatInvite: public Object {
 public:

  static object_ptr<messages_ExportedChatInvite> fetch(TlBufferParser &p);
};

class messages_exportedChatInvite final : public messages_ExportedChatInvite {
 public:
  object_ptr<ExportedChatInvite> invite_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = 410107472;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_ExportedChatInvite> fetch(TlBufferParser &p);

  explicit messages_exportedChatInvite(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_exportedChatInviteReplaced final : public messages_ExportedChatInvite {
 public:
  object_ptr<ExportedChatInvite> invite_;
  object_ptr<ExportedChatInvite> new_invite_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = 572915951;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_ExportedChatInvite> fetch(TlBufferParser &p);

  explicit messages_exportedChatInviteReplaced(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_exportedChatInvites final : public Object {
 public:
  int32 count_;
  array<object_ptr<ExportedChatInvite>> invites_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -1111085620;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_exportedChatInvites> fetch(TlBufferParser &p);

  explicit messages_exportedChatInvites(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_FavedStickers: public Object {
 public:

  static object_ptr<messages_FavedStickers> fetch(TlBufferParser &p);
};

class messages_favedStickersNotModified final : public messages_FavedStickers {
 public:

  static const std::int32_t ID = -1634752813;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_FavedStickers> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_favedStickers final : public messages_FavedStickers {
 public:
  int64 hash_;
  array<object_ptr<stickerPack>> packs_;
  array<object_ptr<Document>> stickers_;

  static const std::int32_t ID = 750063767;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_FavedStickers> fetch(TlBufferParser &p);

  explicit messages_favedStickers(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_FeaturedStickers: public Object {
 public:

  static object_ptr<messages_FeaturedStickers> fetch(TlBufferParser &p);
};

class messages_featuredStickersNotModified final : public messages_FeaturedStickers {
 public:
  int32 count_;

  static const std::int32_t ID = -958657434;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_FeaturedStickers> fetch(TlBufferParser &p);

  explicit messages_featuredStickersNotModified(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_featuredStickers final : public messages_FeaturedStickers {
 public:
  int32 flags_;
  bool premium_;
  int64 hash_;
  int32 count_;
  array<object_ptr<StickerSetCovered>> sets_;
  array<int64> unread_;

  messages_featuredStickers();

  static const std::int32_t ID = -1103615738;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_FeaturedStickers> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_forumTopics final : public Object {
 public:
  int32 flags_;
  bool order_by_create_date_;
  int32 count_;
  array<object_ptr<ForumTopic>> topics_;
  array<object_ptr<Message>> messages_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;
  int32 pts_;

  messages_forumTopics();

  static const std::int32_t ID = 913709011;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_forumTopics> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_FoundStickerSets: public Object {
 public:

  static object_ptr<messages_FoundStickerSets> fetch(TlBufferParser &p);
};

class messages_foundStickerSetsNotModified final : public messages_FoundStickerSets {
 public:

  static const std::int32_t ID = 223655517;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_FoundStickerSets> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_foundStickerSets final : public messages_FoundStickerSets {
 public:
  int64 hash_;
  array<object_ptr<StickerSetCovered>> sets_;

  static const std::int32_t ID = -1963942446;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_FoundStickerSets> fetch(TlBufferParser &p);

  explicit messages_foundStickerSets(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_highScores final : public Object {
 public:
  array<object_ptr<highScore>> scores_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -1707344487;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_highScores> fetch(TlBufferParser &p);

  explicit messages_highScores(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_historyImport final : public Object {
 public:
  int64 id_;

  static const std::int32_t ID = 375566091;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_historyImport> fetch(TlBufferParser &p);

  explicit messages_historyImport(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_historyImportParsed final : public Object {
 public:
  int32 flags_;
  bool pm_;
  bool group_;
  string title_;
  enum Flags : std::int32_t { TITLE_MASK = 4 };

  messages_historyImportParsed();

  static const std::int32_t ID = 1578088377;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_historyImportParsed> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_inactiveChats final : public Object {
 public:
  array<int32> dates_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -1456996667;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_inactiveChats> fetch(TlBufferParser &p);

  explicit messages_inactiveChats(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_messageEditData final : public Object {
 public:
  int32 flags_;
  bool caption_;

  messages_messageEditData();

  static const std::int32_t ID = 649453030;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_messageEditData> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_messageReactionsList final : public Object {
 public:
  int32 flags_;
  int32 count_;
  array<object_ptr<messagePeerReaction>> reactions_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;
  string next_offset_;
  enum Flags : std::int32_t { NEXT_OFFSET_MASK = 1 };

  messages_messageReactionsList();

  static const std::int32_t ID = 834488621;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_messageReactionsList> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_messageViews final : public Object {
 public:
  array<object_ptr<messageViews>> views_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -1228606141;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_messageViews> fetch(TlBufferParser &p);

  explicit messages_messageViews(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_Messages: public Object {
 public:

  static object_ptr<messages_Messages> fetch(TlBufferParser &p);
};

class messages_messages final : public messages_Messages {
 public:
  array<object_ptr<Message>> messages_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -1938715001;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_Messages> fetch(TlBufferParser &p);

  explicit messages_messages(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_messagesSlice final : public messages_Messages {
 public:
  int32 flags_;
  bool inexact_;
  int32 count_;
  int32 next_rate_;
  int32 offset_id_offset_;
  array<object_ptr<Message>> messages_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;
  enum Flags : std::int32_t { NEXT_RATE_MASK = 1, OFFSET_ID_OFFSET_MASK = 4 };

  messages_messagesSlice();

  static const std::int32_t ID = 978610270;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_Messages> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_channelMessages final : public messages_Messages {
 public:
  int32 flags_;
  bool inexact_;
  int32 pts_;
  int32 count_;
  int32 offset_id_offset_;
  array<object_ptr<Message>> messages_;
  array<object_ptr<ForumTopic>> topics_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;
  enum Flags : std::int32_t { OFFSET_ID_OFFSET_MASK = 4 };

  messages_channelMessages();

  static const std::int32_t ID = -948520370;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_Messages> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_messagesNotModified final : public messages_Messages {
 public:
  int32 count_;

  static const std::int32_t ID = 1951620897;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_Messages> fetch(TlBufferParser &p);

  explicit messages_messagesNotModified(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_peerDialogs final : public Object {
 public:
  array<object_ptr<Dialog>> dialogs_;
  array<object_ptr<Message>> messages_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;
  object_ptr<updates_state> state_;

  static const std::int32_t ID = 863093588;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_peerDialogs> fetch(TlBufferParser &p);

  explicit messages_peerDialogs(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_peerSettings final : public Object {
 public:
  object_ptr<peerSettings> settings_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = 1753266509;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_peerSettings> fetch(TlBufferParser &p);

  explicit messages_peerSettings(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_Reactions: public Object {
 public:

  static object_ptr<messages_Reactions> fetch(TlBufferParser &p);
};

class messages_reactionsNotModified final : public messages_Reactions {
 public:

  static const std::int32_t ID = -1334846497;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_Reactions> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_reactions final : public messages_Reactions {
 public:
  int64 hash_;
  array<object_ptr<Reaction>> reactions_;

  static const std::int32_t ID = -352454890;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_Reactions> fetch(TlBufferParser &p);

  explicit messages_reactions(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_RecentStickers: public Object {
 public:

  static object_ptr<messages_RecentStickers> fetch(TlBufferParser &p);
};

class messages_recentStickersNotModified final : public messages_RecentStickers {
 public:

  static const std::int32_t ID = 186120336;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_RecentStickers> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_recentStickers final : public messages_RecentStickers {
 public:
  int64 hash_;
  array<object_ptr<stickerPack>> packs_;
  array<object_ptr<Document>> stickers_;
  array<int32> dates_;

  static const std::int32_t ID = -1999405994;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_RecentStickers> fetch(TlBufferParser &p);

  explicit messages_recentStickers(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_SavedGifs: public Object {
 public:

  static object_ptr<messages_SavedGifs> fetch(TlBufferParser &p);
};

class messages_savedGifsNotModified final : public messages_SavedGifs {
 public:

  static const std::int32_t ID = -402498398;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_SavedGifs> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_savedGifs final : public messages_SavedGifs {
 public:
  int64 hash_;
  array<object_ptr<Document>> gifs_;

  static const std::int32_t ID = -2069878259;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_SavedGifs> fetch(TlBufferParser &p);

  explicit messages_savedGifs(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_searchCounter final : public Object {
 public:
  int32 flags_;
  bool inexact_;
  object_ptr<MessagesFilter> filter_;
  int32 count_;

  messages_searchCounter();

  static const std::int32_t ID = -398136321;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_searchCounter> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_searchResultsCalendar final : public Object {
 public:
  int32 flags_;
  bool inexact_;
  int32 count_;
  int32 min_date_;
  int32 min_msg_id_;
  int32 offset_id_offset_;
  array<object_ptr<searchResultsCalendarPeriod>> periods_;
  array<object_ptr<Message>> messages_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;
  enum Flags : std::int32_t { OFFSET_ID_OFFSET_MASK = 2 };

  messages_searchResultsCalendar();

  static const std::int32_t ID = 343859772;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_searchResultsCalendar> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_searchResultsPositions final : public Object {
 public:
  int32 count_;
  array<object_ptr<searchResultPosition>> positions_;

  static const std::int32_t ID = 1404185519;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_searchResultsPositions> fetch(TlBufferParser &p);

  explicit messages_searchResultsPositions(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_SentEncryptedMessage: public Object {
 public:

  static object_ptr<messages_SentEncryptedMessage> fetch(TlBufferParser &p);
};

class messages_sentEncryptedMessage final : public messages_SentEncryptedMessage {
 public:
  int32 date_;

  static const std::int32_t ID = 1443858741;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_SentEncryptedMessage> fetch(TlBufferParser &p);

  explicit messages_sentEncryptedMessage(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_sentEncryptedFile final : public messages_SentEncryptedMessage {
 public:
  int32 date_;
  object_ptr<EncryptedFile> file_;

  static const std::int32_t ID = -1802240206;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_SentEncryptedMessage> fetch(TlBufferParser &p);

  explicit messages_sentEncryptedFile(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_SponsoredMessages: public Object {
 public:

  static object_ptr<messages_SponsoredMessages> fetch(TlBufferParser &p);
};

class messages_sponsoredMessages final : public messages_SponsoredMessages {
 public:
  int32 flags_;
  int32 posts_between_;
  array<object_ptr<sponsoredMessage>> messages_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;
  enum Flags : std::int32_t { POSTS_BETWEEN_MASK = 1 };

  messages_sponsoredMessages();

  static const std::int32_t ID = -907141753;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_SponsoredMessages> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_sponsoredMessagesEmpty final : public messages_SponsoredMessages {
 public:

  static const std::int32_t ID = 406407439;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_SponsoredMessages> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_StickerSet: public Object {
 public:

  static object_ptr<messages_StickerSet> fetch(TlBufferParser &p);
};

class messages_stickerSet final : public messages_StickerSet {
 public:
  object_ptr<stickerSet> set_;
  array<object_ptr<stickerPack>> packs_;
  array<object_ptr<stickerKeyword>> keywords_;
  array<object_ptr<Document>> documents_;

  messages_stickerSet(object_ptr<stickerSet> &&set_, array<object_ptr<stickerPack>> &&packs_, array<object_ptr<stickerKeyword>> &&keywords_, array<object_ptr<Document>> &&documents_);

  static const std::int32_t ID = 1846886166;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_StickerSet> fetch(TlBufferParser &p);

  explicit messages_stickerSet(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_stickerSetNotModified final : public messages_StickerSet {
 public:

  static const std::int32_t ID = -738646805;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_StickerSet> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_StickerSetInstallResult: public Object {
 public:

  static object_ptr<messages_StickerSetInstallResult> fetch(TlBufferParser &p);
};

class messages_stickerSetInstallResultSuccess final : public messages_StickerSetInstallResult {
 public:

  static const std::int32_t ID = 946083368;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_StickerSetInstallResult> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_stickerSetInstallResultArchive final : public messages_StickerSetInstallResult {
 public:
  array<object_ptr<StickerSetCovered>> sets_;

  static const std::int32_t ID = 904138920;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_StickerSetInstallResult> fetch(TlBufferParser &p);

  explicit messages_stickerSetInstallResultArchive(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_Stickers: public Object {
 public:

  static object_ptr<messages_Stickers> fetch(TlBufferParser &p);
};

class messages_stickersNotModified final : public messages_Stickers {
 public:

  static const std::int32_t ID = -244016606;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_Stickers> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_stickers final : public messages_Stickers {
 public:
  int64 hash_;
  array<object_ptr<Document>> stickers_;

  static const std::int32_t ID = 816245886;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_Stickers> fetch(TlBufferParser &p);

  explicit messages_stickers(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_transcribedAudio final : public Object {
 public:
  int32 flags_;
  bool pending_;
  int64 transcription_id_;
  string text_;

  messages_transcribedAudio();

  static const std::int32_t ID = -1821037486;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_transcribedAudio> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_translateResult final : public Object {
 public:
  array<object_ptr<textWithEntities>> result_;

  static const std::int32_t ID = 870003448;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_translateResult> fetch(TlBufferParser &p);

  explicit messages_translateResult(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class messages_votesList final : public Object {
 public:
  int32 flags_;
  int32 count_;
  array<object_ptr<MessageUserVote>> votes_;
  array<object_ptr<User>> users_;
  string next_offset_;
  enum Flags : std::int32_t { NEXT_OFFSET_MASK = 1 };

  messages_votesList();

  static const std::int32_t ID = 136574537;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<messages_votesList> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class payments_bankCardData final : public Object {
 public:
  string title_;
  array<object_ptr<bankCardOpenUrl>> open_urls_;

  static const std::int32_t ID = 1042605427;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<payments_bankCardData> fetch(TlBufferParser &p);

  explicit payments_bankCardData(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class payments_exportedInvoice final : public Object {
 public:
  string url_;

  static const std::int32_t ID = -1362048039;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<payments_exportedInvoice> fetch(TlBufferParser &p);

  explicit payments_exportedInvoice(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class payments_paymentForm final : public Object {
 public:
  int32 flags_;
  bool can_save_credentials_;
  bool password_missing_;
  int64 form_id_;
  int64 bot_id_;
  string title_;
  string description_;
  object_ptr<WebDocument> photo_;
  object_ptr<invoice> invoice_;
  int64 provider_id_;
  string url_;
  string native_provider_;
  object_ptr<dataJSON> native_params_;
  array<object_ptr<paymentFormMethod>> additional_methods_;
  object_ptr<paymentRequestedInfo> saved_info_;
  array<object_ptr<paymentSavedCredentialsCard>> saved_credentials_;
  array<object_ptr<User>> users_;
  enum Flags : std::int32_t { NATIVE_PROVIDER_MASK = 16 };

  payments_paymentForm();

  static const std::int32_t ID = -1610250415;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<payments_paymentForm> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class payments_paymentReceipt final : public Object {
 public:
  int32 flags_;
  int32 date_;
  int64 bot_id_;
  int64 provider_id_;
  string title_;
  string description_;
  object_ptr<WebDocument> photo_;
  object_ptr<invoice> invoice_;
  object_ptr<paymentRequestedInfo> info_;
  object_ptr<shippingOption> shipping_;
  int64 tip_amount_;
  string currency_;
  int64 total_amount_;
  string credentials_title_;
  array<object_ptr<User>> users_;
  enum Flags : std::int32_t { TIP_AMOUNT_MASK = 8 };

  payments_paymentReceipt();

  static const std::int32_t ID = 1891958275;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<payments_paymentReceipt> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class payments_PaymentResult: public Object {
 public:

  static object_ptr<payments_PaymentResult> fetch(TlBufferParser &p);
};

class payments_paymentResult final : public payments_PaymentResult {
 public:
  object_ptr<Updates> updates_;

  static const std::int32_t ID = 1314881805;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<payments_PaymentResult> fetch(TlBufferParser &p);

  explicit payments_paymentResult(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class payments_paymentVerificationNeeded final : public payments_PaymentResult {
 public:
  string url_;

  static const std::int32_t ID = -666824391;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<payments_PaymentResult> fetch(TlBufferParser &p);

  explicit payments_paymentVerificationNeeded(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class payments_savedInfo final : public Object {
 public:
  int32 flags_;
  bool has_saved_credentials_;
  object_ptr<paymentRequestedInfo> saved_info_;

  payments_savedInfo();

  static const std::int32_t ID = -74456004;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<payments_savedInfo> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class payments_validatedRequestedInfo final : public Object {
 public:
  int32 flags_;
  string id_;
  array<object_ptr<shippingOption>> shipping_options_;
  enum Flags : std::int32_t { ID_MASK = 1 };

  payments_validatedRequestedInfo();

  static const std::int32_t ID = -784000893;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<payments_validatedRequestedInfo> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class phone_exportedGroupCallInvite final : public Object {
 public:
  string link_;

  static const std::int32_t ID = 541839704;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<phone_exportedGroupCallInvite> fetch(TlBufferParser &p);

  explicit phone_exportedGroupCallInvite(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class phone_groupCall final : public Object {
 public:
  object_ptr<GroupCall> call_;
  array<object_ptr<groupCallParticipant>> participants_;
  string participants_next_offset_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -1636664659;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<phone_groupCall> fetch(TlBufferParser &p);

  explicit phone_groupCall(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class phone_groupCallStreamChannels final : public Object {
 public:
  array<object_ptr<groupCallStreamChannel>> channels_;

  static const std::int32_t ID = -790330702;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<phone_groupCallStreamChannels> fetch(TlBufferParser &p);

  explicit phone_groupCallStreamChannels(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class phone_groupCallStreamRtmpUrl final : public Object {
 public:
  string url_;
  string key_;

  static const std::int32_t ID = 767505458;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<phone_groupCallStreamRtmpUrl> fetch(TlBufferParser &p);

  explicit phone_groupCallStreamRtmpUrl(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class phone_groupParticipants final : public Object {
 public:
  int32 count_;
  array<object_ptr<groupCallParticipant>> participants_;
  string next_offset_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;
  int32 version_;

  phone_groupParticipants(int32 count_, array<object_ptr<groupCallParticipant>> &&participants_, string const &next_offset_, array<object_ptr<Chat>> &&chats_, array<object_ptr<User>> &&users_, int32 version_);

  static const std::int32_t ID = -193506890;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<phone_groupParticipants> fetch(TlBufferParser &p);

  explicit phone_groupParticipants(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class phone_joinAsPeers final : public Object {
 public:
  array<object_ptr<Peer>> peers_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -1343921601;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<phone_joinAsPeers> fetch(TlBufferParser &p);

  explicit phone_joinAsPeers(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class phone_phoneCall final : public Object {
 public:
  object_ptr<PhoneCall> phone_call_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -326966976;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<phone_phoneCall> fetch(TlBufferParser &p);

  explicit phone_phoneCall(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class photos_photo final : public Object {
 public:
  object_ptr<Photo> photo_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = 539045032;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<photos_photo> fetch(TlBufferParser &p);

  explicit photos_photo(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class photos_Photos: public Object {
 public:

  static object_ptr<photos_Photos> fetch(TlBufferParser &p);
};

class photos_photos final : public photos_Photos {
 public:
  array<object_ptr<Photo>> photos_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -1916114267;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<photos_Photos> fetch(TlBufferParser &p);

  explicit photos_photos(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class photos_photosSlice final : public photos_Photos {
 public:
  int32 count_;
  array<object_ptr<Photo>> photos_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = 352657236;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<photos_Photos> fetch(TlBufferParser &p);

  explicit photos_photosSlice(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class stats_broadcastStats final : public Object {
 public:
  object_ptr<statsDateRangeDays> period_;
  object_ptr<statsAbsValueAndPrev> followers_;
  object_ptr<statsAbsValueAndPrev> views_per_post_;
  object_ptr<statsAbsValueAndPrev> shares_per_post_;
  object_ptr<statsPercentValue> enabled_notifications_;
  object_ptr<StatsGraph> growth_graph_;
  object_ptr<StatsGraph> followers_graph_;
  object_ptr<StatsGraph> mute_graph_;
  object_ptr<StatsGraph> top_hours_graph_;
  object_ptr<StatsGraph> interactions_graph_;
  object_ptr<StatsGraph> iv_interactions_graph_;
  object_ptr<StatsGraph> views_by_source_graph_;
  object_ptr<StatsGraph> new_followers_by_source_graph_;
  object_ptr<StatsGraph> languages_graph_;
  array<object_ptr<messageInteractionCounters>> recent_message_interactions_;

  static const std::int32_t ID = -1107852396;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<stats_broadcastStats> fetch(TlBufferParser &p);

  explicit stats_broadcastStats(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class stats_megagroupStats final : public Object {
 public:
  object_ptr<statsDateRangeDays> period_;
  object_ptr<statsAbsValueAndPrev> members_;
  object_ptr<statsAbsValueAndPrev> messages_;
  object_ptr<statsAbsValueAndPrev> viewers_;
  object_ptr<statsAbsValueAndPrev> posters_;
  object_ptr<StatsGraph> growth_graph_;
  object_ptr<StatsGraph> members_graph_;
  object_ptr<StatsGraph> new_members_by_source_graph_;
  object_ptr<StatsGraph> languages_graph_;
  object_ptr<StatsGraph> messages_graph_;
  object_ptr<StatsGraph> actions_graph_;
  object_ptr<StatsGraph> top_hours_graph_;
  object_ptr<StatsGraph> weekdays_graph_;
  array<object_ptr<statsGroupTopPoster>> top_posters_;
  array<object_ptr<statsGroupTopAdmin>> top_admins_;
  array<object_ptr<statsGroupTopInviter>> top_inviters_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = -276825834;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<stats_megagroupStats> fetch(TlBufferParser &p);

  explicit stats_megagroupStats(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class stats_messageStats final : public Object {
 public:
  object_ptr<StatsGraph> views_graph_;

  static const std::int32_t ID = -1986399595;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<stats_messageStats> fetch(TlBufferParser &p);

  explicit stats_messageStats(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class stickers_suggestedShortName final : public Object {
 public:
  string short_name_;

  static const std::int32_t ID = -2046910401;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<stickers_suggestedShortName> fetch(TlBufferParser &p);

  explicit stickers_suggestedShortName(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class storage_FileType: public Object {
 public:

  static object_ptr<storage_FileType> fetch(TlBufferParser &p);
};

class storage_fileUnknown final : public storage_FileType {
 public:

  static const std::int32_t ID = -1432995067;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<storage_FileType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class storage_filePartial final : public storage_FileType {
 public:

  static const std::int32_t ID = 1086091090;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<storage_FileType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class storage_fileJpeg final : public storage_FileType {
 public:

  static const std::int32_t ID = 8322574;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<storage_FileType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class storage_fileGif final : public storage_FileType {
 public:

  static const std::int32_t ID = -891180321;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<storage_FileType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class storage_filePng final : public storage_FileType {
 public:

  static const std::int32_t ID = 172975040;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<storage_FileType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class storage_filePdf final : public storage_FileType {
 public:

  static const std::int32_t ID = -1373745011;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<storage_FileType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class storage_fileMp3 final : public storage_FileType {
 public:

  static const std::int32_t ID = 1384777335;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<storage_FileType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class storage_fileMov final : public storage_FileType {
 public:

  static const std::int32_t ID = 1258941372;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<storage_FileType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class storage_fileMp4 final : public storage_FileType {
 public:

  static const std::int32_t ID = -1278304028;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<storage_FileType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class storage_fileWebp final : public storage_FileType {
 public:

  static const std::int32_t ID = 276907596;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<storage_FileType> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updates_ChannelDifference: public Object {
 public:

  static object_ptr<updates_ChannelDifference> fetch(TlBufferParser &p);
};

class updates_channelDifferenceEmpty final : public updates_ChannelDifference {
 public:
  int32 flags_;
  bool final_;
  int32 pts_;
  int32 timeout_;
  enum Flags : std::int32_t { TIMEOUT_MASK = 2 };

  updates_channelDifferenceEmpty();

  static const std::int32_t ID = 1041346555;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<updates_ChannelDifference> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updates_channelDifferenceTooLong final : public updates_ChannelDifference {
 public:
  int32 flags_;
  bool final_;
  int32 timeout_;
  object_ptr<Dialog> dialog_;
  array<object_ptr<Message>> messages_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;
  enum Flags : std::int32_t { TIMEOUT_MASK = 2 };

  updates_channelDifferenceTooLong();

  static const std::int32_t ID = -1531132162;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<updates_ChannelDifference> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updates_channelDifference final : public updates_ChannelDifference {
 public:
  int32 flags_;
  bool final_;
  int32 pts_;
  int32 timeout_;
  array<object_ptr<Message>> new_messages_;
  array<object_ptr<Update>> other_updates_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;
  enum Flags : std::int32_t { TIMEOUT_MASK = 2 };

  updates_channelDifference();

  static const std::int32_t ID = 543450958;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<updates_ChannelDifference> fetch(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updates_Difference: public Object {
 public:

  static object_ptr<updates_Difference> fetch(TlBufferParser &p);
};

class updates_differenceEmpty final : public updates_Difference {
 public:
  int32 date_;
  int32 seq_;

  static const std::int32_t ID = 1567990072;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<updates_Difference> fetch(TlBufferParser &p);

  explicit updates_differenceEmpty(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updates_difference final : public updates_Difference {
 public:
  array<object_ptr<Message>> new_messages_;
  array<object_ptr<EncryptedMessage>> new_encrypted_messages_;
  array<object_ptr<Update>> other_updates_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;
  object_ptr<updates_state> state_;

  static const std::int32_t ID = 16030880;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<updates_Difference> fetch(TlBufferParser &p);

  explicit updates_difference(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updates_differenceSlice final : public updates_Difference {
 public:
  array<object_ptr<Message>> new_messages_;
  array<object_ptr<EncryptedMessage>> new_encrypted_messages_;
  array<object_ptr<Update>> other_updates_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;
  object_ptr<updates_state> intermediate_state_;

  static const std::int32_t ID = -1459938943;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<updates_Difference> fetch(TlBufferParser &p);

  explicit updates_differenceSlice(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updates_differenceTooLong final : public updates_Difference {
 public:
  int32 pts_;

  static const std::int32_t ID = 1258196845;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<updates_Difference> fetch(TlBufferParser &p);

  explicit updates_differenceTooLong(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class updates_state final : public Object {
 public:
  int32 pts_;
  int32 qts_;
  int32 date_;
  int32 seq_;
  int32 unread_count_;

  static const std::int32_t ID = -1519637954;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<updates_state> fetch(TlBufferParser &p);

  explicit updates_state(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class upload_CdnFile: public Object {
 public:

  static object_ptr<upload_CdnFile> fetch(TlBufferParser &p);
};

class upload_cdnFileReuploadNeeded final : public upload_CdnFile {
 public:
  bytes request_token_;

  static const std::int32_t ID = -290921362;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<upload_CdnFile> fetch(TlBufferParser &p);

  explicit upload_cdnFileReuploadNeeded(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class upload_cdnFile final : public upload_CdnFile {
 public:
  bytes bytes_;

  static const std::int32_t ID = -1449145777;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<upload_CdnFile> fetch(TlBufferParser &p);

  explicit upload_cdnFile(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class upload_File: public Object {
 public:

  static object_ptr<upload_File> fetch(TlBufferParser &p);
};

class upload_file final : public upload_File {
 public:
  object_ptr<storage_FileType> type_;
  int32 mtime_;
  bytes bytes_;

  static const std::int32_t ID = 157948117;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<upload_File> fetch(TlBufferParser &p);

  explicit upload_file(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class upload_fileCdnRedirect final : public upload_File {
 public:
  int32 dc_id_;
  bytes file_token_;
  bytes encryption_key_;
  bytes encryption_iv_;
  array<object_ptr<fileHash>> file_hashes_;

  static const std::int32_t ID = -242427324;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<upload_File> fetch(TlBufferParser &p);

  explicit upload_fileCdnRedirect(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class upload_webFile final : public Object {
 public:
  int32 size_;
  string mime_type_;
  object_ptr<storage_FileType> file_type_;
  int32 mtime_;
  bytes bytes_;

  static const std::int32_t ID = 568808380;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<upload_webFile> fetch(TlBufferParser &p);

  explicit upload_webFile(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class users_userFull final : public Object {
 public:
  object_ptr<userFull> full_user_;
  array<object_ptr<Chat>> chats_;
  array<object_ptr<User>> users_;

  static const std::int32_t ID = 997004590;
  std::int32_t get_id() const final {
    return ID;
  }

  static object_ptr<users_userFull> fetch(TlBufferParser &p);

  explicit users_userFull(TlBufferParser &p);

  void store(TlStorerToString &s, const char *field_name) const final;
};

class account_acceptAuthorization final : public Function {
 public:
  int64 bot_id_;
  string scope_;
  string public_key_;
  array<object_ptr<secureValueHash>> value_hashes_;
  object_ptr<secureCredentialsEncrypted> credentials_;

  account_acceptAuthorization(int64 bot_id_, string const &scope_, string const &public_key_, array<object_ptr<secureValueHash>> &&value_hashes_, object_ptr<secureCredentialsEncrypted> &&credentials_);

  static const std::int32_t ID = -202552205;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_cancelPasswordEmail final : public Function {
 public:

  static const std::int32_t ID = -1043606090;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_changeAuthorizationSettings final : public Function {
 public:
  int32 flags_;
  int64 hash_;
  bool encrypted_requests_disabled_;
  bool call_requests_disabled_;
  enum Flags : std::int32_t { ENCRYPTED_REQUESTS_DISABLED_MASK = 1, CALL_REQUESTS_DISABLED_MASK = 2 };
  mutable int32 var0;

  account_changeAuthorizationSettings(int32 flags_, int64 hash_, bool encrypted_requests_disabled_, bool call_requests_disabled_);

  static const std::int32_t ID = 1089766498;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_changePhone final : public Function {
 public:
  string phone_number_;
  string phone_code_hash_;
  string phone_code_;

  account_changePhone(string const &phone_number_, string const &phone_code_hash_, string const &phone_code_);

  static const std::int32_t ID = 1891839707;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<User>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_checkUsername final : public Function {
 public:
  string username_;

  explicit account_checkUsername(string const &username_);

  static const std::int32_t ID = 655677548;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_clearRecentEmojiStatuses final : public Function {
 public:

  static const std::int32_t ID = 404757166;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_confirmPasswordEmail final : public Function {
 public:
  string code_;

  explicit account_confirmPasswordEmail(string const &code_);

  static const std::int32_t ID = -1881204448;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_confirmPhone final : public Function {
 public:
  string phone_code_hash_;
  string phone_code_;

  account_confirmPhone(string const &phone_code_hash_, string const &phone_code_);

  static const std::int32_t ID = 1596029123;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_createTheme final : public Function {
 public:
  int32 flags_;
  string slug_;
  string title_;
  object_ptr<InputDocument> document_;
  array<object_ptr<inputThemeSettings>> settings_;
  enum Flags : std::int32_t { DOCUMENT_MASK = 4, SETTINGS_MASK = 8 };
  mutable int32 var0;

  account_createTheme(int32 flags_, string const &slug_, string const &title_, object_ptr<InputDocument> &&document_, array<object_ptr<inputThemeSettings>> &&settings_);

  static const std::int32_t ID = 1697530880;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<theme>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_declinePasswordReset final : public Function {
 public:

  static const std::int32_t ID = 1284770294;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_deleteAccount final : public Function {
 public:
  int32 flags_;
  string reason_;
  object_ptr<InputCheckPasswordSRP> password_;
  enum Flags : std::int32_t { PASSWORD_MASK = 1 };
  mutable int32 var0;

  account_deleteAccount(int32 flags_, string const &reason_, object_ptr<InputCheckPasswordSRP> &&password_);

  static const std::int32_t ID = -1564422284;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_deleteAutoSaveExceptions final : public Function {
 public:

  static const std::int32_t ID = 1404829728;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_deleteSecureValue final : public Function {
 public:
  array<object_ptr<SecureValueType>> types_;

  explicit account_deleteSecureValue(array<object_ptr<SecureValueType>> &&types_);

  static const std::int32_t ID = -1199522741;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_finishTakeoutSession final : public Function {
 public:
  int32 flags_;
  bool success_;
  enum Flags : std::int32_t { SUCCESS_MASK = 1 };
  mutable int32 var0;

  account_finishTakeoutSession(int32 flags_, bool success_);

  static const std::int32_t ID = 489050862;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getAccountTTL final : public Function {
 public:

  static const std::int32_t ID = 150761757;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<accountDaysTTL>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getAllSecureValues final : public Function {
 public:

  static const std::int32_t ID = -1299661699;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<secureValue>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getAuthorizationForm final : public Function {
 public:
  int64 bot_id_;
  string scope_;
  string public_key_;

  account_getAuthorizationForm(int64 bot_id_, string const &scope_, string const &public_key_);

  static const std::int32_t ID = -1456907910;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_authorizationForm>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getAuthorizations final : public Function {
 public:

  static const std::int32_t ID = -484392616;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_authorizations>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getAutoDownloadSettings final : public Function {
 public:

  static const std::int32_t ID = 1457130303;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_autoDownloadSettings>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getAutoSaveSettings final : public Function {
 public:

  static const std::int32_t ID = -1379156774;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_autoSaveSettings>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getChatThemes final : public Function {
 public:
  int64 hash_;

  explicit account_getChatThemes(int64 hash_);

  static const std::int32_t ID = -700916087;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_Themes>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getContactSignUpNotification final : public Function {
 public:

  static const std::int32_t ID = -1626880216;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getContentSettings final : public Function {
 public:

  static const std::int32_t ID = -1952756306;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_contentSettings>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getDefaultEmojiStatuses final : public Function {
 public:
  int64 hash_;

  explicit account_getDefaultEmojiStatuses(int64 hash_);

  static const std::int32_t ID = -696962170;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_EmojiStatuses>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getDefaultGroupPhotoEmojis final : public Function {
 public:
  int64 hash_;

  explicit account_getDefaultGroupPhotoEmojis(int64 hash_);

  static const std::int32_t ID = -1856479058;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<EmojiList>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getDefaultProfilePhotoEmojis final : public Function {
 public:
  int64 hash_;

  explicit account_getDefaultProfilePhotoEmojis(int64 hash_);

  static const std::int32_t ID = -495647960;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<EmojiList>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getGlobalPrivacySettings final : public Function {
 public:

  static const std::int32_t ID = -349483786;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<globalPrivacySettings>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getMultiWallPapers final : public Function {
 public:
  array<object_ptr<InputWallPaper>> wallpapers_;

  explicit account_getMultiWallPapers(array<object_ptr<InputWallPaper>> &&wallpapers_);

  static const std::int32_t ID = 1705865692;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<WallPaper>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getNotifyExceptions final : public Function {
 public:
  int32 flags_;
  bool compare_sound_;
  object_ptr<InputNotifyPeer> peer_;
  enum Flags : std::int32_t { COMPARE_SOUND_MASK = 2, PEER_MASK = 1 };
  mutable int32 var0;

  account_getNotifyExceptions(int32 flags_, bool compare_sound_, object_ptr<InputNotifyPeer> &&peer_);

  static const std::int32_t ID = 1398240377;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getNotifySettings final : public Function {
 public:
  object_ptr<InputNotifyPeer> peer_;

  explicit account_getNotifySettings(object_ptr<InputNotifyPeer> &&peer_);

  static const std::int32_t ID = 313765169;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<peerNotifySettings>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getPassword final : public Function {
 public:

  static const std::int32_t ID = 1418342645;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_password>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getPasswordSettings final : public Function {
 public:
  object_ptr<InputCheckPasswordSRP> password_;

  explicit account_getPasswordSettings(object_ptr<InputCheckPasswordSRP> &&password_);

  static const std::int32_t ID = -1663767815;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_passwordSettings>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getPrivacy final : public Function {
 public:
  object_ptr<InputPrivacyKey> key_;

  explicit account_getPrivacy(object_ptr<InputPrivacyKey> &&key_);

  static const std::int32_t ID = -623130288;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_privacyRules>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getRecentEmojiStatuses final : public Function {
 public:
  int64 hash_;

  explicit account_getRecentEmojiStatuses(int64 hash_);

  static const std::int32_t ID = 257392901;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_EmojiStatuses>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getSavedRingtones final : public Function {
 public:
  int64 hash_;

  explicit account_getSavedRingtones(int64 hash_);

  static const std::int32_t ID = -510647672;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_SavedRingtones>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getSecureValue final : public Function {
 public:
  array<object_ptr<SecureValueType>> types_;

  explicit account_getSecureValue(array<object_ptr<SecureValueType>> &&types_);

  static const std::int32_t ID = 1936088002;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<secureValue>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getTheme final : public Function {
 public:
  string format_;
  object_ptr<InputTheme> theme_;

  account_getTheme(string const &format_, object_ptr<InputTheme> &&theme_);

  static const std::int32_t ID = 978872812;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<theme>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getThemes final : public Function {
 public:
  string format_;
  int64 hash_;

  account_getThemes(string const &format_, int64 hash_);

  static const std::int32_t ID = 1913054296;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_Themes>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getTmpPassword final : public Function {
 public:
  object_ptr<InputCheckPasswordSRP> password_;
  int32 period_;

  account_getTmpPassword(object_ptr<InputCheckPasswordSRP> &&password_, int32 period_);

  static const std::int32_t ID = 1151208273;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_tmpPassword>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getWallPaper final : public Function {
 public:
  object_ptr<InputWallPaper> wallpaper_;

  explicit account_getWallPaper(object_ptr<InputWallPaper> &&wallpaper_);

  static const std::int32_t ID = -57811990;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<WallPaper>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getWallPapers final : public Function {
 public:
  int64 hash_;

  explicit account_getWallPapers(int64 hash_);

  static const std::int32_t ID = 127302966;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_WallPapers>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_getWebAuthorizations final : public Function {
 public:

  static const std::int32_t ID = 405695855;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_webAuthorizations>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_initTakeoutSession final : public Function {
 public:
  int32 flags_;
  bool contacts_;
  bool message_users_;
  bool message_chats_;
  bool message_megagroups_;
  bool message_channels_;
  bool files_;
  int64 file_max_size_;
  enum Flags : std::int32_t { CONTACTS_MASK = 1, MESSAGE_USERS_MASK = 2, MESSAGE_CHATS_MASK = 4, MESSAGE_MEGAGROUPS_MASK = 8, MESSAGE_CHANNELS_MASK = 16, FILES_MASK = 32, FILE_MAX_SIZE_MASK = 32 };
  mutable int32 var0;

  account_initTakeoutSession(int32 flags_, bool contacts_, bool message_users_, bool message_chats_, bool message_megagroups_, bool message_channels_, bool files_, int64 file_max_size_);

  static const std::int32_t ID = -1896617296;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_takeout>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_installTheme final : public Function {
 public:
  int32 flags_;
  bool dark_;
  object_ptr<InputTheme> theme_;
  string format_;
  object_ptr<BaseTheme> base_theme_;
  enum Flags : std::int32_t { DARK_MASK = 1, THEME_MASK = 2, FORMAT_MASK = 4, BASE_THEME_MASK = 8 };
  mutable int32 var0;

  account_installTheme(int32 flags_, bool dark_, object_ptr<InputTheme> &&theme_, string const &format_, object_ptr<BaseTheme> &&base_theme_);

  static const std::int32_t ID = -953697477;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_installWallPaper final : public Function {
 public:
  object_ptr<InputWallPaper> wallpaper_;
  object_ptr<wallPaperSettings> settings_;

  account_installWallPaper(object_ptr<InputWallPaper> &&wallpaper_, object_ptr<wallPaperSettings> &&settings_);

  static const std::int32_t ID = -18000023;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_registerDevice final : public Function {
 public:
  int32 flags_;
  bool no_muted_;
  int32 token_type_;
  string token_;
  bool app_sandbox_;
  bytes secret_;
  array<int64> other_uids_;
  enum Flags : std::int32_t { NO_MUTED_MASK = 1 };
  mutable int32 var0;

  account_registerDevice(int32 flags_, bool no_muted_, int32 token_type_, string const &token_, bool app_sandbox_, bytes &&secret_, array<int64> &&other_uids_);

  static const std::int32_t ID = -326762118;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_reorderUsernames final : public Function {
 public:
  array<string> order_;

  explicit account_reorderUsernames(array<string> &&order_);

  static const std::int32_t ID = -279966037;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_reportPeer final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  object_ptr<ReportReason> reason_;
  string message_;

  account_reportPeer(object_ptr<InputPeer> &&peer_, object_ptr<ReportReason> &&reason_, string const &message_);

  static const std::int32_t ID = -977650298;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_reportProfilePhoto final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  object_ptr<InputPhoto> photo_id_;
  object_ptr<ReportReason> reason_;
  string message_;

  account_reportProfilePhoto(object_ptr<InputPeer> &&peer_, object_ptr<InputPhoto> &&photo_id_, object_ptr<ReportReason> &&reason_, string const &message_);

  static const std::int32_t ID = -91437323;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_resendPasswordEmail final : public Function {
 public:

  static const std::int32_t ID = 2055154197;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_resetAuthorization final : public Function {
 public:
  int64 hash_;

  explicit account_resetAuthorization(int64 hash_);

  static const std::int32_t ID = -545786948;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_resetNotifySettings final : public Function {
 public:

  static const std::int32_t ID = -612493497;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_resetPassword final : public Function {
 public:

  static const std::int32_t ID = -1828139493;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_ResetPasswordResult>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_resetWallPapers final : public Function {
 public:

  static const std::int32_t ID = -1153722364;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_resetWebAuthorization final : public Function {
 public:
  int64 hash_;

  explicit account_resetWebAuthorization(int64 hash_);

  static const std::int32_t ID = 755087855;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_resetWebAuthorizations final : public Function {
 public:

  static const std::int32_t ID = 1747789204;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_saveAutoDownloadSettings final : public Function {
 public:
  int32 flags_;
  bool low_;
  bool high_;
  object_ptr<autoDownloadSettings> settings_;
  enum Flags : std::int32_t { LOW_MASK = 1, HIGH_MASK = 2 };
  mutable int32 var0;

  account_saveAutoDownloadSettings(int32 flags_, bool low_, bool high_, object_ptr<autoDownloadSettings> &&settings_);

  static const std::int32_t ID = 1995661875;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_saveAutoSaveSettings final : public Function {
 public:
  int32 flags_;
  bool users_;
  bool chats_;
  bool broadcasts_;
  object_ptr<InputPeer> peer_;
  object_ptr<autoSaveSettings> settings_;
  enum Flags : std::int32_t { USERS_MASK = 1, CHATS_MASK = 2, BROADCASTS_MASK = 4, PEER_MASK = 8 };
  mutable int32 var0;

  account_saveAutoSaveSettings(int32 flags_, bool users_, bool chats_, bool broadcasts_, object_ptr<InputPeer> &&peer_, object_ptr<autoSaveSettings> &&settings_);

  static const std::int32_t ID = -694451359;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_saveRingtone final : public Function {
 public:
  object_ptr<InputDocument> id_;
  bool unsave_;

  account_saveRingtone(object_ptr<InputDocument> &&id_, bool unsave_);

  static const std::int32_t ID = 1038768899;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_SavedRingtone>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_saveSecureValue final : public Function {
 public:
  object_ptr<inputSecureValue> value_;
  int64 secure_secret_id_;

  account_saveSecureValue(object_ptr<inputSecureValue> &&value_, int64 secure_secret_id_);

  static const std::int32_t ID = -1986010339;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<secureValue>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_saveTheme final : public Function {
 public:
  object_ptr<InputTheme> theme_;
  bool unsave_;

  account_saveTheme(object_ptr<InputTheme> &&theme_, bool unsave_);

  static const std::int32_t ID = -229175188;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_saveWallPaper final : public Function {
 public:
  object_ptr<InputWallPaper> wallpaper_;
  bool unsave_;
  object_ptr<wallPaperSettings> settings_;

  account_saveWallPaper(object_ptr<InputWallPaper> &&wallpaper_, bool unsave_, object_ptr<wallPaperSettings> &&settings_);

  static const std::int32_t ID = 1817860919;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_sendChangePhoneCode final : public Function {
 public:
  string phone_number_;
  object_ptr<codeSettings> settings_;

  account_sendChangePhoneCode(string const &phone_number_, object_ptr<codeSettings> &&settings_);

  static const std::int32_t ID = -2108208411;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<auth_SentCode>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_sendConfirmPhoneCode final : public Function {
 public:
  string hash_;
  object_ptr<codeSettings> settings_;

  account_sendConfirmPhoneCode(string const &hash_, object_ptr<codeSettings> &&settings_);

  static const std::int32_t ID = 457157256;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<auth_SentCode>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_sendVerifyEmailCode final : public Function {
 public:
  object_ptr<EmailVerifyPurpose> purpose_;
  string email_;

  account_sendVerifyEmailCode(object_ptr<EmailVerifyPurpose> &&purpose_, string const &email_);

  static const std::int32_t ID = -1730136133;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_sentEmailCode>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_sendVerifyPhoneCode final : public Function {
 public:
  string phone_number_;
  object_ptr<codeSettings> settings_;

  account_sendVerifyPhoneCode(string const &phone_number_, object_ptr<codeSettings> &&settings_);

  static const std::int32_t ID = -1516022023;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<auth_SentCode>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_setAccountTTL final : public Function {
 public:
  object_ptr<accountDaysTTL> ttl_;

  explicit account_setAccountTTL(object_ptr<accountDaysTTL> &&ttl_);

  static const std::int32_t ID = 608323678;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_setAuthorizationTTL final : public Function {
 public:
  int32 authorization_ttl_days_;

  explicit account_setAuthorizationTTL(int32 authorization_ttl_days_);

  static const std::int32_t ID = -1081501024;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_setContactSignUpNotification final : public Function {
 public:
  bool silent_;

  explicit account_setContactSignUpNotification(bool silent_);

  static const std::int32_t ID = -806076575;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_setContentSettings final : public Function {
 public:
  int32 flags_;
  bool sensitive_enabled_;
  enum Flags : std::int32_t { SENSITIVE_ENABLED_MASK = 1 };
  mutable int32 var0;

  account_setContentSettings(int32 flags_, bool sensitive_enabled_);

  static const std::int32_t ID = -1250643605;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_setGlobalPrivacySettings final : public Function {
 public:
  object_ptr<globalPrivacySettings> settings_;

  explicit account_setGlobalPrivacySettings(object_ptr<globalPrivacySettings> &&settings_);

  static const std::int32_t ID = 517647042;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<globalPrivacySettings>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_setPrivacy final : public Function {
 public:
  object_ptr<InputPrivacyKey> key_;
  array<object_ptr<InputPrivacyRule>> rules_;

  account_setPrivacy(object_ptr<InputPrivacyKey> &&key_, array<object_ptr<InputPrivacyRule>> &&rules_);

  static const std::int32_t ID = -906486552;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_privacyRules>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_toggleUsername final : public Function {
 public:
  string username_;
  bool active_;

  account_toggleUsername(string const &username_, bool active_);

  static const std::int32_t ID = 1490465654;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_unregisterDevice final : public Function {
 public:
  int32 token_type_;
  string token_;
  array<int64> other_uids_;

  account_unregisterDevice(int32 token_type_, string const &token_, array<int64> &&other_uids_);

  static const std::int32_t ID = 1779249670;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_updateDeviceLocked final : public Function {
 public:
  int32 period_;

  explicit account_updateDeviceLocked(int32 period_);

  static const std::int32_t ID = 954152242;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_updateEmojiStatus final : public Function {
 public:
  object_ptr<EmojiStatus> emoji_status_;

  explicit account_updateEmojiStatus(object_ptr<EmojiStatus> &&emoji_status_);

  static const std::int32_t ID = -70001045;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_updateNotifySettings final : public Function {
 public:
  object_ptr<InputNotifyPeer> peer_;
  object_ptr<inputPeerNotifySettings> settings_;

  account_updateNotifySettings(object_ptr<InputNotifyPeer> &&peer_, object_ptr<inputPeerNotifySettings> &&settings_);

  static const std::int32_t ID = -2067899501;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_updatePasswordSettings final : public Function {
 public:
  object_ptr<InputCheckPasswordSRP> password_;
  object_ptr<account_passwordInputSettings> new_settings_;

  account_updatePasswordSettings(object_ptr<InputCheckPasswordSRP> &&password_, object_ptr<account_passwordInputSettings> &&new_settings_);

  static const std::int32_t ID = -1516564433;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_updateProfile final : public Function {
 public:
  int32 flags_;
  string first_name_;
  string last_name_;
  string about_;
  enum Flags : std::int32_t { FIRST_NAME_MASK = 1, LAST_NAME_MASK = 2, ABOUT_MASK = 4 };
  mutable int32 var0;

  account_updateProfile(int32 flags_, string const &first_name_, string const &last_name_, string const &about_);

  static const std::int32_t ID = 2018596725;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<User>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_updateStatus final : public Function {
 public:
  bool offline_;

  explicit account_updateStatus(bool offline_);

  static const std::int32_t ID = 1713919532;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_updateTheme final : public Function {
 public:
  int32 flags_;
  string format_;
  object_ptr<InputTheme> theme_;
  string slug_;
  string title_;
  object_ptr<InputDocument> document_;
  array<object_ptr<inputThemeSettings>> settings_;
  enum Flags : std::int32_t { SLUG_MASK = 1, TITLE_MASK = 2, DOCUMENT_MASK = 4, SETTINGS_MASK = 8 };
  mutable int32 var0;

  account_updateTheme(int32 flags_, string const &format_, object_ptr<InputTheme> &&theme_, string const &slug_, string const &title_, object_ptr<InputDocument> &&document_, array<object_ptr<inputThemeSettings>> &&settings_);

  static const std::int32_t ID = 737414348;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<theme>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_updateUsername final : public Function {
 public:
  string username_;

  explicit account_updateUsername(string const &username_);

  static const std::int32_t ID = 1040964988;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<User>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_uploadRingtone final : public Function {
 public:
  object_ptr<InputFile> file_;
  string file_name_;
  string mime_type_;

  account_uploadRingtone(object_ptr<InputFile> &&file_, string const &file_name_, string const &mime_type_);

  static const std::int32_t ID = -2095414366;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Document>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_uploadTheme final : public Function {
 public:
  int32 flags_;
  object_ptr<InputFile> file_;
  object_ptr<InputFile> thumb_;
  string file_name_;
  string mime_type_;
  enum Flags : std::int32_t { THUMB_MASK = 1 };
  mutable int32 var0;

  account_uploadTheme(int32 flags_, object_ptr<InputFile> &&file_, object_ptr<InputFile> &&thumb_, string const &file_name_, string const &mime_type_);

  static const std::int32_t ID = 473805619;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Document>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_uploadWallPaper final : public Function {
 public:
  int32 flags_;
  bool for_chat_;
  object_ptr<InputFile> file_;
  string mime_type_;
  object_ptr<wallPaperSettings> settings_;
  enum Flags : std::int32_t { FOR_CHAT_MASK = 1 };
  mutable int32 var0;

  account_uploadWallPaper(int32 flags_, bool for_chat_, object_ptr<InputFile> &&file_, string const &mime_type_, object_ptr<wallPaperSettings> &&settings_);

  static const std::int32_t ID = -476410109;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<WallPaper>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_verifyEmail final : public Function {
 public:
  object_ptr<EmailVerifyPurpose> purpose_;
  object_ptr<EmailVerification> verification_;

  account_verifyEmail(object_ptr<EmailVerifyPurpose> &&purpose_, object_ptr<EmailVerification> &&verification_);

  static const std::int32_t ID = 53322959;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<account_EmailVerified>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class account_verifyPhone final : public Function {
 public:
  string phone_number_;
  string phone_code_hash_;
  string phone_code_;

  account_verifyPhone(string const &phone_number_, string const &phone_code_hash_, string const &phone_code_);

  static const std::int32_t ID = 1305716726;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_acceptLoginToken final : public Function {
 public:
  bytes token_;

  explicit auth_acceptLoginToken(bytes &&token_);

  static const std::int32_t ID = -392909491;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<authorization>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_bindTempAuthKey final : public Function {
 public:
  int64 perm_auth_key_id_;
  int64 nonce_;
  int32 expires_at_;
  bytes encrypted_message_;

  auth_bindTempAuthKey(int64 perm_auth_key_id_, int64 nonce_, int32 expires_at_, bytes &&encrypted_message_);

  static const std::int32_t ID = -841733627;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_cancelCode final : public Function {
 public:
  string phone_number_;
  string phone_code_hash_;

  auth_cancelCode(string const &phone_number_, string const &phone_code_hash_);

  static const std::int32_t ID = 520357240;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_checkPassword final : public Function {
 public:
  object_ptr<InputCheckPasswordSRP> password_;

  explicit auth_checkPassword(object_ptr<InputCheckPasswordSRP> &&password_);

  static const std::int32_t ID = -779399914;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<auth_Authorization>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_checkRecoveryPassword final : public Function {
 public:
  string code_;

  explicit auth_checkRecoveryPassword(string const &code_);

  static const std::int32_t ID = 221691769;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_dropTempAuthKeys final : public Function {
 public:
  array<int64> except_auth_keys_;

  explicit auth_dropTempAuthKeys(array<int64> &&except_auth_keys_);

  static const std::int32_t ID = -1907842680;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_exportAuthorization final : public Function {
 public:
  int32 dc_id_;

  explicit auth_exportAuthorization(int32 dc_id_);

  static const std::int32_t ID = -440401971;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<auth_exportedAuthorization>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_exportLoginToken final : public Function {
 public:
  int32 api_id_;
  string api_hash_;
  array<int64> except_ids_;

  auth_exportLoginToken(int32 api_id_, string const &api_hash_, array<int64> &&except_ids_);

  static const std::int32_t ID = -1210022402;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<auth_LoginToken>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_importAuthorization final : public Function {
 public:
  int64 id_;
  bytes bytes_;

  auth_importAuthorization(int64 id_, bytes &&bytes_);

  static const std::int32_t ID = -1518699091;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<auth_Authorization>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_importBotAuthorization final : public Function {
 public:
  int32 flags_;
  int32 api_id_;
  string api_hash_;
  string bot_auth_token_;

  auth_importBotAuthorization(int32 flags_, int32 api_id_, string const &api_hash_, string const &bot_auth_token_);

  static const std::int32_t ID = 1738800940;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<auth_Authorization>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_importLoginToken final : public Function {
 public:
  bytes token_;

  explicit auth_importLoginToken(bytes &&token_);

  static const std::int32_t ID = -1783866140;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<auth_LoginToken>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_importWebTokenAuthorization final : public Function {
 public:
  int32 api_id_;
  string api_hash_;
  string web_auth_token_;

  auth_importWebTokenAuthorization(int32 api_id_, string const &api_hash_, string const &web_auth_token_);

  static const std::int32_t ID = 767062953;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<auth_Authorization>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_logOut final : public Function {
 public:

  static const std::int32_t ID = 1047706137;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<auth_loggedOut>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_recoverPassword final : public Function {
 public:
  int32 flags_;
  string code_;
  object_ptr<account_passwordInputSettings> new_settings_;
  enum Flags : std::int32_t { NEW_SETTINGS_MASK = 1 };
  mutable int32 var0;

  auth_recoverPassword(int32 flags_, string const &code_, object_ptr<account_passwordInputSettings> &&new_settings_);

  static const std::int32_t ID = 923364464;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<auth_Authorization>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_requestFirebaseSms final : public Function {
 public:
  int32 flags_;
  string phone_number_;
  string phone_code_hash_;
  string safety_net_token_;
  string ios_push_secret_;
  enum Flags : std::int32_t { SAFETY_NET_TOKEN_MASK = 1, IOS_PUSH_SECRET_MASK = 2 };
  mutable int32 var0;

  auth_requestFirebaseSms(int32 flags_, string const &phone_number_, string const &phone_code_hash_, string const &safety_net_token_, string const &ios_push_secret_);

  static const std::int32_t ID = -1991881904;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_requestPasswordRecovery final : public Function {
 public:

  static const std::int32_t ID = -661144474;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<auth_passwordRecovery>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_resendCode final : public Function {
 public:
  string phone_number_;
  string phone_code_hash_;

  auth_resendCode(string const &phone_number_, string const &phone_code_hash_);

  static const std::int32_t ID = 1056025023;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<auth_SentCode>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_resetAuthorizations final : public Function {
 public:

  static const std::int32_t ID = -1616179942;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_resetLoginEmail final : public Function {
 public:
  string phone_number_;
  string phone_code_hash_;

  auth_resetLoginEmail(string const &phone_number_, string const &phone_code_hash_);

  static const std::int32_t ID = 2123760019;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<auth_SentCode>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_sendCode final : public Function {
 public:
  string phone_number_;
  int32 api_id_;
  string api_hash_;
  object_ptr<codeSettings> settings_;

  auth_sendCode(string const &phone_number_, int32 api_id_, string const &api_hash_, object_ptr<codeSettings> &&settings_);

  static const std::int32_t ID = -1502141361;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<auth_SentCode>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_signIn final : public Function {
 public:
  int32 flags_;
  string phone_number_;
  string phone_code_hash_;
  string phone_code_;
  object_ptr<EmailVerification> email_verification_;
  enum Flags : std::int32_t { PHONE_CODE_MASK = 1, EMAIL_VERIFICATION_MASK = 2 };
  mutable int32 var0;

  auth_signIn(int32 flags_, string const &phone_number_, string const &phone_code_hash_, string const &phone_code_, object_ptr<EmailVerification> &&email_verification_);

  static const std::int32_t ID = -1923962543;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<auth_Authorization>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class auth_signUp final : public Function {
 public:
  string phone_number_;
  string phone_code_hash_;
  string first_name_;
  string last_name_;

  auth_signUp(string const &phone_number_, string const &phone_code_hash_, string const &first_name_, string const &last_name_);

  static const std::int32_t ID = -2131827673;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<auth_Authorization>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class bots_answerWebhookJSONQuery final : public Function {
 public:
  int64 query_id_;
  object_ptr<dataJSON> data_;

  bots_answerWebhookJSONQuery(int64 query_id_, object_ptr<dataJSON> &&data_);

  static const std::int32_t ID = -434028723;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class bots_getBotCommands final : public Function {
 public:
  object_ptr<BotCommandScope> scope_;
  string lang_code_;

  bots_getBotCommands(object_ptr<BotCommandScope> &&scope_, string const &lang_code_);

  static const std::int32_t ID = -481554986;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<botCommand>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class bots_getBotInfo final : public Function {
 public:
  int32 flags_;
  object_ptr<InputUser> bot_;
  string lang_code_;
  enum Flags : std::int32_t { BOT_MASK = 1 };
  mutable int32 var0;

  bots_getBotInfo(int32 flags_, object_ptr<InputUser> &&bot_, string const &lang_code_);

  static const std::int32_t ID = -589753091;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<bots_botInfo>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class bots_getBotMenuButton final : public Function {
 public:
  object_ptr<InputUser> user_id_;

  explicit bots_getBotMenuButton(object_ptr<InputUser> &&user_id_);

  static const std::int32_t ID = -1671369944;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<BotMenuButton>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class bots_reorderUsernames final : public Function {
 public:
  object_ptr<InputUser> bot_;
  array<string> order_;

  bots_reorderUsernames(object_ptr<InputUser> &&bot_, array<string> &&order_);

  static const std::int32_t ID = -1760972350;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class bots_resetBotCommands final : public Function {
 public:
  object_ptr<BotCommandScope> scope_;
  string lang_code_;

  bots_resetBotCommands(object_ptr<BotCommandScope> &&scope_, string const &lang_code_);

  static const std::int32_t ID = 1032708345;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class bots_sendCustomRequest final : public Function {
 public:
  string custom_method_;
  object_ptr<dataJSON> params_;

  bots_sendCustomRequest(string const &custom_method_, object_ptr<dataJSON> &&params_);

  static const std::int32_t ID = -1440257555;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<dataJSON>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class bots_setBotBroadcastDefaultAdminRights final : public Function {
 public:
  object_ptr<chatAdminRights> admin_rights_;

  explicit bots_setBotBroadcastDefaultAdminRights(object_ptr<chatAdminRights> &&admin_rights_);

  static const std::int32_t ID = 2021942497;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class bots_setBotCommands final : public Function {
 public:
  object_ptr<BotCommandScope> scope_;
  string lang_code_;
  array<object_ptr<botCommand>> commands_;

  bots_setBotCommands(object_ptr<BotCommandScope> &&scope_, string const &lang_code_, array<object_ptr<botCommand>> &&commands_);

  static const std::int32_t ID = 85399130;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class bots_setBotGroupDefaultAdminRights final : public Function {
 public:
  object_ptr<chatAdminRights> admin_rights_;

  explicit bots_setBotGroupDefaultAdminRights(object_ptr<chatAdminRights> &&admin_rights_);

  static const std::int32_t ID = -1839281686;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class bots_setBotInfo final : public Function {
 public:
  int32 flags_;
  object_ptr<InputUser> bot_;
  string lang_code_;
  string name_;
  string about_;
  string description_;
  enum Flags : std::int32_t { BOT_MASK = 4, NAME_MASK = 8, ABOUT_MASK = 1, DESCRIPTION_MASK = 2 };
  mutable int32 var0;

  bots_setBotInfo(int32 flags_, object_ptr<InputUser> &&bot_, string const &lang_code_, string const &name_, string const &about_, string const &description_);

  static const std::int32_t ID = 282013987;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class bots_setBotMenuButton final : public Function {
 public:
  object_ptr<InputUser> user_id_;
  object_ptr<BotMenuButton> button_;

  bots_setBotMenuButton(object_ptr<InputUser> &&user_id_, object_ptr<BotMenuButton> &&button_);

  static const std::int32_t ID = 1157944655;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class bots_toggleUsername final : public Function {
 public:
  object_ptr<InputUser> bot_;
  string username_;
  bool active_;

  bots_toggleUsername(object_ptr<InputUser> &&bot_, string const &username_, bool active_);

  static const std::int32_t ID = 87861619;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_checkUsername final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  string username_;

  channels_checkUsername(object_ptr<InputChannel> &&channel_, string const &username_);

  static const std::int32_t ID = 283557164;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_convertToGigagroup final : public Function {
 public:
  object_ptr<InputChannel> channel_;

  explicit channels_convertToGigagroup(object_ptr<InputChannel> &&channel_);

  static const std::int32_t ID = 187239529;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_createChannel final : public Function {
 public:
  int32 flags_;
  bool broadcast_;
  bool megagroup_;
  bool for_import_;
  bool forum_;
  string title_;
  string about_;
  object_ptr<InputGeoPoint> geo_point_;
  string address_;
  int32 ttl_period_;
  enum Flags : std::int32_t { BROADCAST_MASK = 1, MEGAGROUP_MASK = 2, FOR_IMPORT_MASK = 8, FORUM_MASK = 32, GEO_POINT_MASK = 4, ADDRESS_MASK = 4, TTL_PERIOD_MASK = 16 };
  mutable int32 var0;

  channels_createChannel(int32 flags_, bool broadcast_, bool megagroup_, bool for_import_, bool forum_, string const &title_, string const &about_, object_ptr<InputGeoPoint> &&geo_point_, string const &address_, int32 ttl_period_);

  static const std::int32_t ID = -1862244601;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_createForumTopic final : public Function {
 public:
  int32 flags_;
  object_ptr<InputChannel> channel_;
  string title_;
  int32 icon_color_;
  int64 icon_emoji_id_;
  int64 random_id_;
  object_ptr<InputPeer> send_as_;
  enum Flags : std::int32_t { ICON_COLOR_MASK = 1, ICON_EMOJI_ID_MASK = 8, SEND_AS_MASK = 4 };
  mutable int32 var0;

  channels_createForumTopic(int32 flags_, object_ptr<InputChannel> &&channel_, string const &title_, int32 icon_color_, int64 icon_emoji_id_, int64 random_id_, object_ptr<InputPeer> &&send_as_);

  static const std::int32_t ID = -200539612;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_deactivateAllUsernames final : public Function {
 public:
  object_ptr<InputChannel> channel_;

  explicit channels_deactivateAllUsernames(object_ptr<InputChannel> &&channel_);

  static const std::int32_t ID = 170155475;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_deleteChannel final : public Function {
 public:
  object_ptr<InputChannel> channel_;

  explicit channels_deleteChannel(object_ptr<InputChannel> &&channel_);

  static const std::int32_t ID = -1072619549;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_deleteHistory final : public Function {
 public:
  int32 flags_;
  bool for_everyone_;
  object_ptr<InputChannel> channel_;
  int32 max_id_;
  enum Flags : std::int32_t { FOR_EVERYONE_MASK = 1 };
  mutable int32 var0;

  channels_deleteHistory(int32 flags_, bool for_everyone_, object_ptr<InputChannel> &&channel_, int32 max_id_);

  static const std::int32_t ID = -1683319225;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_deleteMessages final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  array<int32> id_;

  channels_deleteMessages(object_ptr<InputChannel> &&channel_, array<int32> &&id_);

  static const std::int32_t ID = -2067661490;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_affectedMessages>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_deleteParticipantHistory final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  object_ptr<InputPeer> participant_;

  channels_deleteParticipantHistory(object_ptr<InputChannel> &&channel_, object_ptr<InputPeer> &&participant_);

  static const std::int32_t ID = 913655003;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_affectedHistory>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_deleteTopicHistory final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  int32 top_msg_id_;

  channels_deleteTopicHistory(object_ptr<InputChannel> &&channel_, int32 top_msg_id_);

  static const std::int32_t ID = 876830509;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_affectedHistory>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_editAdmin final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  object_ptr<InputUser> user_id_;
  object_ptr<chatAdminRights> admin_rights_;
  string rank_;

  channels_editAdmin(object_ptr<InputChannel> &&channel_, object_ptr<InputUser> &&user_id_, object_ptr<chatAdminRights> &&admin_rights_, string const &rank_);

  static const std::int32_t ID = -751007486;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_editBanned final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  object_ptr<InputPeer> participant_;
  object_ptr<chatBannedRights> banned_rights_;

  channels_editBanned(object_ptr<InputChannel> &&channel_, object_ptr<InputPeer> &&participant_, object_ptr<chatBannedRights> &&banned_rights_);

  static const std::int32_t ID = -1763259007;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_editCreator final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  object_ptr<InputUser> user_id_;
  object_ptr<InputCheckPasswordSRP> password_;

  channels_editCreator(object_ptr<InputChannel> &&channel_, object_ptr<InputUser> &&user_id_, object_ptr<InputCheckPasswordSRP> &&password_);

  static const std::int32_t ID = -1892102881;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_editForumTopic final : public Function {
 public:
  int32 flags_;
  object_ptr<InputChannel> channel_;
  int32 topic_id_;
  string title_;
  int64 icon_emoji_id_;
  bool closed_;
  bool hidden_;
  enum Flags : std::int32_t { TITLE_MASK = 1, ICON_EMOJI_ID_MASK = 2, CLOSED_MASK = 4, HIDDEN_MASK = 8 };
  mutable int32 var0;

  channels_editForumTopic(int32 flags_, object_ptr<InputChannel> &&channel_, int32 topic_id_, string const &title_, int64 icon_emoji_id_, bool closed_, bool hidden_);

  static const std::int32_t ID = -186670715;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_editLocation final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  object_ptr<InputGeoPoint> geo_point_;
  string address_;

  channels_editLocation(object_ptr<InputChannel> &&channel_, object_ptr<InputGeoPoint> &&geo_point_, string const &address_);

  static const std::int32_t ID = 1491484525;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_editPhoto final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  object_ptr<InputChatPhoto> photo_;

  channels_editPhoto(object_ptr<InputChannel> &&channel_, object_ptr<InputChatPhoto> &&photo_);

  static const std::int32_t ID = -248621111;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_editTitle final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  string title_;

  channels_editTitle(object_ptr<InputChannel> &&channel_, string const &title_);

  static const std::int32_t ID = 1450044624;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_exportMessageLink final : public Function {
 public:
  int32 flags_;
  bool grouped_;
  bool thread_;
  object_ptr<InputChannel> channel_;
  int32 id_;
  enum Flags : std::int32_t { GROUPED_MASK = 1, THREAD_MASK = 2 };
  mutable int32 var0;

  channels_exportMessageLink(int32 flags_, bool grouped_, bool thread_, object_ptr<InputChannel> &&channel_, int32 id_);

  static const std::int32_t ID = -432034325;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<exportedMessageLink>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_getAdminLog final : public Function {
 public:
  int32 flags_;
  object_ptr<InputChannel> channel_;
  string q_;
  object_ptr<channelAdminLogEventsFilter> events_filter_;
  array<object_ptr<InputUser>> admins_;
  int64 max_id_;
  int64 min_id_;
  int32 limit_;
  enum Flags : std::int32_t { EVENTS_FILTER_MASK = 1, ADMINS_MASK = 2 };
  mutable int32 var0;

  channels_getAdminLog(int32 flags_, object_ptr<InputChannel> &&channel_, string const &q_, object_ptr<channelAdminLogEventsFilter> &&events_filter_, array<object_ptr<InputUser>> &&admins_, int64 max_id_, int64 min_id_, int32 limit_);

  static const std::int32_t ID = 870184064;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<channels_adminLogResults>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_getAdminedPublicChannels final : public Function {
 public:
  int32 flags_;
  bool by_location_;
  bool check_limit_;
  enum Flags : std::int32_t { BY_LOCATION_MASK = 1, CHECK_LIMIT_MASK = 2 };
  mutable int32 var0;

  channels_getAdminedPublicChannels(int32 flags_, bool by_location_, bool check_limit_);

  static const std::int32_t ID = -122669393;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Chats>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_getChannels final : public Function {
 public:
  array<object_ptr<InputChannel>> id_;

  explicit channels_getChannels(array<object_ptr<InputChannel>> &&id_);

  static const std::int32_t ID = 176122811;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Chats>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_getForumTopics final : public Function {
 public:
  int32 flags_;
  object_ptr<InputChannel> channel_;
  string q_;
  int32 offset_date_;
  int32 offset_id_;
  int32 offset_topic_;
  int32 limit_;
  enum Flags : std::int32_t { Q_MASK = 1 };
  mutable int32 var0;

  channels_getForumTopics(int32 flags_, object_ptr<InputChannel> &&channel_, string const &q_, int32 offset_date_, int32 offset_id_, int32 offset_topic_, int32 limit_);

  static const std::int32_t ID = 233136337;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_forumTopics>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_getForumTopicsByID final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  array<int32> topics_;

  channels_getForumTopicsByID(object_ptr<InputChannel> &&channel_, array<int32> &&topics_);

  static const std::int32_t ID = -1333584199;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_forumTopics>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_getFullChannel final : public Function {
 public:
  object_ptr<InputChannel> channel_;

  explicit channels_getFullChannel(object_ptr<InputChannel> &&channel_);

  static const std::int32_t ID = 141781513;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_chatFull>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_getGroupsForDiscussion final : public Function {
 public:

  static const std::int32_t ID = -170208392;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Chats>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_getInactiveChannels final : public Function {
 public:

  static const std::int32_t ID = 300429806;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_inactiveChats>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_getLeftChannels final : public Function {
 public:
  int32 offset_;

  explicit channels_getLeftChannels(int32 offset_);

  static const std::int32_t ID = -2092831552;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Chats>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_getMessages final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  array<object_ptr<InputMessage>> id_;

  channels_getMessages(object_ptr<InputChannel> &&channel_, array<object_ptr<InputMessage>> &&id_);

  static const std::int32_t ID = -1383294429;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Messages>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_getParticipant final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  object_ptr<InputPeer> participant_;

  channels_getParticipant(object_ptr<InputChannel> &&channel_, object_ptr<InputPeer> &&participant_);

  static const std::int32_t ID = -1599378234;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<channels_channelParticipant>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_getParticipants final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  object_ptr<ChannelParticipantsFilter> filter_;
  int32 offset_;
  int32 limit_;
  int64 hash_;

  channels_getParticipants(object_ptr<InputChannel> &&channel_, object_ptr<ChannelParticipantsFilter> &&filter_, int32 offset_, int32 limit_, int64 hash_);

  static const std::int32_t ID = 2010044880;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<channels_ChannelParticipants>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_getSendAs final : public Function {
 public:
  object_ptr<InputPeer> peer_;

  explicit channels_getSendAs(object_ptr<InputPeer> &&peer_);

  static const std::int32_t ID = 231174382;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<channels_sendAsPeers>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_getSponsoredMessages final : public Function {
 public:
  object_ptr<InputChannel> channel_;

  explicit channels_getSponsoredMessages(object_ptr<InputChannel> &&channel_);

  static const std::int32_t ID = -333377601;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_SponsoredMessages>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_inviteToChannel final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  array<object_ptr<InputUser>> users_;

  channels_inviteToChannel(object_ptr<InputChannel> &&channel_, array<object_ptr<InputUser>> &&users_);

  static const std::int32_t ID = 429865580;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_joinChannel final : public Function {
 public:
  object_ptr<InputChannel> channel_;

  explicit channels_joinChannel(object_ptr<InputChannel> &&channel_);

  static const std::int32_t ID = 615851205;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_leaveChannel final : public Function {
 public:
  object_ptr<InputChannel> channel_;

  explicit channels_leaveChannel(object_ptr<InputChannel> &&channel_);

  static const std::int32_t ID = -130635115;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_readHistory final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  int32 max_id_;

  channels_readHistory(object_ptr<InputChannel> &&channel_, int32 max_id_);

  static const std::int32_t ID = -871347913;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_readMessageContents final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  array<int32> id_;

  channels_readMessageContents(object_ptr<InputChannel> &&channel_, array<int32> &&id_);

  static const std::int32_t ID = -357180360;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_reorderPinnedForumTopics final : public Function {
 public:
  int32 flags_;
  bool force_;
  object_ptr<InputChannel> channel_;
  array<int32> order_;
  enum Flags : std::int32_t { FORCE_MASK = 1 };
  mutable int32 var0;

  channels_reorderPinnedForumTopics(int32 flags_, bool force_, object_ptr<InputChannel> &&channel_, array<int32> &&order_);

  static const std::int32_t ID = 693150095;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_reorderUsernames final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  array<string> order_;

  channels_reorderUsernames(object_ptr<InputChannel> &&channel_, array<string> &&order_);

  static const std::int32_t ID = -1268978403;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_reportAntiSpamFalsePositive final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  int32 msg_id_;

  channels_reportAntiSpamFalsePositive(object_ptr<InputChannel> &&channel_, int32 msg_id_);

  static const std::int32_t ID = -1471109485;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_reportSpam final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  object_ptr<InputPeer> participant_;
  array<int32> id_;

  channels_reportSpam(object_ptr<InputChannel> &&channel_, object_ptr<InputPeer> &&participant_, array<int32> &&id_);

  static const std::int32_t ID = -196443371;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_setDiscussionGroup final : public Function {
 public:
  object_ptr<InputChannel> broadcast_;
  object_ptr<InputChannel> group_;

  channels_setDiscussionGroup(object_ptr<InputChannel> &&broadcast_, object_ptr<InputChannel> &&group_);

  static const std::int32_t ID = 1079520178;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_setStickers final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  object_ptr<InputStickerSet> stickerset_;

  channels_setStickers(object_ptr<InputChannel> &&channel_, object_ptr<InputStickerSet> &&stickerset_);

  static const std::int32_t ID = -359881479;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_toggleAntiSpam final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  bool enabled_;

  channels_toggleAntiSpam(object_ptr<InputChannel> &&channel_, bool enabled_);

  static const std::int32_t ID = 1760814315;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_toggleForum final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  bool enabled_;

  channels_toggleForum(object_ptr<InputChannel> &&channel_, bool enabled_);

  static const std::int32_t ID = -1540781271;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_toggleJoinRequest final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  bool enabled_;

  channels_toggleJoinRequest(object_ptr<InputChannel> &&channel_, bool enabled_);

  static const std::int32_t ID = 1277789622;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_toggleJoinToSend final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  bool enabled_;

  channels_toggleJoinToSend(object_ptr<InputChannel> &&channel_, bool enabled_);

  static const std::int32_t ID = -456419968;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_toggleParticipantsHidden final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  bool enabled_;

  channels_toggleParticipantsHidden(object_ptr<InputChannel> &&channel_, bool enabled_);

  static const std::int32_t ID = 1785624660;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_togglePreHistoryHidden final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  bool enabled_;

  channels_togglePreHistoryHidden(object_ptr<InputChannel> &&channel_, bool enabled_);

  static const std::int32_t ID = -356796084;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_toggleSignatures final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  bool enabled_;

  channels_toggleSignatures(object_ptr<InputChannel> &&channel_, bool enabled_);

  static const std::int32_t ID = 527021574;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_toggleSlowMode final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  int32 seconds_;

  channels_toggleSlowMode(object_ptr<InputChannel> &&channel_, int32 seconds_);

  static const std::int32_t ID = -304832784;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_toggleUsername final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  string username_;
  bool active_;

  channels_toggleUsername(object_ptr<InputChannel> &&channel_, string const &username_, bool active_);

  static const std::int32_t ID = 1358053637;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_updatePinnedForumTopic final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  int32 topic_id_;
  bool pinned_;

  channels_updatePinnedForumTopic(object_ptr<InputChannel> &&channel_, int32 topic_id_, bool pinned_);

  static const std::int32_t ID = 1814925350;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_updateUsername final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  string username_;

  channels_updateUsername(object_ptr<InputChannel> &&channel_, string const &username_);

  static const std::int32_t ID = 890549214;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class channels_viewSponsoredMessage final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  bytes random_id_;

  channels_viewSponsoredMessage(object_ptr<InputChannel> &&channel_, bytes &&random_id_);

  static const std::int32_t ID = -1095836780;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class chatlists_checkChatlistInvite final : public Function {
 public:
  string slug_;

  explicit chatlists_checkChatlistInvite(string const &slug_);

  static const std::int32_t ID = 1103171583;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatlists_ChatlistInvite>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class chatlists_deleteExportedInvite final : public Function {
 public:
  object_ptr<inputChatlistDialogFilter> chatlist_;
  string slug_;

  chatlists_deleteExportedInvite(object_ptr<inputChatlistDialogFilter> &&chatlist_, string const &slug_);

  static const std::int32_t ID = 1906072670;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class chatlists_editExportedInvite final : public Function {
 public:
  int32 flags_;
  object_ptr<inputChatlistDialogFilter> chatlist_;
  string slug_;
  string title_;
  array<object_ptr<InputPeer>> peers_;
  enum Flags : std::int32_t { TITLE_MASK = 2, PEERS_MASK = 4 };
  mutable int32 var0;

  chatlists_editExportedInvite(int32 flags_, object_ptr<inputChatlistDialogFilter> &&chatlist_, string const &slug_, string const &title_, array<object_ptr<InputPeer>> &&peers_);

  static const std::int32_t ID = 1698543165;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<exportedChatlistInvite>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class chatlists_exportChatlistInvite final : public Function {
 public:
  object_ptr<inputChatlistDialogFilter> chatlist_;
  string title_;
  array<object_ptr<InputPeer>> peers_;

  chatlists_exportChatlistInvite(object_ptr<inputChatlistDialogFilter> &&chatlist_, string const &title_, array<object_ptr<InputPeer>> &&peers_);

  static const std::int32_t ID = -2072885362;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatlists_exportedChatlistInvite>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class chatlists_getChatlistUpdates final : public Function {
 public:
  object_ptr<inputChatlistDialogFilter> chatlist_;

  explicit chatlists_getChatlistUpdates(object_ptr<inputChatlistDialogFilter> &&chatlist_);

  static const std::int32_t ID = -1992190687;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatlists_chatlistUpdates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class chatlists_getExportedInvites final : public Function {
 public:
  object_ptr<inputChatlistDialogFilter> chatlist_;

  explicit chatlists_getExportedInvites(object_ptr<inputChatlistDialogFilter> &&chatlist_);

  static const std::int32_t ID = -838608253;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatlists_exportedInvites>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class chatlists_getLeaveChatlistSuggestions final : public Function {
 public:
  object_ptr<inputChatlistDialogFilter> chatlist_;

  explicit chatlists_getLeaveChatlistSuggestions(object_ptr<inputChatlistDialogFilter> &&chatlist_);

  static const std::int32_t ID = -37955820;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<Peer>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class chatlists_hideChatlistUpdates final : public Function {
 public:
  object_ptr<inputChatlistDialogFilter> chatlist_;

  explicit chatlists_hideChatlistUpdates(object_ptr<inputChatlistDialogFilter> &&chatlist_);

  static const std::int32_t ID = 1726252795;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class chatlists_joinChatlistInvite final : public Function {
 public:
  string slug_;
  array<object_ptr<InputPeer>> peers_;

  chatlists_joinChatlistInvite(string const &slug_, array<object_ptr<InputPeer>> &&peers_);

  static const std::int32_t ID = -1498291302;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class chatlists_joinChatlistUpdates final : public Function {
 public:
  object_ptr<inputChatlistDialogFilter> chatlist_;
  array<object_ptr<InputPeer>> peers_;

  chatlists_joinChatlistUpdates(object_ptr<inputChatlistDialogFilter> &&chatlist_, array<object_ptr<InputPeer>> &&peers_);

  static const std::int32_t ID = -527828747;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class chatlists_leaveChatlist final : public Function {
 public:
  object_ptr<inputChatlistDialogFilter> chatlist_;
  array<object_ptr<InputPeer>> peers_;

  chatlists_leaveChatlist(object_ptr<inputChatlistDialogFilter> &&chatlist_, array<object_ptr<InputPeer>> &&peers_);

  static const std::int32_t ID = 1962598714;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_acceptContact final : public Function {
 public:
  object_ptr<InputUser> id_;

  explicit contacts_acceptContact(object_ptr<InputUser> &&id_);

  static const std::int32_t ID = -130964977;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_addContact final : public Function {
 public:
  int32 flags_;
  bool add_phone_privacy_exception_;
  object_ptr<InputUser> id_;
  string first_name_;
  string last_name_;
  string phone_;
  enum Flags : std::int32_t { ADD_PHONE_PRIVACY_EXCEPTION_MASK = 1 };
  mutable int32 var0;

  contacts_addContact(int32 flags_, bool add_phone_privacy_exception_, object_ptr<InputUser> &&id_, string const &first_name_, string const &last_name_, string const &phone_);

  static const std::int32_t ID = -386636848;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_block final : public Function {
 public:
  object_ptr<InputPeer> id_;

  explicit contacts_block(object_ptr<InputPeer> &&id_);

  static const std::int32_t ID = 1758204945;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_blockFromReplies final : public Function {
 public:
  int32 flags_;
  bool delete_message_;
  bool delete_history_;
  bool report_spam_;
  int32 msg_id_;
  enum Flags : std::int32_t { DELETE_MESSAGE_MASK = 1, DELETE_HISTORY_MASK = 2, REPORT_SPAM_MASK = 4 };
  mutable int32 var0;

  contacts_blockFromReplies(int32 flags_, bool delete_message_, bool delete_history_, bool report_spam_, int32 msg_id_);

  static const std::int32_t ID = 698914348;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_deleteByPhones final : public Function {
 public:
  array<string> phones_;

  explicit contacts_deleteByPhones(array<string> &&phones_);

  static const std::int32_t ID = 269745566;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_deleteContacts final : public Function {
 public:
  array<object_ptr<InputUser>> id_;

  explicit contacts_deleteContacts(array<object_ptr<InputUser>> &&id_);

  static const std::int32_t ID = 157945344;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_exportContactToken final : public Function {
 public:

  static const std::int32_t ID = -127582169;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<exportedContactToken>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_getBlocked final : public Function {
 public:
  int32 offset_;
  int32 limit_;

  contacts_getBlocked(int32 offset_, int32 limit_);

  static const std::int32_t ID = -176409329;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<contacts_Blocked>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_getContactIDs final : public Function {
 public:
  int64 hash_;

  explicit contacts_getContactIDs(int64 hash_);

  static const std::int32_t ID = 2061264541;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<int32>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_getContacts final : public Function {
 public:
  int64 hash_;

  explicit contacts_getContacts(int64 hash_);

  static const std::int32_t ID = 1574346258;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<contacts_Contacts>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_getLocated final : public Function {
 public:
  int32 flags_;
  bool background_;
  object_ptr<InputGeoPoint> geo_point_;
  int32 self_expires_;
  enum Flags : std::int32_t { BACKGROUND_MASK = 2, SELF_EXPIRES_MASK = 1 };
  mutable int32 var0;

  contacts_getLocated(int32 flags_, bool background_, object_ptr<InputGeoPoint> &&geo_point_, int32 self_expires_);

  static const std::int32_t ID = -750207932;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_getSaved final : public Function {
 public:

  static const std::int32_t ID = -2098076769;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<savedPhoneContact>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_getStatuses final : public Function {
 public:

  static const std::int32_t ID = -995929106;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<contactStatus>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_getTopPeers final : public Function {
 public:
  int32 flags_;
  bool correspondents_;
  bool bots_pm_;
  bool bots_inline_;
  bool phone_calls_;
  bool forward_users_;
  bool forward_chats_;
  bool groups_;
  bool channels_;
  int32 offset_;
  int32 limit_;
  int64 hash_;
  enum Flags : std::int32_t { CORRESPONDENTS_MASK = 1, BOTS_PM_MASK = 2, BOTS_INLINE_MASK = 4, PHONE_CALLS_MASK = 8, FORWARD_USERS_MASK = 16, FORWARD_CHATS_MASK = 32, GROUPS_MASK = 1024, CHANNELS_MASK = 32768 };
  mutable int32 var0;

  contacts_getTopPeers(int32 flags_, bool correspondents_, bool bots_pm_, bool bots_inline_, bool phone_calls_, bool forward_users_, bool forward_chats_, bool groups_, bool channels_, int32 offset_, int32 limit_, int64 hash_);

  static const std::int32_t ID = -1758168906;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<contacts_TopPeers>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_importContactToken final : public Function {
 public:
  string token_;

  explicit contacts_importContactToken(string const &token_);

  static const std::int32_t ID = 318789512;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<User>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_importContacts final : public Function {
 public:
  array<object_ptr<inputPhoneContact>> contacts_;

  explicit contacts_importContacts(array<object_ptr<inputPhoneContact>> &&contacts_);

  static const std::int32_t ID = 746589157;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<contacts_importedContacts>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_resetSaved final : public Function {
 public:

  static const std::int32_t ID = -2020263951;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_resetTopPeerRating final : public Function {
 public:
  object_ptr<TopPeerCategory> category_;
  object_ptr<InputPeer> peer_;

  contacts_resetTopPeerRating(object_ptr<TopPeerCategory> &&category_, object_ptr<InputPeer> &&peer_);

  static const std::int32_t ID = 451113900;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_resolvePhone final : public Function {
 public:
  string phone_;

  explicit contacts_resolvePhone(string const &phone_);

  static const std::int32_t ID = -1963375804;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<contacts_resolvedPeer>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_resolveUsername final : public Function {
 public:
  string username_;

  explicit contacts_resolveUsername(string const &username_);

  static const std::int32_t ID = -113456221;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<contacts_resolvedPeer>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_search final : public Function {
 public:
  string q_;
  int32 limit_;

  contacts_search(string const &q_, int32 limit_);

  static const std::int32_t ID = 301470424;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<contacts_found>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_toggleTopPeers final : public Function {
 public:
  bool enabled_;

  explicit contacts_toggleTopPeers(bool enabled_);

  static const std::int32_t ID = -2062238246;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class contacts_unblock final : public Function {
 public:
  object_ptr<InputPeer> id_;

  explicit contacts_unblock(object_ptr<InputPeer> &&id_);

  static const std::int32_t ID = -1096393392;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class folders_editPeerFolders final : public Function {
 public:
  array<object_ptr<inputFolderPeer>> folder_peers_;

  explicit folders_editPeerFolders(array<object_ptr<inputFolderPeer>> &&folder_peers_);

  static const std::int32_t ID = 1749536939;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_acceptTermsOfService final : public Function {
 public:
  object_ptr<dataJSON> id_;

  explicit help_acceptTermsOfService(object_ptr<dataJSON> &&id_);

  static const std::int32_t ID = -294455398;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_dismissSuggestion final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  string suggestion_;

  help_dismissSuggestion(object_ptr<InputPeer> &&peer_, string const &suggestion_);

  static const std::int32_t ID = -183649631;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_editUserInfo final : public Function {
 public:
  object_ptr<InputUser> user_id_;
  string message_;
  array<object_ptr<MessageEntity>> entities_;

  help_editUserInfo(object_ptr<InputUser> &&user_id_, string const &message_, array<object_ptr<MessageEntity>> &&entities_);

  static const std::int32_t ID = 1723407216;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<help_UserInfo>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_getAppChangelog final : public Function {
 public:
  string prev_app_version_;

  explicit help_getAppChangelog(string const &prev_app_version_);

  static const std::int32_t ID = -1877938321;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_getAppConfig final : public Function {
 public:
  int32 hash_;

  explicit help_getAppConfig(int32 hash_);

  static const std::int32_t ID = 1642330196;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<help_AppConfig>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_getAppUpdate final : public Function {
 public:
  string source_;

  explicit help_getAppUpdate(string const &source_);

  static const std::int32_t ID = 1378703997;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<help_AppUpdate>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_getCdnConfig final : public Function {
 public:

  static const std::int32_t ID = 1375900482;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<cdnConfig>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_getConfig final : public Function {
 public:

  static const std::int32_t ID = -990308245;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<config>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_getCountriesList final : public Function {
 public:
  string lang_code_;
  int32 hash_;

  help_getCountriesList(string const &lang_code_, int32 hash_);

  static const std::int32_t ID = 1935116200;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<help_CountriesList>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_getDeepLinkInfo final : public Function {
 public:
  string path_;

  explicit help_getDeepLinkInfo(string const &path_);

  static const std::int32_t ID = 1072547679;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<help_DeepLinkInfo>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_getInviteText final : public Function {
 public:

  static const std::int32_t ID = 1295590211;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<help_inviteText>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_getNearestDc final : public Function {
 public:

  static const std::int32_t ID = 531836966;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<nearestDc>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_getPassportConfig final : public Function {
 public:
  int32 hash_;

  explicit help_getPassportConfig(int32 hash_);

  static const std::int32_t ID = -966677240;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<help_PassportConfig>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_getPremiumPromo final : public Function {
 public:

  static const std::int32_t ID = -1206152236;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<help_premiumPromo>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_getPromoData final : public Function {
 public:

  static const std::int32_t ID = -1063816159;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<help_PromoData>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_getRecentMeUrls final : public Function {
 public:
  string referer_;

  explicit help_getRecentMeUrls(string const &referer_);

  static const std::int32_t ID = 1036054804;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<help_recentMeUrls>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_getSupport final : public Function {
 public:

  static const std::int32_t ID = -1663104819;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<help_support>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_getSupportName final : public Function {
 public:

  static const std::int32_t ID = -748624084;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<help_supportName>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_getTermsOfServiceUpdate final : public Function {
 public:

  static const std::int32_t ID = 749019089;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<help_TermsOfServiceUpdate>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_getUserInfo final : public Function {
 public:
  object_ptr<InputUser> user_id_;

  explicit help_getUserInfo(object_ptr<InputUser> &&user_id_);

  static const std::int32_t ID = 59377875;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<help_UserInfo>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_hidePromoData final : public Function {
 public:
  object_ptr<InputPeer> peer_;

  explicit help_hidePromoData(object_ptr<InputPeer> &&peer_);

  static const std::int32_t ID = 505748629;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_saveAppLog final : public Function {
 public:
  array<object_ptr<inputAppEvent>> events_;

  explicit help_saveAppLog(array<object_ptr<inputAppEvent>> &&events_);

  static const std::int32_t ID = 1862465352;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class help_setBotUpdatesStatus final : public Function {
 public:
  int32 pending_updates_count_;
  string message_;

  help_setBotUpdatesStatus(int32 pending_updates_count_, string const &message_);

  static const std::int32_t ID = -333262899;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class langpack_getDifference final : public Function {
 public:
  string lang_pack_;
  string lang_code_;
  int32 from_version_;

  langpack_getDifference(string const &lang_pack_, string const &lang_code_, int32 from_version_);

  static const std::int32_t ID = -845657435;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<langPackDifference>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class langpack_getLangPack final : public Function {
 public:
  string lang_pack_;
  string lang_code_;

  langpack_getLangPack(string const &lang_pack_, string const &lang_code_);

  static const std::int32_t ID = -219008246;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<langPackDifference>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class langpack_getLanguage final : public Function {
 public:
  string lang_pack_;
  string lang_code_;

  langpack_getLanguage(string const &lang_pack_, string const &lang_code_);

  static const std::int32_t ID = 1784243458;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<langPackLanguage>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class langpack_getLanguages final : public Function {
 public:
  string lang_pack_;

  explicit langpack_getLanguages(string const &lang_pack_);

  static const std::int32_t ID = 1120311183;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<langPackLanguage>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class langpack_getStrings final : public Function {
 public:
  string lang_pack_;
  string lang_code_;
  array<string> keys_;

  langpack_getStrings(string const &lang_pack_, string const &lang_code_, array<string> &&keys_);

  static const std::int32_t ID = -269862909;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<LangPackString>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_acceptEncryption final : public Function {
 public:
  object_ptr<inputEncryptedChat> peer_;
  bytes g_b_;
  int64 key_fingerprint_;

  messages_acceptEncryption(object_ptr<inputEncryptedChat> &&peer_, bytes &&g_b_, int64 key_fingerprint_);

  static const std::int32_t ID = 1035731989;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<EncryptedChat>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_acceptUrlAuth final : public Function {
 public:
  int32 flags_;
  bool write_allowed_;
  object_ptr<InputPeer> peer_;
  int32 msg_id_;
  int32 button_id_;
  string url_;
  enum Flags : std::int32_t { WRITE_ALLOWED_MASK = 1, PEER_MASK = 2, MSG_ID_MASK = 2, BUTTON_ID_MASK = 2, URL_MASK = 4 };
  mutable int32 var0;

  messages_acceptUrlAuth(int32 flags_, bool write_allowed_, object_ptr<InputPeer> &&peer_, int32 msg_id_, int32 button_id_, string const &url_);

  static const std::int32_t ID = -1322487515;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<UrlAuthResult>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_addChatUser final : public Function {
 public:
  int64 chat_id_;
  object_ptr<InputUser> user_id_;
  int32 fwd_limit_;

  messages_addChatUser(int64 chat_id_, object_ptr<InputUser> &&user_id_, int32 fwd_limit_);

  static const std::int32_t ID = -230206493;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_checkChatInvite final : public Function {
 public:
  string hash_;

  explicit messages_checkChatInvite(string const &hash_);

  static const std::int32_t ID = 1051570619;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ChatInvite>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_checkHistoryImport final : public Function {
 public:
  string import_head_;

  explicit messages_checkHistoryImport(string const &import_head_);

  static const std::int32_t ID = 1140726259;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_historyImportParsed>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_checkHistoryImportPeer final : public Function {
 public:
  object_ptr<InputPeer> peer_;

  explicit messages_checkHistoryImportPeer(object_ptr<InputPeer> &&peer_);

  static const std::int32_t ID = 1573261059;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_checkedHistoryImportPeer>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_clearAllDrafts final : public Function {
 public:

  static const std::int32_t ID = 2119757468;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_clearRecentReactions final : public Function {
 public:

  static const std::int32_t ID = -1644236876;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_clearRecentStickers final : public Function {
 public:
  int32 flags_;
  bool attached_;
  enum Flags : std::int32_t { ATTACHED_MASK = 1 };
  mutable int32 var0;

  messages_clearRecentStickers(int32 flags_, bool attached_);

  static const std::int32_t ID = -1986437075;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_createChat final : public Function {
 public:
  int32 flags_;
  array<object_ptr<InputUser>> users_;
  string title_;
  int32 ttl_period_;
  enum Flags : std::int32_t { TTL_PERIOD_MASK = 1 };
  mutable int32 var0;

  messages_createChat(int32 flags_, array<object_ptr<InputUser>> &&users_, string const &title_, int32 ttl_period_);

  static const std::int32_t ID = 3450904;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_deleteChat final : public Function {
 public:
  int64 chat_id_;

  explicit messages_deleteChat(int64 chat_id_);

  static const std::int32_t ID = 1540419152;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_deleteChatUser final : public Function {
 public:
  int32 flags_;
  bool revoke_history_;
  int64 chat_id_;
  object_ptr<InputUser> user_id_;
  enum Flags : std::int32_t { REVOKE_HISTORY_MASK = 1 };
  mutable int32 var0;

  messages_deleteChatUser(int32 flags_, bool revoke_history_, int64 chat_id_, object_ptr<InputUser> &&user_id_);

  static const std::int32_t ID = -1575461717;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_deleteExportedChatInvite final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  string link_;

  messages_deleteExportedChatInvite(object_ptr<InputPeer> &&peer_, string const &link_);

  static const std::int32_t ID = -731601877;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_deleteHistory final : public Function {
 public:
  int32 flags_;
  bool just_clear_;
  bool revoke_;
  object_ptr<InputPeer> peer_;
  int32 max_id_;
  int32 min_date_;
  int32 max_date_;
  enum Flags : std::int32_t { JUST_CLEAR_MASK = 1, REVOKE_MASK = 2, MIN_DATE_MASK = 4, MAX_DATE_MASK = 8 };
  mutable int32 var0;

  messages_deleteHistory(int32 flags_, bool just_clear_, bool revoke_, object_ptr<InputPeer> &&peer_, int32 max_id_, int32 min_date_, int32 max_date_);

  static const std::int32_t ID = -1332768214;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_affectedHistory>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_deleteMessages final : public Function {
 public:
  int32 flags_;
  bool revoke_;
  array<int32> id_;
  enum Flags : std::int32_t { REVOKE_MASK = 1 };
  mutable int32 var0;

  messages_deleteMessages(int32 flags_, bool revoke_, array<int32> &&id_);

  static const std::int32_t ID = -443640366;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_affectedMessages>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_deletePhoneCallHistory final : public Function {
 public:
  int32 flags_;
  bool revoke_;
  enum Flags : std::int32_t { REVOKE_MASK = 1 };
  mutable int32 var0;

  messages_deletePhoneCallHistory(int32 flags_, bool revoke_);

  static const std::int32_t ID = -104078327;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_affectedFoundMessages>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_deleteRevokedExportedChatInvites final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  object_ptr<InputUser> admin_id_;

  messages_deleteRevokedExportedChatInvites(object_ptr<InputPeer> &&peer_, object_ptr<InputUser> &&admin_id_);

  static const std::int32_t ID = 1452833749;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_deleteScheduledMessages final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  array<int32> id_;

  messages_deleteScheduledMessages(object_ptr<InputPeer> &&peer_, array<int32> &&id_);

  static const std::int32_t ID = 1504586518;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_discardEncryption final : public Function {
 public:
  int32 flags_;
  bool delete_history_;
  int32 chat_id_;
  enum Flags : std::int32_t { DELETE_HISTORY_MASK = 1 };
  mutable int32 var0;

  messages_discardEncryption(int32 flags_, bool delete_history_, int32 chat_id_);

  static const std::int32_t ID = -208425312;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_editChatAbout final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  string about_;

  messages_editChatAbout(object_ptr<InputPeer> &&peer_, string const &about_);

  static const std::int32_t ID = -554301545;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_editChatAdmin final : public Function {
 public:
  int64 chat_id_;
  object_ptr<InputUser> user_id_;
  bool is_admin_;

  messages_editChatAdmin(int64 chat_id_, object_ptr<InputUser> &&user_id_, bool is_admin_);

  static const std::int32_t ID = -1470377534;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_editChatDefaultBannedRights final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  object_ptr<chatBannedRights> banned_rights_;

  messages_editChatDefaultBannedRights(object_ptr<InputPeer> &&peer_, object_ptr<chatBannedRights> &&banned_rights_);

  static const std::int32_t ID = -1517917375;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_editChatPhoto final : public Function {
 public:
  int64 chat_id_;
  object_ptr<InputChatPhoto> photo_;

  messages_editChatPhoto(int64 chat_id_, object_ptr<InputChatPhoto> &&photo_);

  static const std::int32_t ID = 903730804;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_editChatTitle final : public Function {
 public:
  int64 chat_id_;
  string title_;

  messages_editChatTitle(int64 chat_id_, string const &title_);

  static const std::int32_t ID = 1937260541;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_editExportedChatInvite final : public Function {
 public:
  int32 flags_;
  bool revoked_;
  object_ptr<InputPeer> peer_;
  string link_;
  int32 expire_date_;
  int32 usage_limit_;
  bool request_needed_;
  string title_;
  enum Flags : std::int32_t { REVOKED_MASK = 4, EXPIRE_DATE_MASK = 1, USAGE_LIMIT_MASK = 2, REQUEST_NEEDED_MASK = 8, TITLE_MASK = 16 };
  mutable int32 var0;

  messages_editExportedChatInvite(int32 flags_, bool revoked_, object_ptr<InputPeer> &&peer_, string const &link_, int32 expire_date_, int32 usage_limit_, bool request_needed_, string const &title_);

  static const std::int32_t ID = -1110823051;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_ExportedChatInvite>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_editInlineBotMessage final : public Function {
 public:
  int32 flags_;
  bool no_webpage_;
  object_ptr<InputBotInlineMessageID> id_;
  string message_;
  object_ptr<InputMedia> media_;
  object_ptr<ReplyMarkup> reply_markup_;
  array<object_ptr<MessageEntity>> entities_;
  enum Flags : std::int32_t { NO_WEBPAGE_MASK = 2, MESSAGE_MASK = 2048, MEDIA_MASK = 16384, REPLY_MARKUP_MASK = 4, ENTITIES_MASK = 8 };
  mutable int32 var0;

  messages_editInlineBotMessage(int32 flags_, bool no_webpage_, object_ptr<InputBotInlineMessageID> &&id_, string const &message_, object_ptr<InputMedia> &&media_, object_ptr<ReplyMarkup> &&reply_markup_, array<object_ptr<MessageEntity>> &&entities_);

  static const std::int32_t ID = -2091549254;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_editMessage final : public Function {
 public:
  int32 flags_;
  bool no_webpage_;
  object_ptr<InputPeer> peer_;
  int32 id_;
  string message_;
  object_ptr<InputMedia> media_;
  object_ptr<ReplyMarkup> reply_markup_;
  array<object_ptr<MessageEntity>> entities_;
  int32 schedule_date_;
  enum Flags : std::int32_t { NO_WEBPAGE_MASK = 2, MESSAGE_MASK = 2048, MEDIA_MASK = 16384, REPLY_MARKUP_MASK = 4, ENTITIES_MASK = 8, SCHEDULE_DATE_MASK = 32768 };
  mutable int32 var0;

  messages_editMessage(int32 flags_, bool no_webpage_, object_ptr<InputPeer> &&peer_, int32 id_, string const &message_, object_ptr<InputMedia> &&media_, object_ptr<ReplyMarkup> &&reply_markup_, array<object_ptr<MessageEntity>> &&entities_, int32 schedule_date_);

  static const std::int32_t ID = 1224152952;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_exportChatInvite final : public Function {
 public:
  int32 flags_;
  bool legacy_revoke_permanent_;
  bool request_needed_;
  object_ptr<InputPeer> peer_;
  int32 expire_date_;
  int32 usage_limit_;
  string title_;
  enum Flags : std::int32_t { LEGACY_REVOKE_PERMANENT_MASK = 4, REQUEST_NEEDED_MASK = 8, EXPIRE_DATE_MASK = 1, USAGE_LIMIT_MASK = 2, TITLE_MASK = 16 };
  mutable int32 var0;

  messages_exportChatInvite(int32 flags_, bool legacy_revoke_permanent_, bool request_needed_, object_ptr<InputPeer> &&peer_, int32 expire_date_, int32 usage_limit_, string const &title_);

  static const std::int32_t ID = -1607670315;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<ExportedChatInvite>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_faveSticker final : public Function {
 public:
  object_ptr<InputDocument> id_;
  bool unfave_;

  messages_faveSticker(object_ptr<InputDocument> &&id_, bool unfave_);

  static const std::int32_t ID = -1174420133;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_forwardMessages final : public Function {
 public:
  int32 flags_;
  bool silent_;
  bool background_;
  bool with_my_score_;
  bool drop_author_;
  bool drop_media_captions_;
  bool noforwards_;
  object_ptr<InputPeer> from_peer_;
  array<int32> id_;
  array<int64> random_id_;
  object_ptr<InputPeer> to_peer_;
  int32 top_msg_id_;
  int32 schedule_date_;
  object_ptr<InputPeer> send_as_;
  enum Flags : std::int32_t { SILENT_MASK = 32, BACKGROUND_MASK = 64, WITH_MY_SCORE_MASK = 256, DROP_AUTHOR_MASK = 2048, DROP_MEDIA_CAPTIONS_MASK = 4096, NOFORWARDS_MASK = 16384, TOP_MSG_ID_MASK = 512, SCHEDULE_DATE_MASK = 1024, SEND_AS_MASK = 8192 };
  mutable int32 var0;

  messages_forwardMessages(int32 flags_, bool silent_, bool background_, bool with_my_score_, bool drop_author_, bool drop_media_captions_, bool noforwards_, object_ptr<InputPeer> &&from_peer_, array<int32> &&id_, array<int64> &&random_id_, object_ptr<InputPeer> &&to_peer_, int32 top_msg_id_, int32 schedule_date_, object_ptr<InputPeer> &&send_as_);

  static const std::int32_t ID = -966673468;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getAdminsWithInvites final : public Function {
 public:
  object_ptr<InputPeer> peer_;

  explicit messages_getAdminsWithInvites(object_ptr<InputPeer> &&peer_);

  static const std::int32_t ID = 958457583;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_chatAdminsWithInvites>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getAllChats final : public Function {
 public:
  array<int64> except_ids_;

  explicit messages_getAllChats(array<int64> &&except_ids_);

  static const std::int32_t ID = -2023787330;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Chats>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getAllDrafts final : public Function {
 public:

  static const std::int32_t ID = 1782549861;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getAllStickers final : public Function {
 public:
  int64 hash_;

  explicit messages_getAllStickers(int64 hash_);

  static const std::int32_t ID = -1197432408;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_AllStickers>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getArchivedStickers final : public Function {
 public:
  int32 flags_;
  bool masks_;
  bool emojis_;
  int64 offset_id_;
  int32 limit_;
  enum Flags : std::int32_t { MASKS_MASK = 1, EMOJIS_MASK = 2 };
  mutable int32 var0;

  messages_getArchivedStickers(int32 flags_, bool masks_, bool emojis_, int64 offset_id_, int32 limit_);

  static const std::int32_t ID = 1475442322;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_archivedStickers>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getAttachMenuBot final : public Function {
 public:
  object_ptr<InputUser> bot_;

  explicit messages_getAttachMenuBot(object_ptr<InputUser> &&bot_);

  static const std::int32_t ID = 1998676370;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<attachMenuBotsBot>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getAttachMenuBots final : public Function {
 public:
  int64 hash_;

  explicit messages_getAttachMenuBots(int64 hash_);

  static const std::int32_t ID = 385663691;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<AttachMenuBots>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getAttachedStickers final : public Function {
 public:
  object_ptr<InputStickeredMedia> media_;

  explicit messages_getAttachedStickers(object_ptr<InputStickeredMedia> &&media_);

  static const std::int32_t ID = -866424884;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<StickerSetCovered>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getAvailableReactions final : public Function {
 public:
  int32 hash_;

  explicit messages_getAvailableReactions(int32 hash_);

  static const std::int32_t ID = 417243308;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_AvailableReactions>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getBotApp final : public Function {
 public:
  object_ptr<InputBotApp> app_;
  int64 hash_;

  messages_getBotApp(object_ptr<InputBotApp> &&app_, int64 hash_);

  static const std::int32_t ID = 889046467;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_botApp>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getBotCallbackAnswer final : public Function {
 public:
  int32 flags_;
  bool game_;
  object_ptr<InputPeer> peer_;
  int32 msg_id_;
  bytes data_;
  object_ptr<InputCheckPasswordSRP> password_;
  enum Flags : std::int32_t { GAME_MASK = 2, DATA_MASK = 1, PASSWORD_MASK = 4 };
  mutable int32 var0;

  messages_getBotCallbackAnswer(int32 flags_, bool game_, object_ptr<InputPeer> &&peer_, int32 msg_id_, bytes &&data_, object_ptr<InputCheckPasswordSRP> &&password_);

  static const std::int32_t ID = -1824339449;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_botCallbackAnswer>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getChatInviteImporters final : public Function {
 public:
  int32 flags_;
  bool requested_;
  object_ptr<InputPeer> peer_;
  string link_;
  string q_;
  int32 offset_date_;
  object_ptr<InputUser> offset_user_;
  int32 limit_;
  enum Flags : std::int32_t { REQUESTED_MASK = 1, LINK_MASK = 2, Q_MASK = 4 };
  mutable int32 var0;

  messages_getChatInviteImporters(int32 flags_, bool requested_, object_ptr<InputPeer> &&peer_, string const &link_, string const &q_, int32 offset_date_, object_ptr<InputUser> &&offset_user_, int32 limit_);

  static const std::int32_t ID = -553329330;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_chatInviteImporters>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getChats final : public Function {
 public:
  array<int64> id_;

  explicit messages_getChats(array<int64> &&id_);

  static const std::int32_t ID = 1240027791;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Chats>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getCommonChats final : public Function {
 public:
  object_ptr<InputUser> user_id_;
  int64 max_id_;
  int32 limit_;

  messages_getCommonChats(object_ptr<InputUser> &&user_id_, int64 max_id_, int32 limit_);

  static const std::int32_t ID = -468934396;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Chats>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getCustomEmojiDocuments final : public Function {
 public:
  array<int64> document_id_;

  explicit messages_getCustomEmojiDocuments(array<int64> &&document_id_);

  static const std::int32_t ID = -643100844;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<Document>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getDefaultHistoryTTL final : public Function {
 public:

  static const std::int32_t ID = 1703637384;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<defaultHistoryTTL>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getDhConfig final : public Function {
 public:
  int32 version_;
  int32 random_length_;

  messages_getDhConfig(int32 version_, int32 random_length_);

  static const std::int32_t ID = 651135312;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_DhConfig>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getDialogFilters final : public Function {
 public:

  static const std::int32_t ID = -241247891;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<DialogFilter>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getDialogUnreadMarks final : public Function {
 public:

  static const std::int32_t ID = 585256482;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<DialogPeer>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getDialogs final : public Function {
 public:
  int32 flags_;
  bool exclude_pinned_;
  int32 folder_id_;
  int32 offset_date_;
  int32 offset_id_;
  object_ptr<InputPeer> offset_peer_;
  int32 limit_;
  int64 hash_;
  enum Flags : std::int32_t { EXCLUDE_PINNED_MASK = 1, FOLDER_ID_MASK = 2 };
  mutable int32 var0;

  messages_getDialogs(int32 flags_, bool exclude_pinned_, int32 folder_id_, int32 offset_date_, int32 offset_id_, object_ptr<InputPeer> &&offset_peer_, int32 limit_, int64 hash_);

  static const std::int32_t ID = -1594569905;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Dialogs>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getDiscussionMessage final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int32 msg_id_;

  messages_getDiscussionMessage(object_ptr<InputPeer> &&peer_, int32 msg_id_);

  static const std::int32_t ID = 1147761405;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_discussionMessage>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getDocumentByHash final : public Function {
 public:
  bytes sha256_;
  int64 size_;
  string mime_type_;

  messages_getDocumentByHash(bytes &&sha256_, int64 size_, string const &mime_type_);

  static const std::int32_t ID = -1309538785;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Document>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getEmojiGroups final : public Function {
 public:
  int32 hash_;

  explicit messages_getEmojiGroups(int32 hash_);

  static const std::int32_t ID = 1955122779;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_EmojiGroups>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getEmojiKeywords final : public Function {
 public:
  string lang_code_;

  explicit messages_getEmojiKeywords(string const &lang_code_);

  static const std::int32_t ID = 899735650;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<emojiKeywordsDifference>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getEmojiKeywordsDifference final : public Function {
 public:
  string lang_code_;
  int32 from_version_;

  messages_getEmojiKeywordsDifference(string const &lang_code_, int32 from_version_);

  static const std::int32_t ID = 352892591;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<emojiKeywordsDifference>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getEmojiKeywordsLanguages final : public Function {
 public:
  array<string> lang_codes_;

  explicit messages_getEmojiKeywordsLanguages(array<string> &&lang_codes_);

  static const std::int32_t ID = 1318675378;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<emojiLanguage>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getEmojiProfilePhotoGroups final : public Function {
 public:
  int32 hash_;

  explicit messages_getEmojiProfilePhotoGroups(int32 hash_);

  static const std::int32_t ID = 564480243;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_EmojiGroups>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getEmojiStatusGroups final : public Function {
 public:
  int32 hash_;

  explicit messages_getEmojiStatusGroups(int32 hash_);

  static const std::int32_t ID = 785209037;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_EmojiGroups>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getEmojiStickers final : public Function {
 public:
  int64 hash_;

  explicit messages_getEmojiStickers(int64 hash_);

  static const std::int32_t ID = -67329649;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_AllStickers>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getEmojiURL final : public Function {
 public:
  string lang_code_;

  explicit messages_getEmojiURL(string const &lang_code_);

  static const std::int32_t ID = -709817306;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<emojiURL>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getExportedChatInvite final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  string link_;

  messages_getExportedChatInvite(object_ptr<InputPeer> &&peer_, string const &link_);

  static const std::int32_t ID = 1937010524;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_ExportedChatInvite>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getExportedChatInvites final : public Function {
 public:
  int32 flags_;
  bool revoked_;
  object_ptr<InputPeer> peer_;
  object_ptr<InputUser> admin_id_;
  int32 offset_date_;
  string offset_link_;
  int32 limit_;
  enum Flags : std::int32_t { REVOKED_MASK = 8, OFFSET_DATE_MASK = 4, OFFSET_LINK_MASK = 4 };
  mutable int32 var0;

  messages_getExportedChatInvites(int32 flags_, bool revoked_, object_ptr<InputPeer> &&peer_, object_ptr<InputUser> &&admin_id_, int32 offset_date_, string const &offset_link_, int32 limit_);

  static const std::int32_t ID = -1565154314;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_exportedChatInvites>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getExtendedMedia final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  array<int32> id_;

  messages_getExtendedMedia(object_ptr<InputPeer> &&peer_, array<int32> &&id_);

  static const std::int32_t ID = -2064119788;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getFavedStickers final : public Function {
 public:
  int64 hash_;

  explicit messages_getFavedStickers(int64 hash_);

  static const std::int32_t ID = 82946729;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_FavedStickers>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getFeaturedEmojiStickers final : public Function {
 public:
  int64 hash_;

  explicit messages_getFeaturedEmojiStickers(int64 hash_);

  static const std::int32_t ID = 248473398;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_FeaturedStickers>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getFeaturedStickers final : public Function {
 public:
  int64 hash_;

  explicit messages_getFeaturedStickers(int64 hash_);

  static const std::int32_t ID = 1685588756;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_FeaturedStickers>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getFullChat final : public Function {
 public:
  int64 chat_id_;

  explicit messages_getFullChat(int64 chat_id_);

  static const std::int32_t ID = -1364194508;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_chatFull>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getGameHighScores final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int32 id_;
  object_ptr<InputUser> user_id_;

  messages_getGameHighScores(object_ptr<InputPeer> &&peer_, int32 id_, object_ptr<InputUser> &&user_id_);

  static const std::int32_t ID = -400399203;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_highScores>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getHistory final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int32 offset_id_;
  int32 offset_date_;
  int32 add_offset_;
  int32 limit_;
  int32 max_id_;
  int32 min_id_;
  int64 hash_;

  messages_getHistory(object_ptr<InputPeer> &&peer_, int32 offset_id_, int32 offset_date_, int32 add_offset_, int32 limit_, int32 max_id_, int32 min_id_, int64 hash_);

  static const std::int32_t ID = 1143203525;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Messages>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getInlineBotResults final : public Function {
 public:
  int32 flags_;
  object_ptr<InputUser> bot_;
  object_ptr<InputPeer> peer_;
  object_ptr<InputGeoPoint> geo_point_;
  string query_;
  string offset_;
  enum Flags : std::int32_t { GEO_POINT_MASK = 1 };
  mutable int32 var0;

  messages_getInlineBotResults(int32 flags_, object_ptr<InputUser> &&bot_, object_ptr<InputPeer> &&peer_, object_ptr<InputGeoPoint> &&geo_point_, string const &query_, string const &offset_);

  static const std::int32_t ID = 1364105629;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_botResults>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getInlineGameHighScores final : public Function {
 public:
  object_ptr<InputBotInlineMessageID> id_;
  object_ptr<InputUser> user_id_;

  messages_getInlineGameHighScores(object_ptr<InputBotInlineMessageID> &&id_, object_ptr<InputUser> &&user_id_);

  static const std::int32_t ID = 258170395;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_highScores>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getMaskStickers final : public Function {
 public:
  int64 hash_;

  explicit messages_getMaskStickers(int64 hash_);

  static const std::int32_t ID = 1678738104;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_AllStickers>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getMessageEditData final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int32 id_;

  messages_getMessageEditData(object_ptr<InputPeer> &&peer_, int32 id_);

  static const std::int32_t ID = -39416522;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_messageEditData>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getMessageReactionsList final : public Function {
 public:
  int32 flags_;
  object_ptr<InputPeer> peer_;
  int32 id_;
  object_ptr<Reaction> reaction_;
  string offset_;
  int32 limit_;
  enum Flags : std::int32_t { REACTION_MASK = 1, OFFSET_MASK = 2 };
  mutable int32 var0;

  messages_getMessageReactionsList(int32 flags_, object_ptr<InputPeer> &&peer_, int32 id_, object_ptr<Reaction> &&reaction_, string const &offset_, int32 limit_);

  static const std::int32_t ID = 1176190792;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_messageReactionsList>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getMessageReadParticipants final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int32 msg_id_;

  messages_getMessageReadParticipants(object_ptr<InputPeer> &&peer_, int32 msg_id_);

  static const std::int32_t ID = 834782287;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<readParticipantDate>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getMessages final : public Function {
 public:
  array<object_ptr<InputMessage>> id_;

  explicit messages_getMessages(array<object_ptr<InputMessage>> &&id_);

  static const std::int32_t ID = 1673946374;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Messages>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getMessagesReactions final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  array<int32> id_;

  messages_getMessagesReactions(object_ptr<InputPeer> &&peer_, array<int32> &&id_);

  static const std::int32_t ID = -1950707482;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getMessagesViews final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  array<int32> id_;
  bool increment_;

  messages_getMessagesViews(object_ptr<InputPeer> &&peer_, array<int32> &&id_, bool increment_);

  static const std::int32_t ID = 1468322785;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_messageViews>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getOldFeaturedStickers final : public Function {
 public:
  int32 offset_;
  int32 limit_;
  int64 hash_;

  messages_getOldFeaturedStickers(int32 offset_, int32 limit_, int64 hash_);

  static const std::int32_t ID = 2127598753;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_FeaturedStickers>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getOnlines final : public Function {
 public:
  object_ptr<InputPeer> peer_;

  explicit messages_getOnlines(object_ptr<InputPeer> &&peer_);

  static const std::int32_t ID = 1848369232;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<chatOnlines>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getPeerDialogs final : public Function {
 public:
  array<object_ptr<InputDialogPeer>> peers_;

  explicit messages_getPeerDialogs(array<object_ptr<InputDialogPeer>> &&peers_);

  static const std::int32_t ID = -462373635;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_peerDialogs>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getPeerSettings final : public Function {
 public:
  object_ptr<InputPeer> peer_;

  explicit messages_getPeerSettings(object_ptr<InputPeer> &&peer_);

  static const std::int32_t ID = -270948702;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_peerSettings>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getPinnedDialogs final : public Function {
 public:
  int32 folder_id_;

  explicit messages_getPinnedDialogs(int32 folder_id_);

  static const std::int32_t ID = -692498958;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_peerDialogs>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getPollResults final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int32 msg_id_;

  messages_getPollResults(object_ptr<InputPeer> &&peer_, int32 msg_id_);

  static const std::int32_t ID = 1941660731;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getPollVotes final : public Function {
 public:
  int32 flags_;
  object_ptr<InputPeer> peer_;
  int32 id_;
  bytes option_;
  string offset_;
  int32 limit_;
  enum Flags : std::int32_t { OPTION_MASK = 1, OFFSET_MASK = 2 };
  mutable int32 var0;

  messages_getPollVotes(int32 flags_, object_ptr<InputPeer> &&peer_, int32 id_, bytes &&option_, string const &offset_, int32 limit_);

  static const std::int32_t ID = -1200736242;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_votesList>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getRecentLocations final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int32 limit_;
  int64 hash_;

  messages_getRecentLocations(object_ptr<InputPeer> &&peer_, int32 limit_, int64 hash_);

  static const std::int32_t ID = 1881817312;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Messages>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getRecentReactions final : public Function {
 public:
  int32 limit_;
  int64 hash_;

  messages_getRecentReactions(int32 limit_, int64 hash_);

  static const std::int32_t ID = 960896434;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Reactions>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getRecentStickers final : public Function {
 public:
  int32 flags_;
  bool attached_;
  int64 hash_;
  enum Flags : std::int32_t { ATTACHED_MASK = 1 };
  mutable int32 var0;

  messages_getRecentStickers(int32 flags_, bool attached_, int64 hash_);

  static const std::int32_t ID = -1649852357;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_RecentStickers>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getReplies final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int32 msg_id_;
  int32 offset_id_;
  int32 offset_date_;
  int32 add_offset_;
  int32 limit_;
  int32 max_id_;
  int32 min_id_;
  int64 hash_;

  messages_getReplies(object_ptr<InputPeer> &&peer_, int32 msg_id_, int32 offset_id_, int32 offset_date_, int32 add_offset_, int32 limit_, int32 max_id_, int32 min_id_, int64 hash_);

  static const std::int32_t ID = 584962828;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Messages>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getSavedGifs final : public Function {
 public:
  int64 hash_;

  explicit messages_getSavedGifs(int64 hash_);

  static const std::int32_t ID = 1559270965;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_SavedGifs>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getScheduledHistory final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int64 hash_;

  messages_getScheduledHistory(object_ptr<InputPeer> &&peer_, int64 hash_);

  static const std::int32_t ID = -183077365;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Messages>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getScheduledMessages final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  array<int32> id_;

  messages_getScheduledMessages(object_ptr<InputPeer> &&peer_, array<int32> &&id_);

  static const std::int32_t ID = -1111817116;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Messages>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getSearchCounters final : public Function {
 public:
  int32 flags_;
  object_ptr<InputPeer> peer_;
  int32 top_msg_id_;
  array<object_ptr<MessagesFilter>> filters_;
  enum Flags : std::int32_t { TOP_MSG_ID_MASK = 1 };
  mutable int32 var0;

  messages_getSearchCounters(int32 flags_, object_ptr<InputPeer> &&peer_, int32 top_msg_id_, array<object_ptr<MessagesFilter>> &&filters_);

  static const std::int32_t ID = 11435201;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<messages_searchCounter>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getSearchResultsCalendar final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  object_ptr<MessagesFilter> filter_;
  int32 offset_id_;
  int32 offset_date_;

  messages_getSearchResultsCalendar(object_ptr<InputPeer> &&peer_, object_ptr<MessagesFilter> &&filter_, int32 offset_id_, int32 offset_date_);

  static const std::int32_t ID = 1240514025;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_searchResultsCalendar>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getSearchResultsPositions final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  object_ptr<MessagesFilter> filter_;
  int32 offset_id_;
  int32 limit_;

  messages_getSearchResultsPositions(object_ptr<InputPeer> &&peer_, object_ptr<MessagesFilter> &&filter_, int32 offset_id_, int32 limit_);

  static const std::int32_t ID = 1855292323;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_searchResultsPositions>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getSplitRanges final : public Function {
 public:

  static const std::int32_t ID = 486505992;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<messageRange>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getStickerSet final : public Function {
 public:
  object_ptr<InputStickerSet> stickerset_;
  int32 hash_;

  messages_getStickerSet(object_ptr<InputStickerSet> &&stickerset_, int32 hash_);

  static const std::int32_t ID = -928977804;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_StickerSet>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getStickers final : public Function {
 public:
  string emoticon_;
  int64 hash_;

  messages_getStickers(string const &emoticon_, int64 hash_);

  static const std::int32_t ID = -710552671;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Stickers>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getSuggestedDialogFilters final : public Function {
 public:

  static const std::int32_t ID = -1566780372;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<dialogFilterSuggested>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getTopReactions final : public Function {
 public:
  int32 limit_;
  int64 hash_;

  messages_getTopReactions(int32 limit_, int64 hash_);

  static const std::int32_t ID = -1149164102;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Reactions>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getUnreadMentions final : public Function {
 public:
  int32 flags_;
  object_ptr<InputPeer> peer_;
  int32 top_msg_id_;
  int32 offset_id_;
  int32 add_offset_;
  int32 limit_;
  int32 max_id_;
  int32 min_id_;
  enum Flags : std::int32_t { TOP_MSG_ID_MASK = 1 };
  mutable int32 var0;

  messages_getUnreadMentions(int32 flags_, object_ptr<InputPeer> &&peer_, int32 top_msg_id_, int32 offset_id_, int32 add_offset_, int32 limit_, int32 max_id_, int32 min_id_);

  static const std::int32_t ID = -251140208;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Messages>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getUnreadReactions final : public Function {
 public:
  int32 flags_;
  object_ptr<InputPeer> peer_;
  int32 top_msg_id_;
  int32 offset_id_;
  int32 add_offset_;
  int32 limit_;
  int32 max_id_;
  int32 min_id_;
  enum Flags : std::int32_t { TOP_MSG_ID_MASK = 1 };
  mutable int32 var0;

  messages_getUnreadReactions(int32 flags_, object_ptr<InputPeer> &&peer_, int32 top_msg_id_, int32 offset_id_, int32 add_offset_, int32 limit_, int32 max_id_, int32 min_id_);

  static const std::int32_t ID = 841173339;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Messages>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getWebPage final : public Function {
 public:
  string url_;
  int32 hash_;

  messages_getWebPage(string const &url_, int32 hash_);

  static const std::int32_t ID = 852135825;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<WebPage>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_getWebPagePreview final : public Function {
 public:
  int32 flags_;
  string message_;
  array<object_ptr<MessageEntity>> entities_;
  enum Flags : std::int32_t { ENTITIES_MASK = 8 };
  mutable int32 var0;

  messages_getWebPagePreview(int32 flags_, string const &message_, array<object_ptr<MessageEntity>> &&entities_);

  static const std::int32_t ID = -1956073268;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<MessageMedia>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_hideAllChatJoinRequests final : public Function {
 public:
  int32 flags_;
  bool approved_;
  object_ptr<InputPeer> peer_;
  string link_;
  enum Flags : std::int32_t { APPROVED_MASK = 1, LINK_MASK = 2 };
  mutable int32 var0;

  messages_hideAllChatJoinRequests(int32 flags_, bool approved_, object_ptr<InputPeer> &&peer_, string const &link_);

  static const std::int32_t ID = -528091926;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_hideChatJoinRequest final : public Function {
 public:
  int32 flags_;
  bool approved_;
  object_ptr<InputPeer> peer_;
  object_ptr<InputUser> user_id_;
  enum Flags : std::int32_t { APPROVED_MASK = 1 };
  mutable int32 var0;

  messages_hideChatJoinRequest(int32 flags_, bool approved_, object_ptr<InputPeer> &&peer_, object_ptr<InputUser> &&user_id_);

  static const std::int32_t ID = 2145904661;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_hidePeerSettingsBar final : public Function {
 public:
  object_ptr<InputPeer> peer_;

  explicit messages_hidePeerSettingsBar(object_ptr<InputPeer> &&peer_);

  static const std::int32_t ID = 1336717624;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_importChatInvite final : public Function {
 public:
  string hash_;

  explicit messages_importChatInvite(string const &hash_);

  static const std::int32_t ID = 1817183516;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_initHistoryImport final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  object_ptr<InputFile> file_;
  int32 media_count_;

  messages_initHistoryImport(object_ptr<InputPeer> &&peer_, object_ptr<InputFile> &&file_, int32 media_count_);

  static const std::int32_t ID = 873008187;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_historyImport>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_installStickerSet final : public Function {
 public:
  object_ptr<InputStickerSet> stickerset_;
  bool archived_;

  messages_installStickerSet(object_ptr<InputStickerSet> &&stickerset_, bool archived_);

  static const std::int32_t ID = -946871200;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_StickerSetInstallResult>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_markDialogUnread final : public Function {
 public:
  int32 flags_;
  bool unread_;
  object_ptr<InputDialogPeer> peer_;
  enum Flags : std::int32_t { UNREAD_MASK = 1 };
  mutable int32 var0;

  messages_markDialogUnread(int32 flags_, bool unread_, object_ptr<InputDialogPeer> &&peer_);

  static const std::int32_t ID = -1031349873;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_migrateChat final : public Function {
 public:
  int64 chat_id_;

  explicit messages_migrateChat(int64 chat_id_);

  static const std::int32_t ID = -1568189671;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_prolongWebView final : public Function {
 public:
  int32 flags_;
  bool silent_;
  object_ptr<InputPeer> peer_;
  object_ptr<InputUser> bot_;
  int64 query_id_;
  int32 reply_to_msg_id_;
  int32 top_msg_id_;
  object_ptr<InputPeer> send_as_;
  enum Flags : std::int32_t { SILENT_MASK = 32, REPLY_TO_MSG_ID_MASK = 1, TOP_MSG_ID_MASK = 512, SEND_AS_MASK = 8192 };
  mutable int32 var0;

  messages_prolongWebView(int32 flags_, bool silent_, object_ptr<InputPeer> &&peer_, object_ptr<InputUser> &&bot_, int64 query_id_, int32 reply_to_msg_id_, int32 top_msg_id_, object_ptr<InputPeer> &&send_as_);

  static const std::int32_t ID = 2146648841;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_rateTranscribedAudio final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int32 msg_id_;
  int64 transcription_id_;
  bool good_;

  messages_rateTranscribedAudio(object_ptr<InputPeer> &&peer_, int32 msg_id_, int64 transcription_id_, bool good_);

  static const std::int32_t ID = 2132608815;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_readDiscussion final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int32 msg_id_;
  int32 read_max_id_;

  messages_readDiscussion(object_ptr<InputPeer> &&peer_, int32 msg_id_, int32 read_max_id_);

  static const std::int32_t ID = -147740172;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_readEncryptedHistory final : public Function {
 public:
  object_ptr<inputEncryptedChat> peer_;
  int32 max_date_;

  messages_readEncryptedHistory(object_ptr<inputEncryptedChat> &&peer_, int32 max_date_);

  static const std::int32_t ID = 2135648522;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_readFeaturedStickers final : public Function {
 public:
  array<int64> id_;

  explicit messages_readFeaturedStickers(array<int64> &&id_);

  static const std::int32_t ID = 1527873830;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_readHistory final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int32 max_id_;

  messages_readHistory(object_ptr<InputPeer> &&peer_, int32 max_id_);

  static const std::int32_t ID = 238054714;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_affectedMessages>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_readMentions final : public Function {
 public:
  int32 flags_;
  object_ptr<InputPeer> peer_;
  int32 top_msg_id_;
  enum Flags : std::int32_t { TOP_MSG_ID_MASK = 1 };
  mutable int32 var0;

  messages_readMentions(int32 flags_, object_ptr<InputPeer> &&peer_, int32 top_msg_id_);

  static const std::int32_t ID = 921026381;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_affectedHistory>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_readMessageContents final : public Function {
 public:
  array<int32> id_;

  explicit messages_readMessageContents(array<int32> &&id_);

  static const std::int32_t ID = 916930423;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_affectedMessages>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_readReactions final : public Function {
 public:
  int32 flags_;
  object_ptr<InputPeer> peer_;
  int32 top_msg_id_;
  enum Flags : std::int32_t { TOP_MSG_ID_MASK = 1 };
  mutable int32 var0;

  messages_readReactions(int32 flags_, object_ptr<InputPeer> &&peer_, int32 top_msg_id_);

  static const std::int32_t ID = 1420459918;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_affectedHistory>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_receivedMessages final : public Function {
 public:
  int32 max_id_;

  explicit messages_receivedMessages(int32 max_id_);

  static const std::int32_t ID = 94983360;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<receivedNotifyMessage>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_receivedQueue final : public Function {
 public:
  int32 max_qts_;

  explicit messages_receivedQueue(int32 max_qts_);

  static const std::int32_t ID = 1436924774;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<int64>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_reorderPinnedDialogs final : public Function {
 public:
  int32 flags_;
  bool force_;
  int32 folder_id_;
  array<object_ptr<InputDialogPeer>> order_;
  enum Flags : std::int32_t { FORCE_MASK = 1 };
  mutable int32 var0;

  messages_reorderPinnedDialogs(int32 flags_, bool force_, int32 folder_id_, array<object_ptr<InputDialogPeer>> &&order_);

  static const std::int32_t ID = 991616823;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_reorderStickerSets final : public Function {
 public:
  int32 flags_;
  bool masks_;
  bool emojis_;
  array<int64> order_;
  enum Flags : std::int32_t { MASKS_MASK = 1, EMOJIS_MASK = 2 };
  mutable int32 var0;

  messages_reorderStickerSets(int32 flags_, bool masks_, bool emojis_, array<int64> &&order_);

  static const std::int32_t ID = 2016638777;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_report final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  array<int32> id_;
  object_ptr<ReportReason> reason_;
  string message_;

  messages_report(object_ptr<InputPeer> &&peer_, array<int32> &&id_, object_ptr<ReportReason> &&reason_, string const &message_);

  static const std::int32_t ID = -1991005362;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_reportEncryptedSpam final : public Function {
 public:
  object_ptr<inputEncryptedChat> peer_;

  explicit messages_reportEncryptedSpam(object_ptr<inputEncryptedChat> &&peer_);

  static const std::int32_t ID = 1259113487;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_reportReaction final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int32 id_;
  object_ptr<InputPeer> reaction_peer_;

  messages_reportReaction(object_ptr<InputPeer> &&peer_, int32 id_, object_ptr<InputPeer> &&reaction_peer_);

  static const std::int32_t ID = 1063567478;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_reportSpam final : public Function {
 public:
  object_ptr<InputPeer> peer_;

  explicit messages_reportSpam(object_ptr<InputPeer> &&peer_);

  static const std::int32_t ID = -820669733;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_requestAppWebView final : public Function {
 public:
  int32 flags_;
  bool write_allowed_;
  object_ptr<InputPeer> peer_;
  object_ptr<InputBotApp> app_;
  string start_param_;
  object_ptr<dataJSON> theme_params_;
  string platform_;
  enum Flags : std::int32_t { WRITE_ALLOWED_MASK = 1, START_PARAM_MASK = 2, THEME_PARAMS_MASK = 4 };
  mutable int32 var0;

  messages_requestAppWebView(int32 flags_, bool write_allowed_, object_ptr<InputPeer> &&peer_, object_ptr<InputBotApp> &&app_, string const &start_param_, object_ptr<dataJSON> &&theme_params_, string const &platform_);

  static const std::int32_t ID = -1940243652;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<appWebViewResultUrl>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_requestEncryption final : public Function {
 public:
  object_ptr<InputUser> user_id_;
  int32 random_id_;
  bytes g_a_;

  messages_requestEncryption(object_ptr<InputUser> &&user_id_, int32 random_id_, bytes &&g_a_);

  static const std::int32_t ID = -162681021;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<EncryptedChat>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_requestSimpleWebView final : public Function {
 public:
  int32 flags_;
  bool from_switch_webview_;
  object_ptr<InputUser> bot_;
  string url_;
  object_ptr<dataJSON> theme_params_;
  string platform_;
  enum Flags : std::int32_t { FROM_SWITCH_WEBVIEW_MASK = 2, THEME_PARAMS_MASK = 1 };
  mutable int32 var0;

  messages_requestSimpleWebView(int32 flags_, bool from_switch_webview_, object_ptr<InputUser> &&bot_, string const &url_, object_ptr<dataJSON> &&theme_params_, string const &platform_);

  static const std::int32_t ID = 698084494;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<simpleWebViewResultUrl>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_requestUrlAuth final : public Function {
 public:
  int32 flags_;
  object_ptr<InputPeer> peer_;
  int32 msg_id_;
  int32 button_id_;
  string url_;
  enum Flags : std::int32_t { PEER_MASK = 2, MSG_ID_MASK = 2, BUTTON_ID_MASK = 2, URL_MASK = 4 };
  mutable int32 var0;

  messages_requestUrlAuth(int32 flags_, object_ptr<InputPeer> &&peer_, int32 msg_id_, int32 button_id_, string const &url_);

  static const std::int32_t ID = 428848198;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<UrlAuthResult>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_requestWebView final : public Function {
 public:
  int32 flags_;
  bool from_bot_menu_;
  bool silent_;
  object_ptr<InputPeer> peer_;
  object_ptr<InputUser> bot_;
  string url_;
  string start_param_;
  object_ptr<dataJSON> theme_params_;
  string platform_;
  int32 reply_to_msg_id_;
  int32 top_msg_id_;
  object_ptr<InputPeer> send_as_;
  enum Flags : std::int32_t { FROM_BOT_MENU_MASK = 16, SILENT_MASK = 32, URL_MASK = 2, START_PARAM_MASK = 8, THEME_PARAMS_MASK = 4, REPLY_TO_MSG_ID_MASK = 1, TOP_MSG_ID_MASK = 512, SEND_AS_MASK = 8192 };
  mutable int32 var0;

  messages_requestWebView(int32 flags_, bool from_bot_menu_, bool silent_, object_ptr<InputPeer> &&peer_, object_ptr<InputUser> &&bot_, string const &url_, string const &start_param_, object_ptr<dataJSON> &&theme_params_, string const &platform_, int32 reply_to_msg_id_, int32 top_msg_id_, object_ptr<InputPeer> &&send_as_);

  static const std::int32_t ID = 395003915;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<webViewResultUrl>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_saveDefaultSendAs final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  object_ptr<InputPeer> send_as_;

  messages_saveDefaultSendAs(object_ptr<InputPeer> &&peer_, object_ptr<InputPeer> &&send_as_);

  static const std::int32_t ID = -855777386;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_saveDraft final : public Function {
 public:
  int32 flags_;
  bool no_webpage_;
  int32 reply_to_msg_id_;
  int32 top_msg_id_;
  object_ptr<InputPeer> peer_;
  string message_;
  array<object_ptr<MessageEntity>> entities_;
  enum Flags : std::int32_t { NO_WEBPAGE_MASK = 2, REPLY_TO_MSG_ID_MASK = 1, TOP_MSG_ID_MASK = 4, ENTITIES_MASK = 8 };
  mutable int32 var0;

  messages_saveDraft(int32 flags_, bool no_webpage_, int32 reply_to_msg_id_, int32 top_msg_id_, object_ptr<InputPeer> &&peer_, string const &message_, array<object_ptr<MessageEntity>> &&entities_);

  static const std::int32_t ID = -1271718337;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_saveGif final : public Function {
 public:
  object_ptr<InputDocument> id_;
  bool unsave_;

  messages_saveGif(object_ptr<InputDocument> &&id_, bool unsave_);

  static const std::int32_t ID = 846868683;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_saveRecentSticker final : public Function {
 public:
  int32 flags_;
  bool attached_;
  object_ptr<InputDocument> id_;
  bool unsave_;
  enum Flags : std::int32_t { ATTACHED_MASK = 1 };
  mutable int32 var0;

  messages_saveRecentSticker(int32 flags_, bool attached_, object_ptr<InputDocument> &&id_, bool unsave_);

  static const std::int32_t ID = 958863608;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_search final : public Function {
 public:
  int32 flags_;
  object_ptr<InputPeer> peer_;
  string q_;
  object_ptr<InputPeer> from_id_;
  int32 top_msg_id_;
  object_ptr<MessagesFilter> filter_;
  int32 min_date_;
  int32 max_date_;
  int32 offset_id_;
  int32 add_offset_;
  int32 limit_;
  int32 max_id_;
  int32 min_id_;
  int64 hash_;
  enum Flags : std::int32_t { FROM_ID_MASK = 1, TOP_MSG_ID_MASK = 2 };
  mutable int32 var0;

  messages_search(int32 flags_, object_ptr<InputPeer> &&peer_, string const &q_, object_ptr<InputPeer> &&from_id_, int32 top_msg_id_, object_ptr<MessagesFilter> &&filter_, int32 min_date_, int32 max_date_, int32 offset_id_, int32 add_offset_, int32 limit_, int32 max_id_, int32 min_id_, int64 hash_);

  static const std::int32_t ID = -1593989278;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Messages>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_searchCustomEmoji final : public Function {
 public:
  string emoticon_;
  int64 hash_;

  messages_searchCustomEmoji(string const &emoticon_, int64 hash_);

  static const std::int32_t ID = 739360983;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<EmojiList>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_searchGlobal final : public Function {
 public:
  int32 flags_;
  int32 folder_id_;
  string q_;
  object_ptr<MessagesFilter> filter_;
  int32 min_date_;
  int32 max_date_;
  int32 offset_rate_;
  object_ptr<InputPeer> offset_peer_;
  int32 offset_id_;
  int32 limit_;
  enum Flags : std::int32_t { FOLDER_ID_MASK = 1 };
  mutable int32 var0;

  messages_searchGlobal(int32 flags_, int32 folder_id_, string const &q_, object_ptr<MessagesFilter> &&filter_, int32 min_date_, int32 max_date_, int32 offset_rate_, object_ptr<InputPeer> &&offset_peer_, int32 offset_id_, int32 limit_);

  static const std::int32_t ID = 1271290010;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Messages>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_searchSentMedia final : public Function {
 public:
  string q_;
  object_ptr<MessagesFilter> filter_;
  int32 limit_;

  messages_searchSentMedia(string const &q_, object_ptr<MessagesFilter> &&filter_, int32 limit_);

  static const std::int32_t ID = 276705696;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Messages>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_searchStickerSets final : public Function {
 public:
  int32 flags_;
  bool exclude_featured_;
  string q_;
  int64 hash_;
  enum Flags : std::int32_t { EXCLUDE_FEATURED_MASK = 1 };
  mutable int32 var0;

  messages_searchStickerSets(int32 flags_, bool exclude_featured_, string const &q_, int64 hash_);

  static const std::int32_t ID = 896555914;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_FoundStickerSets>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_sendBotRequestedPeer final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int32 msg_id_;
  int32 button_id_;
  object_ptr<InputPeer> requested_peer_;

  messages_sendBotRequestedPeer(object_ptr<InputPeer> &&peer_, int32 msg_id_, int32 button_id_, object_ptr<InputPeer> &&requested_peer_);

  static const std::int32_t ID = -29831141;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_sendEncrypted final : public Function {
 public:
  int32 flags_;
  bool silent_;
  object_ptr<inputEncryptedChat> peer_;
  int64 random_id_;
  bytes data_;
  enum Flags : std::int32_t { SILENT_MASK = 1 };
  mutable int32 var0;

  messages_sendEncrypted(int32 flags_, bool silent_, object_ptr<inputEncryptedChat> &&peer_, int64 random_id_, bytes &&data_);

  static const std::int32_t ID = 1157265941;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_SentEncryptedMessage>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_sendEncryptedFile final : public Function {
 public:
  int32 flags_;
  bool silent_;
  object_ptr<inputEncryptedChat> peer_;
  int64 random_id_;
  bytes data_;
  object_ptr<InputEncryptedFile> file_;
  enum Flags : std::int32_t { SILENT_MASK = 1 };
  mutable int32 var0;

  messages_sendEncryptedFile(int32 flags_, bool silent_, object_ptr<inputEncryptedChat> &&peer_, int64 random_id_, bytes &&data_, object_ptr<InputEncryptedFile> &&file_);

  static const std::int32_t ID = 1431914525;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_SentEncryptedMessage>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_sendEncryptedService final : public Function {
 public:
  object_ptr<inputEncryptedChat> peer_;
  int64 random_id_;
  bytes data_;

  messages_sendEncryptedService(object_ptr<inputEncryptedChat> &&peer_, int64 random_id_, bytes &&data_);

  static const std::int32_t ID = 852769188;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_SentEncryptedMessage>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_sendInlineBotResult final : public Function {
 public:
  int32 flags_;
  bool silent_;
  bool background_;
  bool clear_draft_;
  bool hide_via_;
  object_ptr<InputPeer> peer_;
  int32 reply_to_msg_id_;
  int32 top_msg_id_;
  int64 random_id_;
  int64 query_id_;
  string id_;
  int32 schedule_date_;
  object_ptr<InputPeer> send_as_;
  enum Flags : std::int32_t { SILENT_MASK = 32, BACKGROUND_MASK = 64, CLEAR_DRAFT_MASK = 128, HIDE_VIA_MASK = 2048, REPLY_TO_MSG_ID_MASK = 1, TOP_MSG_ID_MASK = 512, SCHEDULE_DATE_MASK = 1024, SEND_AS_MASK = 8192 };
  mutable int32 var0;

  messages_sendInlineBotResult(int32 flags_, bool silent_, bool background_, bool clear_draft_, bool hide_via_, object_ptr<InputPeer> &&peer_, int32 reply_to_msg_id_, int32 top_msg_id_, int64 random_id_, int64 query_id_, string const &id_, int32 schedule_date_, object_ptr<InputPeer> &&send_as_);

  static const std::int32_t ID = -738468661;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_sendMedia final : public Function {
 public:
  int32 flags_;
  bool silent_;
  bool background_;
  bool clear_draft_;
  bool noforwards_;
  bool update_stickersets_order_;
  object_ptr<InputPeer> peer_;
  int32 reply_to_msg_id_;
  int32 top_msg_id_;
  object_ptr<InputMedia> media_;
  string message_;
  int64 random_id_;
  object_ptr<ReplyMarkup> reply_markup_;
  array<object_ptr<MessageEntity>> entities_;
  int32 schedule_date_;
  object_ptr<InputPeer> send_as_;
  enum Flags : std::int32_t { SILENT_MASK = 32, BACKGROUND_MASK = 64, CLEAR_DRAFT_MASK = 128, NOFORWARDS_MASK = 16384, UPDATE_STICKERSETS_ORDER_MASK = 32768, REPLY_TO_MSG_ID_MASK = 1, TOP_MSG_ID_MASK = 512, REPLY_MARKUP_MASK = 4, ENTITIES_MASK = 8, SCHEDULE_DATE_MASK = 1024, SEND_AS_MASK = 8192 };
  mutable int32 var0;

  messages_sendMedia(int32 flags_, bool silent_, bool background_, bool clear_draft_, bool noforwards_, bool update_stickersets_order_, object_ptr<InputPeer> &&peer_, int32 reply_to_msg_id_, int32 top_msg_id_, object_ptr<InputMedia> &&media_, string const &message_, int64 random_id_, object_ptr<ReplyMarkup> &&reply_markup_, array<object_ptr<MessageEntity>> &&entities_, int32 schedule_date_, object_ptr<InputPeer> &&send_as_);

  static const std::int32_t ID = 1967638886;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_sendMessage final : public Function {
 public:
  int32 flags_;
  bool no_webpage_;
  bool silent_;
  bool background_;
  bool clear_draft_;
  bool noforwards_;
  bool update_stickersets_order_;
  object_ptr<InputPeer> peer_;
  int32 reply_to_msg_id_;
  int32 top_msg_id_;
  string message_;
  int64 random_id_;
  object_ptr<ReplyMarkup> reply_markup_;
  array<object_ptr<MessageEntity>> entities_;
  int32 schedule_date_;
  object_ptr<InputPeer> send_as_;
  enum Flags : std::int32_t { NO_WEBPAGE_MASK = 2, SILENT_MASK = 32, BACKGROUND_MASK = 64, CLEAR_DRAFT_MASK = 128, NOFORWARDS_MASK = 16384, UPDATE_STICKERSETS_ORDER_MASK = 32768, REPLY_TO_MSG_ID_MASK = 1, TOP_MSG_ID_MASK = 512, REPLY_MARKUP_MASK = 4, ENTITIES_MASK = 8, SCHEDULE_DATE_MASK = 1024, SEND_AS_MASK = 8192 };
  mutable int32 var0;

  messages_sendMessage(int32 flags_, bool no_webpage_, bool silent_, bool background_, bool clear_draft_, bool noforwards_, bool update_stickersets_order_, object_ptr<InputPeer> &&peer_, int32 reply_to_msg_id_, int32 top_msg_id_, string const &message_, int64 random_id_, object_ptr<ReplyMarkup> &&reply_markup_, array<object_ptr<MessageEntity>> &&entities_, int32 schedule_date_, object_ptr<InputPeer> &&send_as_);

  static const std::int32_t ID = 482476935;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_sendMultiMedia final : public Function {
 public:
  int32 flags_;
  bool silent_;
  bool background_;
  bool clear_draft_;
  bool noforwards_;
  bool update_stickersets_order_;
  object_ptr<InputPeer> peer_;
  int32 reply_to_msg_id_;
  int32 top_msg_id_;
  array<object_ptr<inputSingleMedia>> multi_media_;
  int32 schedule_date_;
  object_ptr<InputPeer> send_as_;
  enum Flags : std::int32_t { SILENT_MASK = 32, BACKGROUND_MASK = 64, CLEAR_DRAFT_MASK = 128, NOFORWARDS_MASK = 16384, UPDATE_STICKERSETS_ORDER_MASK = 32768, REPLY_TO_MSG_ID_MASK = 1, TOP_MSG_ID_MASK = 512, SCHEDULE_DATE_MASK = 1024, SEND_AS_MASK = 8192 };
  mutable int32 var0;

  messages_sendMultiMedia(int32 flags_, bool silent_, bool background_, bool clear_draft_, bool noforwards_, bool update_stickersets_order_, object_ptr<InputPeer> &&peer_, int32 reply_to_msg_id_, int32 top_msg_id_, array<object_ptr<inputSingleMedia>> &&multi_media_, int32 schedule_date_, object_ptr<InputPeer> &&send_as_);

  static const std::int32_t ID = -1225713124;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_sendReaction final : public Function {
 public:
  int32 flags_;
  bool big_;
  bool add_to_recent_;
  object_ptr<InputPeer> peer_;
  int32 msg_id_;
  array<object_ptr<Reaction>> reaction_;
  enum Flags : std::int32_t { BIG_MASK = 2, ADD_TO_RECENT_MASK = 4, REACTION_MASK = 1 };
  mutable int32 var0;

  messages_sendReaction(int32 flags_, bool big_, bool add_to_recent_, object_ptr<InputPeer> &&peer_, int32 msg_id_, array<object_ptr<Reaction>> &&reaction_);

  static const std::int32_t ID = -754091820;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_sendScheduledMessages final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  array<int32> id_;

  messages_sendScheduledMessages(object_ptr<InputPeer> &&peer_, array<int32> &&id_);

  static const std::int32_t ID = -1120369398;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_sendScreenshotNotification final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int32 reply_to_msg_id_;
  int64 random_id_;

  messages_sendScreenshotNotification(object_ptr<InputPeer> &&peer_, int32 reply_to_msg_id_, int64 random_id_);

  static const std::int32_t ID = -914493408;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_sendVote final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int32 msg_id_;
  array<bytes> options_;

  messages_sendVote(object_ptr<InputPeer> &&peer_, int32 msg_id_, array<bytes> &&options_);

  static const std::int32_t ID = 283795844;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_sendWebViewData final : public Function {
 public:
  object_ptr<InputUser> bot_;
  int64 random_id_;
  string button_text_;
  string data_;

  messages_sendWebViewData(object_ptr<InputUser> &&bot_, int64 random_id_, string const &button_text_, string const &data_);

  static const std::int32_t ID = -603831608;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_sendWebViewResultMessage final : public Function {
 public:
  string bot_query_id_;
  object_ptr<InputBotInlineResult> result_;

  messages_sendWebViewResultMessage(string const &bot_query_id_, object_ptr<InputBotInlineResult> &&result_);

  static const std::int32_t ID = 172168437;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<webViewMessageSent>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_setBotCallbackAnswer final : public Function {
 public:
  int32 flags_;
  bool alert_;
  int64 query_id_;
  string message_;
  string url_;
  int32 cache_time_;
  enum Flags : std::int32_t { ALERT_MASK = 2, MESSAGE_MASK = 1, URL_MASK = 4 };
  mutable int32 var0;

  messages_setBotCallbackAnswer(int32 flags_, bool alert_, int64 query_id_, string const &message_, string const &url_, int32 cache_time_);

  static const std::int32_t ID = -712043766;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_setBotPrecheckoutResults final : public Function {
 public:
  int32 flags_;
  bool success_;
  int64 query_id_;
  string error_;
  enum Flags : std::int32_t { SUCCESS_MASK = 2, ERROR_MASK = 1 };
  mutable int32 var0;

  messages_setBotPrecheckoutResults(int32 flags_, bool success_, int64 query_id_, string const &error_);

  static const std::int32_t ID = 163765653;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_setBotShippingResults final : public Function {
 public:
  int32 flags_;
  int64 query_id_;
  string error_;
  array<object_ptr<shippingOption>> shipping_options_;
  enum Flags : std::int32_t { ERROR_MASK = 1, SHIPPING_OPTIONS_MASK = 2 };
  mutable int32 var0;

  messages_setBotShippingResults(int32 flags_, int64 query_id_, string const &error_, array<object_ptr<shippingOption>> &&shipping_options_);

  static const std::int32_t ID = -436833542;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_setChatAvailableReactions final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  object_ptr<ChatReactions> available_reactions_;

  messages_setChatAvailableReactions(object_ptr<InputPeer> &&peer_, object_ptr<ChatReactions> &&available_reactions_);

  static const std::int32_t ID = -21928079;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_setChatTheme final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  string emoticon_;

  messages_setChatTheme(object_ptr<InputPeer> &&peer_, string const &emoticon_);

  static const std::int32_t ID = -432283329;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_setChatWallPaper final : public Function {
 public:
  int32 flags_;
  object_ptr<InputPeer> peer_;
  object_ptr<InputWallPaper> wallpaper_;
  object_ptr<wallPaperSettings> settings_;
  int32 id_;
  enum Flags : std::int32_t { WALLPAPER_MASK = 1, SETTINGS_MASK = 4, ID_MASK = 2 };
  mutable int32 var0;

  messages_setChatWallPaper(int32 flags_, object_ptr<InputPeer> &&peer_, object_ptr<InputWallPaper> &&wallpaper_, object_ptr<wallPaperSettings> &&settings_, int32 id_);

  static const std::int32_t ID = -1879389471;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_setDefaultHistoryTTL final : public Function {
 public:
  int32 period_;

  explicit messages_setDefaultHistoryTTL(int32 period_);

  static const std::int32_t ID = -1632299963;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_setDefaultReaction final : public Function {
 public:
  object_ptr<Reaction> reaction_;

  explicit messages_setDefaultReaction(object_ptr<Reaction> &&reaction_);

  static const std::int32_t ID = 1330094102;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_setEncryptedTyping final : public Function {
 public:
  object_ptr<inputEncryptedChat> peer_;
  bool typing_;

  messages_setEncryptedTyping(object_ptr<inputEncryptedChat> &&peer_, bool typing_);

  static const std::int32_t ID = 2031374829;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_setGameScore final : public Function {
 public:
  int32 flags_;
  bool edit_message_;
  bool force_;
  object_ptr<InputPeer> peer_;
  int32 id_;
  object_ptr<InputUser> user_id_;
  int32 score_;
  enum Flags : std::int32_t { EDIT_MESSAGE_MASK = 1, FORCE_MASK = 2 };
  mutable int32 var0;

  messages_setGameScore(int32 flags_, bool edit_message_, bool force_, object_ptr<InputPeer> &&peer_, int32 id_, object_ptr<InputUser> &&user_id_, int32 score_);

  static const std::int32_t ID = -1896289088;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_setHistoryTTL final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int32 period_;

  messages_setHistoryTTL(object_ptr<InputPeer> &&peer_, int32 period_);

  static const std::int32_t ID = -1207017500;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_setInlineBotResults final : public Function {
 public:
  int32 flags_;
  bool gallery_;
  bool private_;
  int64 query_id_;
  array<object_ptr<InputBotInlineResult>> results_;
  int32 cache_time_;
  string next_offset_;
  object_ptr<inlineBotSwitchPM> switch_pm_;
  object_ptr<inlineBotWebView> switch_webview_;
  enum Flags : std::int32_t { GALLERY_MASK = 1, PRIVATE_MASK = 2, NEXT_OFFSET_MASK = 4, SWITCH_PM_MASK = 8, SWITCH_WEBVIEW_MASK = 16 };
  mutable int32 var0;

  messages_setInlineBotResults(int32 flags_, bool gallery_, bool private_, int64 query_id_, array<object_ptr<InputBotInlineResult>> &&results_, int32 cache_time_, string const &next_offset_, object_ptr<inlineBotSwitchPM> &&switch_pm_, object_ptr<inlineBotWebView> &&switch_webview_);

  static const std::int32_t ID = -1156406247;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_setInlineGameScore final : public Function {
 public:
  int32 flags_;
  bool edit_message_;
  bool force_;
  object_ptr<InputBotInlineMessageID> id_;
  object_ptr<InputUser> user_id_;
  int32 score_;
  enum Flags : std::int32_t { EDIT_MESSAGE_MASK = 1, FORCE_MASK = 2 };
  mutable int32 var0;

  messages_setInlineGameScore(int32 flags_, bool edit_message_, bool force_, object_ptr<InputBotInlineMessageID> &&id_, object_ptr<InputUser> &&user_id_, int32 score_);

  static const std::int32_t ID = 363700068;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_setTyping final : public Function {
 public:
  int32 flags_;
  object_ptr<InputPeer> peer_;
  int32 top_msg_id_;
  object_ptr<SendMessageAction> action_;
  enum Flags : std::int32_t { TOP_MSG_ID_MASK = 1 };
  mutable int32 var0;

  messages_setTyping(int32 flags_, object_ptr<InputPeer> &&peer_, int32 top_msg_id_, object_ptr<SendMessageAction> &&action_);

  static const std::int32_t ID = 1486110434;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_startBot final : public Function {
 public:
  object_ptr<InputUser> bot_;
  object_ptr<InputPeer> peer_;
  int64 random_id_;
  string start_param_;

  messages_startBot(object_ptr<InputUser> &&bot_, object_ptr<InputPeer> &&peer_, int64 random_id_, string const &start_param_);

  static const std::int32_t ID = -421563528;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_startHistoryImport final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int64 import_id_;

  messages_startHistoryImport(object_ptr<InputPeer> &&peer_, int64 import_id_);

  static const std::int32_t ID = -1271008444;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_toggleBotInAttachMenu final : public Function {
 public:
  int32 flags_;
  bool write_allowed_;
  object_ptr<InputUser> bot_;
  bool enabled_;
  enum Flags : std::int32_t { WRITE_ALLOWED_MASK = 1 };
  mutable int32 var0;

  messages_toggleBotInAttachMenu(int32 flags_, bool write_allowed_, object_ptr<InputUser> &&bot_, bool enabled_);

  static const std::int32_t ID = 1777704297;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_toggleDialogPin final : public Function {
 public:
  int32 flags_;
  bool pinned_;
  object_ptr<InputDialogPeer> peer_;
  enum Flags : std::int32_t { PINNED_MASK = 1 };
  mutable int32 var0;

  messages_toggleDialogPin(int32 flags_, bool pinned_, object_ptr<InputDialogPeer> &&peer_);

  static const std::int32_t ID = -1489903017;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_toggleNoForwards final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  bool enabled_;

  messages_toggleNoForwards(object_ptr<InputPeer> &&peer_, bool enabled_);

  static const std::int32_t ID = -1323389022;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_togglePeerTranslations final : public Function {
 public:
  int32 flags_;
  bool disabled_;
  object_ptr<InputPeer> peer_;
  enum Flags : std::int32_t { DISABLED_MASK = 1 };
  mutable int32 var0;

  messages_togglePeerTranslations(int32 flags_, bool disabled_, object_ptr<InputPeer> &&peer_);

  static const std::int32_t ID = -461589127;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_toggleStickerSets final : public Function {
 public:
  int32 flags_;
  bool uninstall_;
  bool archive_;
  bool unarchive_;
  array<object_ptr<InputStickerSet>> stickersets_;
  enum Flags : std::int32_t { UNINSTALL_MASK = 1, ARCHIVE_MASK = 2, UNARCHIVE_MASK = 4 };
  mutable int32 var0;

  messages_toggleStickerSets(int32 flags_, bool uninstall_, bool archive_, bool unarchive_, array<object_ptr<InputStickerSet>> &&stickersets_);

  static const std::int32_t ID = -1257951254;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_transcribeAudio final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int32 msg_id_;

  messages_transcribeAudio(object_ptr<InputPeer> &&peer_, int32 msg_id_);

  static const std::int32_t ID = 647928393;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_transcribedAudio>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_translateText final : public Function {
 public:
  int32 flags_;
  object_ptr<InputPeer> peer_;
  array<int32> id_;
  array<object_ptr<textWithEntities>> text_;
  string to_lang_;
  enum Flags : std::int32_t { PEER_MASK = 1, ID_MASK = 1, TEXT_MASK = 2 };
  mutable int32 var0;

  messages_translateText(int32 flags_, object_ptr<InputPeer> &&peer_, array<int32> &&id_, array<object_ptr<textWithEntities>> &&text_, string const &to_lang_);

  static const std::int32_t ID = 1662529584;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_translateResult>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_uninstallStickerSet final : public Function {
 public:
  object_ptr<InputStickerSet> stickerset_;

  explicit messages_uninstallStickerSet(object_ptr<InputStickerSet> &&stickerset_);

  static const std::int32_t ID = -110209570;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_unpinAllMessages final : public Function {
 public:
  int32 flags_;
  object_ptr<InputPeer> peer_;
  int32 top_msg_id_;
  enum Flags : std::int32_t { TOP_MSG_ID_MASK = 1 };
  mutable int32 var0;

  messages_unpinAllMessages(int32 flags_, object_ptr<InputPeer> &&peer_, int32 top_msg_id_);

  static const std::int32_t ID = -299714136;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_affectedHistory>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_updateDialogFilter final : public Function {
 public:
  int32 flags_;
  int32 id_;
  object_ptr<DialogFilter> filter_;
  enum Flags : std::int32_t { FILTER_MASK = 1 };
  mutable int32 var0;

  messages_updateDialogFilter(int32 flags_, int32 id_, object_ptr<DialogFilter> &&filter_);

  static const std::int32_t ID = 450142282;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_updateDialogFiltersOrder final : public Function {
 public:
  array<int32> order_;

  explicit messages_updateDialogFiltersOrder(array<int32> &&order_);

  static const std::int32_t ID = -983318044;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_updatePinnedMessage final : public Function {
 public:
  int32 flags_;
  bool silent_;
  bool unpin_;
  bool pm_oneside_;
  object_ptr<InputPeer> peer_;
  int32 id_;
  enum Flags : std::int32_t { SILENT_MASK = 1, UNPIN_MASK = 2, PM_ONESIDE_MASK = 4 };
  mutable int32 var0;

  messages_updatePinnedMessage(int32 flags_, bool silent_, bool unpin_, bool pm_oneside_, object_ptr<InputPeer> &&peer_, int32 id_);

  static const std::int32_t ID = -760547348;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_uploadEncryptedFile final : public Function {
 public:
  object_ptr<inputEncryptedChat> peer_;
  object_ptr<InputEncryptedFile> file_;

  messages_uploadEncryptedFile(object_ptr<inputEncryptedChat> &&peer_, object_ptr<InputEncryptedFile> &&file_);

  static const std::int32_t ID = 1347929239;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<EncryptedFile>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_uploadImportedMedia final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int64 import_id_;
  string file_name_;
  object_ptr<InputMedia> media_;

  messages_uploadImportedMedia(object_ptr<InputPeer> &&peer_, int64 import_id_, string const &file_name_, object_ptr<InputMedia> &&media_);

  static const std::int32_t ID = 713433234;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<MessageMedia>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class messages_uploadMedia final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  object_ptr<InputMedia> media_;

  messages_uploadMedia(object_ptr<InputPeer> &&peer_, object_ptr<InputMedia> &&media_);

  static const std::int32_t ID = 1369162417;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<MessageMedia>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class payments_assignAppStoreTransaction final : public Function {
 public:
  bytes receipt_;
  object_ptr<InputStorePaymentPurpose> purpose_;

  payments_assignAppStoreTransaction(bytes &&receipt_, object_ptr<InputStorePaymentPurpose> &&purpose_);

  static const std::int32_t ID = -2131921795;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class payments_assignPlayMarketTransaction final : public Function {
 public:
  object_ptr<dataJSON> receipt_;
  object_ptr<InputStorePaymentPurpose> purpose_;

  payments_assignPlayMarketTransaction(object_ptr<dataJSON> &&receipt_, object_ptr<InputStorePaymentPurpose> &&purpose_);

  static const std::int32_t ID = -537046829;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class payments_canPurchasePremium final : public Function {
 public:
  object_ptr<InputStorePaymentPurpose> purpose_;

  explicit payments_canPurchasePremium(object_ptr<InputStorePaymentPurpose> &&purpose_);

  static const std::int32_t ID = -1614700874;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class payments_clearSavedInfo final : public Function {
 public:
  int32 flags_;
  bool credentials_;
  bool info_;
  enum Flags : std::int32_t { CREDENTIALS_MASK = 1, INFO_MASK = 2 };
  mutable int32 var0;

  payments_clearSavedInfo(int32 flags_, bool credentials_, bool info_);

  static const std::int32_t ID = -667062079;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class payments_exportInvoice final : public Function {
 public:
  object_ptr<InputMedia> invoice_media_;

  explicit payments_exportInvoice(object_ptr<InputMedia> &&invoice_media_);

  static const std::int32_t ID = 261206117;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<payments_exportedInvoice>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class payments_getBankCardData final : public Function {
 public:
  string number_;

  explicit payments_getBankCardData(string const &number_);

  static const std::int32_t ID = 779736953;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<payments_bankCardData>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class payments_getPaymentForm final : public Function {
 public:
  int32 flags_;
  object_ptr<InputInvoice> invoice_;
  object_ptr<dataJSON> theme_params_;
  enum Flags : std::int32_t { THEME_PARAMS_MASK = 1 };
  mutable int32 var0;

  payments_getPaymentForm(int32 flags_, object_ptr<InputInvoice> &&invoice_, object_ptr<dataJSON> &&theme_params_);

  static const std::int32_t ID = 924093883;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<payments_paymentForm>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class payments_getPaymentReceipt final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  int32 msg_id_;

  payments_getPaymentReceipt(object_ptr<InputPeer> &&peer_, int32 msg_id_);

  static const std::int32_t ID = 611897804;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<payments_paymentReceipt>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class payments_getSavedInfo final : public Function {
 public:

  static const std::int32_t ID = 578650699;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<payments_savedInfo>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class payments_sendPaymentForm final : public Function {
 public:
  int32 flags_;
  int64 form_id_;
  object_ptr<InputInvoice> invoice_;
  string requested_info_id_;
  string shipping_option_id_;
  object_ptr<InputPaymentCredentials> credentials_;
  int64 tip_amount_;
  enum Flags : std::int32_t { REQUESTED_INFO_ID_MASK = 1, SHIPPING_OPTION_ID_MASK = 2, TIP_AMOUNT_MASK = 4 };
  mutable int32 var0;

  payments_sendPaymentForm(int32 flags_, int64 form_id_, object_ptr<InputInvoice> &&invoice_, string const &requested_info_id_, string const &shipping_option_id_, object_ptr<InputPaymentCredentials> &&credentials_, int64 tip_amount_);

  static const std::int32_t ID = 755192367;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<payments_PaymentResult>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class payments_validateRequestedInfo final : public Function {
 public:
  int32 flags_;
  bool save_;
  object_ptr<InputInvoice> invoice_;
  object_ptr<paymentRequestedInfo> info_;
  enum Flags : std::int32_t { SAVE_MASK = 1 };
  mutable int32 var0;

  payments_validateRequestedInfo(int32 flags_, bool save_, object_ptr<InputInvoice> &&invoice_, object_ptr<paymentRequestedInfo> &&info_);

  static const std::int32_t ID = -1228345045;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<payments_validatedRequestedInfo>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_acceptCall final : public Function {
 public:
  object_ptr<inputPhoneCall> peer_;
  bytes g_b_;
  object_ptr<phoneCallProtocol> protocol_;

  phone_acceptCall(object_ptr<inputPhoneCall> &&peer_, bytes &&g_b_, object_ptr<phoneCallProtocol> &&protocol_);

  static const std::int32_t ID = 1003664544;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<phone_phoneCall>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_checkGroupCall final : public Function {
 public:
  object_ptr<inputGroupCall> call_;
  array<int32> sources_;

  phone_checkGroupCall(object_ptr<inputGroupCall> &&call_, array<int32> &&sources_);

  static const std::int32_t ID = -1248003721;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<int32>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_confirmCall final : public Function {
 public:
  object_ptr<inputPhoneCall> peer_;
  bytes g_a_;
  int64 key_fingerprint_;
  object_ptr<phoneCallProtocol> protocol_;

  phone_confirmCall(object_ptr<inputPhoneCall> &&peer_, bytes &&g_a_, int64 key_fingerprint_, object_ptr<phoneCallProtocol> &&protocol_);

  static const std::int32_t ID = 788404002;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<phone_phoneCall>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_createGroupCall final : public Function {
 public:
  int32 flags_;
  bool rtmp_stream_;
  object_ptr<InputPeer> peer_;
  int32 random_id_;
  string title_;
  int32 schedule_date_;
  enum Flags : std::int32_t { RTMP_STREAM_MASK = 4, TITLE_MASK = 1, SCHEDULE_DATE_MASK = 2 };
  mutable int32 var0;

  phone_createGroupCall(int32 flags_, bool rtmp_stream_, object_ptr<InputPeer> &&peer_, int32 random_id_, string const &title_, int32 schedule_date_);

  static const std::int32_t ID = 1221445336;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_discardCall final : public Function {
 public:
  int32 flags_;
  bool video_;
  object_ptr<inputPhoneCall> peer_;
  int32 duration_;
  object_ptr<PhoneCallDiscardReason> reason_;
  int64 connection_id_;
  enum Flags : std::int32_t { VIDEO_MASK = 1 };
  mutable int32 var0;

  phone_discardCall(int32 flags_, bool video_, object_ptr<inputPhoneCall> &&peer_, int32 duration_, object_ptr<PhoneCallDiscardReason> &&reason_, int64 connection_id_);

  static const std::int32_t ID = -1295269440;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_discardGroupCall final : public Function {
 public:
  object_ptr<inputGroupCall> call_;

  explicit phone_discardGroupCall(object_ptr<inputGroupCall> &&call_);

  static const std::int32_t ID = 2054648117;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_editGroupCallParticipant final : public Function {
 public:
  int32 flags_;
  object_ptr<inputGroupCall> call_;
  object_ptr<InputPeer> participant_;
  bool muted_;
  int32 volume_;
  bool raise_hand_;
  bool video_stopped_;
  bool video_paused_;
  bool presentation_paused_;
  enum Flags : std::int32_t { MUTED_MASK = 1, VOLUME_MASK = 2, RAISE_HAND_MASK = 4, VIDEO_STOPPED_MASK = 8, VIDEO_PAUSED_MASK = 16, PRESENTATION_PAUSED_MASK = 32 };
  mutable int32 var0;

  phone_editGroupCallParticipant(int32 flags_, object_ptr<inputGroupCall> &&call_, object_ptr<InputPeer> &&participant_, bool muted_, int32 volume_, bool raise_hand_, bool video_stopped_, bool video_paused_, bool presentation_paused_);

  static const std::int32_t ID = -1524155713;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_editGroupCallTitle final : public Function {
 public:
  object_ptr<inputGroupCall> call_;
  string title_;

  phone_editGroupCallTitle(object_ptr<inputGroupCall> &&call_, string const &title_);

  static const std::int32_t ID = 480685066;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_exportGroupCallInvite final : public Function {
 public:
  int32 flags_;
  bool can_self_unmute_;
  object_ptr<inputGroupCall> call_;
  enum Flags : std::int32_t { CAN_SELF_UNMUTE_MASK = 1 };
  mutable int32 var0;

  phone_exportGroupCallInvite(int32 flags_, bool can_self_unmute_, object_ptr<inputGroupCall> &&call_);

  static const std::int32_t ID = -425040769;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<phone_exportedGroupCallInvite>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_getCallConfig final : public Function {
 public:

  static const std::int32_t ID = 1430593449;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<dataJSON>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_getGroupCall final : public Function {
 public:
  object_ptr<inputGroupCall> call_;
  int32 limit_;

  phone_getGroupCall(object_ptr<inputGroupCall> &&call_, int32 limit_);

  static const std::int32_t ID = 68699611;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<phone_groupCall>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_getGroupCallJoinAs final : public Function {
 public:
  object_ptr<InputPeer> peer_;

  explicit phone_getGroupCallJoinAs(object_ptr<InputPeer> &&peer_);

  static const std::int32_t ID = -277077702;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<phone_joinAsPeers>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_getGroupCallStreamChannels final : public Function {
 public:
  object_ptr<inputGroupCall> call_;

  explicit phone_getGroupCallStreamChannels(object_ptr<inputGroupCall> &&call_);

  static const std::int32_t ID = 447879488;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<phone_groupCallStreamChannels>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_getGroupCallStreamRtmpUrl final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  bool revoke_;

  phone_getGroupCallStreamRtmpUrl(object_ptr<InputPeer> &&peer_, bool revoke_);

  static const std::int32_t ID = -558650433;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<phone_groupCallStreamRtmpUrl>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_getGroupParticipants final : public Function {
 public:
  object_ptr<inputGroupCall> call_;
  array<object_ptr<InputPeer>> ids_;
  array<int32> sources_;
  string offset_;
  int32 limit_;

  phone_getGroupParticipants(object_ptr<inputGroupCall> &&call_, array<object_ptr<InputPeer>> &&ids_, array<int32> &&sources_, string const &offset_, int32 limit_);

  static const std::int32_t ID = -984033109;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<phone_groupParticipants>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_inviteToGroupCall final : public Function {
 public:
  object_ptr<inputGroupCall> call_;
  array<object_ptr<InputUser>> users_;

  phone_inviteToGroupCall(object_ptr<inputGroupCall> &&call_, array<object_ptr<InputUser>> &&users_);

  static const std::int32_t ID = 2067345760;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_joinGroupCall final : public Function {
 public:
  int32 flags_;
  bool muted_;
  bool video_stopped_;
  object_ptr<inputGroupCall> call_;
  object_ptr<InputPeer> join_as_;
  string invite_hash_;
  object_ptr<dataJSON> params_;
  enum Flags : std::int32_t { MUTED_MASK = 1, VIDEO_STOPPED_MASK = 4, INVITE_HASH_MASK = 2 };
  mutable int32 var0;

  phone_joinGroupCall(int32 flags_, bool muted_, bool video_stopped_, object_ptr<inputGroupCall> &&call_, object_ptr<InputPeer> &&join_as_, string const &invite_hash_, object_ptr<dataJSON> &&params_);

  static const std::int32_t ID = -1322057861;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_joinGroupCallPresentation final : public Function {
 public:
  object_ptr<inputGroupCall> call_;
  object_ptr<dataJSON> params_;

  phone_joinGroupCallPresentation(object_ptr<inputGroupCall> &&call_, object_ptr<dataJSON> &&params_);

  static const std::int32_t ID = -873829436;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_leaveGroupCall final : public Function {
 public:
  object_ptr<inputGroupCall> call_;
  int32 source_;

  phone_leaveGroupCall(object_ptr<inputGroupCall> &&call_, int32 source_);

  static const std::int32_t ID = 1342404601;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_leaveGroupCallPresentation final : public Function {
 public:
  object_ptr<inputGroupCall> call_;

  explicit phone_leaveGroupCallPresentation(object_ptr<inputGroupCall> &&call_);

  static const std::int32_t ID = 475058500;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_receivedCall final : public Function {
 public:
  object_ptr<inputPhoneCall> peer_;

  explicit phone_receivedCall(object_ptr<inputPhoneCall> &&peer_);

  static const std::int32_t ID = 399855457;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_requestCall final : public Function {
 public:
  int32 flags_;
  bool video_;
  object_ptr<InputUser> user_id_;
  int32 random_id_;
  bytes g_a_hash_;
  object_ptr<phoneCallProtocol> protocol_;
  enum Flags : std::int32_t { VIDEO_MASK = 1 };
  mutable int32 var0;

  phone_requestCall(int32 flags_, bool video_, object_ptr<InputUser> &&user_id_, int32 random_id_, bytes &&g_a_hash_, object_ptr<phoneCallProtocol> &&protocol_);

  static const std::int32_t ID = 1124046573;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<phone_phoneCall>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_saveCallDebug final : public Function {
 public:
  object_ptr<inputPhoneCall> peer_;
  object_ptr<dataJSON> debug_;

  phone_saveCallDebug(object_ptr<inputPhoneCall> &&peer_, object_ptr<dataJSON> &&debug_);

  static const std::int32_t ID = 662363518;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_saveCallLog final : public Function {
 public:
  object_ptr<inputPhoneCall> peer_;
  object_ptr<InputFile> file_;

  phone_saveCallLog(object_ptr<inputPhoneCall> &&peer_, object_ptr<InputFile> &&file_);

  static const std::int32_t ID = 1092913030;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_saveDefaultGroupCallJoinAs final : public Function {
 public:
  object_ptr<InputPeer> peer_;
  object_ptr<InputPeer> join_as_;

  phone_saveDefaultGroupCallJoinAs(object_ptr<InputPeer> &&peer_, object_ptr<InputPeer> &&join_as_);

  static const std::int32_t ID = 1465786252;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_sendSignalingData final : public Function {
 public:
  object_ptr<inputPhoneCall> peer_;
  bytes data_;

  phone_sendSignalingData(object_ptr<inputPhoneCall> &&peer_, bytes &&data_);

  static const std::int32_t ID = -8744061;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_setCallRating final : public Function {
 public:
  int32 flags_;
  bool user_initiative_;
  object_ptr<inputPhoneCall> peer_;
  int32 rating_;
  string comment_;
  enum Flags : std::int32_t { USER_INITIATIVE_MASK = 1 };
  mutable int32 var0;

  phone_setCallRating(int32 flags_, bool user_initiative_, object_ptr<inputPhoneCall> &&peer_, int32 rating_, string const &comment_);

  static const std::int32_t ID = 1508562471;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_startScheduledGroupCall final : public Function {
 public:
  object_ptr<inputGroupCall> call_;

  explicit phone_startScheduledGroupCall(object_ptr<inputGroupCall> &&call_);

  static const std::int32_t ID = 1451287362;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_toggleGroupCallRecord final : public Function {
 public:
  int32 flags_;
  bool start_;
  bool video_;
  object_ptr<inputGroupCall> call_;
  string title_;
  bool video_portrait_;
  enum Flags : std::int32_t { START_MASK = 1, VIDEO_MASK = 4, TITLE_MASK = 2, VIDEO_PORTRAIT_MASK = 4 };
  mutable int32 var0;

  phone_toggleGroupCallRecord(int32 flags_, bool start_, bool video_, object_ptr<inputGroupCall> &&call_, string const &title_, bool video_portrait_);

  static const std::int32_t ID = -248985848;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_toggleGroupCallSettings final : public Function {
 public:
  int32 flags_;
  bool reset_invite_hash_;
  object_ptr<inputGroupCall> call_;
  bool join_muted_;
  enum Flags : std::int32_t { RESET_INVITE_HASH_MASK = 2, JOIN_MUTED_MASK = 1 };
  mutable int32 var0;

  phone_toggleGroupCallSettings(int32 flags_, bool reset_invite_hash_, object_ptr<inputGroupCall> &&call_, bool join_muted_);

  static const std::int32_t ID = 1958458429;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class phone_toggleGroupCallStartSubscription final : public Function {
 public:
  object_ptr<inputGroupCall> call_;
  bool subscribed_;

  phone_toggleGroupCallStartSubscription(object_ptr<inputGroupCall> &&call_, bool subscribed_);

  static const std::int32_t ID = 563885286;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<Updates>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class photos_deletePhotos final : public Function {
 public:
  array<object_ptr<InputPhoto>> id_;

  explicit photos_deletePhotos(array<object_ptr<InputPhoto>> &&id_);

  static const std::int32_t ID = -2016444625;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<int64>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class photos_getUserPhotos final : public Function {
 public:
  object_ptr<InputUser> user_id_;
  int32 offset_;
  int64 max_id_;
  int32 limit_;

  photos_getUserPhotos(object_ptr<InputUser> &&user_id_, int32 offset_, int64 max_id_, int32 limit_);

  static const std::int32_t ID = -1848823128;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<photos_Photos>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class photos_updateProfilePhoto final : public Function {
 public:
  int32 flags_;
  bool fallback_;
  object_ptr<InputUser> bot_;
  object_ptr<InputPhoto> id_;
  enum Flags : std::int32_t { FALLBACK_MASK = 1, BOT_MASK = 2 };
  mutable int32 var0;

  photos_updateProfilePhoto(int32 flags_, bool fallback_, object_ptr<InputUser> &&bot_, object_ptr<InputPhoto> &&id_);

  static const std::int32_t ID = 166207545;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<photos_photo>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class photos_uploadContactProfilePhoto final : public Function {
 public:
  int32 flags_;
  bool suggest_;
  bool save_;
  object_ptr<InputUser> user_id_;
  object_ptr<InputFile> file_;
  object_ptr<InputFile> video_;
  double video_start_ts_;
  object_ptr<VideoSize> video_emoji_markup_;
  enum Flags : std::int32_t { SUGGEST_MASK = 8, SAVE_MASK = 16, FILE_MASK = 1, VIDEO_MASK = 2, VIDEO_START_TS_MASK = 4, VIDEO_EMOJI_MARKUP_MASK = 32 };
  mutable int32 var0;

  photos_uploadContactProfilePhoto(int32 flags_, bool suggest_, bool save_, object_ptr<InputUser> &&user_id_, object_ptr<InputFile> &&file_, object_ptr<InputFile> &&video_, double video_start_ts_, object_ptr<VideoSize> &&video_emoji_markup_);

  static const std::int32_t ID = -515093903;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<photos_photo>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class photos_uploadProfilePhoto final : public Function {
 public:
  int32 flags_;
  bool fallback_;
  object_ptr<InputUser> bot_;
  object_ptr<InputFile> file_;
  object_ptr<InputFile> video_;
  double video_start_ts_;
  object_ptr<VideoSize> video_emoji_markup_;
  enum Flags : std::int32_t { FALLBACK_MASK = 8, BOT_MASK = 32, FILE_MASK = 1, VIDEO_MASK = 2, VIDEO_START_TS_MASK = 4, VIDEO_EMOJI_MARKUP_MASK = 16 };
  mutable int32 var0;

  photos_uploadProfilePhoto(int32 flags_, bool fallback_, object_ptr<InputUser> &&bot_, object_ptr<InputFile> &&file_, object_ptr<InputFile> &&video_, double video_start_ts_, object_ptr<VideoSize> &&video_emoji_markup_);

  static const std::int32_t ID = 59286453;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<photos_photo>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class stats_getBroadcastStats final : public Function {
 public:
  int32 flags_;
  bool dark_;
  object_ptr<InputChannel> channel_;
  enum Flags : std::int32_t { DARK_MASK = 1 };
  mutable int32 var0;

  stats_getBroadcastStats(int32 flags_, bool dark_, object_ptr<InputChannel> &&channel_);

  static const std::int32_t ID = -1421720550;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stats_broadcastStats>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class stats_getMegagroupStats final : public Function {
 public:
  int32 flags_;
  bool dark_;
  object_ptr<InputChannel> channel_;
  enum Flags : std::int32_t { DARK_MASK = 1 };
  mutable int32 var0;

  stats_getMegagroupStats(int32 flags_, bool dark_, object_ptr<InputChannel> &&channel_);

  static const std::int32_t ID = -589330937;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stats_megagroupStats>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class stats_getMessagePublicForwards final : public Function {
 public:
  object_ptr<InputChannel> channel_;
  int32 msg_id_;
  int32 offset_rate_;
  object_ptr<InputPeer> offset_peer_;
  int32 offset_id_;
  int32 limit_;

  stats_getMessagePublicForwards(object_ptr<InputChannel> &&channel_, int32 msg_id_, int32 offset_rate_, object_ptr<InputPeer> &&offset_peer_, int32 offset_id_, int32 limit_);

  static const std::int32_t ID = 1445996571;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_Messages>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class stats_getMessageStats final : public Function {
 public:
  int32 flags_;
  bool dark_;
  object_ptr<InputChannel> channel_;
  int32 msg_id_;
  enum Flags : std::int32_t { DARK_MASK = 1 };
  mutable int32 var0;

  stats_getMessageStats(int32 flags_, bool dark_, object_ptr<InputChannel> &&channel_, int32 msg_id_);

  static const std::int32_t ID = -1226791947;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stats_messageStats>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class stats_loadAsyncGraph final : public Function {
 public:
  int32 flags_;
  string token_;
  int64 x_;
  enum Flags : std::int32_t { X_MASK = 1 };
  mutable int32 var0;

  stats_loadAsyncGraph(int32 flags_, string const &token_, int64 x_);

  static const std::int32_t ID = 1646092192;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<StatsGraph>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class stickers_addStickerToSet final : public Function {
 public:
  object_ptr<InputStickerSet> stickerset_;
  object_ptr<inputStickerSetItem> sticker_;

  stickers_addStickerToSet(object_ptr<InputStickerSet> &&stickerset_, object_ptr<inputStickerSetItem> &&sticker_);

  static const std::int32_t ID = -2041315650;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_StickerSet>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class stickers_changeSticker final : public Function {
 public:
  int32 flags_;
  object_ptr<InputDocument> sticker_;
  string emoji_;
  object_ptr<maskCoords> mask_coords_;
  string keywords_;
  enum Flags : std::int32_t { EMOJI_MASK = 1, MASK_COORDS_MASK = 2, KEYWORDS_MASK = 4 };
  mutable int32 var0;

  stickers_changeSticker(int32 flags_, object_ptr<InputDocument> &&sticker_, string const &emoji_, object_ptr<maskCoords> &&mask_coords_, string const &keywords_);

  static const std::int32_t ID = -179077444;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_StickerSet>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class stickers_changeStickerPosition final : public Function {
 public:
  object_ptr<InputDocument> sticker_;
  int32 position_;

  stickers_changeStickerPosition(object_ptr<InputDocument> &&sticker_, int32 position_);

  static const std::int32_t ID = -4795190;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_StickerSet>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class stickers_checkShortName final : public Function {
 public:
  string short_name_;

  explicit stickers_checkShortName(string const &short_name_);

  static const std::int32_t ID = 676017721;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class stickers_createStickerSet final : public Function {
 public:
  int32 flags_;
  bool masks_;
  bool animated_;
  bool videos_;
  bool emojis_;
  bool text_color_;
  object_ptr<InputUser> user_id_;
  string title_;
  string short_name_;
  object_ptr<InputDocument> thumb_;
  array<object_ptr<inputStickerSetItem>> stickers_;
  string software_;
  enum Flags : std::int32_t { MASKS_MASK = 1, ANIMATED_MASK = 2, VIDEOS_MASK = 16, EMOJIS_MASK = 32, TEXT_COLOR_MASK = 64, THUMB_MASK = 4, SOFTWARE_MASK = 8 };
  mutable int32 var0;

  stickers_createStickerSet(int32 flags_, bool masks_, bool animated_, bool videos_, bool emojis_, bool text_color_, object_ptr<InputUser> &&user_id_, string const &title_, string const &short_name_, object_ptr<InputDocument> &&thumb_, array<object_ptr<inputStickerSetItem>> &&stickers_, string const &software_);

  static const std::int32_t ID = -1876841625;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_StickerSet>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class stickers_deleteStickerSet final : public Function {
 public:
  object_ptr<InputStickerSet> stickerset_;

  explicit stickers_deleteStickerSet(object_ptr<InputStickerSet> &&stickerset_);

  static const std::int32_t ID = -2022685804;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class stickers_removeStickerFromSet final : public Function {
 public:
  object_ptr<InputDocument> sticker_;

  explicit stickers_removeStickerFromSet(object_ptr<InputDocument> &&sticker_);

  static const std::int32_t ID = -143257775;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_StickerSet>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class stickers_renameStickerSet final : public Function {
 public:
  object_ptr<InputStickerSet> stickerset_;
  string title_;

  stickers_renameStickerSet(object_ptr<InputStickerSet> &&stickerset_, string const &title_);

  static const std::int32_t ID = 306912256;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_StickerSet>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class stickers_setStickerSetThumb final : public Function {
 public:
  int32 flags_;
  object_ptr<InputStickerSet> stickerset_;
  object_ptr<InputDocument> thumb_;
  int64 thumb_document_id_;
  enum Flags : std::int32_t { THUMB_MASK = 1, THUMB_DOCUMENT_ID_MASK = 2 };
  mutable int32 var0;

  stickers_setStickerSetThumb(int32 flags_, object_ptr<InputStickerSet> &&stickerset_, object_ptr<InputDocument> &&thumb_, int64 thumb_document_id_);

  static const std::int32_t ID = -1486204014;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<messages_StickerSet>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class stickers_suggestShortName final : public Function {
 public:
  string title_;

  explicit stickers_suggestShortName(string const &title_);

  static const std::int32_t ID = 1303364867;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<stickers_suggestedShortName>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class test_parseInputAppEvent final : public Function {
 public:

  static const std::int32_t ID = -1156741135;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<inputAppEvent>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class test_useConfigSimple final : public Function {
 public:

  static const std::int32_t ID = -105401795;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<help_configSimple>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class test_useError final : public Function {
 public:

  static const std::int32_t ID = -294277375;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<error>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class updates_getChannelDifference final : public Function {
 public:
  int32 flags_;
  bool force_;
  object_ptr<InputChannel> channel_;
  object_ptr<ChannelMessagesFilter> filter_;
  int32 pts_;
  int32 limit_;
  enum Flags : std::int32_t { FORCE_MASK = 1 };
  mutable int32 var0;

  updates_getChannelDifference(int32 flags_, bool force_, object_ptr<InputChannel> &&channel_, object_ptr<ChannelMessagesFilter> &&filter_, int32 pts_, int32 limit_);

  static const std::int32_t ID = 51854712;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<updates_ChannelDifference>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class updates_getDifference final : public Function {
 public:
  int32 flags_;
  int32 pts_;
  int32 pts_total_limit_;
  int32 date_;
  int32 qts_;
  enum Flags : std::int32_t { PTS_TOTAL_LIMIT_MASK = 1 };
  mutable int32 var0;

  updates_getDifference(int32 flags_, int32 pts_, int32 pts_total_limit_, int32 date_, int32 qts_);

  static const std::int32_t ID = 630429265;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<updates_Difference>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class updates_getState final : public Function {
 public:

  static const std::int32_t ID = -304838614;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<updates_state>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class upload_getCdnFile final : public Function {
 public:
  bytes file_token_;
  int64 offset_;
  int32 limit_;

  upload_getCdnFile(bytes &&file_token_, int64 offset_, int32 limit_);

  static const std::int32_t ID = 962554330;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<upload_CdnFile>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class upload_getCdnFileHashes final : public Function {
 public:
  bytes file_token_;
  int64 offset_;

  upload_getCdnFileHashes(bytes &&file_token_, int64 offset_);

  static const std::int32_t ID = -1847836879;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<fileHash>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class upload_getFile final : public Function {
 public:
  int32 flags_;
  bool precise_;
  bool cdn_supported_;
  object_ptr<InputFileLocation> location_;
  int64 offset_;
  int32 limit_;
  enum Flags : std::int32_t { PRECISE_MASK = 1, CDN_SUPPORTED_MASK = 2 };
  mutable int32 var0;

  upload_getFile(int32 flags_, bool precise_, bool cdn_supported_, object_ptr<InputFileLocation> &&location_, int64 offset_, int32 limit_);

  static const std::int32_t ID = -1101843010;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<upload_File>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class upload_getFileHashes final : public Function {
 public:
  object_ptr<InputFileLocation> location_;
  int64 offset_;

  upload_getFileHashes(object_ptr<InputFileLocation> &&location_, int64 offset_);

  static const std::int32_t ID = -1856595926;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<fileHash>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class upload_getWebFile final : public Function {
 public:
  object_ptr<InputWebFileLocation> location_;
  int32 offset_;
  int32 limit_;

  upload_getWebFile(object_ptr<InputWebFileLocation> &&location_, int32 offset_, int32 limit_);

  static const std::int32_t ID = 619086221;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<upload_webFile>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class upload_reuploadCdnFile final : public Function {
 public:
  bytes file_token_;
  bytes request_token_;

  upload_reuploadCdnFile(bytes &&file_token_, bytes &&request_token_);

  static const std::int32_t ID = -1691921240;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<fileHash>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class upload_saveBigFilePart final : public Function {
 public:
  int64 file_id_;
  int32 file_part_;
  int32 file_total_parts_;
  bytes bytes_;

  upload_saveBigFilePart(int64 file_id_, int32 file_part_, int32 file_total_parts_, bytes &&bytes_);

  static const std::int32_t ID = -562337987;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class upload_saveFilePart final : public Function {
 public:
  int64 file_id_;
  int32 file_part_;
  bytes bytes_;

  upload_saveFilePart(int64 file_id_, int32 file_part_, bytes &&bytes_);

  static const std::int32_t ID = -1291540959;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class users_getFullUser final : public Function {
 public:
  object_ptr<InputUser> id_;

  explicit users_getFullUser(object_ptr<InputUser> &&id_);

  static const std::int32_t ID = -1240508136;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = object_ptr<users_userFull>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class users_getUsers final : public Function {
 public:
  array<object_ptr<InputUser>> id_;

  explicit users_getUsers(array<object_ptr<InputUser>> &&id_);

  static const std::int32_t ID = 227648840;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = array<object_ptr<User>>;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

class users_setSecureValueErrors final : public Function {
 public:
  object_ptr<InputUser> id_;
  array<object_ptr<SecureValueError>> errors_;

  users_setSecureValueErrors(object_ptr<InputUser> &&id_, array<object_ptr<SecureValueError>> &&errors_);

  static const std::int32_t ID = -1865902923;
  std::int32_t get_id() const final {
    return ID;
  }

  using ReturnType = bool;

  void store(TlStorerCalcLength &s) const final;

  void store(TlStorerUnsafe &s) const final;

  void store(TlStorerToString &s, const char *field_name) const final;

  static ReturnType fetch_result(TlBufferParser &p);
};

}  // namespace telegram_api
}  // namespace td
