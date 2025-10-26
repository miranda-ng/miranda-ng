#include "td_api.h"

#include "td/utils/common.h"
#include "td/utils/format.h"
#include "td/utils/logging.h"
#include "td/utils/SliceBuilder.h"
#include "td/utils/tl_parsers.h"
#include "td/utils/tl_storers.h"
#include "td/utils/TlStorerToString.h"

namespace td {
namespace td_api {


accentColor::accentColor()
  : id_()
  , built_in_accent_color_id_()
  , light_theme_colors_()
  , dark_theme_colors_()
  , min_channel_chat_boost_level_()
{}

accentColor::accentColor(int32 id_, int32 built_in_accent_color_id_, array<int32> &&light_theme_colors_, array<int32> &&dark_theme_colors_, int32 min_channel_chat_boost_level_)
  : id_(id_)
  , built_in_accent_color_id_(built_in_accent_color_id_)
  , light_theme_colors_(std::move(light_theme_colors_))
  , dark_theme_colors_(std::move(dark_theme_colors_))
  , min_channel_chat_boost_level_(min_channel_chat_boost_level_)
{}

const std::int32_t accentColor::ID;

void accentColor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "accentColor");
    s.store_field("id", id_);
    s.store_field("built_in_accent_color_id", built_in_accent_color_id_);
    { s.store_vector_begin("light_theme_colors", light_theme_colors_.size()); for (const auto &_value : light_theme_colors_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("dark_theme_colors", dark_theme_colors_.size()); for (const auto &_value : dark_theme_colors_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("min_channel_chat_boost_level", min_channel_chat_boost_level_);
    s.store_class_end();
  }
}

affiliateInfo::affiliateInfo()
  : commission_per_mille_()
  , affiliate_chat_id_()
  , star_amount_()
{}

affiliateInfo::affiliateInfo(int32 commission_per_mille_, int53 affiliate_chat_id_, object_ptr<starAmount> &&star_amount_)
  : commission_per_mille_(commission_per_mille_)
  , affiliate_chat_id_(affiliate_chat_id_)
  , star_amount_(std::move(star_amount_))
{}

const std::int32_t affiliateInfo::ID;

void affiliateInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "affiliateInfo");
    s.store_field("commission_per_mille", commission_per_mille_);
    s.store_field("affiliate_chat_id", affiliate_chat_id_);
    s.store_object_field("star_amount", static_cast<const BaseObject *>(star_amount_.get()));
    s.store_class_end();
  }
}

affiliateTypeCurrentUser::affiliateTypeCurrentUser() {
}

const std::int32_t affiliateTypeCurrentUser::ID;

void affiliateTypeCurrentUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "affiliateTypeCurrentUser");
    s.store_class_end();
  }
}

affiliateTypeBot::affiliateTypeBot()
  : user_id_()
{}

affiliateTypeBot::affiliateTypeBot(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t affiliateTypeBot::ID;

void affiliateTypeBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "affiliateTypeBot");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

affiliateTypeChannel::affiliateTypeChannel()
  : chat_id_()
{}

affiliateTypeChannel::affiliateTypeChannel(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t affiliateTypeChannel::ID;

void affiliateTypeChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "affiliateTypeChannel");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

animatedEmoji::animatedEmoji()
  : sticker_()
  , sticker_width_()
  , sticker_height_()
  , fitzpatrick_type_()
  , sound_()
{}

animatedEmoji::animatedEmoji(object_ptr<sticker> &&sticker_, int32 sticker_width_, int32 sticker_height_, int32 fitzpatrick_type_, object_ptr<file> &&sound_)
  : sticker_(std::move(sticker_))
  , sticker_width_(sticker_width_)
  , sticker_height_(sticker_height_)
  , fitzpatrick_type_(fitzpatrick_type_)
  , sound_(std::move(sound_))
{}

const std::int32_t animatedEmoji::ID;

void animatedEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "animatedEmoji");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_field("sticker_width", sticker_width_);
    s.store_field("sticker_height", sticker_height_);
    s.store_field("fitzpatrick_type", fitzpatrick_type_);
    s.store_object_field("sound", static_cast<const BaseObject *>(sound_.get()));
    s.store_class_end();
  }
}

autosaveSettings::autosaveSettings()
  : private_chat_settings_()
  , group_settings_()
  , channel_settings_()
  , exceptions_()
{}

autosaveSettings::autosaveSettings(object_ptr<scopeAutosaveSettings> &&private_chat_settings_, object_ptr<scopeAutosaveSettings> &&group_settings_, object_ptr<scopeAutosaveSettings> &&channel_settings_, array<object_ptr<autosaveSettingsException>> &&exceptions_)
  : private_chat_settings_(std::move(private_chat_settings_))
  , group_settings_(std::move(group_settings_))
  , channel_settings_(std::move(channel_settings_))
  , exceptions_(std::move(exceptions_))
{}

const std::int32_t autosaveSettings::ID;

void autosaveSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "autosaveSettings");
    s.store_object_field("private_chat_settings", static_cast<const BaseObject *>(private_chat_settings_.get()));
    s.store_object_field("group_settings", static_cast<const BaseObject *>(group_settings_.get()));
    s.store_object_field("channel_settings", static_cast<const BaseObject *>(channel_settings_.get()));
    { s.store_vector_begin("exceptions", exceptions_.size()); for (const auto &_value : exceptions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

autosaveSettingsScopePrivateChats::autosaveSettingsScopePrivateChats() {
}

const std::int32_t autosaveSettingsScopePrivateChats::ID;

void autosaveSettingsScopePrivateChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "autosaveSettingsScopePrivateChats");
    s.store_class_end();
  }
}

autosaveSettingsScopeGroupChats::autosaveSettingsScopeGroupChats() {
}

const std::int32_t autosaveSettingsScopeGroupChats::ID;

void autosaveSettingsScopeGroupChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "autosaveSettingsScopeGroupChats");
    s.store_class_end();
  }
}

autosaveSettingsScopeChannelChats::autosaveSettingsScopeChannelChats() {
}

const std::int32_t autosaveSettingsScopeChannelChats::ID;

void autosaveSettingsScopeChannelChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "autosaveSettingsScopeChannelChats");
    s.store_class_end();
  }
}

autosaveSettingsScopeChat::autosaveSettingsScopeChat()
  : chat_id_()
{}

autosaveSettingsScopeChat::autosaveSettingsScopeChat(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t autosaveSettingsScopeChat::ID;

void autosaveSettingsScopeChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "autosaveSettingsScopeChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

backgrounds::backgrounds()
  : backgrounds_()
{}

backgrounds::backgrounds(array<object_ptr<background>> &&backgrounds_)
  : backgrounds_(std::move(backgrounds_))
{}

const std::int32_t backgrounds::ID;

void backgrounds::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "backgrounds");
    { s.store_vector_begin("backgrounds", backgrounds_.size()); for (const auto &_value : backgrounds_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

botMediaPreviewInfo::botMediaPreviewInfo()
  : previews_()
  , language_codes_()
{}

botMediaPreviewInfo::botMediaPreviewInfo(array<object_ptr<botMediaPreview>> &&previews_, array<string> &&language_codes_)
  : previews_(std::move(previews_))
  , language_codes_(std::move(language_codes_))
{}

const std::int32_t botMediaPreviewInfo::ID;

void botMediaPreviewInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botMediaPreviewInfo");
    { s.store_vector_begin("previews", previews_.size()); for (const auto &_value : previews_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("language_codes", language_codes_.size()); for (const auto &_value : language_codes_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

businessChatLinks::businessChatLinks()
  : links_()
{}

businessChatLinks::businessChatLinks(array<object_ptr<businessChatLink>> &&links_)
  : links_(std::move(links_))
{}

const std::int32_t businessChatLinks::ID;

void businessChatLinks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessChatLinks");
    { s.store_vector_begin("links", links_.size()); for (const auto &_value : links_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

businessFeatures::businessFeatures()
  : features_()
{}

businessFeatures::businessFeatures(array<object_ptr<BusinessFeature>> &&features_)
  : features_(std::move(features_))
{}

const std::int32_t businessFeatures::ID;

void businessFeatures::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessFeatures");
    { s.store_vector_begin("features", features_.size()); for (const auto &_value : features_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

businessStartPage::businessStartPage()
  : title_()
  , message_()
  , sticker_()
{}

businessStartPage::businessStartPage(string const &title_, string const &message_, object_ptr<sticker> &&sticker_)
  : title_(title_)
  , message_(message_)
  , sticker_(std::move(sticker_))
{}

const std::int32_t businessStartPage::ID;

void businessStartPage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessStartPage");
    s.store_field("title", title_);
    s.store_field("message", message_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

callServerTypeTelegramReflector::callServerTypeTelegramReflector()
  : peer_tag_()
  , is_tcp_()
{}

callServerTypeTelegramReflector::callServerTypeTelegramReflector(bytes const &peer_tag_, bool is_tcp_)
  : peer_tag_(std::move(peer_tag_))
  , is_tcp_(is_tcp_)
{}

const std::int32_t callServerTypeTelegramReflector::ID;

void callServerTypeTelegramReflector::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callServerTypeTelegramReflector");
    s.store_bytes_field("peer_tag", peer_tag_);
    s.store_field("is_tcp", is_tcp_);
    s.store_class_end();
  }
}

callServerTypeWebrtc::callServerTypeWebrtc()
  : username_()
  , password_()
  , supports_turn_()
  , supports_stun_()
{}

callServerTypeWebrtc::callServerTypeWebrtc(string const &username_, string const &password_, bool supports_turn_, bool supports_stun_)
  : username_(username_)
  , password_(password_)
  , supports_turn_(supports_turn_)
  , supports_stun_(supports_stun_)
{}

const std::int32_t callServerTypeWebrtc::ID;

void callServerTypeWebrtc::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callServerTypeWebrtc");
    s.store_field("username", username_);
    s.store_field("password", password_);
    s.store_field("supports_turn", supports_turn_);
    s.store_field("supports_stun", supports_stun_);
    s.store_class_end();
  }
}

canSendGiftResultOk::canSendGiftResultOk() {
}

const std::int32_t canSendGiftResultOk::ID;

void canSendGiftResultOk::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canSendGiftResultOk");
    s.store_class_end();
  }
}

canSendGiftResultFail::canSendGiftResultFail()
  : reason_()
{}

canSendGiftResultFail::canSendGiftResultFail(object_ptr<formattedText> &&reason_)
  : reason_(std::move(reason_))
{}

const std::int32_t canSendGiftResultFail::ID;

void canSendGiftResultFail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canSendGiftResultFail");
    s.store_object_field("reason", static_cast<const BaseObject *>(reason_.get()));
    s.store_class_end();
  }
}

chatAdministrator::chatAdministrator()
  : user_id_()
  , custom_title_()
  , is_owner_()
{}

chatAdministrator::chatAdministrator(int53 user_id_, string const &custom_title_, bool is_owner_)
  : user_id_(user_id_)
  , custom_title_(custom_title_)
  , is_owner_(is_owner_)
{}

const std::int32_t chatAdministrator::ID;

void chatAdministrator::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatAdministrator");
    s.store_field("user_id", user_id_);
    s.store_field("custom_title", custom_title_);
    s.store_field("is_owner", is_owner_);
    s.store_class_end();
  }
}

chatBoostLinkInfo::chatBoostLinkInfo()
  : is_public_()
  , chat_id_()
{}

chatBoostLinkInfo::chatBoostLinkInfo(bool is_public_, int53 chat_id_)
  : is_public_(is_public_)
  , chat_id_(chat_id_)
{}

const std::int32_t chatBoostLinkInfo::ID;

void chatBoostLinkInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatBoostLinkInfo");
    s.store_field("is_public", is_public_);
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

chatFolderInviteLinkInfo::chatFolderInviteLinkInfo()
  : chat_folder_info_()
  , missing_chat_ids_()
  , added_chat_ids_()
{}

chatFolderInviteLinkInfo::chatFolderInviteLinkInfo(object_ptr<chatFolderInfo> &&chat_folder_info_, array<int53> &&missing_chat_ids_, array<int53> &&added_chat_ids_)
  : chat_folder_info_(std::move(chat_folder_info_))
  , missing_chat_ids_(std::move(missing_chat_ids_))
  , added_chat_ids_(std::move(added_chat_ids_))
{}

const std::int32_t chatFolderInviteLinkInfo::ID;

void chatFolderInviteLinkInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatFolderInviteLinkInfo");
    s.store_object_field("chat_folder_info", static_cast<const BaseObject *>(chat_folder_info_.get()));
    { s.store_vector_begin("missing_chat_ids", missing_chat_ids_.size()); for (const auto &_value : missing_chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("added_chat_ids", added_chat_ids_.size()); for (const auto &_value : added_chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatMembers::chatMembers()
  : total_count_()
  , members_()
{}

chatMembers::chatMembers(int32 total_count_, array<object_ptr<chatMember>> &&members_)
  : total_count_(total_count_)
  , members_(std::move(members_))
{}

const std::int32_t chatMembers::ID;

void chatMembers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatMembers");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("members", members_.size()); for (const auto &_value : members_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatPhotoSticker::chatPhotoSticker()
  : type_()
  , background_fill_()
{}

chatPhotoSticker::chatPhotoSticker(object_ptr<ChatPhotoStickerType> &&type_, object_ptr<BackgroundFill> &&background_fill_)
  : type_(std::move(type_))
  , background_fill_(std::move(background_fill_))
{}

const std::int32_t chatPhotoSticker::ID;

void chatPhotoSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatPhotoSticker");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_object_field("background_fill", static_cast<const BaseObject *>(background_fill_.get()));
    s.store_class_end();
  }
}

chatPhotos::chatPhotos()
  : total_count_()
  , photos_()
{}

chatPhotos::chatPhotos(int32 total_count_, array<object_ptr<chatPhoto>> &&photos_)
  : total_count_(total_count_)
  , photos_(std::move(photos_))
{}

const std::int32_t chatPhotos::ID;

void chatPhotos::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatPhotos");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("photos", photos_.size()); for (const auto &_value : photos_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatStatisticsMessageSenderInfo::chatStatisticsMessageSenderInfo()
  : user_id_()
  , sent_message_count_()
  , average_character_count_()
{}

chatStatisticsMessageSenderInfo::chatStatisticsMessageSenderInfo(int53 user_id_, int32 sent_message_count_, int32 average_character_count_)
  : user_id_(user_id_)
  , sent_message_count_(sent_message_count_)
  , average_character_count_(average_character_count_)
{}

const std::int32_t chatStatisticsMessageSenderInfo::ID;

void chatStatisticsMessageSenderInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatStatisticsMessageSenderInfo");
    s.store_field("user_id", user_id_);
    s.store_field("sent_message_count", sent_message_count_);
    s.store_field("average_character_count", average_character_count_);
    s.store_class_end();
  }
}

chats::chats()
  : total_count_()
  , chat_ids_()
{}

chats::chats(int32 total_count_, array<int53> &&chat_ids_)
  : total_count_(total_count_)
  , chat_ids_(std::move(chat_ids_))
{}

const std::int32_t chats::ID;

void chats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chats");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("chat_ids", chat_ids_.size()); for (const auto &_value : chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

checkChatUsernameResultOk::checkChatUsernameResultOk() {
}

const std::int32_t checkChatUsernameResultOk::ID;

void checkChatUsernameResultOk::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkChatUsernameResultOk");
    s.store_class_end();
  }
}

checkChatUsernameResultUsernameInvalid::checkChatUsernameResultUsernameInvalid() {
}

const std::int32_t checkChatUsernameResultUsernameInvalid::ID;

void checkChatUsernameResultUsernameInvalid::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkChatUsernameResultUsernameInvalid");
    s.store_class_end();
  }
}

checkChatUsernameResultUsernameOccupied::checkChatUsernameResultUsernameOccupied() {
}

const std::int32_t checkChatUsernameResultUsernameOccupied::ID;

void checkChatUsernameResultUsernameOccupied::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkChatUsernameResultUsernameOccupied");
    s.store_class_end();
  }
}

checkChatUsernameResultUsernamePurchasable::checkChatUsernameResultUsernamePurchasable() {
}

const std::int32_t checkChatUsernameResultUsernamePurchasable::ID;

void checkChatUsernameResultUsernamePurchasable::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkChatUsernameResultUsernamePurchasable");
    s.store_class_end();
  }
}

checkChatUsernameResultPublicChatsTooMany::checkChatUsernameResultPublicChatsTooMany() {
}

const std::int32_t checkChatUsernameResultPublicChatsTooMany::ID;

void checkChatUsernameResultPublicChatsTooMany::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkChatUsernameResultPublicChatsTooMany");
    s.store_class_end();
  }
}

checkChatUsernameResultPublicGroupsUnavailable::checkChatUsernameResultPublicGroupsUnavailable() {
}

const std::int32_t checkChatUsernameResultPublicGroupsUnavailable::ID;

void checkChatUsernameResultPublicGroupsUnavailable::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkChatUsernameResultPublicGroupsUnavailable");
    s.store_class_end();
  }
}

connectedWebsites::connectedWebsites()
  : websites_()
{}

connectedWebsites::connectedWebsites(array<object_ptr<connectedWebsite>> &&websites_)
  : websites_(std::move(websites_))
{}

const std::int32_t connectedWebsites::ID;

void connectedWebsites::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "connectedWebsites");
    { s.store_vector_begin("websites", websites_.size()); for (const auto &_value : websites_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

connectionStateWaitingForNetwork::connectionStateWaitingForNetwork() {
}

const std::int32_t connectionStateWaitingForNetwork::ID;

void connectionStateWaitingForNetwork::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "connectionStateWaitingForNetwork");
    s.store_class_end();
  }
}

connectionStateConnectingToProxy::connectionStateConnectingToProxy() {
}

const std::int32_t connectionStateConnectingToProxy::ID;

void connectionStateConnectingToProxy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "connectionStateConnectingToProxy");
    s.store_class_end();
  }
}

connectionStateConnecting::connectionStateConnecting() {
}

const std::int32_t connectionStateConnecting::ID;

void connectionStateConnecting::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "connectionStateConnecting");
    s.store_class_end();
  }
}

connectionStateUpdating::connectionStateUpdating() {
}

const std::int32_t connectionStateUpdating::ID;

void connectionStateUpdating::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "connectionStateUpdating");
    s.store_class_end();
  }
}

connectionStateReady::connectionStateReady() {
}

const std::int32_t connectionStateReady::ID;

void connectionStateReady::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "connectionStateReady");
    s.store_class_end();
  }
}

customRequestResult::customRequestResult()
  : result_()
{}

customRequestResult::customRequestResult(string const &result_)
  : result_(result_)
{}

const std::int32_t customRequestResult::ID;

void customRequestResult::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "customRequestResult");
    s.store_field("result", result_);
    s.store_class_end();
  }
}

document::document()
  : file_name_()
  , mime_type_()
  , minithumbnail_()
  , thumbnail_()
  , document_()
{}

document::document(string const &file_name_, string const &mime_type_, object_ptr<minithumbnail> &&minithumbnail_, object_ptr<thumbnail> &&thumbnail_, object_ptr<file> &&document_)
  : file_name_(file_name_)
  , mime_type_(mime_type_)
  , minithumbnail_(std::move(minithumbnail_))
  , thumbnail_(std::move(thumbnail_))
  , document_(std::move(document_))
{}

const std::int32_t document::ID;

void document::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "document");
    s.store_field("file_name", file_name_);
    s.store_field("mime_type", mime_type_);
    s.store_object_field("minithumbnail", static_cast<const BaseObject *>(minithumbnail_.get()));
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_object_field("document", static_cast<const BaseObject *>(document_.get()));
    s.store_class_end();
  }
}

giftForResale::giftForResale()
  : gift_()
  , received_gift_id_()
{}

giftForResale::giftForResale(object_ptr<upgradedGift> &&gift_, string const &received_gift_id_)
  : gift_(std::move(gift_))
  , received_gift_id_(received_gift_id_)
{}

const std::int32_t giftForResale::ID;

void giftForResale::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giftForResale");
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_field("received_gift_id", received_gift_id_);
    s.store_class_end();
  }
}

giftPurchaseLimits::giftPurchaseLimits()
  : total_count_()
  , remaining_count_()
{}

giftPurchaseLimits::giftPurchaseLimits(int32 total_count_, int32 remaining_count_)
  : total_count_(total_count_)
  , remaining_count_(remaining_count_)
{}

const std::int32_t giftPurchaseLimits::ID;

void giftPurchaseLimits::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giftPurchaseLimits");
    s.store_field("total_count", total_count_);
    s.store_field("remaining_count", remaining_count_);
    s.store_class_end();
  }
}

giftResalePriceStar::giftResalePriceStar()
  : star_count_()
{}

giftResalePriceStar::giftResalePriceStar(int53 star_count_)
  : star_count_(star_count_)
{}

const std::int32_t giftResalePriceStar::ID;

void giftResalePriceStar::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giftResalePriceStar");
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

giftResalePriceTon::giftResalePriceTon()
  : toncoin_cent_count_()
{}

giftResalePriceTon::giftResalePriceTon(int53 toncoin_cent_count_)
  : toncoin_cent_count_(toncoin_cent_count_)
{}

const std::int32_t giftResalePriceTon::ID;

void giftResalePriceTon::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giftResalePriceTon");
    s.store_field("toncoin_cent_count", toncoin_cent_count_);
    s.store_class_end();
  }
}

giveawayInfoOngoing::giveawayInfoOngoing()
  : creation_date_()
  , status_()
  , is_ended_()
{}

giveawayInfoOngoing::giveawayInfoOngoing(int32 creation_date_, object_ptr<GiveawayParticipantStatus> &&status_, bool is_ended_)
  : creation_date_(creation_date_)
  , status_(std::move(status_))
  , is_ended_(is_ended_)
{}

const std::int32_t giveawayInfoOngoing::ID;

void giveawayInfoOngoing::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giveawayInfoOngoing");
    s.store_field("creation_date", creation_date_);
    s.store_object_field("status", static_cast<const BaseObject *>(status_.get()));
    s.store_field("is_ended", is_ended_);
    s.store_class_end();
  }
}

giveawayInfoCompleted::giveawayInfoCompleted()
  : creation_date_()
  , actual_winners_selection_date_()
  , was_refunded_()
  , is_winner_()
  , winner_count_()
  , activation_count_()
  , gift_code_()
  , won_star_count_()
{}

giveawayInfoCompleted::giveawayInfoCompleted(int32 creation_date_, int32 actual_winners_selection_date_, bool was_refunded_, bool is_winner_, int32 winner_count_, int32 activation_count_, string const &gift_code_, int53 won_star_count_)
  : creation_date_(creation_date_)
  , actual_winners_selection_date_(actual_winners_selection_date_)
  , was_refunded_(was_refunded_)
  , is_winner_(is_winner_)
  , winner_count_(winner_count_)
  , activation_count_(activation_count_)
  , gift_code_(gift_code_)
  , won_star_count_(won_star_count_)
{}

const std::int32_t giveawayInfoCompleted::ID;

void giveawayInfoCompleted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giveawayInfoCompleted");
    s.store_field("creation_date", creation_date_);
    s.store_field("actual_winners_selection_date", actual_winners_selection_date_);
    s.store_field("was_refunded", was_refunded_);
    s.store_field("is_winner", is_winner_);
    s.store_field("winner_count", winner_count_);
    s.store_field("activation_count", activation_count_);
    s.store_field("gift_code", gift_code_);
    s.store_field("won_star_count", won_star_count_);
    s.store_class_end();
  }
}

groupCall::groupCall()
  : id_()
  , title_()
  , invite_link_()
  , scheduled_start_date_()
  , enabled_start_notification_()
  , is_active_()
  , is_video_chat_()
  , is_rtmp_stream_()
  , is_joined_()
  , need_rejoin_()
  , is_owned_()
  , can_be_managed_()
  , participant_count_()
  , has_hidden_listeners_()
  , loaded_all_participants_()
  , recent_speakers_()
  , is_my_video_enabled_()
  , is_my_video_paused_()
  , can_enable_video_()
  , mute_new_participants_()
  , can_toggle_mute_new_participants_()
  , can_send_messages_()
  , can_toggle_can_send_messages_()
  , record_duration_()
  , is_video_recorded_()
  , duration_()
{}

groupCall::groupCall(int32 id_, string const &title_, string const &invite_link_, int32 scheduled_start_date_, bool enabled_start_notification_, bool is_active_, bool is_video_chat_, bool is_rtmp_stream_, bool is_joined_, bool need_rejoin_, bool is_owned_, bool can_be_managed_, int32 participant_count_, bool has_hidden_listeners_, bool loaded_all_participants_, array<object_ptr<groupCallRecentSpeaker>> &&recent_speakers_, bool is_my_video_enabled_, bool is_my_video_paused_, bool can_enable_video_, bool mute_new_participants_, bool can_toggle_mute_new_participants_, bool can_send_messages_, bool can_toggle_can_send_messages_, int32 record_duration_, bool is_video_recorded_, int32 duration_)
  : id_(id_)
  , title_(title_)
  , invite_link_(invite_link_)
  , scheduled_start_date_(scheduled_start_date_)
  , enabled_start_notification_(enabled_start_notification_)
  , is_active_(is_active_)
  , is_video_chat_(is_video_chat_)
  , is_rtmp_stream_(is_rtmp_stream_)
  , is_joined_(is_joined_)
  , need_rejoin_(need_rejoin_)
  , is_owned_(is_owned_)
  , can_be_managed_(can_be_managed_)
  , participant_count_(participant_count_)
  , has_hidden_listeners_(has_hidden_listeners_)
  , loaded_all_participants_(loaded_all_participants_)
  , recent_speakers_(std::move(recent_speakers_))
  , is_my_video_enabled_(is_my_video_enabled_)
  , is_my_video_paused_(is_my_video_paused_)
  , can_enable_video_(can_enable_video_)
  , mute_new_participants_(mute_new_participants_)
  , can_toggle_mute_new_participants_(can_toggle_mute_new_participants_)
  , can_send_messages_(can_send_messages_)
  , can_toggle_can_send_messages_(can_toggle_can_send_messages_)
  , record_duration_(record_duration_)
  , is_video_recorded_(is_video_recorded_)
  , duration_(duration_)
{}

const std::int32_t groupCall::ID;

void groupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCall");
    s.store_field("id", id_);
    s.store_field("title", title_);
    s.store_field("invite_link", invite_link_);
    s.store_field("scheduled_start_date", scheduled_start_date_);
    s.store_field("enabled_start_notification", enabled_start_notification_);
    s.store_field("is_active", is_active_);
    s.store_field("is_video_chat", is_video_chat_);
    s.store_field("is_rtmp_stream", is_rtmp_stream_);
    s.store_field("is_joined", is_joined_);
    s.store_field("need_rejoin", need_rejoin_);
    s.store_field("is_owned", is_owned_);
    s.store_field("can_be_managed", can_be_managed_);
    s.store_field("participant_count", participant_count_);
    s.store_field("has_hidden_listeners", has_hidden_listeners_);
    s.store_field("loaded_all_participants", loaded_all_participants_);
    { s.store_vector_begin("recent_speakers", recent_speakers_.size()); for (const auto &_value : recent_speakers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("is_my_video_enabled", is_my_video_enabled_);
    s.store_field("is_my_video_paused", is_my_video_paused_);
    s.store_field("can_enable_video", can_enable_video_);
    s.store_field("mute_new_participants", mute_new_participants_);
    s.store_field("can_toggle_mute_new_participants", can_toggle_mute_new_participants_);
    s.store_field("can_send_messages", can_send_messages_);
    s.store_field("can_toggle_can_send_messages", can_toggle_can_send_messages_);
    s.store_field("record_duration", record_duration_);
    s.store_field("is_video_recorded", is_video_recorded_);
    s.store_field("duration", duration_);
    s.store_class_end();
  }
}

inputChecklistTask::inputChecklistTask()
  : id_()
  , text_()
{}

inputChecklistTask::inputChecklistTask(int32 id_, object_ptr<formattedText> &&text_)
  : id_(id_)
  , text_(std::move(text_))
{}

const std::int32_t inputChecklistTask::ID;

void inputChecklistTask::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputChecklistTask");
    s.store_field("id", id_);
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

inputPassportElementErrorSourceUnspecified::inputPassportElementErrorSourceUnspecified()
  : element_hash_()
{}

inputPassportElementErrorSourceUnspecified::inputPassportElementErrorSourceUnspecified(bytes const &element_hash_)
  : element_hash_(std::move(element_hash_))
{}

const std::int32_t inputPassportElementErrorSourceUnspecified::ID;

void inputPassportElementErrorSourceUnspecified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementErrorSourceUnspecified");
    s.store_bytes_field("element_hash", element_hash_);
    s.store_class_end();
  }
}

inputPassportElementErrorSourceDataField::inputPassportElementErrorSourceDataField()
  : field_name_()
  , data_hash_()
{}

inputPassportElementErrorSourceDataField::inputPassportElementErrorSourceDataField(string const &field_name_, bytes const &data_hash_)
  : field_name_(field_name_)
  , data_hash_(std::move(data_hash_))
{}

const std::int32_t inputPassportElementErrorSourceDataField::ID;

void inputPassportElementErrorSourceDataField::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementErrorSourceDataField");
    s.store_field("field_name", field_name_);
    s.store_bytes_field("data_hash", data_hash_);
    s.store_class_end();
  }
}

inputPassportElementErrorSourceFrontSide::inputPassportElementErrorSourceFrontSide()
  : file_hash_()
{}

inputPassportElementErrorSourceFrontSide::inputPassportElementErrorSourceFrontSide(bytes const &file_hash_)
  : file_hash_(std::move(file_hash_))
{}

const std::int32_t inputPassportElementErrorSourceFrontSide::ID;

void inputPassportElementErrorSourceFrontSide::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementErrorSourceFrontSide");
    s.store_bytes_field("file_hash", file_hash_);
    s.store_class_end();
  }
}

inputPassportElementErrorSourceReverseSide::inputPassportElementErrorSourceReverseSide()
  : file_hash_()
{}

inputPassportElementErrorSourceReverseSide::inputPassportElementErrorSourceReverseSide(bytes const &file_hash_)
  : file_hash_(std::move(file_hash_))
{}

const std::int32_t inputPassportElementErrorSourceReverseSide::ID;

void inputPassportElementErrorSourceReverseSide::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementErrorSourceReverseSide");
    s.store_bytes_field("file_hash", file_hash_);
    s.store_class_end();
  }
}

inputPassportElementErrorSourceSelfie::inputPassportElementErrorSourceSelfie()
  : file_hash_()
{}

inputPassportElementErrorSourceSelfie::inputPassportElementErrorSourceSelfie(bytes const &file_hash_)
  : file_hash_(std::move(file_hash_))
{}

const std::int32_t inputPassportElementErrorSourceSelfie::ID;

void inputPassportElementErrorSourceSelfie::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementErrorSourceSelfie");
    s.store_bytes_field("file_hash", file_hash_);
    s.store_class_end();
  }
}

inputPassportElementErrorSourceTranslationFile::inputPassportElementErrorSourceTranslationFile()
  : file_hash_()
{}

inputPassportElementErrorSourceTranslationFile::inputPassportElementErrorSourceTranslationFile(bytes const &file_hash_)
  : file_hash_(std::move(file_hash_))
{}

const std::int32_t inputPassportElementErrorSourceTranslationFile::ID;

void inputPassportElementErrorSourceTranslationFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementErrorSourceTranslationFile");
    s.store_bytes_field("file_hash", file_hash_);
    s.store_class_end();
  }
}

inputPassportElementErrorSourceTranslationFiles::inputPassportElementErrorSourceTranslationFiles()
  : file_hashes_()
{}

inputPassportElementErrorSourceTranslationFiles::inputPassportElementErrorSourceTranslationFiles(array<bytes> &&file_hashes_)
  : file_hashes_(std::move(file_hashes_))
{}

const std::int32_t inputPassportElementErrorSourceTranslationFiles::ID;

void inputPassportElementErrorSourceTranslationFiles::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementErrorSourceTranslationFiles");
    { s.store_vector_begin("file_hashes", file_hashes_.size()); for (const auto &_value : file_hashes_) { s.store_bytes_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

inputPassportElementErrorSourceFile::inputPassportElementErrorSourceFile()
  : file_hash_()
{}

inputPassportElementErrorSourceFile::inputPassportElementErrorSourceFile(bytes const &file_hash_)
  : file_hash_(std::move(file_hash_))
{}

const std::int32_t inputPassportElementErrorSourceFile::ID;

void inputPassportElementErrorSourceFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementErrorSourceFile");
    s.store_bytes_field("file_hash", file_hash_);
    s.store_class_end();
  }
}

inputPassportElementErrorSourceFiles::inputPassportElementErrorSourceFiles()
  : file_hashes_()
{}

inputPassportElementErrorSourceFiles::inputPassportElementErrorSourceFiles(array<bytes> &&file_hashes_)
  : file_hashes_(std::move(file_hashes_))
{}

const std::int32_t inputPassportElementErrorSourceFiles::ID;

void inputPassportElementErrorSourceFiles::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementErrorSourceFiles");
    { s.store_vector_begin("file_hashes", file_hashes_.size()); for (const auto &_value : file_hashes_) { s.store_bytes_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

inputTextQuote::inputTextQuote()
  : text_()
  , position_()
{}

inputTextQuote::inputTextQuote(object_ptr<formattedText> &&text_, int32 position_)
  : text_(std::move(text_))
  , position_(position_)
{}

const std::int32_t inputTextQuote::ID;

void inputTextQuote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputTextQuote");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("position", position_);
    s.store_class_end();
  }
}

keyboardButtonTypeText::keyboardButtonTypeText() {
}

const std::int32_t keyboardButtonTypeText::ID;

void keyboardButtonTypeText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonTypeText");
    s.store_class_end();
  }
}

keyboardButtonTypeRequestPhoneNumber::keyboardButtonTypeRequestPhoneNumber() {
}

const std::int32_t keyboardButtonTypeRequestPhoneNumber::ID;

void keyboardButtonTypeRequestPhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonTypeRequestPhoneNumber");
    s.store_class_end();
  }
}

keyboardButtonTypeRequestLocation::keyboardButtonTypeRequestLocation() {
}

const std::int32_t keyboardButtonTypeRequestLocation::ID;

void keyboardButtonTypeRequestLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonTypeRequestLocation");
    s.store_class_end();
  }
}

keyboardButtonTypeRequestPoll::keyboardButtonTypeRequestPoll()
  : force_regular_()
  , force_quiz_()
{}

keyboardButtonTypeRequestPoll::keyboardButtonTypeRequestPoll(bool force_regular_, bool force_quiz_)
  : force_regular_(force_regular_)
  , force_quiz_(force_quiz_)
{}

const std::int32_t keyboardButtonTypeRequestPoll::ID;

void keyboardButtonTypeRequestPoll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonTypeRequestPoll");
    s.store_field("force_regular", force_regular_);
    s.store_field("force_quiz", force_quiz_);
    s.store_class_end();
  }
}

keyboardButtonTypeRequestUsers::keyboardButtonTypeRequestUsers()
  : id_()
  , restrict_user_is_bot_()
  , user_is_bot_()
  , restrict_user_is_premium_()
  , user_is_premium_()
  , max_quantity_()
  , request_name_()
  , request_username_()
  , request_photo_()
{}

keyboardButtonTypeRequestUsers::keyboardButtonTypeRequestUsers(int32 id_, bool restrict_user_is_bot_, bool user_is_bot_, bool restrict_user_is_premium_, bool user_is_premium_, int32 max_quantity_, bool request_name_, bool request_username_, bool request_photo_)
  : id_(id_)
  , restrict_user_is_bot_(restrict_user_is_bot_)
  , user_is_bot_(user_is_bot_)
  , restrict_user_is_premium_(restrict_user_is_premium_)
  , user_is_premium_(user_is_premium_)
  , max_quantity_(max_quantity_)
  , request_name_(request_name_)
  , request_username_(request_username_)
  , request_photo_(request_photo_)
{}

const std::int32_t keyboardButtonTypeRequestUsers::ID;

void keyboardButtonTypeRequestUsers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonTypeRequestUsers");
    s.store_field("id", id_);
    s.store_field("restrict_user_is_bot", restrict_user_is_bot_);
    s.store_field("user_is_bot", user_is_bot_);
    s.store_field("restrict_user_is_premium", restrict_user_is_premium_);
    s.store_field("user_is_premium", user_is_premium_);
    s.store_field("max_quantity", max_quantity_);
    s.store_field("request_name", request_name_);
    s.store_field("request_username", request_username_);
    s.store_field("request_photo", request_photo_);
    s.store_class_end();
  }
}

keyboardButtonTypeRequestChat::keyboardButtonTypeRequestChat()
  : id_()
  , chat_is_channel_()
  , restrict_chat_is_forum_()
  , chat_is_forum_()
  , restrict_chat_has_username_()
  , chat_has_username_()
  , chat_is_created_()
  , user_administrator_rights_()
  , bot_administrator_rights_()
  , bot_is_member_()
  , request_title_()
  , request_username_()
  , request_photo_()
{}

keyboardButtonTypeRequestChat::keyboardButtonTypeRequestChat(int32 id_, bool chat_is_channel_, bool restrict_chat_is_forum_, bool chat_is_forum_, bool restrict_chat_has_username_, bool chat_has_username_, bool chat_is_created_, object_ptr<chatAdministratorRights> &&user_administrator_rights_, object_ptr<chatAdministratorRights> &&bot_administrator_rights_, bool bot_is_member_, bool request_title_, bool request_username_, bool request_photo_)
  : id_(id_)
  , chat_is_channel_(chat_is_channel_)
  , restrict_chat_is_forum_(restrict_chat_is_forum_)
  , chat_is_forum_(chat_is_forum_)
  , restrict_chat_has_username_(restrict_chat_has_username_)
  , chat_has_username_(chat_has_username_)
  , chat_is_created_(chat_is_created_)
  , user_administrator_rights_(std::move(user_administrator_rights_))
  , bot_administrator_rights_(std::move(bot_administrator_rights_))
  , bot_is_member_(bot_is_member_)
  , request_title_(request_title_)
  , request_username_(request_username_)
  , request_photo_(request_photo_)
{}

const std::int32_t keyboardButtonTypeRequestChat::ID;

void keyboardButtonTypeRequestChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonTypeRequestChat");
    s.store_field("id", id_);
    s.store_field("chat_is_channel", chat_is_channel_);
    s.store_field("restrict_chat_is_forum", restrict_chat_is_forum_);
    s.store_field("chat_is_forum", chat_is_forum_);
    s.store_field("restrict_chat_has_username", restrict_chat_has_username_);
    s.store_field("chat_has_username", chat_has_username_);
    s.store_field("chat_is_created", chat_is_created_);
    s.store_object_field("user_administrator_rights", static_cast<const BaseObject *>(user_administrator_rights_.get()));
    s.store_object_field("bot_administrator_rights", static_cast<const BaseObject *>(bot_administrator_rights_.get()));
    s.store_field("bot_is_member", bot_is_member_);
    s.store_field("request_title", request_title_);
    s.store_field("request_username", request_username_);
    s.store_field("request_photo", request_photo_);
    s.store_class_end();
  }
}

keyboardButtonTypeWebApp::keyboardButtonTypeWebApp()
  : url_()
{}

keyboardButtonTypeWebApp::keyboardButtonTypeWebApp(string const &url_)
  : url_(url_)
{}

const std::int32_t keyboardButtonTypeWebApp::ID;

void keyboardButtonTypeWebApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButtonTypeWebApp");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

labeledPricePart::labeledPricePart()
  : label_()
  , amount_()
{}

labeledPricePart::labeledPricePart(string const &label_, int53 amount_)
  : label_(label_)
  , amount_(amount_)
{}

const std::int32_t labeledPricePart::ID;

void labeledPricePart::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "labeledPricePart");
    s.store_field("label", label_);
    s.store_field("amount", amount_);
    s.store_class_end();
  }
}

location::location()
  : latitude_()
  , longitude_()
  , horizontal_accuracy_()
{}

location::location(double latitude_, double longitude_, double horizontal_accuracy_)
  : latitude_(latitude_)
  , longitude_(longitude_)
  , horizontal_accuracy_(horizontal_accuracy_)
{}

const std::int32_t location::ID;

void location::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "location");
    s.store_field("latitude", latitude_);
    s.store_field("longitude", longitude_);
    s.store_field("horizontal_accuracy", horizontal_accuracy_);
    s.store_class_end();
  }
}

logVerbosityLevel::logVerbosityLevel()
  : verbosity_level_()
{}

logVerbosityLevel::logVerbosityLevel(int32 verbosity_level_)
  : verbosity_level_(verbosity_level_)
{}

const std::int32_t logVerbosityLevel::ID;

void logVerbosityLevel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "logVerbosityLevel");
    s.store_field("verbosity_level", verbosity_level_);
    s.store_class_end();
  }
}

messageLinkInfo::messageLinkInfo()
  : is_public_()
  , chat_id_()
  , topic_id_()
  , message_()
  , media_timestamp_()
  , for_album_()
{}

messageLinkInfo::messageLinkInfo(bool is_public_, int53 chat_id_, object_ptr<MessageTopic> &&topic_id_, object_ptr<message> &&message_, int32 media_timestamp_, bool for_album_)
  : is_public_(is_public_)
  , chat_id_(chat_id_)
  , topic_id_(std::move(topic_id_))
  , message_(std::move(message_))
  , media_timestamp_(media_timestamp_)
  , for_album_(for_album_)
{}

const std::int32_t messageLinkInfo::ID;

void messageLinkInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageLinkInfo");
    s.store_field("is_public", is_public_);
    s.store_field("chat_id", chat_id_);
    s.store_object_field("topic_id", static_cast<const BaseObject *>(topic_id_.get()));
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_field("media_timestamp", media_timestamp_);
    s.store_field("for_album", for_album_);
    s.store_class_end();
  }
}

messageReplyInfo::messageReplyInfo()
  : reply_count_()
  , recent_replier_ids_()
  , last_read_inbox_message_id_()
  , last_read_outbox_message_id_()
  , last_message_id_()
{}

messageReplyInfo::messageReplyInfo(int32 reply_count_, array<object_ptr<MessageSender>> &&recent_replier_ids_, int53 last_read_inbox_message_id_, int53 last_read_outbox_message_id_, int53 last_message_id_)
  : reply_count_(reply_count_)
  , recent_replier_ids_(std::move(recent_replier_ids_))
  , last_read_inbox_message_id_(last_read_inbox_message_id_)
  , last_read_outbox_message_id_(last_read_outbox_message_id_)
  , last_message_id_(last_message_id_)
{}

const std::int32_t messageReplyInfo::ID;

void messageReplyInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageReplyInfo");
    s.store_field("reply_count", reply_count_);
    { s.store_vector_begin("recent_replier_ids", recent_replier_ids_.size()); for (const auto &_value : recent_replier_ids_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("last_read_inbox_message_id", last_read_inbox_message_id_);
    s.store_field("last_read_outbox_message_id", last_read_outbox_message_id_);
    s.store_field("last_message_id", last_message_id_);
    s.store_class_end();
  }
}

minithumbnail::minithumbnail()
  : width_()
  , height_()
  , data_()
{}

minithumbnail::minithumbnail(int32 width_, int32 height_, bytes const &data_)
  : width_(width_)
  , height_(height_)
  , data_(std::move(data_))
{}

const std::int32_t minithumbnail::ID;

void minithumbnail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "minithumbnail");
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_bytes_field("data", data_);
    s.store_class_end();
  }
}

pageBlockListItem::pageBlockListItem()
  : label_()
  , page_blocks_()
{}

pageBlockListItem::pageBlockListItem(string const &label_, array<object_ptr<PageBlock>> &&page_blocks_)
  : label_(label_)
  , page_blocks_(std::move(page_blocks_))
{}

const std::int32_t pageBlockListItem::ID;

void pageBlockListItem::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockListItem");
    s.store_field("label", label_);
    { s.store_vector_begin("page_blocks", page_blocks_.size()); for (const auto &_value : page_blocks_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

pageBlockRelatedArticle::pageBlockRelatedArticle()
  : url_()
  , title_()
  , description_()
  , photo_()
  , author_()
  , publish_date_()
{}

pageBlockRelatedArticle::pageBlockRelatedArticle(string const &url_, string const &title_, string const &description_, object_ptr<photo> &&photo_, string const &author_, int32 publish_date_)
  : url_(url_)
  , title_(title_)
  , description_(description_)
  , photo_(std::move(photo_))
  , author_(author_)
  , publish_date_(publish_date_)
{}

const std::int32_t pageBlockRelatedArticle::ID;

void pageBlockRelatedArticle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockRelatedArticle");
    s.store_field("url", url_);
    s.store_field("title", title_);
    s.store_field("description", description_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_field("author", author_);
    s.store_field("publish_date", publish_date_);
    s.store_class_end();
  }
}

pageBlockVerticalAlignmentTop::pageBlockVerticalAlignmentTop() {
}

const std::int32_t pageBlockVerticalAlignmentTop::ID;

void pageBlockVerticalAlignmentTop::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockVerticalAlignmentTop");
    s.store_class_end();
  }
}

pageBlockVerticalAlignmentMiddle::pageBlockVerticalAlignmentMiddle() {
}

const std::int32_t pageBlockVerticalAlignmentMiddle::ID;

void pageBlockVerticalAlignmentMiddle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockVerticalAlignmentMiddle");
    s.store_class_end();
  }
}

pageBlockVerticalAlignmentBottom::pageBlockVerticalAlignmentBottom() {
}

const std::int32_t pageBlockVerticalAlignmentBottom::ID;

void pageBlockVerticalAlignmentBottom::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockVerticalAlignmentBottom");
    s.store_class_end();
  }
}

passportElements::passportElements()
  : elements_()
{}

passportElements::passportElements(array<object_ptr<PassportElement>> &&elements_)
  : elements_(std::move(elements_))
{}

const std::int32_t passportElements::ID;

void passportElements::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElements");
    { s.store_vector_begin("elements", elements_.size()); for (const auto &_value : elements_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

premiumGiveawayPaymentOption::premiumGiveawayPaymentOption()
  : currency_()
  , amount_()
  , winner_count_()
  , month_count_()
  , store_product_id_()
  , store_product_quantity_()
{}

premiumGiveawayPaymentOption::premiumGiveawayPaymentOption(string const &currency_, int53 amount_, int32 winner_count_, int32 month_count_, string const &store_product_id_, int32 store_product_quantity_)
  : currency_(currency_)
  , amount_(amount_)
  , winner_count_(winner_count_)
  , month_count_(month_count_)
  , store_product_id_(store_product_id_)
  , store_product_quantity_(store_product_quantity_)
{}

const std::int32_t premiumGiveawayPaymentOption::ID;

void premiumGiveawayPaymentOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumGiveawayPaymentOption");
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("winner_count", winner_count_);
    s.store_field("month_count", month_count_);
    s.store_field("store_product_id", store_product_id_);
    s.store_field("store_product_quantity", store_product_quantity_);
    s.store_class_end();
  }
}

premiumStatePaymentOption::premiumStatePaymentOption()
  : payment_option_()
  , is_current_()
  , is_upgrade_()
  , last_transaction_id_()
{}

premiumStatePaymentOption::premiumStatePaymentOption(object_ptr<premiumPaymentOption> &&payment_option_, bool is_current_, bool is_upgrade_, string const &last_transaction_id_)
  : payment_option_(std::move(payment_option_))
  , is_current_(is_current_)
  , is_upgrade_(is_upgrade_)
  , last_transaction_id_(last_transaction_id_)
{}

const std::int32_t premiumStatePaymentOption::ID;

void premiumStatePaymentOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumStatePaymentOption");
    s.store_object_field("payment_option", static_cast<const BaseObject *>(payment_option_.get()));
    s.store_field("is_current", is_current_);
    s.store_field("is_upgrade", is_upgrade_);
    s.store_field("last_transaction_id", last_transaction_id_);
    s.store_class_end();
  }
}

productInfo::productInfo()
  : title_()
  , description_()
  , photo_()
{}

productInfo::productInfo(string const &title_, object_ptr<formattedText> &&description_, object_ptr<photo> &&photo_)
  : title_(title_)
  , description_(std::move(description_))
  , photo_(std::move(photo_))
{}

const std::int32_t productInfo::ID;

void productInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "productInfo");
    s.store_field("title", title_);
    s.store_object_field("description", static_cast<const BaseObject *>(description_.get()));
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

profilePhoto::profilePhoto()
  : id_()
  , small_()
  , big_()
  , minithumbnail_()
  , has_animation_()
  , is_personal_()
{}

profilePhoto::profilePhoto(int64 id_, object_ptr<file> &&small_, object_ptr<file> &&big_, object_ptr<minithumbnail> &&minithumbnail_, bool has_animation_, bool is_personal_)
  : id_(id_)
  , small_(std::move(small_))
  , big_(std::move(big_))
  , minithumbnail_(std::move(minithumbnail_))
  , has_animation_(has_animation_)
  , is_personal_(is_personal_)
{}

const std::int32_t profilePhoto::ID;

void profilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "profilePhoto");
    s.store_field("id", id_);
    s.store_object_field("small", static_cast<const BaseObject *>(small_.get()));
    s.store_object_field("big", static_cast<const BaseObject *>(big_.get()));
    s.store_object_field("minithumbnail", static_cast<const BaseObject *>(minithumbnail_.get()));
    s.store_field("has_animation", has_animation_);
    s.store_field("is_personal", is_personal_);
    s.store_class_end();
  }
}

publicForwards::publicForwards()
  : total_count_()
  , forwards_()
  , next_offset_()
{}

publicForwards::publicForwards(int32 total_count_, array<object_ptr<PublicForward>> &&forwards_, string const &next_offset_)
  : total_count_(total_count_)
  , forwards_(std::move(forwards_))
  , next_offset_(next_offset_)
{}

const std::int32_t publicForwards::ID;

void publicForwards::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "publicForwards");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("forwards", forwards_.size()); for (const auto &_value : forwards_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

publicPostSearchLimits::publicPostSearchLimits()
  : daily_free_query_count_()
  , remaining_free_query_count_()
  , next_free_query_in_()
  , star_count_()
  , is_current_query_free_()
{}

publicPostSearchLimits::publicPostSearchLimits(int32 daily_free_query_count_, int32 remaining_free_query_count_, int32 next_free_query_in_, int64 star_count_, bool is_current_query_free_)
  : daily_free_query_count_(daily_free_query_count_)
  , remaining_free_query_count_(remaining_free_query_count_)
  , next_free_query_in_(next_free_query_in_)
  , star_count_(star_count_)
  , is_current_query_free_(is_current_query_free_)
{}

const std::int32_t publicPostSearchLimits::ID;

void publicPostSearchLimits::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "publicPostSearchLimits");
    s.store_field("daily_free_query_count", daily_free_query_count_);
    s.store_field("remaining_free_query_count", remaining_free_query_count_);
    s.store_field("next_free_query_in", next_free_query_in_);
    s.store_field("star_count", star_count_);
    s.store_field("is_current_query_free", is_current_query_free_);
    s.store_class_end();
  }
}

reactionNotificationSettings::reactionNotificationSettings()
  : message_reaction_source_()
  , story_reaction_source_()
  , sound_id_()
  , show_preview_()
{}

reactionNotificationSettings::reactionNotificationSettings(object_ptr<ReactionNotificationSource> &&message_reaction_source_, object_ptr<ReactionNotificationSource> &&story_reaction_source_, int64 sound_id_, bool show_preview_)
  : message_reaction_source_(std::move(message_reaction_source_))
  , story_reaction_source_(std::move(story_reaction_source_))
  , sound_id_(sound_id_)
  , show_preview_(show_preview_)
{}

const std::int32_t reactionNotificationSettings::ID;

void reactionNotificationSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reactionNotificationSettings");
    s.store_object_field("message_reaction_source", static_cast<const BaseObject *>(message_reaction_source_.get()));
    s.store_object_field("story_reaction_source", static_cast<const BaseObject *>(story_reaction_source_.get()));
    s.store_field("sound_id", sound_id_);
    s.store_field("show_preview", show_preview_);
    s.store_class_end();
  }
}

reactionNotificationSourceNone::reactionNotificationSourceNone() {
}

const std::int32_t reactionNotificationSourceNone::ID;

void reactionNotificationSourceNone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reactionNotificationSourceNone");
    s.store_class_end();
  }
}

reactionNotificationSourceContacts::reactionNotificationSourceContacts() {
}

const std::int32_t reactionNotificationSourceContacts::ID;

void reactionNotificationSourceContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reactionNotificationSourceContacts");
    s.store_class_end();
  }
}

reactionNotificationSourceAll::reactionNotificationSourceAll() {
}

const std::int32_t reactionNotificationSourceAll::ID;

void reactionNotificationSourceAll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reactionNotificationSourceAll");
    s.store_class_end();
  }
}

receivedGift::receivedGift()
  : received_gift_id_()
  , sender_id_()
  , text_()
  , is_private_()
  , is_saved_()
  , is_pinned_()
  , can_be_upgraded_()
  , can_be_transferred_()
  , was_refunded_()
  , date_()
  , gift_()
  , collection_ids_()
  , sell_star_count_()
  , prepaid_upgrade_star_count_()
  , is_upgrade_separate_()
  , transfer_star_count_()
  , drop_original_details_star_count_()
  , next_transfer_date_()
  , next_resale_date_()
  , export_date_()
  , prepaid_upgrade_hash_()
{}

receivedGift::receivedGift(string const &received_gift_id_, object_ptr<MessageSender> &&sender_id_, object_ptr<formattedText> &&text_, bool is_private_, bool is_saved_, bool is_pinned_, bool can_be_upgraded_, bool can_be_transferred_, bool was_refunded_, int32 date_, object_ptr<SentGift> &&gift_, array<int32> &&collection_ids_, int53 sell_star_count_, int53 prepaid_upgrade_star_count_, bool is_upgrade_separate_, int53 transfer_star_count_, int53 drop_original_details_star_count_, int32 next_transfer_date_, int32 next_resale_date_, int32 export_date_, string const &prepaid_upgrade_hash_)
  : received_gift_id_(received_gift_id_)
  , sender_id_(std::move(sender_id_))
  , text_(std::move(text_))
  , is_private_(is_private_)
  , is_saved_(is_saved_)
  , is_pinned_(is_pinned_)
  , can_be_upgraded_(can_be_upgraded_)
  , can_be_transferred_(can_be_transferred_)
  , was_refunded_(was_refunded_)
  , date_(date_)
  , gift_(std::move(gift_))
  , collection_ids_(std::move(collection_ids_))
  , sell_star_count_(sell_star_count_)
  , prepaid_upgrade_star_count_(prepaid_upgrade_star_count_)
  , is_upgrade_separate_(is_upgrade_separate_)
  , transfer_star_count_(transfer_star_count_)
  , drop_original_details_star_count_(drop_original_details_star_count_)
  , next_transfer_date_(next_transfer_date_)
  , next_resale_date_(next_resale_date_)
  , export_date_(export_date_)
  , prepaid_upgrade_hash_(prepaid_upgrade_hash_)
{}

const std::int32_t receivedGift::ID;

void receivedGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "receivedGift");
    s.store_field("received_gift_id", received_gift_id_);
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("is_private", is_private_);
    s.store_field("is_saved", is_saved_);
    s.store_field("is_pinned", is_pinned_);
    s.store_field("can_be_upgraded", can_be_upgraded_);
    s.store_field("can_be_transferred", can_be_transferred_);
    s.store_field("was_refunded", was_refunded_);
    s.store_field("date", date_);
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    { s.store_vector_begin("collection_ids", collection_ids_.size()); for (const auto &_value : collection_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("sell_star_count", sell_star_count_);
    s.store_field("prepaid_upgrade_star_count", prepaid_upgrade_star_count_);
    s.store_field("is_upgrade_separate", is_upgrade_separate_);
    s.store_field("transfer_star_count", transfer_star_count_);
    s.store_field("drop_original_details_star_count", drop_original_details_star_count_);
    s.store_field("next_transfer_date", next_transfer_date_);
    s.store_field("next_resale_date", next_resale_date_);
    s.store_field("export_date", export_date_);
    s.store_field("prepaid_upgrade_hash", prepaid_upgrade_hash_);
    s.store_class_end();
  }
}

reportOption::reportOption()
  : id_()
  , text_()
{}

reportOption::reportOption(bytes const &id_, string const &text_)
  : id_(std::move(id_))
  , text_(text_)
{}

const std::int32_t reportOption::ID;

void reportOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportOption");
    s.store_bytes_field("id", id_);
    s.store_field("text", text_);
    s.store_class_end();
  }
}

starCount::starCount()
  : star_count_()
{}

starCount::starCount(int53 star_count_)
  : star_count_(star_count_)
{}

const std::int32_t starCount::ID;

void starCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starCount");
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

statisticalGraphData::statisticalGraphData()
  : json_data_()
  , zoom_token_()
{}

statisticalGraphData::statisticalGraphData(string const &json_data_, string const &zoom_token_)
  : json_data_(json_data_)
  , zoom_token_(zoom_token_)
{}

const std::int32_t statisticalGraphData::ID;

void statisticalGraphData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "statisticalGraphData");
    s.store_field("json_data", json_data_);
    s.store_field("zoom_token", zoom_token_);
    s.store_class_end();
  }
}

statisticalGraphAsync::statisticalGraphAsync()
  : token_()
{}

statisticalGraphAsync::statisticalGraphAsync(string const &token_)
  : token_(token_)
{}

const std::int32_t statisticalGraphAsync::ID;

void statisticalGraphAsync::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "statisticalGraphAsync");
    s.store_field("token", token_);
    s.store_class_end();
  }
}

statisticalGraphError::statisticalGraphError()
  : error_message_()
{}

statisticalGraphError::statisticalGraphError(string const &error_message_)
  : error_message_(error_message_)
{}

const std::int32_t statisticalGraphError::ID;

void statisticalGraphError::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "statisticalGraphError");
    s.store_field("error_message", error_message_);
    s.store_class_end();
  }
}

stickerTypeRegular::stickerTypeRegular() {
}

const std::int32_t stickerTypeRegular::ID;

void stickerTypeRegular::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickerTypeRegular");
    s.store_class_end();
  }
}

stickerTypeMask::stickerTypeMask() {
}

const std::int32_t stickerTypeMask::ID;

void stickerTypeMask::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickerTypeMask");
    s.store_class_end();
  }
}

stickerTypeCustomEmoji::stickerTypeCustomEmoji() {
}

const std::int32_t stickerTypeCustomEmoji::ID;

void stickerTypeCustomEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickerTypeCustomEmoji");
    s.store_class_end();
  }
}

stories::stories()
  : total_count_()
  , stories_()
  , pinned_story_ids_()
{}

stories::stories(int32 total_count_, array<object_ptr<story>> &&stories_, array<int32> &&pinned_story_ids_)
  : total_count_(total_count_)
  , stories_(std::move(stories_))
  , pinned_story_ids_(std::move(pinned_story_ids_))
{}

const std::int32_t stories::ID;

void stories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stories");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("stories", stories_.size()); for (const auto &_value : stories_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("pinned_story_ids", pinned_story_ids_.size()); for (const auto &_value : pinned_story_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

supergroupFullInfo::supergroupFullInfo()
  : photo_()
  , description_()
  , member_count_()
  , administrator_count_()
  , restricted_count_()
  , banned_count_()
  , linked_chat_id_()
  , direct_messages_chat_id_()
  , slow_mode_delay_()
  , slow_mode_delay_expires_in_()
  , can_enable_paid_messages_()
  , can_enable_paid_reaction_()
  , can_get_members_()
  , has_hidden_members_()
  , can_hide_members_()
  , can_set_sticker_set_()
  , can_set_location_()
  , can_get_statistics_()
  , can_get_revenue_statistics_()
  , can_get_star_revenue_statistics_()
  , can_send_gift_()
  , can_toggle_aggressive_anti_spam_()
  , is_all_history_available_()
  , can_have_sponsored_messages_()
  , has_aggressive_anti_spam_enabled_()
  , has_paid_media_allowed_()
  , has_pinned_stories_()
  , gift_count_()
  , my_boost_count_()
  , unrestrict_boost_count_()
  , outgoing_paid_message_star_count_()
  , sticker_set_id_()
  , custom_emoji_sticker_set_id_()
  , location_()
  , invite_link_()
  , bot_commands_()
  , bot_verification_()
  , main_profile_tab_()
  , upgraded_from_basic_group_id_()
  , upgraded_from_max_message_id_()
{}

supergroupFullInfo::supergroupFullInfo(object_ptr<chatPhoto> &&photo_, string const &description_, int32 member_count_, int32 administrator_count_, int32 restricted_count_, int32 banned_count_, int53 linked_chat_id_, int53 direct_messages_chat_id_, int32 slow_mode_delay_, double slow_mode_delay_expires_in_, bool can_enable_paid_messages_, bool can_enable_paid_reaction_, bool can_get_members_, bool has_hidden_members_, bool can_hide_members_, bool can_set_sticker_set_, bool can_set_location_, bool can_get_statistics_, bool can_get_revenue_statistics_, bool can_get_star_revenue_statistics_, bool can_send_gift_, bool can_toggle_aggressive_anti_spam_, bool is_all_history_available_, bool can_have_sponsored_messages_, bool has_aggressive_anti_spam_enabled_, bool has_paid_media_allowed_, bool has_pinned_stories_, int32 gift_count_, int32 my_boost_count_, int32 unrestrict_boost_count_, int53 outgoing_paid_message_star_count_, int64 sticker_set_id_, int64 custom_emoji_sticker_set_id_, object_ptr<chatLocation> &&location_, object_ptr<chatInviteLink> &&invite_link_, array<object_ptr<botCommands>> &&bot_commands_, object_ptr<botVerification> &&bot_verification_, object_ptr<ProfileTab> &&main_profile_tab_, int53 upgraded_from_basic_group_id_, int53 upgraded_from_max_message_id_)
  : photo_(std::move(photo_))
  , description_(description_)
  , member_count_(member_count_)
  , administrator_count_(administrator_count_)
  , restricted_count_(restricted_count_)
  , banned_count_(banned_count_)
  , linked_chat_id_(linked_chat_id_)
  , direct_messages_chat_id_(direct_messages_chat_id_)
  , slow_mode_delay_(slow_mode_delay_)
  , slow_mode_delay_expires_in_(slow_mode_delay_expires_in_)
  , can_enable_paid_messages_(can_enable_paid_messages_)
  , can_enable_paid_reaction_(can_enable_paid_reaction_)
  , can_get_members_(can_get_members_)
  , has_hidden_members_(has_hidden_members_)
  , can_hide_members_(can_hide_members_)
  , can_set_sticker_set_(can_set_sticker_set_)
  , can_set_location_(can_set_location_)
  , can_get_statistics_(can_get_statistics_)
  , can_get_revenue_statistics_(can_get_revenue_statistics_)
  , can_get_star_revenue_statistics_(can_get_star_revenue_statistics_)
  , can_send_gift_(can_send_gift_)
  , can_toggle_aggressive_anti_spam_(can_toggle_aggressive_anti_spam_)
  , is_all_history_available_(is_all_history_available_)
  , can_have_sponsored_messages_(can_have_sponsored_messages_)
  , has_aggressive_anti_spam_enabled_(has_aggressive_anti_spam_enabled_)
  , has_paid_media_allowed_(has_paid_media_allowed_)
  , has_pinned_stories_(has_pinned_stories_)
  , gift_count_(gift_count_)
  , my_boost_count_(my_boost_count_)
  , unrestrict_boost_count_(unrestrict_boost_count_)
  , outgoing_paid_message_star_count_(outgoing_paid_message_star_count_)
  , sticker_set_id_(sticker_set_id_)
  , custom_emoji_sticker_set_id_(custom_emoji_sticker_set_id_)
  , location_(std::move(location_))
  , invite_link_(std::move(invite_link_))
  , bot_commands_(std::move(bot_commands_))
  , bot_verification_(std::move(bot_verification_))
  , main_profile_tab_(std::move(main_profile_tab_))
  , upgraded_from_basic_group_id_(upgraded_from_basic_group_id_)
  , upgraded_from_max_message_id_(upgraded_from_max_message_id_)
{}

const std::int32_t supergroupFullInfo::ID;

void supergroupFullInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "supergroupFullInfo");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_field("description", description_);
    s.store_field("member_count", member_count_);
    s.store_field("administrator_count", administrator_count_);
    s.store_field("restricted_count", restricted_count_);
    s.store_field("banned_count", banned_count_);
    s.store_field("linked_chat_id", linked_chat_id_);
    s.store_field("direct_messages_chat_id", direct_messages_chat_id_);
    s.store_field("slow_mode_delay", slow_mode_delay_);
    s.store_field("slow_mode_delay_expires_in", slow_mode_delay_expires_in_);
    s.store_field("can_enable_paid_messages", can_enable_paid_messages_);
    s.store_field("can_enable_paid_reaction", can_enable_paid_reaction_);
    s.store_field("can_get_members", can_get_members_);
    s.store_field("has_hidden_members", has_hidden_members_);
    s.store_field("can_hide_members", can_hide_members_);
    s.store_field("can_set_sticker_set", can_set_sticker_set_);
    s.store_field("can_set_location", can_set_location_);
    s.store_field("can_get_statistics", can_get_statistics_);
    s.store_field("can_get_revenue_statistics", can_get_revenue_statistics_);
    s.store_field("can_get_star_revenue_statistics", can_get_star_revenue_statistics_);
    s.store_field("can_send_gift", can_send_gift_);
    s.store_field("can_toggle_aggressive_anti_spam", can_toggle_aggressive_anti_spam_);
    s.store_field("is_all_history_available", is_all_history_available_);
    s.store_field("can_have_sponsored_messages", can_have_sponsored_messages_);
    s.store_field("has_aggressive_anti_spam_enabled", has_aggressive_anti_spam_enabled_);
    s.store_field("has_paid_media_allowed", has_paid_media_allowed_);
    s.store_field("has_pinned_stories", has_pinned_stories_);
    s.store_field("gift_count", gift_count_);
    s.store_field("my_boost_count", my_boost_count_);
    s.store_field("unrestrict_boost_count", unrestrict_boost_count_);
    s.store_field("outgoing_paid_message_star_count", outgoing_paid_message_star_count_);
    s.store_field("sticker_set_id", sticker_set_id_);
    s.store_field("custom_emoji_sticker_set_id", custom_emoji_sticker_set_id_);
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_object_field("invite_link", static_cast<const BaseObject *>(invite_link_.get()));
    { s.store_vector_begin("bot_commands", bot_commands_.size()); for (const auto &_value : bot_commands_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("bot_verification", static_cast<const BaseObject *>(bot_verification_.get()));
    s.store_object_field("main_profile_tab", static_cast<const BaseObject *>(main_profile_tab_.get()));
    s.store_field("upgraded_from_basic_group_id", upgraded_from_basic_group_id_);
    s.store_field("upgraded_from_max_message_id", upgraded_from_max_message_id_);
    s.store_class_end();
  }
}

testInt::testInt()
  : value_()
{}

testInt::testInt(int32 value_)
  : value_(value_)
{}

const std::int32_t testInt::ID;

void testInt::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testInt");
    s.store_field("value", value_);
    s.store_class_end();
  }
}

testVectorStringObject::testVectorStringObject()
  : value_()
{}

testVectorStringObject::testVectorStringObject(array<object_ptr<testString>> &&value_)
  : value_(std::move(value_))
{}

const std::int32_t testVectorStringObject::ID;

void testVectorStringObject::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testVectorStringObject");
    { s.store_vector_begin("value", value_.size()); for (const auto &_value : value_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

thumbnailFormatJpeg::thumbnailFormatJpeg() {
}

const std::int32_t thumbnailFormatJpeg::ID;

void thumbnailFormatJpeg::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "thumbnailFormatJpeg");
    s.store_class_end();
  }
}

thumbnailFormatGif::thumbnailFormatGif() {
}

const std::int32_t thumbnailFormatGif::ID;

void thumbnailFormatGif::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "thumbnailFormatGif");
    s.store_class_end();
  }
}

thumbnailFormatMpeg4::thumbnailFormatMpeg4() {
}

const std::int32_t thumbnailFormatMpeg4::ID;

void thumbnailFormatMpeg4::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "thumbnailFormatMpeg4");
    s.store_class_end();
  }
}

thumbnailFormatPng::thumbnailFormatPng() {
}

const std::int32_t thumbnailFormatPng::ID;

void thumbnailFormatPng::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "thumbnailFormatPng");
    s.store_class_end();
  }
}

thumbnailFormatTgs::thumbnailFormatTgs() {
}

const std::int32_t thumbnailFormatTgs::ID;

void thumbnailFormatTgs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "thumbnailFormatTgs");
    s.store_class_end();
  }
}

thumbnailFormatWebm::thumbnailFormatWebm() {
}

const std::int32_t thumbnailFormatWebm::ID;

void thumbnailFormatWebm::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "thumbnailFormatWebm");
    s.store_class_end();
  }
}

thumbnailFormatWebp::thumbnailFormatWebp() {
}

const std::int32_t thumbnailFormatWebp::ID;

void thumbnailFormatWebp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "thumbnailFormatWebp");
    s.store_class_end();
  }
}

upgradedGiftModel::upgradedGiftModel()
  : name_()
  , sticker_()
  , rarity_per_mille_()
{}

upgradedGiftModel::upgradedGiftModel(string const &name_, object_ptr<sticker> &&sticker_, int32 rarity_per_mille_)
  : name_(name_)
  , sticker_(std::move(sticker_))
  , rarity_per_mille_(rarity_per_mille_)
{}

const std::int32_t upgradedGiftModel::ID;

void upgradedGiftModel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradedGiftModel");
    s.store_field("name", name_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_field("rarity_per_mille", rarity_per_mille_);
    s.store_class_end();
  }
}

upgradedGiftValueInfo::upgradedGiftValueInfo()
  : currency_()
  , value_()
  , is_value_average_()
  , initial_sale_date_()
  , initial_sale_star_count_()
  , initial_sale_price_()
  , last_sale_date_()
  , last_sale_price_()
  , is_last_sale_on_fragment_()
  , minimum_price_()
  , average_sale_price_()
  , telegram_listed_gift_count_()
  , fragment_listed_gift_count_()
  , fragment_url_()
{}

upgradedGiftValueInfo::upgradedGiftValueInfo(string const &currency_, int53 value_, bool is_value_average_, int32 initial_sale_date_, int53 initial_sale_star_count_, int53 initial_sale_price_, int32 last_sale_date_, int53 last_sale_price_, bool is_last_sale_on_fragment_, int53 minimum_price_, int53 average_sale_price_, int32 telegram_listed_gift_count_, int32 fragment_listed_gift_count_, string const &fragment_url_)
  : currency_(currency_)
  , value_(value_)
  , is_value_average_(is_value_average_)
  , initial_sale_date_(initial_sale_date_)
  , initial_sale_star_count_(initial_sale_star_count_)
  , initial_sale_price_(initial_sale_price_)
  , last_sale_date_(last_sale_date_)
  , last_sale_price_(last_sale_price_)
  , is_last_sale_on_fragment_(is_last_sale_on_fragment_)
  , minimum_price_(minimum_price_)
  , average_sale_price_(average_sale_price_)
  , telegram_listed_gift_count_(telegram_listed_gift_count_)
  , fragment_listed_gift_count_(fragment_listed_gift_count_)
  , fragment_url_(fragment_url_)
{}

const std::int32_t upgradedGiftValueInfo::ID;

void upgradedGiftValueInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradedGiftValueInfo");
    s.store_field("currency", currency_);
    s.store_field("value", value_);
    s.store_field("is_value_average", is_value_average_);
    s.store_field("initial_sale_date", initial_sale_date_);
    s.store_field("initial_sale_star_count", initial_sale_star_count_);
    s.store_field("initial_sale_price", initial_sale_price_);
    s.store_field("last_sale_date", last_sale_date_);
    s.store_field("last_sale_price", last_sale_price_);
    s.store_field("is_last_sale_on_fragment", is_last_sale_on_fragment_);
    s.store_field("minimum_price", minimum_price_);
    s.store_field("average_sale_price", average_sale_price_);
    s.store_field("telegram_listed_gift_count", telegram_listed_gift_count_);
    s.store_field("fragment_listed_gift_count", fragment_listed_gift_count_);
    s.store_field("fragment_url", fragment_url_);
    s.store_class_end();
  }
}

user::user()
  : id_()
  , first_name_()
  , last_name_()
  , usernames_()
  , phone_number_()
  , status_()
  , profile_photo_()
  , accent_color_id_()
  , background_custom_emoji_id_()
  , upgraded_gift_colors_()
  , profile_accent_color_id_()
  , profile_background_custom_emoji_id_()
  , emoji_status_()
  , is_contact_()
  , is_mutual_contact_()
  , is_close_friend_()
  , verification_status_()
  , is_premium_()
  , is_support_()
  , restriction_info_()
  , has_active_stories_()
  , has_unread_active_stories_()
  , restricts_new_chats_()
  , paid_message_star_count_()
  , have_access_()
  , type_()
  , language_code_()
  , added_to_attachment_menu_()
{}

user::user(int53 id_, string const &first_name_, string const &last_name_, object_ptr<usernames> &&usernames_, string const &phone_number_, object_ptr<UserStatus> &&status_, object_ptr<profilePhoto> &&profile_photo_, int32 accent_color_id_, int64 background_custom_emoji_id_, object_ptr<upgradedGiftColors> &&upgraded_gift_colors_, int32 profile_accent_color_id_, int64 profile_background_custom_emoji_id_, object_ptr<emojiStatus> &&emoji_status_, bool is_contact_, bool is_mutual_contact_, bool is_close_friend_, object_ptr<verificationStatus> &&verification_status_, bool is_premium_, bool is_support_, object_ptr<restrictionInfo> &&restriction_info_, bool has_active_stories_, bool has_unread_active_stories_, bool restricts_new_chats_, int53 paid_message_star_count_, bool have_access_, object_ptr<UserType> &&type_, string const &language_code_, bool added_to_attachment_menu_)
  : id_(id_)
  , first_name_(first_name_)
  , last_name_(last_name_)
  , usernames_(std::move(usernames_))
  , phone_number_(phone_number_)
  , status_(std::move(status_))
  , profile_photo_(std::move(profile_photo_))
  , accent_color_id_(accent_color_id_)
  , background_custom_emoji_id_(background_custom_emoji_id_)
  , upgraded_gift_colors_(std::move(upgraded_gift_colors_))
  , profile_accent_color_id_(profile_accent_color_id_)
  , profile_background_custom_emoji_id_(profile_background_custom_emoji_id_)
  , emoji_status_(std::move(emoji_status_))
  , is_contact_(is_contact_)
  , is_mutual_contact_(is_mutual_contact_)
  , is_close_friend_(is_close_friend_)
  , verification_status_(std::move(verification_status_))
  , is_premium_(is_premium_)
  , is_support_(is_support_)
  , restriction_info_(std::move(restriction_info_))
  , has_active_stories_(has_active_stories_)
  , has_unread_active_stories_(has_unread_active_stories_)
  , restricts_new_chats_(restricts_new_chats_)
  , paid_message_star_count_(paid_message_star_count_)
  , have_access_(have_access_)
  , type_(std::move(type_))
  , language_code_(language_code_)
  , added_to_attachment_menu_(added_to_attachment_menu_)
{}

const std::int32_t user::ID;

void user::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "user");
    s.store_field("id", id_);
    s.store_field("first_name", first_name_);
    s.store_field("last_name", last_name_);
    s.store_object_field("usernames", static_cast<const BaseObject *>(usernames_.get()));
    s.store_field("phone_number", phone_number_);
    s.store_object_field("status", static_cast<const BaseObject *>(status_.get()));
    s.store_object_field("profile_photo", static_cast<const BaseObject *>(profile_photo_.get()));
    s.store_field("accent_color_id", accent_color_id_);
    s.store_field("background_custom_emoji_id", background_custom_emoji_id_);
    s.store_object_field("upgraded_gift_colors", static_cast<const BaseObject *>(upgraded_gift_colors_.get()));
    s.store_field("profile_accent_color_id", profile_accent_color_id_);
    s.store_field("profile_background_custom_emoji_id", profile_background_custom_emoji_id_);
    s.store_object_field("emoji_status", static_cast<const BaseObject *>(emoji_status_.get()));
    s.store_field("is_contact", is_contact_);
    s.store_field("is_mutual_contact", is_mutual_contact_);
    s.store_field("is_close_friend", is_close_friend_);
    s.store_object_field("verification_status", static_cast<const BaseObject *>(verification_status_.get()));
    s.store_field("is_premium", is_premium_);
    s.store_field("is_support", is_support_);
    s.store_object_field("restriction_info", static_cast<const BaseObject *>(restriction_info_.get()));
    s.store_field("has_active_stories", has_active_stories_);
    s.store_field("has_unread_active_stories", has_unread_active_stories_);
    s.store_field("restricts_new_chats", restricts_new_chats_);
    s.store_field("paid_message_star_count", paid_message_star_count_);
    s.store_field("have_access", have_access_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("language_code", language_code_);
    s.store_field("added_to_attachment_menu", added_to_attachment_menu_);
    s.store_class_end();
  }
}

verificationStatus::verificationStatus()
  : is_verified_()
  , is_scam_()
  , is_fake_()
  , bot_verification_icon_custom_emoji_id_()
{}

verificationStatus::verificationStatus(bool is_verified_, bool is_scam_, bool is_fake_, int64 bot_verification_icon_custom_emoji_id_)
  : is_verified_(is_verified_)
  , is_scam_(is_scam_)
  , is_fake_(is_fake_)
  , bot_verification_icon_custom_emoji_id_(bot_verification_icon_custom_emoji_id_)
{}

const std::int32_t verificationStatus::ID;

void verificationStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "verificationStatus");
    s.store_field("is_verified", is_verified_);
    s.store_field("is_scam", is_scam_);
    s.store_field("is_fake", is_fake_);
    s.store_field("bot_verification_icon_custom_emoji_id", bot_verification_icon_custom_emoji_id_);
    s.store_class_end();
  }
}

video::video()
  : duration_()
  , width_()
  , height_()
  , file_name_()
  , mime_type_()
  , has_stickers_()
  , supports_streaming_()
  , minithumbnail_()
  , thumbnail_()
  , video_()
{}

video::video(int32 duration_, int32 width_, int32 height_, string const &file_name_, string const &mime_type_, bool has_stickers_, bool supports_streaming_, object_ptr<minithumbnail> &&minithumbnail_, object_ptr<thumbnail> &&thumbnail_, object_ptr<file> &&video_)
  : duration_(duration_)
  , width_(width_)
  , height_(height_)
  , file_name_(file_name_)
  , mime_type_(mime_type_)
  , has_stickers_(has_stickers_)
  , supports_streaming_(supports_streaming_)
  , minithumbnail_(std::move(minithumbnail_))
  , thumbnail_(std::move(thumbnail_))
  , video_(std::move(video_))
{}

const std::int32_t video::ID;

void video::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "video");
    s.store_field("duration", duration_);
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_field("file_name", file_name_);
    s.store_field("mime_type", mime_type_);
    s.store_field("has_stickers", has_stickers_);
    s.store_field("supports_streaming", supports_streaming_);
    s.store_object_field("minithumbnail", static_cast<const BaseObject *>(minithumbnail_.get()));
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_object_field("video", static_cast<const BaseObject *>(video_.get()));
    s.store_class_end();
  }
}

videoChat::videoChat()
  : group_call_id_()
  , has_participants_()
  , default_participant_id_()
{}

videoChat::videoChat(int32 group_call_id_, bool has_participants_, object_ptr<MessageSender> &&default_participant_id_)
  : group_call_id_(group_call_id_)
  , has_participants_(has_participants_)
  , default_participant_id_(std::move(default_participant_id_))
{}

const std::int32_t videoChat::ID;

void videoChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "videoChat");
    s.store_field("group_call_id", group_call_id_);
    s.store_field("has_participants", has_participants_);
    s.store_object_field("default_participant_id", static_cast<const BaseObject *>(default_participant_id_.get()));
    s.store_class_end();
  }
}

videoNote::videoNote()
  : duration_()
  , waveform_()
  , length_()
  , minithumbnail_()
  , thumbnail_()
  , speech_recognition_result_()
  , video_()
{}

videoNote::videoNote(int32 duration_, bytes const &waveform_, int32 length_, object_ptr<minithumbnail> &&minithumbnail_, object_ptr<thumbnail> &&thumbnail_, object_ptr<SpeechRecognitionResult> &&speech_recognition_result_, object_ptr<file> &&video_)
  : duration_(duration_)
  , waveform_(std::move(waveform_))
  , length_(length_)
  , minithumbnail_(std::move(minithumbnail_))
  , thumbnail_(std::move(thumbnail_))
  , speech_recognition_result_(std::move(speech_recognition_result_))
  , video_(std::move(video_))
{}

const std::int32_t videoNote::ID;

void videoNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "videoNote");
    s.store_field("duration", duration_);
    s.store_bytes_field("waveform", waveform_);
    s.store_field("length", length_);
    s.store_object_field("minithumbnail", static_cast<const BaseObject *>(minithumbnail_.get()));
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_object_field("speech_recognition_result", static_cast<const BaseObject *>(speech_recognition_result_.get()));
    s.store_object_field("video", static_cast<const BaseObject *>(video_.get()));
    s.store_class_end();
  }
}

acceptTermsOfService::acceptTermsOfService()
  : terms_of_service_id_()
{}

acceptTermsOfService::acceptTermsOfService(string const &terms_of_service_id_)
  : terms_of_service_id_(terms_of_service_id_)
{}

const std::int32_t acceptTermsOfService::ID;

void acceptTermsOfService::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "acceptTermsOfService");
    s.store_field("terms_of_service_id", terms_of_service_id_);
    s.store_class_end();
  }
}

addQuickReplyShortcutInlineQueryResultMessage::addQuickReplyShortcutInlineQueryResultMessage()
  : shortcut_name_()
  , reply_to_message_id_()
  , query_id_()
  , result_id_()
  , hide_via_bot_()
{}

addQuickReplyShortcutInlineQueryResultMessage::addQuickReplyShortcutInlineQueryResultMessage(string const &shortcut_name_, int53 reply_to_message_id_, int64 query_id_, string const &result_id_, bool hide_via_bot_)
  : shortcut_name_(shortcut_name_)
  , reply_to_message_id_(reply_to_message_id_)
  , query_id_(query_id_)
  , result_id_(result_id_)
  , hide_via_bot_(hide_via_bot_)
{}

const std::int32_t addQuickReplyShortcutInlineQueryResultMessage::ID;

void addQuickReplyShortcutInlineQueryResultMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addQuickReplyShortcutInlineQueryResultMessage");
    s.store_field("shortcut_name", shortcut_name_);
    s.store_field("reply_to_message_id", reply_to_message_id_);
    s.store_field("query_id", query_id_);
    s.store_field("result_id", result_id_);
    s.store_field("hide_via_bot", hide_via_bot_);
    s.store_class_end();
  }
}

addRecentSticker::addRecentSticker()
  : is_attached_()
  , sticker_()
{}

addRecentSticker::addRecentSticker(bool is_attached_, object_ptr<InputFile> &&sticker_)
  : is_attached_(is_attached_)
  , sticker_(std::move(sticker_))
{}

const std::int32_t addRecentSticker::ID;

void addRecentSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addRecentSticker");
    s.store_field("is_attached", is_attached_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

addRecentlyFoundChat::addRecentlyFoundChat()
  : chat_id_()
{}

addRecentlyFoundChat::addRecentlyFoundChat(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t addRecentlyFoundChat::ID;

void addRecentlyFoundChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addRecentlyFoundChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

answerInlineQuery::answerInlineQuery()
  : inline_query_id_()
  , is_personal_()
  , button_()
  , results_()
  , cache_time_()
  , next_offset_()
{}

answerInlineQuery::answerInlineQuery(int64 inline_query_id_, bool is_personal_, object_ptr<inlineQueryResultsButton> &&button_, array<object_ptr<InputInlineQueryResult>> &&results_, int32 cache_time_, string const &next_offset_)
  : inline_query_id_(inline_query_id_)
  , is_personal_(is_personal_)
  , button_(std::move(button_))
  , results_(std::move(results_))
  , cache_time_(cache_time_)
  , next_offset_(next_offset_)
{}

const std::int32_t answerInlineQuery::ID;

void answerInlineQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "answerInlineQuery");
    s.store_field("inline_query_id", inline_query_id_);
    s.store_field("is_personal", is_personal_);
    s.store_object_field("button", static_cast<const BaseObject *>(button_.get()));
    { s.store_vector_begin("results", results_.size()); for (const auto &_value : results_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("cache_time", cache_time_);
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

canBotSendMessages::canBotSendMessages()
  : bot_user_id_()
{}

canBotSendMessages::canBotSendMessages(int53 bot_user_id_)
  : bot_user_id_(bot_user_id_)
{}

const std::int32_t canBotSendMessages::ID;

void canBotSendMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canBotSendMessages");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_class_end();
  }
}

canPostStory::canPostStory()
  : chat_id_()
{}

canPostStory::canPostStory(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t canPostStory::ID;

void canPostStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canPostStory");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

cancelPreliminaryUploadFile::cancelPreliminaryUploadFile()
  : file_id_()
{}

cancelPreliminaryUploadFile::cancelPreliminaryUploadFile(int32 file_id_)
  : file_id_(file_id_)
{}

const std::int32_t cancelPreliminaryUploadFile::ID;

void cancelPreliminaryUploadFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "cancelPreliminaryUploadFile");
    s.store_field("file_id", file_id_);
    s.store_class_end();
  }
}

checkAuthenticationPassword::checkAuthenticationPassword()
  : password_()
{}

checkAuthenticationPassword::checkAuthenticationPassword(string const &password_)
  : password_(password_)
{}

const std::int32_t checkAuthenticationPassword::ID;

void checkAuthenticationPassword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkAuthenticationPassword");
    s.store_field("password", password_);
    s.store_class_end();
  }
}

checkAuthenticationPasswordRecoveryCode::checkAuthenticationPasswordRecoveryCode()
  : recovery_code_()
{}

checkAuthenticationPasswordRecoveryCode::checkAuthenticationPasswordRecoveryCode(string const &recovery_code_)
  : recovery_code_(recovery_code_)
{}

const std::int32_t checkAuthenticationPasswordRecoveryCode::ID;

void checkAuthenticationPasswordRecoveryCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkAuthenticationPasswordRecoveryCode");
    s.store_field("recovery_code", recovery_code_);
    s.store_class_end();
  }
}

checkChatFolderInviteLink::checkChatFolderInviteLink()
  : invite_link_()
{}

checkChatFolderInviteLink::checkChatFolderInviteLink(string const &invite_link_)
  : invite_link_(invite_link_)
{}

const std::int32_t checkChatFolderInviteLink::ID;

void checkChatFolderInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkChatFolderInviteLink");
    s.store_field("invite_link", invite_link_);
    s.store_class_end();
  }
}

checkCreatedPublicChatsLimit::checkCreatedPublicChatsLimit()
  : type_()
{}

checkCreatedPublicChatsLimit::checkCreatedPublicChatsLimit(object_ptr<PublicChatType> &&type_)
  : type_(std::move(type_))
{}

const std::int32_t checkCreatedPublicChatsLimit::ID;

void checkCreatedPublicChatsLimit::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkCreatedPublicChatsLimit");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

clickVideoMessageAdvertisement::clickVideoMessageAdvertisement()
  : advertisement_unique_id_()
{}

clickVideoMessageAdvertisement::clickVideoMessageAdvertisement(int53 advertisement_unique_id_)
  : advertisement_unique_id_(advertisement_unique_id_)
{}

const std::int32_t clickVideoMessageAdvertisement::ID;

void clickVideoMessageAdvertisement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "clickVideoMessageAdvertisement");
    s.store_field("advertisement_unique_id", advertisement_unique_id_);
    s.store_class_end();
  }
}

closeStory::closeStory()
  : story_poster_chat_id_()
  , story_id_()
{}

closeStory::closeStory(int53 story_poster_chat_id_, int32 story_id_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , story_id_(story_id_)
{}

const std::int32_t closeStory::ID;

void closeStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "closeStory");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("story_id", story_id_);
    s.store_class_end();
  }
}

createChatFolderInviteLink::createChatFolderInviteLink()
  : chat_folder_id_()
  , name_()
  , chat_ids_()
{}

createChatFolderInviteLink::createChatFolderInviteLink(int32 chat_folder_id_, string const &name_, array<int53> &&chat_ids_)
  : chat_folder_id_(chat_folder_id_)
  , name_(name_)
  , chat_ids_(std::move(chat_ids_))
{}

const std::int32_t createChatFolderInviteLink::ID;

void createChatFolderInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createChatFolderInviteLink");
    s.store_field("chat_folder_id", chat_folder_id_);
    s.store_field("name", name_);
    { s.store_vector_begin("chat_ids", chat_ids_.size()); for (const auto &_value : chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

createChatInviteLink::createChatInviteLink()
  : chat_id_()
  , name_()
  , expiration_date_()
  , member_limit_()
  , creates_join_request_()
{}

createChatInviteLink::createChatInviteLink(int53 chat_id_, string const &name_, int32 expiration_date_, int32 member_limit_, bool creates_join_request_)
  : chat_id_(chat_id_)
  , name_(name_)
  , expiration_date_(expiration_date_)
  , member_limit_(member_limit_)
  , creates_join_request_(creates_join_request_)
{}

const std::int32_t createChatInviteLink::ID;

void createChatInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createChatInviteLink");
    s.store_field("chat_id", chat_id_);
    s.store_field("name", name_);
    s.store_field("expiration_date", expiration_date_);
    s.store_field("member_limit", member_limit_);
    s.store_field("creates_join_request", creates_join_request_);
    s.store_class_end();
  }
}

createGroupCall::createGroupCall()
  : join_parameters_()
{}

createGroupCall::createGroupCall(object_ptr<groupCallJoinParameters> &&join_parameters_)
  : join_parameters_(std::move(join_parameters_))
{}

const std::int32_t createGroupCall::ID;

void createGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createGroupCall");
    s.store_object_field("join_parameters", static_cast<const BaseObject *>(join_parameters_.get()));
    s.store_class_end();
  }
}

createInvoiceLink::createInvoiceLink()
  : business_connection_id_()
  , invoice_()
{}

createInvoiceLink::createInvoiceLink(string const &business_connection_id_, object_ptr<InputMessageContent> &&invoice_)
  : business_connection_id_(business_connection_id_)
  , invoice_(std::move(invoice_))
{}

const std::int32_t createInvoiceLink::ID;

void createInvoiceLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createInvoiceLink");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_object_field("invoice", static_cast<const BaseObject *>(invoice_.get()));
    s.store_class_end();
  }
}

declineSuggestedPost::declineSuggestedPost()
  : chat_id_()
  , message_id_()
  , comment_()
{}

declineSuggestedPost::declineSuggestedPost(int53 chat_id_, int53 message_id_, string const &comment_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , comment_(comment_)
{}

const std::int32_t declineSuggestedPost::ID;

void declineSuggestedPost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "declineSuggestedPost");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("comment", comment_);
    s.store_class_end();
  }
}

deleteBotMediaPreviews::deleteBotMediaPreviews()
  : bot_user_id_()
  , language_code_()
  , file_ids_()
{}

deleteBotMediaPreviews::deleteBotMediaPreviews(int53 bot_user_id_, string const &language_code_, array<int32> &&file_ids_)
  : bot_user_id_(bot_user_id_)
  , language_code_(language_code_)
  , file_ids_(std::move(file_ids_))
{}

const std::int32_t deleteBotMediaPreviews::ID;

void deleteBotMediaPreviews::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteBotMediaPreviews");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("language_code", language_code_);
    { s.store_vector_begin("file_ids", file_ids_.size()); for (const auto &_value : file_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

deleteBusinessChatLink::deleteBusinessChatLink()
  : link_()
{}

deleteBusinessChatLink::deleteBusinessChatLink(string const &link_)
  : link_(link_)
{}

const std::int32_t deleteBusinessChatLink::ID;

void deleteBusinessChatLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteBusinessChatLink");
    s.store_field("link", link_);
    s.store_class_end();
  }
}

deleteChatFolder::deleteChatFolder()
  : chat_folder_id_()
  , leave_chat_ids_()
{}

deleteChatFolder::deleteChatFolder(int32 chat_folder_id_, array<int53> &&leave_chat_ids_)
  : chat_folder_id_(chat_folder_id_)
  , leave_chat_ids_(std::move(leave_chat_ids_))
{}

const std::int32_t deleteChatFolder::ID;

void deleteChatFolder::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteChatFolder");
    s.store_field("chat_folder_id", chat_folder_id_);
    { s.store_vector_begin("leave_chat_ids", leave_chat_ids_.size()); for (const auto &_value : leave_chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

deleteMessages::deleteMessages()
  : chat_id_()
  , message_ids_()
  , revoke_()
{}

deleteMessages::deleteMessages(int53 chat_id_, array<int53> &&message_ids_, bool revoke_)
  : chat_id_(chat_id_)
  , message_ids_(std::move(message_ids_))
  , revoke_(revoke_)
{}

const std::int32_t deleteMessages::ID;

void deleteMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteMessages");
    s.store_field("chat_id", chat_id_);
    { s.store_vector_begin("message_ids", message_ids_.size()); for (const auto &_value : message_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("revoke", revoke_);
    s.store_class_end();
  }
}

deleteSavedMessagesTopicHistory::deleteSavedMessagesTopicHistory()
  : saved_messages_topic_id_()
{}

deleteSavedMessagesTopicHistory::deleteSavedMessagesTopicHistory(int53 saved_messages_topic_id_)
  : saved_messages_topic_id_(saved_messages_topic_id_)
{}

const std::int32_t deleteSavedMessagesTopicHistory::ID;

void deleteSavedMessagesTopicHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteSavedMessagesTopicHistory");
    s.store_field("saved_messages_topic_id", saved_messages_topic_id_);
    s.store_class_end();
  }
}

editInlineMessageText::editInlineMessageText()
  : inline_message_id_()
  , reply_markup_()
  , input_message_content_()
{}

editInlineMessageText::editInlineMessageText(string const &inline_message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_)
  : inline_message_id_(inline_message_id_)
  , reply_markup_(std::move(reply_markup_))
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t editInlineMessageText::ID;

void editInlineMessageText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editInlineMessageText");
    s.store_field("inline_message_id", inline_message_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

editMessageChecklist::editMessageChecklist()
  : chat_id_()
  , message_id_()
  , reply_markup_()
  , checklist_()
{}

editMessageChecklist::editMessageChecklist(int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<inputChecklist> &&checklist_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , reply_markup_(std::move(reply_markup_))
  , checklist_(std::move(checklist_))
{}

const std::int32_t editMessageChecklist::ID;

void editMessageChecklist::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editMessageChecklist");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("checklist", static_cast<const BaseObject *>(checklist_.get()));
    s.store_class_end();
  }
}

finishFileGeneration::finishFileGeneration()
  : generation_id_()
  , error_()
{}

finishFileGeneration::finishFileGeneration(int64 generation_id_, object_ptr<error> &&error_)
  : generation_id_(generation_id_)
  , error_(std::move(error_))
{}

const std::int32_t finishFileGeneration::ID;

void finishFileGeneration::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "finishFileGeneration");
    s.store_field("generation_id", generation_id_);
    s.store_object_field("error", static_cast<const BaseObject *>(error_.get()));
    s.store_class_end();
  }
}

getBlockedMessageSenders::getBlockedMessageSenders()
  : block_list_()
  , offset_()
  , limit_()
{}

getBlockedMessageSenders::getBlockedMessageSenders(object_ptr<BlockList> &&block_list_, int32 offset_, int32 limit_)
  : block_list_(std::move(block_list_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getBlockedMessageSenders::ID;

void getBlockedMessageSenders::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getBlockedMessageSenders");
    s.store_object_field("block_list", static_cast<const BaseObject *>(block_list_.get()));
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getBotInfoShortDescription::getBotInfoShortDescription()
  : bot_user_id_()
  , language_code_()
{}

getBotInfoShortDescription::getBotInfoShortDescription(int53 bot_user_id_, string const &language_code_)
  : bot_user_id_(bot_user_id_)
  , language_code_(language_code_)
{}

const std::int32_t getBotInfoShortDescription::ID;

void getBotInfoShortDescription::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getBotInfoShortDescription");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("language_code", language_code_);
    s.store_class_end();
  }
}

getBusinessFeatures::getBusinessFeatures()
  : source_()
{}

getBusinessFeatures::getBusinessFeatures(object_ptr<BusinessFeature> &&source_)
  : source_(std::move(source_))
{}

const std::int32_t getBusinessFeatures::ID;

void getBusinessFeatures::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getBusinessFeatures");
    s.store_object_field("source", static_cast<const BaseObject *>(source_.get()));
    s.store_class_end();
  }
}

getChatFolderNewChats::getChatFolderNewChats()
  : chat_folder_id_()
{}

getChatFolderNewChats::getChatFolderNewChats(int32 chat_folder_id_)
  : chat_folder_id_(chat_folder_id_)
{}

const std::int32_t getChatFolderNewChats::ID;

void getChatFolderNewChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatFolderNewChats");
    s.store_field("chat_folder_id", chat_folder_id_);
    s.store_class_end();
  }
}

getChatInviteLink::getChatInviteLink()
  : chat_id_()
  , invite_link_()
{}

getChatInviteLink::getChatInviteLink(int53 chat_id_, string const &invite_link_)
  : chat_id_(chat_id_)
  , invite_link_(invite_link_)
{}

const std::int32_t getChatInviteLink::ID;

void getChatInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatInviteLink");
    s.store_field("chat_id", chat_id_);
    s.store_field("invite_link", invite_link_);
    s.store_class_end();
  }
}

getChatRevenueTransactions::getChatRevenueTransactions()
  : chat_id_()
  , offset_()
  , limit_()
{}

getChatRevenueTransactions::getChatRevenueTransactions(int53 chat_id_, string const &offset_, int32 limit_)
  : chat_id_(chat_id_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getChatRevenueTransactions::ID;

void getChatRevenueTransactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatRevenueTransactions");
    s.store_field("chat_id", chat_id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getChatStoryAlbums::getChatStoryAlbums()
  : chat_id_()
{}

getChatStoryAlbums::getChatStoryAlbums(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t getChatStoryAlbums::ID;

void getChatStoryAlbums::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatStoryAlbums");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

getChats::getChats()
  : chat_list_()
  , limit_()
{}

getChats::getChats(object_ptr<ChatList> &&chat_list_, int32 limit_)
  : chat_list_(std::move(chat_list_))
  , limit_(limit_)
{}

const std::int32_t getChats::ID;

void getChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChats");
    s.store_object_field("chat_list", static_cast<const BaseObject *>(chat_list_.get()));
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getChatsForChatFolderInviteLink::getChatsForChatFolderInviteLink()
  : chat_folder_id_()
{}

getChatsForChatFolderInviteLink::getChatsForChatFolderInviteLink(int32 chat_folder_id_)
  : chat_folder_id_(chat_folder_id_)
{}

const std::int32_t getChatsForChatFolderInviteLink::ID;

void getChatsForChatFolderInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatsForChatFolderInviteLink");
    s.store_field("chat_folder_id", chat_folder_id_);
    s.store_class_end();
  }
}

getCurrentWeather::getCurrentWeather()
  : location_()
{}

getCurrentWeather::getCurrentWeather(object_ptr<location> &&location_)
  : location_(std::move(location_))
{}

const std::int32_t getCurrentWeather::ID;

void getCurrentWeather::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getCurrentWeather");
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_class_end();
  }
}

getDefaultMessageAutoDeleteTime::getDefaultMessageAutoDeleteTime() {
}

const std::int32_t getDefaultMessageAutoDeleteTime::ID;

void getDefaultMessageAutoDeleteTime::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getDefaultMessageAutoDeleteTime");
    s.store_class_end();
  }
}

getExternalLinkInfo::getExternalLinkInfo()
  : link_()
{}

getExternalLinkInfo::getExternalLinkInfo(string const &link_)
  : link_(link_)
{}

const std::int32_t getExternalLinkInfo::ID;

void getExternalLinkInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getExternalLinkInfo");
    s.store_field("link", link_);
    s.store_class_end();
  }
}

getForumTopics::getForumTopics()
  : chat_id_()
  , query_()
  , offset_date_()
  , offset_message_id_()
  , offset_forum_topic_id_()
  , limit_()
{}

getForumTopics::getForumTopics(int53 chat_id_, string const &query_, int32 offset_date_, int53 offset_message_id_, int32 offset_forum_topic_id_, int32 limit_)
  : chat_id_(chat_id_)
  , query_(query_)
  , offset_date_(offset_date_)
  , offset_message_id_(offset_message_id_)
  , offset_forum_topic_id_(offset_forum_topic_id_)
  , limit_(limit_)
{}

const std::int32_t getForumTopics::ID;

void getForumTopics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getForumTopics");
    s.store_field("chat_id", chat_id_);
    s.store_field("query", query_);
    s.store_field("offset_date", offset_date_);
    s.store_field("offset_message_id", offset_message_id_);
    s.store_field("offset_forum_topic_id", offset_forum_topic_id_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getGameHighScores::getGameHighScores()
  : chat_id_()
  , message_id_()
  , user_id_()
{}

getGameHighScores::getGameHighScores(int53 chat_id_, int53 message_id_, int53 user_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , user_id_(user_id_)
{}

const std::int32_t getGameHighScores::ID;

void getGameHighScores::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getGameHighScores");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

getInternalLinkType::getInternalLinkType()
  : link_()
{}

getInternalLinkType::getInternalLinkType(string const &link_)
  : link_(link_)
{}

const std::int32_t getInternalLinkType::ID;

void getInternalLinkType::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getInternalLinkType");
    s.store_field("link", link_);
    s.store_class_end();
  }
}

getJsonValue::getJsonValue()
  : json_()
{}

getJsonValue::getJsonValue(string const &json_)
  : json_(json_)
{}

const std::int32_t getJsonValue::ID;

void getJsonValue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getJsonValue");
    s.store_field("json", json_);
    s.store_class_end();
  }
}

getLogTagVerbosityLevel::getLogTagVerbosityLevel()
  : tag_()
{}

getLogTagVerbosityLevel::getLogTagVerbosityLevel(string const &tag_)
  : tag_(tag_)
{}

const std::int32_t getLogTagVerbosityLevel::ID;

void getLogTagVerbosityLevel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getLogTagVerbosityLevel");
    s.store_field("tag", tag_);
    s.store_class_end();
  }
}

getMessageEmbeddingCode::getMessageEmbeddingCode()
  : chat_id_()
  , message_id_()
  , for_album_()
{}

getMessageEmbeddingCode::getMessageEmbeddingCode(int53 chat_id_, int53 message_id_, bool for_album_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , for_album_(for_album_)
{}

const std::int32_t getMessageEmbeddingCode::ID;

void getMessageEmbeddingCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMessageEmbeddingCode");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("for_album", for_album_);
    s.store_class_end();
  }
}

getMessageReadDate::getMessageReadDate()
  : chat_id_()
  , message_id_()
{}

getMessageReadDate::getMessageReadDate(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t getMessageReadDate::ID;

void getMessageReadDate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMessageReadDate");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

getPremiumGiftPaymentOptions::getPremiumGiftPaymentOptions() {
}

const std::int32_t getPremiumGiftPaymentOptions::ID;

void getPremiumGiftPaymentOptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPremiumGiftPaymentOptions");
    s.store_class_end();
  }
}

getReadDatePrivacySettings::getReadDatePrivacySettings() {
}

const std::int32_t getReadDatePrivacySettings::ID;

void getReadDatePrivacySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getReadDatePrivacySettings");
    s.store_class_end();
  }
}

getStarGiveawayPaymentOptions::getStarGiveawayPaymentOptions() {
}

const std::int32_t getStarGiveawayPaymentOptions::ID;

void getStarGiveawayPaymentOptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStarGiveawayPaymentOptions");
    s.store_class_end();
  }
}

getStarRevenueStatistics::getStarRevenueStatistics()
  : owner_id_()
  , is_dark_()
{}

getStarRevenueStatistics::getStarRevenueStatistics(object_ptr<MessageSender> &&owner_id_, bool is_dark_)
  : owner_id_(std::move(owner_id_))
  , is_dark_(is_dark_)
{}

const std::int32_t getStarRevenueStatistics::ID;

void getStarRevenueStatistics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStarRevenueStatistics");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_field("is_dark", is_dark_);
    s.store_class_end();
  }
}

getStoryAlbumStories::getStoryAlbumStories()
  : chat_id_()
  , story_album_id_()
  , offset_()
  , limit_()
{}

getStoryAlbumStories::getStoryAlbumStories(int53 chat_id_, int32 story_album_id_, int32 offset_, int32 limit_)
  : chat_id_(chat_id_)
  , story_album_id_(story_album_id_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getStoryAlbumStories::ID;

void getStoryAlbumStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStoryAlbumStories");
    s.store_field("chat_id", chat_id_);
    s.store_field("story_album_id", story_album_id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getSupergroupFullInfo::getSupergroupFullInfo()
  : supergroup_id_()
{}

getSupergroupFullInfo::getSupergroupFullInfo(int53 supergroup_id_)
  : supergroup_id_(supergroup_id_)
{}

const std::int32_t getSupergroupFullInfo::ID;

void getSupergroupFullInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getSupergroupFullInfo");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_class_end();
  }
}

inviteGroupCallParticipant::inviteGroupCallParticipant()
  : group_call_id_()
  , user_id_()
  , is_video_()
{}

inviteGroupCallParticipant::inviteGroupCallParticipant(int32 group_call_id_, int53 user_id_, bool is_video_)
  : group_call_id_(group_call_id_)
  , user_id_(user_id_)
  , is_video_(is_video_)
{}

const std::int32_t inviteGroupCallParticipant::ID;

void inviteGroupCallParticipant::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inviteGroupCallParticipant");
    s.store_field("group_call_id", group_call_id_);
    s.store_field("user_id", user_id_);
    s.store_field("is_video", is_video_);
    s.store_class_end();
  }
}

leaveChat::leaveChat()
  : chat_id_()
{}

leaveChat::leaveChat(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t leaveChat::ID;

void leaveChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "leaveChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

readFilePart::readFilePart()
  : file_id_()
  , offset_()
  , count_()
{}

readFilePart::readFilePart(int32 file_id_, int53 offset_, int53 count_)
  : file_id_(file_id_)
  , offset_(offset_)
  , count_(count_)
{}

const std::int32_t readFilePart::ID;

void readFilePart::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "readFilePart");
    s.store_field("file_id", file_id_);
    s.store_field("offset", offset_);
    s.store_field("count", count_);
    s.store_class_end();
  }
}

removeProfileAudio::removeProfileAudio()
  : file_id_()
{}

removeProfileAudio::removeProfileAudio(int32 file_id_)
  : file_id_(file_id_)
{}

const std::int32_t removeProfileAudio::ID;

void removeProfileAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeProfileAudio");
    s.store_field("file_id", file_id_);
    s.store_class_end();
  }
}

removeRecentHashtag::removeRecentHashtag()
  : hashtag_()
{}

removeRecentHashtag::removeRecentHashtag(string const &hashtag_)
  : hashtag_(hashtag_)
{}

const std::int32_t removeRecentHashtag::ID;

void removeRecentHashtag::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeRecentHashtag");
    s.store_field("hashtag", hashtag_);
    s.store_class_end();
  }
}

reorderBotMediaPreviews::reorderBotMediaPreviews()
  : bot_user_id_()
  , language_code_()
  , file_ids_()
{}

reorderBotMediaPreviews::reorderBotMediaPreviews(int53 bot_user_id_, string const &language_code_, array<int32> &&file_ids_)
  : bot_user_id_(bot_user_id_)
  , language_code_(language_code_)
  , file_ids_(std::move(file_ids_))
{}

const std::int32_t reorderBotMediaPreviews::ID;

void reorderBotMediaPreviews::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reorderBotMediaPreviews");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("language_code", language_code_);
    { s.store_vector_begin("file_ids", file_ids_.size()); for (const auto &_value : file_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

reorderQuickReplyShortcuts::reorderQuickReplyShortcuts()
  : shortcut_ids_()
{}

reorderQuickReplyShortcuts::reorderQuickReplyShortcuts(array<int32> &&shortcut_ids_)
  : shortcut_ids_(std::move(shortcut_ids_))
{}

const std::int32_t reorderQuickReplyShortcuts::ID;

void reorderQuickReplyShortcuts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reorderQuickReplyShortcuts");
    { s.store_vector_begin("shortcut_ids", shortcut_ids_.size()); for (const auto &_value : shortcut_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

replacePrimaryChatInviteLink::replacePrimaryChatInviteLink()
  : chat_id_()
{}

replacePrimaryChatInviteLink::replacePrimaryChatInviteLink(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t replacePrimaryChatInviteLink::ID;

void replacePrimaryChatInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "replacePrimaryChatInviteLink");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

replaceVideoChatRtmpUrl::replaceVideoChatRtmpUrl()
  : chat_id_()
{}

replaceVideoChatRtmpUrl::replaceVideoChatRtmpUrl(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t replaceVideoChatRtmpUrl::ID;

void replaceVideoChatRtmpUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "replaceVideoChatRtmpUrl");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

reportStory::reportStory()
  : story_poster_chat_id_()
  , story_id_()
  , option_id_()
  , text_()
{}

reportStory::reportStory(int53 story_poster_chat_id_, int32 story_id_, bytes const &option_id_, string const &text_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , story_id_(story_id_)
  , option_id_(std::move(option_id_))
  , text_(text_)
{}

const std::int32_t reportStory::ID;

void reportStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportStory");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("story_id", story_id_);
    s.store_bytes_field("option_id", option_id_);
    s.store_field("text", text_);
    s.store_class_end();
  }
}

resendEmailAddressVerificationCode::resendEmailAddressVerificationCode() {
}

const std::int32_t resendEmailAddressVerificationCode::ID;

void resendEmailAddressVerificationCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "resendEmailAddressVerificationCode");
    s.store_class_end();
  }
}

searchChatsOnServer::searchChatsOnServer()
  : query_()
  , limit_()
{}

searchChatsOnServer::searchChatsOnServer(string const &query_, int32 limit_)
  : query_(query_)
  , limit_(limit_)
{}

const std::int32_t searchChatsOnServer::ID;

void searchChatsOnServer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchChatsOnServer");
    s.store_field("query", query_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

searchPublicChat::searchPublicChat()
  : username_()
{}

searchPublicChat::searchPublicChat(string const &username_)
  : username_(username_)
{}

const std::int32_t searchPublicChat::ID;

void searchPublicChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchPublicChat");
    s.store_field("username", username_);
    s.store_class_end();
  }
}

searchUserByPhoneNumber::searchUserByPhoneNumber()
  : phone_number_()
  , only_local_()
{}

searchUserByPhoneNumber::searchUserByPhoneNumber(string const &phone_number_, bool only_local_)
  : phone_number_(phone_number_)
  , only_local_(only_local_)
{}

const std::int32_t searchUserByPhoneNumber::ID;

void searchUserByPhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchUserByPhoneNumber");
    s.store_field("phone_number", phone_number_);
    s.store_field("only_local", only_local_);
    s.store_class_end();
  }
}

searchUserByToken::searchUserByToken()
  : token_()
{}

searchUserByToken::searchUserByToken(string const &token_)
  : token_(token_)
{}

const std::int32_t searchUserByToken::ID;

void searchUserByToken::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchUserByToken");
    s.store_field("token", token_);
    s.store_class_end();
  }
}

sendCallDebugInformation::sendCallDebugInformation()
  : call_id_()
  , debug_information_()
{}

sendCallDebugInformation::sendCallDebugInformation(int32 call_id_, string const &debug_information_)
  : call_id_(call_id_)
  , debug_information_(debug_information_)
{}

const std::int32_t sendCallDebugInformation::ID;

void sendCallDebugInformation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendCallDebugInformation");
    s.store_field("call_id", call_id_);
    s.store_field("debug_information", debug_information_);
    s.store_class_end();
  }
}

sendPhoneNumberFirebaseSms::sendPhoneNumberFirebaseSms()
  : token_()
{}

sendPhoneNumberFirebaseSms::sendPhoneNumberFirebaseSms(string const &token_)
  : token_(token_)
{}

const std::int32_t sendPhoneNumberFirebaseSms::ID;

void sendPhoneNumberFirebaseSms::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendPhoneNumberFirebaseSms");
    s.store_field("token", token_);
    s.store_class_end();
  }
}

sendTextMessageDraft::sendTextMessageDraft()
  : chat_id_()
  , forum_topic_id_()
  , draft_id_()
  , text_()
{}

sendTextMessageDraft::sendTextMessageDraft(int53 chat_id_, int32 forum_topic_id_, int64 draft_id_, object_ptr<formattedText> &&text_)
  : chat_id_(chat_id_)
  , forum_topic_id_(forum_topic_id_)
  , draft_id_(draft_id_)
  , text_(std::move(text_))
{}

const std::int32_t sendTextMessageDraft::ID;

void sendTextMessageDraft::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendTextMessageDraft");
    s.store_field("chat_id", chat_id_);
    s.store_field("forum_topic_id", forum_topic_id_);
    s.store_field("draft_id", draft_id_);
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

setAccountTtl::setAccountTtl()
  : ttl_()
{}

setAccountTtl::setAccountTtl(object_ptr<accountTtl> &&ttl_)
  : ttl_(std::move(ttl_))
{}

const std::int32_t setAccountTtl::ID;

void setAccountTtl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setAccountTtl");
    s.store_object_field("ttl", static_cast<const BaseObject *>(ttl_.get()));
    s.store_class_end();
  }
}

setAlarm::setAlarm()
  : seconds_()
{}

setAlarm::setAlarm(double seconds_)
  : seconds_(seconds_)
{}

const std::int32_t setAlarm::ID;

void setAlarm::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setAlarm");
    s.store_field("seconds", seconds_);
    s.store_class_end();
  }
}

setBusinessGreetingMessageSettings::setBusinessGreetingMessageSettings()
  : greeting_message_settings_()
{}

setBusinessGreetingMessageSettings::setBusinessGreetingMessageSettings(object_ptr<businessGreetingMessageSettings> &&greeting_message_settings_)
  : greeting_message_settings_(std::move(greeting_message_settings_))
{}

const std::int32_t setBusinessGreetingMessageSettings::ID;

void setBusinessGreetingMessageSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setBusinessGreetingMessageSettings");
    s.store_object_field("greeting_message_settings", static_cast<const BaseObject *>(greeting_message_settings_.get()));
    s.store_class_end();
  }
}

setBusinessMessageIsPinned::setBusinessMessageIsPinned()
  : business_connection_id_()
  , chat_id_()
  , message_id_()
  , is_pinned_()
{}

setBusinessMessageIsPinned::setBusinessMessageIsPinned(string const &business_connection_id_, int53 chat_id_, int53 message_id_, bool is_pinned_)
  : business_connection_id_(business_connection_id_)
  , chat_id_(chat_id_)
  , message_id_(message_id_)
  , is_pinned_(is_pinned_)
{}

const std::int32_t setBusinessMessageIsPinned::ID;

void setBusinessMessageIsPinned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setBusinessMessageIsPinned");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

setChatAccentColor::setChatAccentColor()
  : chat_id_()
  , accent_color_id_()
  , background_custom_emoji_id_()
{}

setChatAccentColor::setChatAccentColor(int53 chat_id_, int32 accent_color_id_, int64 background_custom_emoji_id_)
  : chat_id_(chat_id_)
  , accent_color_id_(accent_color_id_)
  , background_custom_emoji_id_(background_custom_emoji_id_)
{}

const std::int32_t setChatAccentColor::ID;

void setChatAccentColor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatAccentColor");
    s.store_field("chat_id", chat_id_);
    s.store_field("accent_color_id", accent_color_id_);
    s.store_field("background_custom_emoji_id", background_custom_emoji_id_);
    s.store_class_end();
  }
}

setChatClientData::setChatClientData()
  : chat_id_()
  , client_data_()
{}

setChatClientData::setChatClientData(int53 chat_id_, string const &client_data_)
  : chat_id_(chat_id_)
  , client_data_(client_data_)
{}

const std::int32_t setChatClientData::ID;

void setChatClientData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatClientData");
    s.store_field("chat_id", chat_id_);
    s.store_field("client_data", client_data_);
    s.store_class_end();
  }
}

setChatTheme::setChatTheme()
  : chat_id_()
  , theme_()
{}

setChatTheme::setChatTheme(int53 chat_id_, object_ptr<InputChatTheme> &&theme_)
  : chat_id_(chat_id_)
  , theme_(std::move(theme_))
{}

const std::int32_t setChatTheme::ID;

void setChatTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatTheme");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("theme", static_cast<const BaseObject *>(theme_.get()));
    s.store_class_end();
  }
}

setCustomLanguagePackString::setCustomLanguagePackString()
  : language_pack_id_()
  , new_string_()
{}

setCustomLanguagePackString::setCustomLanguagePackString(string const &language_pack_id_, object_ptr<languagePackString> &&new_string_)
  : language_pack_id_(language_pack_id_)
  , new_string_(std::move(new_string_))
{}

const std::int32_t setCustomLanguagePackString::ID;

void setCustomLanguagePackString::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setCustomLanguagePackString");
    s.store_field("language_pack_id", language_pack_id_);
    s.store_object_field("new_string", static_cast<const BaseObject *>(new_string_.get()));
    s.store_class_end();
  }
}

setGiftCollectionName::setGiftCollectionName()
  : owner_id_()
  , collection_id_()
  , name_()
{}

setGiftCollectionName::setGiftCollectionName(object_ptr<MessageSender> &&owner_id_, int32 collection_id_, string const &name_)
  : owner_id_(std::move(owner_id_))
  , collection_id_(collection_id_)
  , name_(name_)
{}

const std::int32_t setGiftCollectionName::ID;

void setGiftCollectionName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setGiftCollectionName");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_field("collection_id", collection_id_);
    s.store_field("name", name_);
    s.store_class_end();
  }
}

setLogTagVerbosityLevel::setLogTagVerbosityLevel()
  : tag_()
  , new_verbosity_level_()
{}

setLogTagVerbosityLevel::setLogTagVerbosityLevel(string const &tag_, int32 new_verbosity_level_)
  : tag_(tag_)
  , new_verbosity_level_(new_verbosity_level_)
{}

const std::int32_t setLogTagVerbosityLevel::ID;

void setLogTagVerbosityLevel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setLogTagVerbosityLevel");
    s.store_field("tag", tag_);
    s.store_field("new_verbosity_level", new_verbosity_level_);
    s.store_class_end();
  }
}

setPinnedGifts::setPinnedGifts()
  : owner_id_()
  , received_gift_ids_()
{}

setPinnedGifts::setPinnedGifts(object_ptr<MessageSender> &&owner_id_, array<string> &&received_gift_ids_)
  : owner_id_(std::move(owner_id_))
  , received_gift_ids_(std::move(received_gift_ids_))
{}

const std::int32_t setPinnedGifts::ID;

void setPinnedGifts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setPinnedGifts");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    { s.store_vector_begin("received_gift_ids", received_gift_ids_.size()); for (const auto &_value : received_gift_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

setProfilePhoto::setProfilePhoto()
  : photo_()
  , is_public_()
{}

setProfilePhoto::setProfilePhoto(object_ptr<InputChatPhoto> &&photo_, bool is_public_)
  : photo_(std::move(photo_))
  , is_public_(is_public_)
{}

const std::int32_t setProfilePhoto::ID;

void setProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setProfilePhoto");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_field("is_public", is_public_);
    s.store_class_end();
  }
}

setSupergroupMainProfileTab::setSupergroupMainProfileTab()
  : supergroup_id_()
  , main_profile_tab_()
{}

setSupergroupMainProfileTab::setSupergroupMainProfileTab(int53 supergroup_id_, object_ptr<ProfileTab> &&main_profile_tab_)
  : supergroup_id_(supergroup_id_)
  , main_profile_tab_(std::move(main_profile_tab_))
{}

const std::int32_t setSupergroupMainProfileTab::ID;

void setSupergroupMainProfileTab::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setSupergroupMainProfileTab");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_object_field("main_profile_tab", static_cast<const BaseObject *>(main_profile_tab_.get()));
    s.store_class_end();
  }
}

setUserNote::setUserNote()
  : user_id_()
  , note_()
{}

setUserNote::setUserNote(int53 user_id_, object_ptr<formattedText> &&note_)
  : user_id_(user_id_)
  , note_(std::move(note_))
{}

const std::int32_t setUserNote::ID;

void setUserNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setUserNote");
    s.store_field("user_id", user_id_);
    s.store_object_field("note", static_cast<const BaseObject *>(note_.get()));
    s.store_class_end();
  }
}

setVideoChatTitle::setVideoChatTitle()
  : group_call_id_()
  , title_()
{}

setVideoChatTitle::setVideoChatTitle(int32 group_call_id_, string const &title_)
  : group_call_id_(group_call_id_)
  , title_(title_)
{}

const std::int32_t setVideoChatTitle::ID;

void setVideoChatTitle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setVideoChatTitle");
    s.store_field("group_call_id", group_call_id_);
    s.store_field("title", title_);
    s.store_class_end();
  }
}

stopBusinessPoll::stopBusinessPoll()
  : business_connection_id_()
  , chat_id_()
  , message_id_()
  , reply_markup_()
{}

stopBusinessPoll::stopBusinessPoll(string const &business_connection_id_, int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_)
  : business_connection_id_(business_connection_id_)
  , chat_id_(chat_id_)
  , message_id_(message_id_)
  , reply_markup_(std::move(reply_markup_))
{}

const std::int32_t stopBusinessPoll::ID;

void stopBusinessPoll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stopBusinessPoll");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_class_end();
  }
}

testCallVectorStringObject::testCallVectorStringObject()
  : x_()
{}

testCallVectorStringObject::testCallVectorStringObject(array<object_ptr<testString>> &&x_)
  : x_(std::move(x_))
{}

const std::int32_t testCallVectorStringObject::ID;

void testCallVectorStringObject::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testCallVectorStringObject");
    { s.store_vector_begin("x", x_.size()); for (const auto &_value : x_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

toggleChatIsMarkedAsUnread::toggleChatIsMarkedAsUnread()
  : chat_id_()
  , is_marked_as_unread_()
{}

toggleChatIsMarkedAsUnread::toggleChatIsMarkedAsUnread(int53 chat_id_, bool is_marked_as_unread_)
  : chat_id_(chat_id_)
  , is_marked_as_unread_(is_marked_as_unread_)
{}

const std::int32_t toggleChatIsMarkedAsUnread::ID;

void toggleChatIsMarkedAsUnread::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleChatIsMarkedAsUnread");
    s.store_field("chat_id", chat_id_);
    s.store_field("is_marked_as_unread", is_marked_as_unread_);
    s.store_class_end();
  }
}

transferGift::transferGift()
  : business_connection_id_()
  , received_gift_id_()
  , new_owner_id_()
  , star_count_()
{}

transferGift::transferGift(string const &business_connection_id_, string const &received_gift_id_, object_ptr<MessageSender> &&new_owner_id_, int53 star_count_)
  : business_connection_id_(business_connection_id_)
  , received_gift_id_(received_gift_id_)
  , new_owner_id_(std::move(new_owner_id_))
  , star_count_(star_count_)
{}

const std::int32_t transferGift::ID;

void transferGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "transferGift");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_field("received_gift_id", received_gift_id_);
    s.store_object_field("new_owner_id", static_cast<const BaseObject *>(new_owner_id_.get()));
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

unpinAllDirectMessagesChatTopicMessages::unpinAllDirectMessagesChatTopicMessages()
  : chat_id_()
  , topic_id_()
{}

unpinAllDirectMessagesChatTopicMessages::unpinAllDirectMessagesChatTopicMessages(int53 chat_id_, int53 topic_id_)
  : chat_id_(chat_id_)
  , topic_id_(topic_id_)
{}

const std::int32_t unpinAllDirectMessagesChatTopicMessages::ID;

void unpinAllDirectMessagesChatTopicMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "unpinAllDirectMessagesChatTopicMessages");
    s.store_field("chat_id", chat_id_);
    s.store_field("topic_id", topic_id_);
    s.store_class_end();
  }
}
}  // namespace td_api
}  // namespace td
