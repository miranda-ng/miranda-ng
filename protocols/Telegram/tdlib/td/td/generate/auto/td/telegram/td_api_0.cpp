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

std::string to_string(const BaseObject &value) {
  TlStorerToString storer;
  value.store(storer, "");
  return storer.move_as_string();
}

accountTtl::accountTtl()
  : days_()
{}

accountTtl::accountTtl(int32 days_)
  : days_(days_)
{}

const std::int32_t accountTtl::ID;

void accountTtl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "accountTtl");
    s.store_field("days", days_);
    s.store_class_end();
  }
}

affiliateProgramParameters::affiliateProgramParameters()
  : commission_per_mille_()
  , month_count_()
{}

affiliateProgramParameters::affiliateProgramParameters(int32 commission_per_mille_, int32 month_count_)
  : commission_per_mille_(commission_per_mille_)
  , month_count_(month_count_)
{}

const std::int32_t affiliateProgramParameters::ID;

void affiliateProgramParameters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "affiliateProgramParameters");
    s.store_field("commission_per_mille", commission_per_mille_);
    s.store_field("month_count", month_count_);
    s.store_class_end();
  }
}

animations::animations()
  : animations_()
{}

animations::animations(array<object_ptr<animation>> &&animations_)
  : animations_(std::move(animations_))
{}

const std::int32_t animations::ID;

void animations::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "animations");
    { s.store_vector_begin("animations", animations_.size()); for (const auto &_value : animations_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

archiveChatListSettings::archiveChatListSettings()
  : archive_and_mute_new_chats_from_unknown_users_()
  , keep_unmuted_chats_archived_()
  , keep_chats_from_folders_archived_()
{}

archiveChatListSettings::archiveChatListSettings(bool archive_and_mute_new_chats_from_unknown_users_, bool keep_unmuted_chats_archived_, bool keep_chats_from_folders_archived_)
  : archive_and_mute_new_chats_from_unknown_users_(archive_and_mute_new_chats_from_unknown_users_)
  , keep_unmuted_chats_archived_(keep_unmuted_chats_archived_)
  , keep_chats_from_folders_archived_(keep_chats_from_folders_archived_)
{}

const std::int32_t archiveChatListSettings::ID;

void archiveChatListSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "archiveChatListSettings");
    s.store_field("archive_and_mute_new_chats_from_unknown_users", archive_and_mute_new_chats_from_unknown_users_);
    s.store_field("keep_unmuted_chats_archived", keep_unmuted_chats_archived_);
    s.store_field("keep_chats_from_folders_archived", keep_chats_from_folders_archived_);
    s.store_class_end();
  }
}

attachmentMenuBot::attachmentMenuBot()
  : bot_user_id_()
  , supports_self_chat_()
  , supports_user_chats_()
  , supports_bot_chats_()
  , supports_group_chats_()
  , supports_channel_chats_()
  , request_write_access_()
  , is_added_()
  , show_in_attachment_menu_()
  , show_in_side_menu_()
  , show_disclaimer_in_side_menu_()
  , name_()
  , name_color_()
  , default_icon_()
  , ios_static_icon_()
  , ios_animated_icon_()
  , ios_side_menu_icon_()
  , android_icon_()
  , android_side_menu_icon_()
  , macos_icon_()
  , macos_side_menu_icon_()
  , icon_color_()
  , web_app_placeholder_()
{}

attachmentMenuBot::attachmentMenuBot(int53 bot_user_id_, bool supports_self_chat_, bool supports_user_chats_, bool supports_bot_chats_, bool supports_group_chats_, bool supports_channel_chats_, bool request_write_access_, bool is_added_, bool show_in_attachment_menu_, bool show_in_side_menu_, bool show_disclaimer_in_side_menu_, string const &name_, object_ptr<attachmentMenuBotColor> &&name_color_, object_ptr<file> &&default_icon_, object_ptr<file> &&ios_static_icon_, object_ptr<file> &&ios_animated_icon_, object_ptr<file> &&ios_side_menu_icon_, object_ptr<file> &&android_icon_, object_ptr<file> &&android_side_menu_icon_, object_ptr<file> &&macos_icon_, object_ptr<file> &&macos_side_menu_icon_, object_ptr<attachmentMenuBotColor> &&icon_color_, object_ptr<file> &&web_app_placeholder_)
  : bot_user_id_(bot_user_id_)
  , supports_self_chat_(supports_self_chat_)
  , supports_user_chats_(supports_user_chats_)
  , supports_bot_chats_(supports_bot_chats_)
  , supports_group_chats_(supports_group_chats_)
  , supports_channel_chats_(supports_channel_chats_)
  , request_write_access_(request_write_access_)
  , is_added_(is_added_)
  , show_in_attachment_menu_(show_in_attachment_menu_)
  , show_in_side_menu_(show_in_side_menu_)
  , show_disclaimer_in_side_menu_(show_disclaimer_in_side_menu_)
  , name_(name_)
  , name_color_(std::move(name_color_))
  , default_icon_(std::move(default_icon_))
  , ios_static_icon_(std::move(ios_static_icon_))
  , ios_animated_icon_(std::move(ios_animated_icon_))
  , ios_side_menu_icon_(std::move(ios_side_menu_icon_))
  , android_icon_(std::move(android_icon_))
  , android_side_menu_icon_(std::move(android_side_menu_icon_))
  , macos_icon_(std::move(macos_icon_))
  , macos_side_menu_icon_(std::move(macos_side_menu_icon_))
  , icon_color_(std::move(icon_color_))
  , web_app_placeholder_(std::move(web_app_placeholder_))
{}

const std::int32_t attachmentMenuBot::ID;

void attachmentMenuBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "attachmentMenuBot");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("supports_self_chat", supports_self_chat_);
    s.store_field("supports_user_chats", supports_user_chats_);
    s.store_field("supports_bot_chats", supports_bot_chats_);
    s.store_field("supports_group_chats", supports_group_chats_);
    s.store_field("supports_channel_chats", supports_channel_chats_);
    s.store_field("request_write_access", request_write_access_);
    s.store_field("is_added", is_added_);
    s.store_field("show_in_attachment_menu", show_in_attachment_menu_);
    s.store_field("show_in_side_menu", show_in_side_menu_);
    s.store_field("show_disclaimer_in_side_menu", show_disclaimer_in_side_menu_);
    s.store_field("name", name_);
    s.store_object_field("name_color", static_cast<const BaseObject *>(name_color_.get()));
    s.store_object_field("default_icon", static_cast<const BaseObject *>(default_icon_.get()));
    s.store_object_field("ios_static_icon", static_cast<const BaseObject *>(ios_static_icon_.get()));
    s.store_object_field("ios_animated_icon", static_cast<const BaseObject *>(ios_animated_icon_.get()));
    s.store_object_field("ios_side_menu_icon", static_cast<const BaseObject *>(ios_side_menu_icon_.get()));
    s.store_object_field("android_icon", static_cast<const BaseObject *>(android_icon_.get()));
    s.store_object_field("android_side_menu_icon", static_cast<const BaseObject *>(android_side_menu_icon_.get()));
    s.store_object_field("macos_icon", static_cast<const BaseObject *>(macos_icon_.get()));
    s.store_object_field("macos_side_menu_icon", static_cast<const BaseObject *>(macos_side_menu_icon_.get()));
    s.store_object_field("icon_color", static_cast<const BaseObject *>(icon_color_.get()));
    s.store_object_field("web_app_placeholder", static_cast<const BaseObject *>(web_app_placeholder_.get()));
    s.store_class_end();
  }
}

audios::audios()
  : total_count_()
  , audios_()
{}

audios::audios(int32 total_count_, array<object_ptr<audio>> &&audios_)
  : total_count_(total_count_)
  , audios_(std::move(audios_))
{}

const std::int32_t audios::ID;

void audios::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "audios");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("audios", audios_.size()); for (const auto &_value : audios_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

backgroundFillSolid::backgroundFillSolid()
  : color_()
{}

backgroundFillSolid::backgroundFillSolid(int32 color_)
  : color_(color_)
{}

const std::int32_t backgroundFillSolid::ID;

void backgroundFillSolid::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "backgroundFillSolid");
    s.store_field("color", color_);
    s.store_class_end();
  }
}

backgroundFillGradient::backgroundFillGradient()
  : top_color_()
  , bottom_color_()
  , rotation_angle_()
{}

backgroundFillGradient::backgroundFillGradient(int32 top_color_, int32 bottom_color_, int32 rotation_angle_)
  : top_color_(top_color_)
  , bottom_color_(bottom_color_)
  , rotation_angle_(rotation_angle_)
{}

const std::int32_t backgroundFillGradient::ID;

void backgroundFillGradient::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "backgroundFillGradient");
    s.store_field("top_color", top_color_);
    s.store_field("bottom_color", bottom_color_);
    s.store_field("rotation_angle", rotation_angle_);
    s.store_class_end();
  }
}

backgroundFillFreeformGradient::backgroundFillFreeformGradient()
  : colors_()
{}

backgroundFillFreeformGradient::backgroundFillFreeformGradient(array<int32> &&colors_)
  : colors_(std::move(colors_))
{}

const std::int32_t backgroundFillFreeformGradient::ID;

void backgroundFillFreeformGradient::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "backgroundFillFreeformGradient");
    { s.store_vector_begin("colors", colors_.size()); for (const auto &_value : colors_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

botCommands::botCommands()
  : bot_user_id_()
  , commands_()
{}

botCommands::botCommands(int53 bot_user_id_, array<object_ptr<botCommand>> &&commands_)
  : bot_user_id_(bot_user_id_)
  , commands_(std::move(commands_))
{}

const std::int32_t botCommands::ID;

void botCommands::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botCommands");
    s.store_field("bot_user_id", bot_user_id_);
    { s.store_vector_begin("commands", commands_.size()); for (const auto &_value : commands_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

botInfo::botInfo()
  : short_description_()
  , description_()
  , photo_()
  , animation_()
  , menu_button_()
  , commands_()
  , privacy_policy_url_()
  , default_group_administrator_rights_()
  , default_channel_administrator_rights_()
  , affiliate_program_()
  , web_app_background_light_color_()
  , web_app_background_dark_color_()
  , web_app_header_light_color_()
  , web_app_header_dark_color_()
  , verification_parameters_()
  , can_get_revenue_statistics_()
  , can_manage_emoji_status_()
  , has_media_previews_()
  , edit_commands_link_()
  , edit_description_link_()
  , edit_description_media_link_()
  , edit_settings_link_()
{}

botInfo::botInfo(string const &short_description_, string const &description_, object_ptr<photo> &&photo_, object_ptr<animation> &&animation_, object_ptr<botMenuButton> &&menu_button_, array<object_ptr<botCommand>> &&commands_, string const &privacy_policy_url_, object_ptr<chatAdministratorRights> &&default_group_administrator_rights_, object_ptr<chatAdministratorRights> &&default_channel_administrator_rights_, object_ptr<affiliateProgramInfo> &&affiliate_program_, int32 web_app_background_light_color_, int32 web_app_background_dark_color_, int32 web_app_header_light_color_, int32 web_app_header_dark_color_, object_ptr<botVerificationParameters> &&verification_parameters_, bool can_get_revenue_statistics_, bool can_manage_emoji_status_, bool has_media_previews_, object_ptr<InternalLinkType> &&edit_commands_link_, object_ptr<InternalLinkType> &&edit_description_link_, object_ptr<InternalLinkType> &&edit_description_media_link_, object_ptr<InternalLinkType> &&edit_settings_link_)
  : short_description_(short_description_)
  , description_(description_)
  , photo_(std::move(photo_))
  , animation_(std::move(animation_))
  , menu_button_(std::move(menu_button_))
  , commands_(std::move(commands_))
  , privacy_policy_url_(privacy_policy_url_)
  , default_group_administrator_rights_(std::move(default_group_administrator_rights_))
  , default_channel_administrator_rights_(std::move(default_channel_administrator_rights_))
  , affiliate_program_(std::move(affiliate_program_))
  , web_app_background_light_color_(web_app_background_light_color_)
  , web_app_background_dark_color_(web_app_background_dark_color_)
  , web_app_header_light_color_(web_app_header_light_color_)
  , web_app_header_dark_color_(web_app_header_dark_color_)
  , verification_parameters_(std::move(verification_parameters_))
  , can_get_revenue_statistics_(can_get_revenue_statistics_)
  , can_manage_emoji_status_(can_manage_emoji_status_)
  , has_media_previews_(has_media_previews_)
  , edit_commands_link_(std::move(edit_commands_link_))
  , edit_description_link_(std::move(edit_description_link_))
  , edit_description_media_link_(std::move(edit_description_media_link_))
  , edit_settings_link_(std::move(edit_settings_link_))
{}

const std::int32_t botInfo::ID;

void botInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botInfo");
    s.store_field("short_description", short_description_);
    s.store_field("description", description_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_object_field("animation", static_cast<const BaseObject *>(animation_.get()));
    s.store_object_field("menu_button", static_cast<const BaseObject *>(menu_button_.get()));
    { s.store_vector_begin("commands", commands_.size()); for (const auto &_value : commands_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("privacy_policy_url", privacy_policy_url_);
    s.store_object_field("default_group_administrator_rights", static_cast<const BaseObject *>(default_group_administrator_rights_.get()));
    s.store_object_field("default_channel_administrator_rights", static_cast<const BaseObject *>(default_channel_administrator_rights_.get()));
    s.store_object_field("affiliate_program", static_cast<const BaseObject *>(affiliate_program_.get()));
    s.store_field("web_app_background_light_color", web_app_background_light_color_);
    s.store_field("web_app_background_dark_color", web_app_background_dark_color_);
    s.store_field("web_app_header_light_color", web_app_header_light_color_);
    s.store_field("web_app_header_dark_color", web_app_header_dark_color_);
    s.store_object_field("verification_parameters", static_cast<const BaseObject *>(verification_parameters_.get()));
    s.store_field("can_get_revenue_statistics", can_get_revenue_statistics_);
    s.store_field("can_manage_emoji_status", can_manage_emoji_status_);
    s.store_field("has_media_previews", has_media_previews_);
    s.store_object_field("edit_commands_link", static_cast<const BaseObject *>(edit_commands_link_.get()));
    s.store_object_field("edit_description_link", static_cast<const BaseObject *>(edit_description_link_.get()));
    s.store_object_field("edit_description_media_link", static_cast<const BaseObject *>(edit_description_media_link_.get()));
    s.store_object_field("edit_settings_link", static_cast<const BaseObject *>(edit_settings_link_.get()));
    s.store_class_end();
  }
}

businessGreetingMessageSettings::businessGreetingMessageSettings()
  : shortcut_id_()
  , recipients_()
  , inactivity_days_()
{}

businessGreetingMessageSettings::businessGreetingMessageSettings(int32 shortcut_id_, object_ptr<businessRecipients> &&recipients_, int32 inactivity_days_)
  : shortcut_id_(shortcut_id_)
  , recipients_(std::move(recipients_))
  , inactivity_days_(inactivity_days_)
{}

const std::int32_t businessGreetingMessageSettings::ID;

void businessGreetingMessageSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessGreetingMessageSettings");
    s.store_field("shortcut_id", shortcut_id_);
    s.store_object_field("recipients", static_cast<const BaseObject *>(recipients_.get()));
    s.store_field("inactivity_days", inactivity_days_);
    s.store_class_end();
  }
}

businessMessage::businessMessage()
  : message_()
  , reply_to_message_()
{}

businessMessage::businessMessage(object_ptr<message> &&message_, object_ptr<message> &&reply_to_message_)
  : message_(std::move(message_))
  , reply_to_message_(std::move(reply_to_message_))
{}

const std::int32_t businessMessage::ID;

void businessMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessMessage");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_object_field("reply_to_message", static_cast<const BaseObject *>(reply_to_message_.get()));
    s.store_class_end();
  }
}

businessOpeningHours::businessOpeningHours()
  : time_zone_id_()
  , opening_hours_()
{}

businessOpeningHours::businessOpeningHours(string const &time_zone_id_, array<object_ptr<businessOpeningHoursInterval>> &&opening_hours_)
  : time_zone_id_(time_zone_id_)
  , opening_hours_(std::move(opening_hours_))
{}

const std::int32_t businessOpeningHours::ID;

void businessOpeningHours::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessOpeningHours");
    s.store_field("time_zone_id", time_zone_id_);
    { s.store_vector_begin("opening_hours", opening_hours_.size()); for (const auto &_value : opening_hours_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

callProblemEcho::callProblemEcho() {
}

const std::int32_t callProblemEcho::ID;

void callProblemEcho::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callProblemEcho");
    s.store_class_end();
  }
}

callProblemNoise::callProblemNoise() {
}

const std::int32_t callProblemNoise::ID;

void callProblemNoise::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callProblemNoise");
    s.store_class_end();
  }
}

callProblemInterruptions::callProblemInterruptions() {
}

const std::int32_t callProblemInterruptions::ID;

void callProblemInterruptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callProblemInterruptions");
    s.store_class_end();
  }
}

callProblemDistortedSpeech::callProblemDistortedSpeech() {
}

const std::int32_t callProblemDistortedSpeech::ID;

void callProblemDistortedSpeech::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callProblemDistortedSpeech");
    s.store_class_end();
  }
}

callProblemSilentLocal::callProblemSilentLocal() {
}

const std::int32_t callProblemSilentLocal::ID;

void callProblemSilentLocal::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callProblemSilentLocal");
    s.store_class_end();
  }
}

callProblemSilentRemote::callProblemSilentRemote() {
}

const std::int32_t callProblemSilentRemote::ID;

void callProblemSilentRemote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callProblemSilentRemote");
    s.store_class_end();
  }
}

callProblemDropped::callProblemDropped() {
}

const std::int32_t callProblemDropped::ID;

void callProblemDropped::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callProblemDropped");
    s.store_class_end();
  }
}

callProblemDistortedVideo::callProblemDistortedVideo() {
}

const std::int32_t callProblemDistortedVideo::ID;

void callProblemDistortedVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callProblemDistortedVideo");
    s.store_class_end();
  }
}

callProblemPixelatedVideo::callProblemPixelatedVideo() {
}

const std::int32_t callProblemPixelatedVideo::ID;

void callProblemPixelatedVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callProblemPixelatedVideo");
    s.store_class_end();
  }
}

callServer::callServer()
  : id_()
  , ip_address_()
  , ipv6_address_()
  , port_()
  , type_()
{}

callServer::callServer(int64 id_, string const &ip_address_, string const &ipv6_address_, int32 port_, object_ptr<CallServerType> &&type_)
  : id_(id_)
  , ip_address_(ip_address_)
  , ipv6_address_(ipv6_address_)
  , port_(port_)
  , type_(std::move(type_))
{}

const std::int32_t callServer::ID;

void callServer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callServer");
    s.store_field("id", id_);
    s.store_field("ip_address", ip_address_);
    s.store_field("ipv6_address", ipv6_address_);
    s.store_field("port", port_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

callbackQueryAnswer::callbackQueryAnswer()
  : text_()
  , show_alert_()
  , url_()
{}

callbackQueryAnswer::callbackQueryAnswer(string const &text_, bool show_alert_, string const &url_)
  : text_(text_)
  , show_alert_(show_alert_)
  , url_(url_)
{}

const std::int32_t callbackQueryAnswer::ID;

void callbackQueryAnswer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callbackQueryAnswer");
    s.store_field("text", text_);
    s.store_field("show_alert", show_alert_);
    s.store_field("url", url_);
    s.store_class_end();
  }
}

chatAvailableReactionsAll::chatAvailableReactionsAll()
  : max_reaction_count_()
{}

chatAvailableReactionsAll::chatAvailableReactionsAll(int32 max_reaction_count_)
  : max_reaction_count_(max_reaction_count_)
{}

const std::int32_t chatAvailableReactionsAll::ID;

void chatAvailableReactionsAll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatAvailableReactionsAll");
    s.store_field("max_reaction_count", max_reaction_count_);
    s.store_class_end();
  }
}

chatAvailableReactionsSome::chatAvailableReactionsSome()
  : reactions_()
  , max_reaction_count_()
{}

chatAvailableReactionsSome::chatAvailableReactionsSome(array<object_ptr<ReactionType>> &&reactions_, int32 max_reaction_count_)
  : reactions_(std::move(reactions_))
  , max_reaction_count_(max_reaction_count_)
{}

const std::int32_t chatAvailableReactionsSome::ID;

void chatAvailableReactionsSome::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatAvailableReactionsSome");
    { s.store_vector_begin("reactions", reactions_.size()); for (const auto &_value : reactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("max_reaction_count", max_reaction_count_);
    s.store_class_end();
  }
}

chatBoostStatus::chatBoostStatus()
  : boost_url_()
  , applied_slot_ids_()
  , level_()
  , gift_code_boost_count_()
  , boost_count_()
  , current_level_boost_count_()
  , next_level_boost_count_()
  , premium_member_count_()
  , premium_member_percentage_()
  , prepaid_giveaways_()
{}

chatBoostStatus::chatBoostStatus(string const &boost_url_, array<int32> &&applied_slot_ids_, int32 level_, int32 gift_code_boost_count_, int32 boost_count_, int32 current_level_boost_count_, int32 next_level_boost_count_, int32 premium_member_count_, double premium_member_percentage_, array<object_ptr<prepaidGiveaway>> &&prepaid_giveaways_)
  : boost_url_(boost_url_)
  , applied_slot_ids_(std::move(applied_slot_ids_))
  , level_(level_)
  , gift_code_boost_count_(gift_code_boost_count_)
  , boost_count_(boost_count_)
  , current_level_boost_count_(current_level_boost_count_)
  , next_level_boost_count_(next_level_boost_count_)
  , premium_member_count_(premium_member_count_)
  , premium_member_percentage_(premium_member_percentage_)
  , prepaid_giveaways_(std::move(prepaid_giveaways_))
{}

const std::int32_t chatBoostStatus::ID;

void chatBoostStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatBoostStatus");
    s.store_field("boost_url", boost_url_);
    { s.store_vector_begin("applied_slot_ids", applied_slot_ids_.size()); for (const auto &_value : applied_slot_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("level", level_);
    s.store_field("gift_code_boost_count", gift_code_boost_count_);
    s.store_field("boost_count", boost_count_);
    s.store_field("current_level_boost_count", current_level_boost_count_);
    s.store_field("next_level_boost_count", next_level_boost_count_);
    s.store_field("premium_member_count", premium_member_count_);
    s.store_field("premium_member_percentage", premium_member_percentage_);
    { s.store_vector_begin("prepaid_giveaways", prepaid_giveaways_.size()); for (const auto &_value : prepaid_giveaways_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatEvents::chatEvents()
  : events_()
{}

chatEvents::chatEvents(array<object_ptr<chatEvent>> &&events_)
  : events_(std::move(events_))
{}

const std::int32_t chatEvents::ID;

void chatEvents::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEvents");
    { s.store_vector_begin("events", events_.size()); for (const auto &_value : events_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatJoinRequest::chatJoinRequest()
  : user_id_()
  , date_()
  , bio_()
{}

chatJoinRequest::chatJoinRequest(int53 user_id_, int32 date_, string const &bio_)
  : user_id_(user_id_)
  , date_(date_)
  , bio_(bio_)
{}

const std::int32_t chatJoinRequest::ID;

void chatJoinRequest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatJoinRequest");
    s.store_field("user_id", user_id_);
    s.store_field("date", date_);
    s.store_field("bio", bio_);
    s.store_class_end();
  }
}

chatMembersFilterContacts::chatMembersFilterContacts() {
}

const std::int32_t chatMembersFilterContacts::ID;

void chatMembersFilterContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatMembersFilterContacts");
    s.store_class_end();
  }
}

chatMembersFilterAdministrators::chatMembersFilterAdministrators() {
}

const std::int32_t chatMembersFilterAdministrators::ID;

void chatMembersFilterAdministrators::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatMembersFilterAdministrators");
    s.store_class_end();
  }
}

chatMembersFilterMembers::chatMembersFilterMembers() {
}

const std::int32_t chatMembersFilterMembers::ID;

void chatMembersFilterMembers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatMembersFilterMembers");
    s.store_class_end();
  }
}

chatMembersFilterMention::chatMembersFilterMention()
  : topic_id_()
{}

chatMembersFilterMention::chatMembersFilterMention(object_ptr<MessageTopic> &&topic_id_)
  : topic_id_(std::move(topic_id_))
{}

const std::int32_t chatMembersFilterMention::ID;

void chatMembersFilterMention::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatMembersFilterMention");
    s.store_object_field("topic_id", static_cast<const BaseObject *>(topic_id_.get()));
    s.store_class_end();
  }
}

chatMembersFilterRestricted::chatMembersFilterRestricted() {
}

const std::int32_t chatMembersFilterRestricted::ID;

void chatMembersFilterRestricted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatMembersFilterRestricted");
    s.store_class_end();
  }
}

chatMembersFilterBanned::chatMembersFilterBanned() {
}

const std::int32_t chatMembersFilterBanned::ID;

void chatMembersFilterBanned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatMembersFilterBanned");
    s.store_class_end();
  }
}

chatMembersFilterBots::chatMembersFilterBots() {
}

const std::int32_t chatMembersFilterBots::ID;

void chatMembersFilterBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatMembersFilterBots");
    s.store_class_end();
  }
}

chatRevenueTransactions::chatRevenueTransactions()
  : ton_amount_()
  , transactions_()
  , next_offset_()
{}

chatRevenueTransactions::chatRevenueTransactions(int53 ton_amount_, array<object_ptr<chatRevenueTransaction>> &&transactions_, string const &next_offset_)
  : ton_amount_(ton_amount_)
  , transactions_(std::move(transactions_))
  , next_offset_(next_offset_)
{}

const std::int32_t chatRevenueTransactions::ID;

void chatRevenueTransactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatRevenueTransactions");
    s.store_field("ton_amount", ton_amount_);
    { s.store_vector_begin("transactions", transactions_.size()); for (const auto &_value : transactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

chatStatisticsInteractionInfo::chatStatisticsInteractionInfo()
  : object_type_()
  , view_count_()
  , forward_count_()
  , reaction_count_()
{}

chatStatisticsInteractionInfo::chatStatisticsInteractionInfo(object_ptr<ChatStatisticsObjectType> &&object_type_, int32 view_count_, int32 forward_count_, int32 reaction_count_)
  : object_type_(std::move(object_type_))
  , view_count_(view_count_)
  , forward_count_(forward_count_)
  , reaction_count_(reaction_count_)
{}

const std::int32_t chatStatisticsInteractionInfo::ID;

void chatStatisticsInteractionInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatStatisticsInteractionInfo");
    s.store_object_field("object_type", static_cast<const BaseObject *>(object_type_.get()));
    s.store_field("view_count", view_count_);
    s.store_field("forward_count", forward_count_);
    s.store_field("reaction_count", reaction_count_);
    s.store_class_end();
  }
}

collectibleItemInfo::collectibleItemInfo()
  : purchase_date_()
  , currency_()
  , amount_()
  , cryptocurrency_()
  , cryptocurrency_amount_()
  , url_()
{}

collectibleItemInfo::collectibleItemInfo(int32 purchase_date_, string const &currency_, int53 amount_, string const &cryptocurrency_, int64 cryptocurrency_amount_, string const &url_)
  : purchase_date_(purchase_date_)
  , currency_(currency_)
  , amount_(amount_)
  , cryptocurrency_(cryptocurrency_)
  , cryptocurrency_amount_(cryptocurrency_amount_)
  , url_(url_)
{}

const std::int32_t collectibleItemInfo::ID;

void collectibleItemInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "collectibleItemInfo");
    s.store_field("purchase_date", purchase_date_);
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("cryptocurrency", cryptocurrency_);
    s.store_field("cryptocurrency_amount", cryptocurrency_amount_);
    s.store_field("url", url_);
    s.store_class_end();
  }
}

connectedAffiliatePrograms::connectedAffiliatePrograms()
  : total_count_()
  , programs_()
  , next_offset_()
{}

connectedAffiliatePrograms::connectedAffiliatePrograms(int32 total_count_, array<object_ptr<connectedAffiliateProgram>> &&programs_, string const &next_offset_)
  : total_count_(total_count_)
  , programs_(std::move(programs_))
  , next_offset_(next_offset_)
{}

const std::int32_t connectedAffiliatePrograms::ID;

void connectedAffiliatePrograms::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "connectedAffiliatePrograms");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("programs", programs_.size()); for (const auto &_value : programs_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

createdBasicGroupChat::createdBasicGroupChat()
  : chat_id_()
  , failed_to_add_members_()
{}

createdBasicGroupChat::createdBasicGroupChat(int53 chat_id_, object_ptr<failedToAddMembers> &&failed_to_add_members_)
  : chat_id_(chat_id_)
  , failed_to_add_members_(std::move(failed_to_add_members_))
{}

const std::int32_t createdBasicGroupChat::ID;

void createdBasicGroupChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createdBasicGroupChat");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("failed_to_add_members", static_cast<const BaseObject *>(failed_to_add_members_.get()));
    s.store_class_end();
  }
}

diceStickersRegular::diceStickersRegular()
  : sticker_()
{}

diceStickersRegular::diceStickersRegular(object_ptr<sticker> &&sticker_)
  : sticker_(std::move(sticker_))
{}

const std::int32_t diceStickersRegular::ID;

void diceStickersRegular::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "diceStickersRegular");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

diceStickersSlotMachine::diceStickersSlotMachine()
  : background_()
  , lever_()
  , left_reel_()
  , center_reel_()
  , right_reel_()
{}

diceStickersSlotMachine::diceStickersSlotMachine(object_ptr<sticker> &&background_, object_ptr<sticker> &&lever_, object_ptr<sticker> &&left_reel_, object_ptr<sticker> &&center_reel_, object_ptr<sticker> &&right_reel_)
  : background_(std::move(background_))
  , lever_(std::move(lever_))
  , left_reel_(std::move(left_reel_))
  , center_reel_(std::move(center_reel_))
  , right_reel_(std::move(right_reel_))
{}

const std::int32_t diceStickersSlotMachine::ID;

void diceStickersSlotMachine::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "diceStickersSlotMachine");
    s.store_object_field("background", static_cast<const BaseObject *>(background_.get()));
    s.store_object_field("lever", static_cast<const BaseObject *>(lever_.get()));
    s.store_object_field("left_reel", static_cast<const BaseObject *>(left_reel_.get()));
    s.store_object_field("center_reel", static_cast<const BaseObject *>(center_reel_.get()));
    s.store_object_field("right_reel", static_cast<const BaseObject *>(right_reel_.get()));
    s.store_class_end();
  }
}

emojiCategorySourceSearch::emojiCategorySourceSearch()
  : emojis_()
{}

emojiCategorySourceSearch::emojiCategorySourceSearch(array<string> &&emojis_)
  : emojis_(std::move(emojis_))
{}

const std::int32_t emojiCategorySourceSearch::ID;

void emojiCategorySourceSearch::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiCategorySourceSearch");
    { s.store_vector_begin("emojis", emojis_.size()); for (const auto &_value : emojis_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

emojiCategorySourcePremium::emojiCategorySourcePremium() {
}

const std::int32_t emojiCategorySourcePremium::ID;

void emojiCategorySourcePremium::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiCategorySourcePremium");
    s.store_class_end();
  }
}

emojiChatTheme::emojiChatTheme()
  : name_()
  , light_settings_()
  , dark_settings_()
{}

emojiChatTheme::emojiChatTheme(string const &name_, object_ptr<themeSettings> &&light_settings_, object_ptr<themeSettings> &&dark_settings_)
  : name_(name_)
  , light_settings_(std::move(light_settings_))
  , dark_settings_(std::move(dark_settings_))
{}

const std::int32_t emojiChatTheme::ID;

void emojiChatTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiChatTheme");
    s.store_field("name", name_);
    s.store_object_field("light_settings", static_cast<const BaseObject *>(light_settings_.get()));
    s.store_object_field("dark_settings", static_cast<const BaseObject *>(dark_settings_.get()));
    s.store_class_end();
  }
}

failedToAddMember::failedToAddMember()
  : user_id_()
  , premium_would_allow_invite_()
  , premium_required_to_send_messages_()
{}

failedToAddMember::failedToAddMember(int53 user_id_, bool premium_would_allow_invite_, bool premium_required_to_send_messages_)
  : user_id_(user_id_)
  , premium_would_allow_invite_(premium_would_allow_invite_)
  , premium_required_to_send_messages_(premium_required_to_send_messages_)
{}

const std::int32_t failedToAddMember::ID;

void failedToAddMember::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "failedToAddMember");
    s.store_field("user_id", user_id_);
    s.store_field("premium_would_allow_invite", premium_would_allow_invite_);
    s.store_field("premium_required_to_send_messages", premium_required_to_send_messages_);
    s.store_class_end();
  }
}

forumTopics::forumTopics()
  : total_count_()
  , topics_()
  , next_offset_date_()
  , next_offset_message_id_()
  , next_offset_forum_topic_id_()
{}

forumTopics::forumTopics(int32 total_count_, array<object_ptr<forumTopic>> &&topics_, int32 next_offset_date_, int53 next_offset_message_id_, int32 next_offset_forum_topic_id_)
  : total_count_(total_count_)
  , topics_(std::move(topics_))
  , next_offset_date_(next_offset_date_)
  , next_offset_message_id_(next_offset_message_id_)
  , next_offset_forum_topic_id_(next_offset_forum_topic_id_)
{}

const std::int32_t forumTopics::ID;

void forumTopics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "forumTopics");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("topics", topics_.size()); for (const auto &_value : topics_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("next_offset_date", next_offset_date_);
    s.store_field("next_offset_message_id", next_offset_message_id_);
    s.store_field("next_offset_forum_topic_id", next_offset_forum_topic_id_);
    s.store_class_end();
  }
}

foundPosition::foundPosition()
  : position_()
{}

foundPosition::foundPosition(int32 position_)
  : position_(position_)
{}

const std::int32_t foundPosition::ID;

void foundPosition::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "foundPosition");
    s.store_field("position", position_);
    s.store_class_end();
  }
}

giftForResaleOrderPrice::giftForResaleOrderPrice() {
}

const std::int32_t giftForResaleOrderPrice::ID;

void giftForResaleOrderPrice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giftForResaleOrderPrice");
    s.store_class_end();
  }
}

giftForResaleOrderPriceChangeDate::giftForResaleOrderPriceChangeDate() {
}

const std::int32_t giftForResaleOrderPriceChangeDate::ID;

void giftForResaleOrderPriceChangeDate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giftForResaleOrderPriceChangeDate");
    s.store_class_end();
  }
}

giftForResaleOrderNumber::giftForResaleOrderNumber() {
}

const std::int32_t giftForResaleOrderNumber::ID;

void giftForResaleOrderNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giftForResaleOrderNumber");
    s.store_class_end();
  }
}

giftSettings::giftSettings()
  : show_gift_button_()
  , accepted_gift_types_()
{}

giftSettings::giftSettings(bool show_gift_button_, object_ptr<acceptedGiftTypes> &&accepted_gift_types_)
  : show_gift_button_(show_gift_button_)
  , accepted_gift_types_(std::move(accepted_gift_types_))
{}

const std::int32_t giftSettings::ID;

void giftSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giftSettings");
    s.store_field("show_gift_button", show_gift_button_);
    s.store_object_field("accepted_gift_types", static_cast<const BaseObject *>(accepted_gift_types_.get()));
    s.store_class_end();
  }
}

giveawayParameters::giveawayParameters()
  : boosted_chat_id_()
  , additional_chat_ids_()
  , winners_selection_date_()
  , only_new_members_()
  , has_public_winners_()
  , country_codes_()
  , prize_description_()
{}

giveawayParameters::giveawayParameters(int53 boosted_chat_id_, array<int53> &&additional_chat_ids_, int32 winners_selection_date_, bool only_new_members_, bool has_public_winners_, array<string> &&country_codes_, string const &prize_description_)
  : boosted_chat_id_(boosted_chat_id_)
  , additional_chat_ids_(std::move(additional_chat_ids_))
  , winners_selection_date_(winners_selection_date_)
  , only_new_members_(only_new_members_)
  , has_public_winners_(has_public_winners_)
  , country_codes_(std::move(country_codes_))
  , prize_description_(prize_description_)
{}

const std::int32_t giveawayParameters::ID;

void giveawayParameters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giveawayParameters");
    s.store_field("boosted_chat_id", boosted_chat_id_);
    { s.store_vector_begin("additional_chat_ids", additional_chat_ids_.size()); for (const auto &_value : additional_chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("winners_selection_date", winners_selection_date_);
    s.store_field("only_new_members", only_new_members_);
    s.store_field("has_public_winners", has_public_winners_);
    { s.store_vector_begin("country_codes", country_codes_.size()); for (const auto &_value : country_codes_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("prize_description", prize_description_);
    s.store_class_end();
  }
}

groupCallInfo::groupCallInfo()
  : group_call_id_()
  , join_payload_()
{}

groupCallInfo::groupCallInfo(int32 group_call_id_, string const &join_payload_)
  : group_call_id_(group_call_id_)
  , join_payload_(join_payload_)
{}

const std::int32_t groupCallInfo::ID;

void groupCallInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCallInfo");
    s.store_field("group_call_id", group_call_id_);
    s.store_field("join_payload", join_payload_);
    s.store_class_end();
  }
}

groupCallJoinParameters::groupCallJoinParameters()
  : audio_source_id_()
  , payload_()
  , is_muted_()
  , is_my_video_enabled_()
{}

groupCallJoinParameters::groupCallJoinParameters(int32 audio_source_id_, string const &payload_, bool is_muted_, bool is_my_video_enabled_)
  : audio_source_id_(audio_source_id_)
  , payload_(payload_)
  , is_muted_(is_muted_)
  , is_my_video_enabled_(is_my_video_enabled_)
{}

const std::int32_t groupCallJoinParameters::ID;

void groupCallJoinParameters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCallJoinParameters");
    s.store_field("audio_source_id", audio_source_id_);
    s.store_field("payload", payload_);
    s.store_field("is_muted", is_muted_);
    s.store_field("is_my_video_enabled", is_my_video_enabled_);
    s.store_class_end();
  }
}

groupCallParticipantVideoInfo::groupCallParticipantVideoInfo()
  : source_groups_()
  , endpoint_id_()
  , is_paused_()
{}

groupCallParticipantVideoInfo::groupCallParticipantVideoInfo(array<object_ptr<groupCallVideoSourceGroup>> &&source_groups_, string const &endpoint_id_, bool is_paused_)
  : source_groups_(std::move(source_groups_))
  , endpoint_id_(endpoint_id_)
  , is_paused_(is_paused_)
{}

const std::int32_t groupCallParticipantVideoInfo::ID;

void groupCallParticipantVideoInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCallParticipantVideoInfo");
    { s.store_vector_begin("source_groups", source_groups_.size()); for (const auto &_value : source_groups_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("endpoint_id", endpoint_id_);
    s.store_field("is_paused", is_paused_);
    s.store_class_end();
  }
}

identityDocument::identityDocument()
  : number_()
  , expiration_date_()
  , front_side_()
  , reverse_side_()
  , selfie_()
  , translation_()
{}

identityDocument::identityDocument(string const &number_, object_ptr<date> &&expiration_date_, object_ptr<datedFile> &&front_side_, object_ptr<datedFile> &&reverse_side_, object_ptr<datedFile> &&selfie_, array<object_ptr<datedFile>> &&translation_)
  : number_(number_)
  , expiration_date_(std::move(expiration_date_))
  , front_side_(std::move(front_side_))
  , reverse_side_(std::move(reverse_side_))
  , selfie_(std::move(selfie_))
  , translation_(std::move(translation_))
{}

const std::int32_t identityDocument::ID;

void identityDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "identityDocument");
    s.store_field("number", number_);
    s.store_object_field("expiration_date", static_cast<const BaseObject *>(expiration_date_.get()));
    s.store_object_field("front_side", static_cast<const BaseObject *>(front_side_.get()));
    s.store_object_field("reverse_side", static_cast<const BaseObject *>(reverse_side_.get()));
    s.store_object_field("selfie", static_cast<const BaseObject *>(selfie_.get()));
    { s.store_vector_begin("translation", translation_.size()); for (const auto &_value : translation_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

inlineQueryResults::inlineQueryResults()
  : inline_query_id_()
  , button_()
  , results_()
  , next_offset_()
{}

inlineQueryResults::inlineQueryResults(int64 inline_query_id_, object_ptr<inlineQueryResultsButton> &&button_, array<object_ptr<InlineQueryResult>> &&results_, string const &next_offset_)
  : inline_query_id_(inline_query_id_)
  , button_(std::move(button_))
  , results_(std::move(results_))
  , next_offset_(next_offset_)
{}

const std::int32_t inlineQueryResults::ID;

void inlineQueryResults::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryResults");
    s.store_field("inline_query_id", inline_query_id_);
    s.store_object_field("button", static_cast<const BaseObject *>(button_.get()));
    { s.store_vector_begin("results", results_.size()); for (const auto &_value : results_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

inlineQueryResultsButton::inlineQueryResultsButton()
  : text_()
  , type_()
{}

inlineQueryResultsButton::inlineQueryResultsButton(string const &text_, object_ptr<InlineQueryResultsButtonType> &&type_)
  : text_(text_)
  , type_(std::move(type_))
{}

const std::int32_t inlineQueryResultsButton::ID;

void inlineQueryResultsButton::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryResultsButton");
    s.store_field("text", text_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

inputIdentityDocument::inputIdentityDocument()
  : number_()
  , expiration_date_()
  , front_side_()
  , reverse_side_()
  , selfie_()
  , translation_()
{}

inputIdentityDocument::inputIdentityDocument(string const &number_, object_ptr<date> &&expiration_date_, object_ptr<InputFile> &&front_side_, object_ptr<InputFile> &&reverse_side_, object_ptr<InputFile> &&selfie_, array<object_ptr<InputFile>> &&translation_)
  : number_(number_)
  , expiration_date_(std::move(expiration_date_))
  , front_side_(std::move(front_side_))
  , reverse_side_(std::move(reverse_side_))
  , selfie_(std::move(selfie_))
  , translation_(std::move(translation_))
{}

const std::int32_t inputIdentityDocument::ID;

void inputIdentityDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputIdentityDocument");
    s.store_field("number", number_);
    s.store_object_field("expiration_date", static_cast<const BaseObject *>(expiration_date_.get()));
    s.store_object_field("front_side", static_cast<const BaseObject *>(front_side_.get()));
    s.store_object_field("reverse_side", static_cast<const BaseObject *>(reverse_side_.get()));
    s.store_object_field("selfie", static_cast<const BaseObject *>(selfie_.get()));
    { s.store_vector_begin("translation", translation_.size()); for (const auto &_value : translation_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

inputPersonalDocument::inputPersonalDocument()
  : files_()
  , translation_()
{}

inputPersonalDocument::inputPersonalDocument(array<object_ptr<InputFile>> &&files_, array<object_ptr<InputFile>> &&translation_)
  : files_(std::move(files_))
  , translation_(std::move(translation_))
{}

const std::int32_t inputPersonalDocument::ID;

void inputPersonalDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPersonalDocument");
    { s.store_vector_begin("files", files_.size()); for (const auto &_value : files_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("translation", translation_.size()); for (const auto &_value : translation_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

internalLinkTypeActiveSessions::internalLinkTypeActiveSessions() {
}

const std::int32_t internalLinkTypeActiveSessions::ID;

void internalLinkTypeActiveSessions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeActiveSessions");
    s.store_class_end();
  }
}

internalLinkTypeAttachmentMenuBot::internalLinkTypeAttachmentMenuBot()
  : target_chat_()
  , bot_username_()
  , url_()
{}

internalLinkTypeAttachmentMenuBot::internalLinkTypeAttachmentMenuBot(object_ptr<TargetChat> &&target_chat_, string const &bot_username_, string const &url_)
  : target_chat_(std::move(target_chat_))
  , bot_username_(bot_username_)
  , url_(url_)
{}

const std::int32_t internalLinkTypeAttachmentMenuBot::ID;

void internalLinkTypeAttachmentMenuBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeAttachmentMenuBot");
    s.store_object_field("target_chat", static_cast<const BaseObject *>(target_chat_.get()));
    s.store_field("bot_username", bot_username_);
    s.store_field("url", url_);
    s.store_class_end();
  }
}

internalLinkTypeAuthenticationCode::internalLinkTypeAuthenticationCode()
  : code_()
{}

internalLinkTypeAuthenticationCode::internalLinkTypeAuthenticationCode(string const &code_)
  : code_(code_)
{}

const std::int32_t internalLinkTypeAuthenticationCode::ID;

void internalLinkTypeAuthenticationCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeAuthenticationCode");
    s.store_field("code", code_);
    s.store_class_end();
  }
}

internalLinkTypeBackground::internalLinkTypeBackground()
  : background_name_()
{}

internalLinkTypeBackground::internalLinkTypeBackground(string const &background_name_)
  : background_name_(background_name_)
{}

const std::int32_t internalLinkTypeBackground::ID;

void internalLinkTypeBackground::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeBackground");
    s.store_field("background_name", background_name_);
    s.store_class_end();
  }
}

internalLinkTypeBotAddToChannel::internalLinkTypeBotAddToChannel()
  : bot_username_()
  , administrator_rights_()
{}

internalLinkTypeBotAddToChannel::internalLinkTypeBotAddToChannel(string const &bot_username_, object_ptr<chatAdministratorRights> &&administrator_rights_)
  : bot_username_(bot_username_)
  , administrator_rights_(std::move(administrator_rights_))
{}

const std::int32_t internalLinkTypeBotAddToChannel::ID;

void internalLinkTypeBotAddToChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeBotAddToChannel");
    s.store_field("bot_username", bot_username_);
    s.store_object_field("administrator_rights", static_cast<const BaseObject *>(administrator_rights_.get()));
    s.store_class_end();
  }
}

internalLinkTypeBotStart::internalLinkTypeBotStart()
  : bot_username_()
  , start_parameter_()
  , autostart_()
{}

internalLinkTypeBotStart::internalLinkTypeBotStart(string const &bot_username_, string const &start_parameter_, bool autostart_)
  : bot_username_(bot_username_)
  , start_parameter_(start_parameter_)
  , autostart_(autostart_)
{}

const std::int32_t internalLinkTypeBotStart::ID;

void internalLinkTypeBotStart::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeBotStart");
    s.store_field("bot_username", bot_username_);
    s.store_field("start_parameter", start_parameter_);
    s.store_field("autostart", autostart_);
    s.store_class_end();
  }
}

internalLinkTypeBotStartInGroup::internalLinkTypeBotStartInGroup()
  : bot_username_()
  , start_parameter_()
  , administrator_rights_()
{}

internalLinkTypeBotStartInGroup::internalLinkTypeBotStartInGroup(string const &bot_username_, string const &start_parameter_, object_ptr<chatAdministratorRights> &&administrator_rights_)
  : bot_username_(bot_username_)
  , start_parameter_(start_parameter_)
  , administrator_rights_(std::move(administrator_rights_))
{}

const std::int32_t internalLinkTypeBotStartInGroup::ID;

void internalLinkTypeBotStartInGroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeBotStartInGroup");
    s.store_field("bot_username", bot_username_);
    s.store_field("start_parameter", start_parameter_);
    s.store_object_field("administrator_rights", static_cast<const BaseObject *>(administrator_rights_.get()));
    s.store_class_end();
  }
}

internalLinkTypeBusinessChat::internalLinkTypeBusinessChat()
  : link_name_()
{}

internalLinkTypeBusinessChat::internalLinkTypeBusinessChat(string const &link_name_)
  : link_name_(link_name_)
{}

const std::int32_t internalLinkTypeBusinessChat::ID;

void internalLinkTypeBusinessChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeBusinessChat");
    s.store_field("link_name", link_name_);
    s.store_class_end();
  }
}

internalLinkTypeBuyStars::internalLinkTypeBuyStars()
  : star_count_()
  , purpose_()
{}

internalLinkTypeBuyStars::internalLinkTypeBuyStars(int53 star_count_, string const &purpose_)
  : star_count_(star_count_)
  , purpose_(purpose_)
{}

const std::int32_t internalLinkTypeBuyStars::ID;

void internalLinkTypeBuyStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeBuyStars");
    s.store_field("star_count", star_count_);
    s.store_field("purpose", purpose_);
    s.store_class_end();
  }
}

internalLinkTypeChangePhoneNumber::internalLinkTypeChangePhoneNumber() {
}

const std::int32_t internalLinkTypeChangePhoneNumber::ID;

void internalLinkTypeChangePhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeChangePhoneNumber");
    s.store_class_end();
  }
}

internalLinkTypeChatAffiliateProgram::internalLinkTypeChatAffiliateProgram()
  : username_()
  , referrer_()
{}

internalLinkTypeChatAffiliateProgram::internalLinkTypeChatAffiliateProgram(string const &username_, string const &referrer_)
  : username_(username_)
  , referrer_(referrer_)
{}

const std::int32_t internalLinkTypeChatAffiliateProgram::ID;

void internalLinkTypeChatAffiliateProgram::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeChatAffiliateProgram");
    s.store_field("username", username_);
    s.store_field("referrer", referrer_);
    s.store_class_end();
  }
}

internalLinkTypeChatBoost::internalLinkTypeChatBoost()
  : url_()
{}

internalLinkTypeChatBoost::internalLinkTypeChatBoost(string const &url_)
  : url_(url_)
{}

const std::int32_t internalLinkTypeChatBoost::ID;

void internalLinkTypeChatBoost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeChatBoost");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

internalLinkTypeChatFolderInvite::internalLinkTypeChatFolderInvite()
  : invite_link_()
{}

internalLinkTypeChatFolderInvite::internalLinkTypeChatFolderInvite(string const &invite_link_)
  : invite_link_(invite_link_)
{}

const std::int32_t internalLinkTypeChatFolderInvite::ID;

void internalLinkTypeChatFolderInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeChatFolderInvite");
    s.store_field("invite_link", invite_link_);
    s.store_class_end();
  }
}

internalLinkTypeChatFolderSettings::internalLinkTypeChatFolderSettings() {
}

const std::int32_t internalLinkTypeChatFolderSettings::ID;

void internalLinkTypeChatFolderSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeChatFolderSettings");
    s.store_class_end();
  }
}

internalLinkTypeChatInvite::internalLinkTypeChatInvite()
  : invite_link_()
{}

internalLinkTypeChatInvite::internalLinkTypeChatInvite(string const &invite_link_)
  : invite_link_(invite_link_)
{}

const std::int32_t internalLinkTypeChatInvite::ID;

void internalLinkTypeChatInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeChatInvite");
    s.store_field("invite_link", invite_link_);
    s.store_class_end();
  }
}

internalLinkTypeDefaultMessageAutoDeleteTimerSettings::internalLinkTypeDefaultMessageAutoDeleteTimerSettings() {
}

const std::int32_t internalLinkTypeDefaultMessageAutoDeleteTimerSettings::ID;

void internalLinkTypeDefaultMessageAutoDeleteTimerSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeDefaultMessageAutoDeleteTimerSettings");
    s.store_class_end();
  }
}

internalLinkTypeDirectMessagesChat::internalLinkTypeDirectMessagesChat()
  : channel_username_()
{}

internalLinkTypeDirectMessagesChat::internalLinkTypeDirectMessagesChat(string const &channel_username_)
  : channel_username_(channel_username_)
{}

const std::int32_t internalLinkTypeDirectMessagesChat::ID;

void internalLinkTypeDirectMessagesChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeDirectMessagesChat");
    s.store_field("channel_username", channel_username_);
    s.store_class_end();
  }
}

internalLinkTypeEditProfileSettings::internalLinkTypeEditProfileSettings() {
}

const std::int32_t internalLinkTypeEditProfileSettings::ID;

void internalLinkTypeEditProfileSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeEditProfileSettings");
    s.store_class_end();
  }
}

internalLinkTypeGame::internalLinkTypeGame()
  : bot_username_()
  , game_short_name_()
{}

internalLinkTypeGame::internalLinkTypeGame(string const &bot_username_, string const &game_short_name_)
  : bot_username_(bot_username_)
  , game_short_name_(game_short_name_)
{}

const std::int32_t internalLinkTypeGame::ID;

void internalLinkTypeGame::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeGame");
    s.store_field("bot_username", bot_username_);
    s.store_field("game_short_name", game_short_name_);
    s.store_class_end();
  }
}

internalLinkTypeGiftCollection::internalLinkTypeGiftCollection()
  : gift_owner_username_()
  , collection_id_()
{}

internalLinkTypeGiftCollection::internalLinkTypeGiftCollection(string const &gift_owner_username_, int32 collection_id_)
  : gift_owner_username_(gift_owner_username_)
  , collection_id_(collection_id_)
{}

const std::int32_t internalLinkTypeGiftCollection::ID;

void internalLinkTypeGiftCollection::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeGiftCollection");
    s.store_field("gift_owner_username", gift_owner_username_);
    s.store_field("collection_id", collection_id_);
    s.store_class_end();
  }
}

internalLinkTypeGroupCall::internalLinkTypeGroupCall()
  : invite_link_()
{}

internalLinkTypeGroupCall::internalLinkTypeGroupCall(string const &invite_link_)
  : invite_link_(invite_link_)
{}

const std::int32_t internalLinkTypeGroupCall::ID;

void internalLinkTypeGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeGroupCall");
    s.store_field("invite_link", invite_link_);
    s.store_class_end();
  }
}

internalLinkTypeInstantView::internalLinkTypeInstantView()
  : url_()
  , fallback_url_()
{}

internalLinkTypeInstantView::internalLinkTypeInstantView(string const &url_, string const &fallback_url_)
  : url_(url_)
  , fallback_url_(fallback_url_)
{}

const std::int32_t internalLinkTypeInstantView::ID;

void internalLinkTypeInstantView::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeInstantView");
    s.store_field("url", url_);
    s.store_field("fallback_url", fallback_url_);
    s.store_class_end();
  }
}

internalLinkTypeInvoice::internalLinkTypeInvoice()
  : invoice_name_()
{}

internalLinkTypeInvoice::internalLinkTypeInvoice(string const &invoice_name_)
  : invoice_name_(invoice_name_)
{}

const std::int32_t internalLinkTypeInvoice::ID;

void internalLinkTypeInvoice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeInvoice");
    s.store_field("invoice_name", invoice_name_);
    s.store_class_end();
  }
}

internalLinkTypeLanguagePack::internalLinkTypeLanguagePack()
  : language_pack_id_()
{}

internalLinkTypeLanguagePack::internalLinkTypeLanguagePack(string const &language_pack_id_)
  : language_pack_id_(language_pack_id_)
{}

const std::int32_t internalLinkTypeLanguagePack::ID;

void internalLinkTypeLanguagePack::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeLanguagePack");
    s.store_field("language_pack_id", language_pack_id_);
    s.store_class_end();
  }
}

internalLinkTypeLanguageSettings::internalLinkTypeLanguageSettings() {
}

const std::int32_t internalLinkTypeLanguageSettings::ID;

void internalLinkTypeLanguageSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeLanguageSettings");
    s.store_class_end();
  }
}

internalLinkTypeMainWebApp::internalLinkTypeMainWebApp()
  : bot_username_()
  , start_parameter_()
  , mode_()
{}

internalLinkTypeMainWebApp::internalLinkTypeMainWebApp(string const &bot_username_, string const &start_parameter_, object_ptr<WebAppOpenMode> &&mode_)
  : bot_username_(bot_username_)
  , start_parameter_(start_parameter_)
  , mode_(std::move(mode_))
{}

const std::int32_t internalLinkTypeMainWebApp::ID;

void internalLinkTypeMainWebApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeMainWebApp");
    s.store_field("bot_username", bot_username_);
    s.store_field("start_parameter", start_parameter_);
    s.store_object_field("mode", static_cast<const BaseObject *>(mode_.get()));
    s.store_class_end();
  }
}

internalLinkTypeMessage::internalLinkTypeMessage()
  : url_()
{}

internalLinkTypeMessage::internalLinkTypeMessage(string const &url_)
  : url_(url_)
{}

const std::int32_t internalLinkTypeMessage::ID;

void internalLinkTypeMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeMessage");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

internalLinkTypeMessageDraft::internalLinkTypeMessageDraft()
  : text_()
  , contains_link_()
{}

internalLinkTypeMessageDraft::internalLinkTypeMessageDraft(object_ptr<formattedText> &&text_, bool contains_link_)
  : text_(std::move(text_))
  , contains_link_(contains_link_)
{}

const std::int32_t internalLinkTypeMessageDraft::ID;

void internalLinkTypeMessageDraft::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeMessageDraft");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("contains_link", contains_link_);
    s.store_class_end();
  }
}

internalLinkTypeMyStars::internalLinkTypeMyStars() {
}

const std::int32_t internalLinkTypeMyStars::ID;

void internalLinkTypeMyStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeMyStars");
    s.store_class_end();
  }
}

internalLinkTypeMyToncoins::internalLinkTypeMyToncoins() {
}

const std::int32_t internalLinkTypeMyToncoins::ID;

void internalLinkTypeMyToncoins::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeMyToncoins");
    s.store_class_end();
  }
}

internalLinkTypePassportDataRequest::internalLinkTypePassportDataRequest()
  : bot_user_id_()
  , scope_()
  , public_key_()
  , nonce_()
  , callback_url_()
{}

internalLinkTypePassportDataRequest::internalLinkTypePassportDataRequest(int53 bot_user_id_, string const &scope_, string const &public_key_, string const &nonce_, string const &callback_url_)
  : bot_user_id_(bot_user_id_)
  , scope_(scope_)
  , public_key_(public_key_)
  , nonce_(nonce_)
  , callback_url_(callback_url_)
{}

const std::int32_t internalLinkTypePassportDataRequest::ID;

void internalLinkTypePassportDataRequest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypePassportDataRequest");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("scope", scope_);
    s.store_field("public_key", public_key_);
    s.store_field("nonce", nonce_);
    s.store_field("callback_url", callback_url_);
    s.store_class_end();
  }
}

internalLinkTypePhoneNumberConfirmation::internalLinkTypePhoneNumberConfirmation()
  : hash_()
  , phone_number_()
{}

internalLinkTypePhoneNumberConfirmation::internalLinkTypePhoneNumberConfirmation(string const &hash_, string const &phone_number_)
  : hash_(hash_)
  , phone_number_(phone_number_)
{}

const std::int32_t internalLinkTypePhoneNumberConfirmation::ID;

void internalLinkTypePhoneNumberConfirmation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypePhoneNumberConfirmation");
    s.store_field("hash", hash_);
    s.store_field("phone_number", phone_number_);
    s.store_class_end();
  }
}

internalLinkTypePremiumFeatures::internalLinkTypePremiumFeatures()
  : referrer_()
{}

internalLinkTypePremiumFeatures::internalLinkTypePremiumFeatures(string const &referrer_)
  : referrer_(referrer_)
{}

const std::int32_t internalLinkTypePremiumFeatures::ID;

void internalLinkTypePremiumFeatures::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypePremiumFeatures");
    s.store_field("referrer", referrer_);
    s.store_class_end();
  }
}

internalLinkTypePremiumGift::internalLinkTypePremiumGift()
  : referrer_()
{}

internalLinkTypePremiumGift::internalLinkTypePremiumGift(string const &referrer_)
  : referrer_(referrer_)
{}

const std::int32_t internalLinkTypePremiumGift::ID;

void internalLinkTypePremiumGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypePremiumGift");
    s.store_field("referrer", referrer_);
    s.store_class_end();
  }
}

internalLinkTypePremiumGiftCode::internalLinkTypePremiumGiftCode()
  : code_()
{}

internalLinkTypePremiumGiftCode::internalLinkTypePremiumGiftCode(string const &code_)
  : code_(code_)
{}

const std::int32_t internalLinkTypePremiumGiftCode::ID;

void internalLinkTypePremiumGiftCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypePremiumGiftCode");
    s.store_field("code", code_);
    s.store_class_end();
  }
}

internalLinkTypePrivacyAndSecuritySettings::internalLinkTypePrivacyAndSecuritySettings() {
}

const std::int32_t internalLinkTypePrivacyAndSecuritySettings::ID;

void internalLinkTypePrivacyAndSecuritySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypePrivacyAndSecuritySettings");
    s.store_class_end();
  }
}

internalLinkTypeProxy::internalLinkTypeProxy()
  : server_()
  , port_()
  , type_()
{}

internalLinkTypeProxy::internalLinkTypeProxy(string const &server_, int32 port_, object_ptr<ProxyType> &&type_)
  : server_(server_)
  , port_(port_)
  , type_(std::move(type_))
{}

const std::int32_t internalLinkTypeProxy::ID;

void internalLinkTypeProxy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeProxy");
    s.store_field("server", server_);
    s.store_field("port", port_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

internalLinkTypePublicChat::internalLinkTypePublicChat()
  : chat_username_()
  , draft_text_()
  , open_profile_()
{}

internalLinkTypePublicChat::internalLinkTypePublicChat(string const &chat_username_, string const &draft_text_, bool open_profile_)
  : chat_username_(chat_username_)
  , draft_text_(draft_text_)
  , open_profile_(open_profile_)
{}

const std::int32_t internalLinkTypePublicChat::ID;

void internalLinkTypePublicChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypePublicChat");
    s.store_field("chat_username", chat_username_);
    s.store_field("draft_text", draft_text_);
    s.store_field("open_profile", open_profile_);
    s.store_class_end();
  }
}

internalLinkTypeQrCodeAuthentication::internalLinkTypeQrCodeAuthentication() {
}

const std::int32_t internalLinkTypeQrCodeAuthentication::ID;

void internalLinkTypeQrCodeAuthentication::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeQrCodeAuthentication");
    s.store_class_end();
  }
}

internalLinkTypeRestorePurchases::internalLinkTypeRestorePurchases() {
}

const std::int32_t internalLinkTypeRestorePurchases::ID;

void internalLinkTypeRestorePurchases::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeRestorePurchases");
    s.store_class_end();
  }
}

internalLinkTypeSettings::internalLinkTypeSettings() {
}

const std::int32_t internalLinkTypeSettings::ID;

void internalLinkTypeSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeSettings");
    s.store_class_end();
  }
}

internalLinkTypeStickerSet::internalLinkTypeStickerSet()
  : sticker_set_name_()
  , expect_custom_emoji_()
{}

internalLinkTypeStickerSet::internalLinkTypeStickerSet(string const &sticker_set_name_, bool expect_custom_emoji_)
  : sticker_set_name_(sticker_set_name_)
  , expect_custom_emoji_(expect_custom_emoji_)
{}

const std::int32_t internalLinkTypeStickerSet::ID;

void internalLinkTypeStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeStickerSet");
    s.store_field("sticker_set_name", sticker_set_name_);
    s.store_field("expect_custom_emoji", expect_custom_emoji_);
    s.store_class_end();
  }
}

internalLinkTypeStory::internalLinkTypeStory()
  : story_poster_username_()
  , story_id_()
{}

internalLinkTypeStory::internalLinkTypeStory(string const &story_poster_username_, int32 story_id_)
  : story_poster_username_(story_poster_username_)
  , story_id_(story_id_)
{}

const std::int32_t internalLinkTypeStory::ID;

void internalLinkTypeStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeStory");
    s.store_field("story_poster_username", story_poster_username_);
    s.store_field("story_id", story_id_);
    s.store_class_end();
  }
}

internalLinkTypeStoryAlbum::internalLinkTypeStoryAlbum()
  : story_album_owner_username_()
  , story_album_id_()
{}

internalLinkTypeStoryAlbum::internalLinkTypeStoryAlbum(string const &story_album_owner_username_, int32 story_album_id_)
  : story_album_owner_username_(story_album_owner_username_)
  , story_album_id_(story_album_id_)
{}

const std::int32_t internalLinkTypeStoryAlbum::ID;

void internalLinkTypeStoryAlbum::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeStoryAlbum");
    s.store_field("story_album_owner_username", story_album_owner_username_);
    s.store_field("story_album_id", story_album_id_);
    s.store_class_end();
  }
}

internalLinkTypeTheme::internalLinkTypeTheme()
  : theme_name_()
{}

internalLinkTypeTheme::internalLinkTypeTheme(string const &theme_name_)
  : theme_name_(theme_name_)
{}

const std::int32_t internalLinkTypeTheme::ID;

void internalLinkTypeTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeTheme");
    s.store_field("theme_name", theme_name_);
    s.store_class_end();
  }
}

internalLinkTypeThemeSettings::internalLinkTypeThemeSettings() {
}

const std::int32_t internalLinkTypeThemeSettings::ID;

void internalLinkTypeThemeSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeThemeSettings");
    s.store_class_end();
  }
}

internalLinkTypeUnknownDeepLink::internalLinkTypeUnknownDeepLink()
  : link_()
{}

internalLinkTypeUnknownDeepLink::internalLinkTypeUnknownDeepLink(string const &link_)
  : link_(link_)
{}

const std::int32_t internalLinkTypeUnknownDeepLink::ID;

void internalLinkTypeUnknownDeepLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeUnknownDeepLink");
    s.store_field("link", link_);
    s.store_class_end();
  }
}

internalLinkTypeUnsupportedProxy::internalLinkTypeUnsupportedProxy() {
}

const std::int32_t internalLinkTypeUnsupportedProxy::ID;

void internalLinkTypeUnsupportedProxy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeUnsupportedProxy");
    s.store_class_end();
  }
}

internalLinkTypeUpgradedGift::internalLinkTypeUpgradedGift()
  : name_()
{}

internalLinkTypeUpgradedGift::internalLinkTypeUpgradedGift(string const &name_)
  : name_(name_)
{}

const std::int32_t internalLinkTypeUpgradedGift::ID;

void internalLinkTypeUpgradedGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeUpgradedGift");
    s.store_field("name", name_);
    s.store_class_end();
  }
}

internalLinkTypeUserPhoneNumber::internalLinkTypeUserPhoneNumber()
  : phone_number_()
  , draft_text_()
  , open_profile_()
{}

internalLinkTypeUserPhoneNumber::internalLinkTypeUserPhoneNumber(string const &phone_number_, string const &draft_text_, bool open_profile_)
  : phone_number_(phone_number_)
  , draft_text_(draft_text_)
  , open_profile_(open_profile_)
{}

const std::int32_t internalLinkTypeUserPhoneNumber::ID;

void internalLinkTypeUserPhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeUserPhoneNumber");
    s.store_field("phone_number", phone_number_);
    s.store_field("draft_text", draft_text_);
    s.store_field("open_profile", open_profile_);
    s.store_class_end();
  }
}

internalLinkTypeUserToken::internalLinkTypeUserToken()
  : token_()
{}

internalLinkTypeUserToken::internalLinkTypeUserToken(string const &token_)
  : token_(token_)
{}

const std::int32_t internalLinkTypeUserToken::ID;

void internalLinkTypeUserToken::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeUserToken");
    s.store_field("token", token_);
    s.store_class_end();
  }
}

internalLinkTypeVideoChat::internalLinkTypeVideoChat()
  : chat_username_()
  , invite_hash_()
  , is_live_stream_()
{}

internalLinkTypeVideoChat::internalLinkTypeVideoChat(string const &chat_username_, string const &invite_hash_, bool is_live_stream_)
  : chat_username_(chat_username_)
  , invite_hash_(invite_hash_)
  , is_live_stream_(is_live_stream_)
{}

const std::int32_t internalLinkTypeVideoChat::ID;

void internalLinkTypeVideoChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeVideoChat");
    s.store_field("chat_username", chat_username_);
    s.store_field("invite_hash", invite_hash_);
    s.store_field("is_live_stream", is_live_stream_);
    s.store_class_end();
  }
}

internalLinkTypeWebApp::internalLinkTypeWebApp()
  : bot_username_()
  , web_app_short_name_()
  , start_parameter_()
  , mode_()
{}

internalLinkTypeWebApp::internalLinkTypeWebApp(string const &bot_username_, string const &web_app_short_name_, string const &start_parameter_, object_ptr<WebAppOpenMode> &&mode_)
  : bot_username_(bot_username_)
  , web_app_short_name_(web_app_short_name_)
  , start_parameter_(start_parameter_)
  , mode_(std::move(mode_))
{}

const std::int32_t internalLinkTypeWebApp::ID;

void internalLinkTypeWebApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "internalLinkTypeWebApp");
    s.store_field("bot_username", bot_username_);
    s.store_field("web_app_short_name", web_app_short_name_);
    s.store_field("start_parameter", start_parameter_);
    s.store_object_field("mode", static_cast<const BaseObject *>(mode_.get()));
    s.store_class_end();
  }
}

keyboardButton::keyboardButton()
  : text_()
  , type_()
{}

keyboardButton::keyboardButton(string const &text_, object_ptr<KeyboardButtonType> &&type_)
  : text_(text_)
  , type_(std::move(type_))
{}

const std::int32_t keyboardButton::ID;

void keyboardButton::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "keyboardButton");
    s.store_field("text", text_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

messageCalendar::messageCalendar()
  : total_count_()
  , days_()
{}

messageCalendar::messageCalendar(int32 total_count_, array<object_ptr<messageCalendarDay>> &&days_)
  : total_count_(total_count_)
  , days_(std::move(days_))
{}

const std::int32_t messageCalendar::ID;

void messageCalendar::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageCalendar");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("days", days_.size()); for (const auto &_value : days_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messageInteractionInfo::messageInteractionInfo()
  : view_count_()
  , forward_count_()
  , reply_info_()
  , reactions_()
{}

messageInteractionInfo::messageInteractionInfo(int32 view_count_, int32 forward_count_, object_ptr<messageReplyInfo> &&reply_info_, object_ptr<messageReactions> &&reactions_)
  : view_count_(view_count_)
  , forward_count_(forward_count_)
  , reply_info_(std::move(reply_info_))
  , reactions_(std::move(reactions_))
{}

const std::int32_t messageInteractionInfo::ID;

void messageInteractionInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageInteractionInfo");
    s.store_field("view_count", view_count_);
    s.store_field("forward_count", forward_count_);
    s.store_object_field("reply_info", static_cast<const BaseObject *>(reply_info_.get()));
    s.store_object_field("reactions", static_cast<const BaseObject *>(reactions_.get()));
    s.store_class_end();
  }
}

messageReadDateRead::messageReadDateRead()
  : read_date_()
{}

messageReadDateRead::messageReadDateRead(int32 read_date_)
  : read_date_(read_date_)
{}

const std::int32_t messageReadDateRead::ID;

void messageReadDateRead::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageReadDateRead");
    s.store_field("read_date", read_date_);
    s.store_class_end();
  }
}

messageReadDateUnread::messageReadDateUnread() {
}

const std::int32_t messageReadDateUnread::ID;

void messageReadDateUnread::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageReadDateUnread");
    s.store_class_end();
  }
}

messageReadDateTooOld::messageReadDateTooOld() {
}

const std::int32_t messageReadDateTooOld::ID;

void messageReadDateTooOld::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageReadDateTooOld");
    s.store_class_end();
  }
}

messageReadDateUserPrivacyRestricted::messageReadDateUserPrivacyRestricted() {
}

const std::int32_t messageReadDateUserPrivacyRestricted::ID;

void messageReadDateUserPrivacyRestricted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageReadDateUserPrivacyRestricted");
    s.store_class_end();
  }
}

messageReadDateMyPrivacyRestricted::messageReadDateMyPrivacyRestricted() {
}

const std::int32_t messageReadDateMyPrivacyRestricted::ID;

void messageReadDateMyPrivacyRestricted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageReadDateMyPrivacyRestricted");
    s.store_class_end();
  }
}

networkStatistics::networkStatistics()
  : since_date_()
  , entries_()
{}

networkStatistics::networkStatistics(int32 since_date_, array<object_ptr<NetworkStatisticsEntry>> &&entries_)
  : since_date_(since_date_)
  , entries_(std::move(entries_))
{}

const std::int32_t networkStatistics::ID;

void networkStatistics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "networkStatistics");
    s.store_field("since_date", since_date_);
    { s.store_vector_begin("entries", entries_.size()); for (const auto &_value : entries_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

networkStatisticsEntryFile::networkStatisticsEntryFile()
  : file_type_()
  , network_type_()
  , sent_bytes_()
  , received_bytes_()
{}

networkStatisticsEntryFile::networkStatisticsEntryFile(object_ptr<FileType> &&file_type_, object_ptr<NetworkType> &&network_type_, int53 sent_bytes_, int53 received_bytes_)
  : file_type_(std::move(file_type_))
  , network_type_(std::move(network_type_))
  , sent_bytes_(sent_bytes_)
  , received_bytes_(received_bytes_)
{}

const std::int32_t networkStatisticsEntryFile::ID;

void networkStatisticsEntryFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "networkStatisticsEntryFile");
    s.store_object_field("file_type", static_cast<const BaseObject *>(file_type_.get()));
    s.store_object_field("network_type", static_cast<const BaseObject *>(network_type_.get()));
    s.store_field("sent_bytes", sent_bytes_);
    s.store_field("received_bytes", received_bytes_);
    s.store_class_end();
  }
}

networkStatisticsEntryCall::networkStatisticsEntryCall()
  : network_type_()
  , sent_bytes_()
  , received_bytes_()
  , duration_()
{}

networkStatisticsEntryCall::networkStatisticsEntryCall(object_ptr<NetworkType> &&network_type_, int53 sent_bytes_, int53 received_bytes_, double duration_)
  : network_type_(std::move(network_type_))
  , sent_bytes_(sent_bytes_)
  , received_bytes_(received_bytes_)
  , duration_(duration_)
{}

const std::int32_t networkStatisticsEntryCall::ID;

void networkStatisticsEntryCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "networkStatisticsEntryCall");
    s.store_object_field("network_type", static_cast<const BaseObject *>(network_type_.get()));
    s.store_field("sent_bytes", sent_bytes_);
    s.store_field("received_bytes", received_bytes_);
    s.store_field("duration", duration_);
    s.store_class_end();
  }
}

notificationSounds::notificationSounds()
  : notification_sounds_()
{}

notificationSounds::notificationSounds(array<object_ptr<notificationSound>> &&notification_sounds_)
  : notification_sounds_(std::move(notification_sounds_))
{}

const std::int32_t notificationSounds::ID;

void notificationSounds::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notificationSounds");
    { s.store_vector_begin("notification_sounds", notification_sounds_.size()); for (const auto &_value : notification_sounds_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

paidReactionTypeRegular::paidReactionTypeRegular() {
}

const std::int32_t paidReactionTypeRegular::ID;

void paidReactionTypeRegular::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paidReactionTypeRegular");
    s.store_class_end();
  }
}

paidReactionTypeAnonymous::paidReactionTypeAnonymous() {
}

const std::int32_t paidReactionTypeAnonymous::ID;

void paidReactionTypeAnonymous::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paidReactionTypeAnonymous");
    s.store_class_end();
  }
}

paidReactionTypeChat::paidReactionTypeChat()
  : chat_id_()
{}

paidReactionTypeChat::paidReactionTypeChat(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t paidReactionTypeChat::ID;

void paidReactionTypeChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paidReactionTypeChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

passportRequiredElement::passportRequiredElement()
  : suitable_elements_()
{}

passportRequiredElement::passportRequiredElement(array<object_ptr<passportSuitableElement>> &&suitable_elements_)
  : suitable_elements_(std::move(suitable_elements_))
{}

const std::int32_t passportRequiredElement::ID;

void passportRequiredElement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportRequiredElement");
    { s.store_vector_begin("suitable_elements", suitable_elements_.size()); for (const auto &_value : suitable_elements_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

passportSuitableElement::passportSuitableElement()
  : type_()
  , is_selfie_required_()
  , is_translation_required_()
  , is_native_name_required_()
{}

passportSuitableElement::passportSuitableElement(object_ptr<PassportElementType> &&type_, bool is_selfie_required_, bool is_translation_required_, bool is_native_name_required_)
  : type_(std::move(type_))
  , is_selfie_required_(is_selfie_required_)
  , is_translation_required_(is_translation_required_)
  , is_native_name_required_(is_native_name_required_)
{}

const std::int32_t passportSuitableElement::ID;

void passportSuitableElement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportSuitableElement");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("is_selfie_required", is_selfie_required_);
    s.store_field("is_translation_required", is_translation_required_);
    s.store_field("is_native_name_required", is_native_name_required_);
    s.store_class_end();
  }
}

photoSize::photoSize()
  : type_()
  , photo_()
  , width_()
  , height_()
  , progressive_sizes_()
{}

photoSize::photoSize(string const &type_, object_ptr<file> &&photo_, int32 width_, int32 height_, array<int32> &&progressive_sizes_)
  : type_(type_)
  , photo_(std::move(photo_))
  , width_(width_)
  , height_(height_)
  , progressive_sizes_(std::move(progressive_sizes_))
{}

const std::int32_t photoSize::ID;

void photoSize::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photoSize");
    s.store_field("type", type_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_field("width", width_);
    s.store_field("height", height_);
    { s.store_vector_begin("progressive_sizes", progressive_sizes_.size()); for (const auto &_value : progressive_sizes_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

premiumGiftPaymentOptions::premiumGiftPaymentOptions()
  : options_()
{}

premiumGiftPaymentOptions::premiumGiftPaymentOptions(array<object_ptr<premiumGiftPaymentOption>> &&options_)
  : options_(std::move(options_))
{}

const std::int32_t premiumGiftPaymentOptions::ID;

void premiumGiftPaymentOptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumGiftPaymentOptions");
    { s.store_vector_begin("options", options_.size()); for (const auto &_value : options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

publicChatTypeHasUsername::publicChatTypeHasUsername() {
}

const std::int32_t publicChatTypeHasUsername::ID;

void publicChatTypeHasUsername::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "publicChatTypeHasUsername");
    s.store_class_end();
  }
}

publicChatTypeIsLocationBased::publicChatTypeIsLocationBased() {
}

const std::int32_t publicChatTypeIsLocationBased::ID;

void publicChatTypeIsLocationBased::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "publicChatTypeIsLocationBased");
    s.store_class_end();
  }
}

quickReplyShortcut::quickReplyShortcut()
  : id_()
  , name_()
  , first_message_()
  , message_count_()
{}

quickReplyShortcut::quickReplyShortcut(int32 id_, string const &name_, object_ptr<quickReplyMessage> &&first_message_, int32 message_count_)
  : id_(id_)
  , name_(name_)
  , first_message_(std::move(first_message_))
  , message_count_(message_count_)
{}

const std::int32_t quickReplyShortcut::ID;

void quickReplyShortcut::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "quickReplyShortcut");
    s.store_field("id", id_);
    s.store_field("name", name_);
    s.store_object_field("first_message", static_cast<const BaseObject *>(first_message_.get()));
    s.store_field("message_count", message_count_);
    s.store_class_end();
  }
}

reactionTypeEmoji::reactionTypeEmoji()
  : emoji_()
{}

reactionTypeEmoji::reactionTypeEmoji(string const &emoji_)
  : emoji_(emoji_)
{}

const std::int32_t reactionTypeEmoji::ID;

void reactionTypeEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reactionTypeEmoji");
    s.store_field("emoji", emoji_);
    s.store_class_end();
  }
}

reactionTypeCustomEmoji::reactionTypeCustomEmoji()
  : custom_emoji_id_()
{}

reactionTypeCustomEmoji::reactionTypeCustomEmoji(int64 custom_emoji_id_)
  : custom_emoji_id_(custom_emoji_id_)
{}

const std::int32_t reactionTypeCustomEmoji::ID;

void reactionTypeCustomEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reactionTypeCustomEmoji");
    s.store_field("custom_emoji_id", custom_emoji_id_);
    s.store_class_end();
  }
}

reactionTypePaid::reactionTypePaid() {
}

const std::int32_t reactionTypePaid::ID;

void reactionTypePaid::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reactionTypePaid");
    s.store_class_end();
  }
}

reactionUnavailabilityReasonAnonymousAdministrator::reactionUnavailabilityReasonAnonymousAdministrator() {
}

const std::int32_t reactionUnavailabilityReasonAnonymousAdministrator::ID;

void reactionUnavailabilityReasonAnonymousAdministrator::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reactionUnavailabilityReasonAnonymousAdministrator");
    s.store_class_end();
  }
}

reactionUnavailabilityReasonGuest::reactionUnavailabilityReasonGuest() {
}

const std::int32_t reactionUnavailabilityReasonGuest::ID;

void reactionUnavailabilityReasonGuest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reactionUnavailabilityReasonGuest");
    s.store_class_end();
  }
}

recoveryEmailAddress::recoveryEmailAddress()
  : recovery_email_address_()
{}

recoveryEmailAddress::recoveryEmailAddress(string const &recovery_email_address_)
  : recovery_email_address_(recovery_email_address_)
{}

const std::int32_t recoveryEmailAddress::ID;

void recoveryEmailAddress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "recoveryEmailAddress");
    s.store_field("recovery_email_address", recovery_email_address_);
    s.store_class_end();
  }
}

savedCredentials::savedCredentials()
  : id_()
  , title_()
{}

savedCredentials::savedCredentials(string const &id_, string const &title_)
  : id_(id_)
  , title_(title_)
{}

const std::int32_t savedCredentials::ID;

void savedCredentials::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "savedCredentials");
    s.store_field("id", id_);
    s.store_field("title", title_);
    s.store_class_end();
  }
}

scopeAutosaveSettings::scopeAutosaveSettings()
  : autosave_photos_()
  , autosave_videos_()
  , max_video_file_size_()
{}

scopeAutosaveSettings::scopeAutosaveSettings(bool autosave_photos_, bool autosave_videos_, int53 max_video_file_size_)
  : autosave_photos_(autosave_photos_)
  , autosave_videos_(autosave_videos_)
  , max_video_file_size_(max_video_file_size_)
{}

const std::int32_t scopeAutosaveSettings::ID;

void scopeAutosaveSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "scopeAutosaveSettings");
    s.store_field("autosave_photos", autosave_photos_);
    s.store_field("autosave_videos", autosave_videos_);
    s.store_field("max_video_file_size", max_video_file_size_);
    s.store_class_end();
  }
}

starGiveawayPaymentOptions::starGiveawayPaymentOptions()
  : options_()
{}

starGiveawayPaymentOptions::starGiveawayPaymentOptions(array<object_ptr<starGiveawayPaymentOption>> &&options_)
  : options_(std::move(options_))
{}

const std::int32_t starGiveawayPaymentOptions::ID;

void starGiveawayPaymentOptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starGiveawayPaymentOptions");
    { s.store_vector_begin("options", options_.size()); for (const auto &_value : options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

starPaymentOption::starPaymentOption()
  : currency_()
  , amount_()
  , star_count_()
  , store_product_id_()
  , is_additional_()
{}

starPaymentOption::starPaymentOption(string const &currency_, int53 amount_, int53 star_count_, string const &store_product_id_, bool is_additional_)
  : currency_(currency_)
  , amount_(amount_)
  , star_count_(star_count_)
  , store_product_id_(store_product_id_)
  , is_additional_(is_additional_)
{}

const std::int32_t starPaymentOption::ID;

void starPaymentOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starPaymentOption");
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("star_count", star_count_);
    s.store_field("store_product_id", store_product_id_);
    s.store_field("is_additional", is_additional_);
    s.store_class_end();
  }
}

starSubscription::starSubscription()
  : id_()
  , chat_id_()
  , expiration_date_()
  , is_canceled_()
  , is_expiring_()
  , pricing_()
  , type_()
{}

starSubscription::starSubscription(string const &id_, int53 chat_id_, int32 expiration_date_, bool is_canceled_, bool is_expiring_, object_ptr<starSubscriptionPricing> &&pricing_, object_ptr<StarSubscriptionType> &&type_)
  : id_(id_)
  , chat_id_(chat_id_)
  , expiration_date_(expiration_date_)
  , is_canceled_(is_canceled_)
  , is_expiring_(is_expiring_)
  , pricing_(std::move(pricing_))
  , type_(std::move(type_))
{}

const std::int32_t starSubscription::ID;

void starSubscription::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starSubscription");
    s.store_field("id", id_);
    s.store_field("chat_id", chat_id_);
    s.store_field("expiration_date", expiration_date_);
    s.store_field("is_canceled", is_canceled_);
    s.store_field("is_expiring", is_expiring_);
    s.store_object_field("pricing", static_cast<const BaseObject *>(pricing_.get()));
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

starSubscriptionPricing::starSubscriptionPricing()
  : period_()
  , star_count_()
{}

starSubscriptionPricing::starSubscriptionPricing(int32 period_, int53 star_count_)
  : period_(period_)
  , star_count_(star_count_)
{}

const std::int32_t starSubscriptionPricing::ID;

void starSubscriptionPricing::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starSubscriptionPricing");
    s.store_field("period", period_);
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

stickerSetInfo::stickerSetInfo()
  : id_()
  , title_()
  , name_()
  , thumbnail_()
  , thumbnail_outline_()
  , is_owned_()
  , is_installed_()
  , is_archived_()
  , is_official_()
  , sticker_type_()
  , needs_repainting_()
  , is_allowed_as_chat_emoji_status_()
  , is_viewed_()
  , size_()
  , covers_()
{}

stickerSetInfo::stickerSetInfo(int64 id_, string const &title_, string const &name_, object_ptr<thumbnail> &&thumbnail_, object_ptr<outline> &&thumbnail_outline_, bool is_owned_, bool is_installed_, bool is_archived_, bool is_official_, object_ptr<StickerType> &&sticker_type_, bool needs_repainting_, bool is_allowed_as_chat_emoji_status_, bool is_viewed_, int32 size_, array<object_ptr<sticker>> &&covers_)
  : id_(id_)
  , title_(title_)
  , name_(name_)
  , thumbnail_(std::move(thumbnail_))
  , thumbnail_outline_(std::move(thumbnail_outline_))
  , is_owned_(is_owned_)
  , is_installed_(is_installed_)
  , is_archived_(is_archived_)
  , is_official_(is_official_)
  , sticker_type_(std::move(sticker_type_))
  , needs_repainting_(needs_repainting_)
  , is_allowed_as_chat_emoji_status_(is_allowed_as_chat_emoji_status_)
  , is_viewed_(is_viewed_)
  , size_(size_)
  , covers_(std::move(covers_))
{}

const std::int32_t stickerSetInfo::ID;

void stickerSetInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickerSetInfo");
    s.store_field("id", id_);
    s.store_field("title", title_);
    s.store_field("name", name_);
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_object_field("thumbnail_outline", static_cast<const BaseObject *>(thumbnail_outline_.get()));
    s.store_field("is_owned", is_owned_);
    s.store_field("is_installed", is_installed_);
    s.store_field("is_archived", is_archived_);
    s.store_field("is_official", is_official_);
    s.store_object_field("sticker_type", static_cast<const BaseObject *>(sticker_type_.get()));
    s.store_field("needs_repainting", needs_repainting_);
    s.store_field("is_allowed_as_chat_emoji_status", is_allowed_as_chat_emoji_status_);
    s.store_field("is_viewed", is_viewed_);
    s.store_field("size", size_);
    { s.store_vector_begin("covers", covers_.size()); for (const auto &_value : covers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

storePaymentPurposePremiumSubscription::storePaymentPurposePremiumSubscription()
  : is_restore_()
  , is_upgrade_()
{}

storePaymentPurposePremiumSubscription::storePaymentPurposePremiumSubscription(bool is_restore_, bool is_upgrade_)
  : is_restore_(is_restore_)
  , is_upgrade_(is_upgrade_)
{}

const std::int32_t storePaymentPurposePremiumSubscription::ID;

void storePaymentPurposePremiumSubscription::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storePaymentPurposePremiumSubscription");
    s.store_field("is_restore", is_restore_);
    s.store_field("is_upgrade", is_upgrade_);
    s.store_class_end();
  }
}

storePaymentPurposePremiumGift::storePaymentPurposePremiumGift()
  : currency_()
  , amount_()
  , user_id_()
  , text_()
{}

storePaymentPurposePremiumGift::storePaymentPurposePremiumGift(string const &currency_, int53 amount_, int53 user_id_, object_ptr<formattedText> &&text_)
  : currency_(currency_)
  , amount_(amount_)
  , user_id_(user_id_)
  , text_(std::move(text_))
{}

const std::int32_t storePaymentPurposePremiumGift::ID;

void storePaymentPurposePremiumGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storePaymentPurposePremiumGift");
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("user_id", user_id_);
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

storePaymentPurposePremiumGiftCodes::storePaymentPurposePremiumGiftCodes()
  : boosted_chat_id_()
  , currency_()
  , amount_()
  , user_ids_()
  , text_()
{}

storePaymentPurposePremiumGiftCodes::storePaymentPurposePremiumGiftCodes(int53 boosted_chat_id_, string const &currency_, int53 amount_, array<int53> &&user_ids_, object_ptr<formattedText> &&text_)
  : boosted_chat_id_(boosted_chat_id_)
  , currency_(currency_)
  , amount_(amount_)
  , user_ids_(std::move(user_ids_))
  , text_(std::move(text_))
{}

const std::int32_t storePaymentPurposePremiumGiftCodes::ID;

void storePaymentPurposePremiumGiftCodes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storePaymentPurposePremiumGiftCodes");
    s.store_field("boosted_chat_id", boosted_chat_id_);
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    { s.store_vector_begin("user_ids", user_ids_.size()); for (const auto &_value : user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

storePaymentPurposePremiumGiveaway::storePaymentPurposePremiumGiveaway()
  : parameters_()
  , currency_()
  , amount_()
{}

storePaymentPurposePremiumGiveaway::storePaymentPurposePremiumGiveaway(object_ptr<giveawayParameters> &&parameters_, string const &currency_, int53 amount_)
  : parameters_(std::move(parameters_))
  , currency_(currency_)
  , amount_(amount_)
{}

const std::int32_t storePaymentPurposePremiumGiveaway::ID;

void storePaymentPurposePremiumGiveaway::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storePaymentPurposePremiumGiveaway");
    s.store_object_field("parameters", static_cast<const BaseObject *>(parameters_.get()));
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_class_end();
  }
}

storePaymentPurposeStarGiveaway::storePaymentPurposeStarGiveaway()
  : parameters_()
  , currency_()
  , amount_()
  , winner_count_()
  , star_count_()
{}

storePaymentPurposeStarGiveaway::storePaymentPurposeStarGiveaway(object_ptr<giveawayParameters> &&parameters_, string const &currency_, int53 amount_, int32 winner_count_, int53 star_count_)
  : parameters_(std::move(parameters_))
  , currency_(currency_)
  , amount_(amount_)
  , winner_count_(winner_count_)
  , star_count_(star_count_)
{}

const std::int32_t storePaymentPurposeStarGiveaway::ID;

void storePaymentPurposeStarGiveaway::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storePaymentPurposeStarGiveaway");
    s.store_object_field("parameters", static_cast<const BaseObject *>(parameters_.get()));
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("winner_count", winner_count_);
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

storePaymentPurposeStars::storePaymentPurposeStars()
  : currency_()
  , amount_()
  , star_count_()
  , chat_id_()
{}

storePaymentPurposeStars::storePaymentPurposeStars(string const &currency_, int53 amount_, int53 star_count_, int53 chat_id_)
  : currency_(currency_)
  , amount_(amount_)
  , star_count_(star_count_)
  , chat_id_(chat_id_)
{}

const std::int32_t storePaymentPurposeStars::ID;

void storePaymentPurposeStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storePaymentPurposeStars");
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("star_count", star_count_);
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

storePaymentPurposeGiftedStars::storePaymentPurposeGiftedStars()
  : user_id_()
  , currency_()
  , amount_()
  , star_count_()
{}

storePaymentPurposeGiftedStars::storePaymentPurposeGiftedStars(int53 user_id_, string const &currency_, int53 amount_, int53 star_count_)
  : user_id_(user_id_)
  , currency_(currency_)
  , amount_(amount_)
  , star_count_(star_count_)
{}

const std::int32_t storePaymentPurposeGiftedStars::ID;

void storePaymentPurposeGiftedStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storePaymentPurposeGiftedStars");
    s.store_field("user_id", user_id_);
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

storeTransactionAppStore::storeTransactionAppStore()
  : receipt_()
{}

storeTransactionAppStore::storeTransactionAppStore(bytes const &receipt_)
  : receipt_(std::move(receipt_))
{}

const std::int32_t storeTransactionAppStore::ID;

void storeTransactionAppStore::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storeTransactionAppStore");
    s.store_bytes_field("receipt", receipt_);
    s.store_class_end();
  }
}

storeTransactionGooglePlay::storeTransactionGooglePlay()
  : package_name_()
  , store_product_id_()
  , purchase_token_()
{}

storeTransactionGooglePlay::storeTransactionGooglePlay(string const &package_name_, string const &store_product_id_, string const &purchase_token_)
  : package_name_(package_name_)
  , store_product_id_(store_product_id_)
  , purchase_token_(purchase_token_)
{}

const std::int32_t storeTransactionGooglePlay::ID;

void storeTransactionGooglePlay::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storeTransactionGooglePlay");
    s.store_field("package_name", package_name_);
    s.store_field("store_product_id", store_product_id_);
    s.store_field("purchase_token", purchase_token_);
    s.store_class_end();
  }
}

storyListMain::storyListMain() {
}

const std::int32_t storyListMain::ID;

void storyListMain::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyListMain");
    s.store_class_end();
  }
}

storyListArchive::storyListArchive() {
}

const std::int32_t storyListArchive::ID;

void storyListArchive::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyListArchive");
    s.store_class_end();
  }
}

tMeUrlTypeUser::tMeUrlTypeUser()
  : user_id_()
{}

tMeUrlTypeUser::tMeUrlTypeUser(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t tMeUrlTypeUser::ID;

void tMeUrlTypeUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "tMeUrlTypeUser");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

tMeUrlTypeSupergroup::tMeUrlTypeSupergroup()
  : supergroup_id_()
{}

tMeUrlTypeSupergroup::tMeUrlTypeSupergroup(int53 supergroup_id_)
  : supergroup_id_(supergroup_id_)
{}

const std::int32_t tMeUrlTypeSupergroup::ID;

void tMeUrlTypeSupergroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "tMeUrlTypeSupergroup");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_class_end();
  }
}

tMeUrlTypeChatInvite::tMeUrlTypeChatInvite()
  : info_()
{}

tMeUrlTypeChatInvite::tMeUrlTypeChatInvite(object_ptr<chatInviteLinkInfo> &&info_)
  : info_(std::move(info_))
{}

const std::int32_t tMeUrlTypeChatInvite::ID;

void tMeUrlTypeChatInvite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "tMeUrlTypeChatInvite");
    s.store_object_field("info", static_cast<const BaseObject *>(info_.get()));
    s.store_class_end();
  }
}

tMeUrlTypeStickerSet::tMeUrlTypeStickerSet()
  : sticker_set_id_()
{}

tMeUrlTypeStickerSet::tMeUrlTypeStickerSet(int64 sticker_set_id_)
  : sticker_set_id_(sticker_set_id_)
{}

const std::int32_t tMeUrlTypeStickerSet::ID;

void tMeUrlTypeStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "tMeUrlTypeStickerSet");
    s.store_field("sticker_set_id", sticker_set_id_);
    s.store_class_end();
  }
}

upgradeGiftResult::upgradeGiftResult()
  : gift_()
  , received_gift_id_()
  , is_saved_()
  , can_be_transferred_()
  , transfer_star_count_()
  , drop_original_details_star_count_()
  , next_transfer_date_()
  , next_resale_date_()
  , export_date_()
{}

upgradeGiftResult::upgradeGiftResult(object_ptr<upgradedGift> &&gift_, string const &received_gift_id_, bool is_saved_, bool can_be_transferred_, int53 transfer_star_count_, int53 drop_original_details_star_count_, int32 next_transfer_date_, int32 next_resale_date_, int32 export_date_)
  : gift_(std::move(gift_))
  , received_gift_id_(received_gift_id_)
  , is_saved_(is_saved_)
  , can_be_transferred_(can_be_transferred_)
  , transfer_star_count_(transfer_star_count_)
  , drop_original_details_star_count_(drop_original_details_star_count_)
  , next_transfer_date_(next_transfer_date_)
  , next_resale_date_(next_resale_date_)
  , export_date_(export_date_)
{}

const std::int32_t upgradeGiftResult::ID;

void upgradeGiftResult::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradeGiftResult");
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_field("received_gift_id", received_gift_id_);
    s.store_field("is_saved", is_saved_);
    s.store_field("can_be_transferred", can_be_transferred_);
    s.store_field("transfer_star_count", transfer_star_count_);
    s.store_field("drop_original_details_star_count", drop_original_details_star_count_);
    s.store_field("next_transfer_date", next_transfer_date_);
    s.store_field("next_resale_date", next_resale_date_);
    s.store_field("export_date", export_date_);
    s.store_class_end();
  }
}

upgradedGiftBackdropCount::upgradedGiftBackdropCount()
  : backdrop_()
  , total_count_()
{}

upgradedGiftBackdropCount::upgradedGiftBackdropCount(object_ptr<upgradedGiftBackdrop> &&backdrop_, int32 total_count_)
  : backdrop_(std::move(backdrop_))
  , total_count_(total_count_)
{}

const std::int32_t upgradedGiftBackdropCount::ID;

void upgradedGiftBackdropCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradedGiftBackdropCount");
    s.store_object_field("backdrop", static_cast<const BaseObject *>(backdrop_.get()));
    s.store_field("total_count", total_count_);
    s.store_class_end();
  }
}

venue::venue()
  : location_()
  , title_()
  , address_()
  , provider_()
  , id_()
  , type_()
{}

venue::venue(object_ptr<location> &&location_, string const &title_, string const &address_, string const &provider_, string const &id_, string const &type_)
  : location_(std::move(location_))
  , title_(title_)
  , address_(address_)
  , provider_(provider_)
  , id_(id_)
  , type_(type_)
{}

const std::int32_t venue::ID;

void venue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "venue");
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_field("title", title_);
    s.store_field("address", address_);
    s.store_field("provider", provider_);
    s.store_field("id", id_);
    s.store_field("type", type_);
    s.store_class_end();
  }
}

webAppOpenParameters::webAppOpenParameters()
  : theme_()
  , application_name_()
  , mode_()
{}

webAppOpenParameters::webAppOpenParameters(object_ptr<themeParameters> &&theme_, string const &application_name_, object_ptr<WebAppOpenMode> &&mode_)
  : theme_(std::move(theme_))
  , application_name_(application_name_)
  , mode_(std::move(mode_))
{}

const std::int32_t webAppOpenParameters::ID;

void webAppOpenParameters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webAppOpenParameters");
    s.store_object_field("theme", static_cast<const BaseObject *>(theme_.get()));
    s.store_field("application_name", application_name_);
    s.store_object_field("mode", static_cast<const BaseObject *>(mode_.get()));
    s.store_class_end();
  }
}

allowUnpaidMessagesFromUser::allowUnpaidMessagesFromUser()
  : user_id_()
  , refund_payments_()
{}

allowUnpaidMessagesFromUser::allowUnpaidMessagesFromUser(int53 user_id_, bool refund_payments_)
  : user_id_(user_id_)
  , refund_payments_(refund_payments_)
{}

const std::int32_t allowUnpaidMessagesFromUser::ID;

void allowUnpaidMessagesFromUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "allowUnpaidMessagesFromUser");
    s.store_field("user_id", user_id_);
    s.store_field("refund_payments", refund_payments_);
    s.store_class_end();
  }
}

answerPreCheckoutQuery::answerPreCheckoutQuery()
  : pre_checkout_query_id_()
  , error_message_()
{}

answerPreCheckoutQuery::answerPreCheckoutQuery(int64 pre_checkout_query_id_, string const &error_message_)
  : pre_checkout_query_id_(pre_checkout_query_id_)
  , error_message_(error_message_)
{}

const std::int32_t answerPreCheckoutQuery::ID;

void answerPreCheckoutQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "answerPreCheckoutQuery");
    s.store_field("pre_checkout_query_id", pre_checkout_query_id_);
    s.store_field("error_message", error_message_);
    s.store_class_end();
  }
}

blockMessageSenderFromReplies::blockMessageSenderFromReplies()
  : message_id_()
  , delete_message_()
  , delete_all_messages_()
  , report_spam_()
{}

blockMessageSenderFromReplies::blockMessageSenderFromReplies(int53 message_id_, bool delete_message_, bool delete_all_messages_, bool report_spam_)
  : message_id_(message_id_)
  , delete_message_(delete_message_)
  , delete_all_messages_(delete_all_messages_)
  , report_spam_(report_spam_)
{}

const std::int32_t blockMessageSenderFromReplies::ID;

void blockMessageSenderFromReplies::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "blockMessageSenderFromReplies");
    s.store_field("message_id", message_id_);
    s.store_field("delete_message", delete_message_);
    s.store_field("delete_all_messages", delete_all_messages_);
    s.store_field("report_spam", report_spam_);
    s.store_class_end();
  }
}

boostChat::boostChat()
  : chat_id_()
  , slot_ids_()
{}

boostChat::boostChat(int53 chat_id_, array<int32> &&slot_ids_)
  : chat_id_(chat_id_)
  , slot_ids_(std::move(slot_ids_))
{}

const std::int32_t boostChat::ID;

void boostChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "boostChat");
    s.store_field("chat_id", chat_id_);
    { s.store_vector_begin("slot_ids", slot_ids_.size()); for (const auto &_value : slot_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

checkPasswordRecoveryCode::checkPasswordRecoveryCode()
  : recovery_code_()
{}

checkPasswordRecoveryCode::checkPasswordRecoveryCode(string const &recovery_code_)
  : recovery_code_(recovery_code_)
{}

const std::int32_t checkPasswordRecoveryCode::ID;

void checkPasswordRecoveryCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkPasswordRecoveryCode");
    s.store_field("recovery_code", recovery_code_);
    s.store_class_end();
  }
}

createNewStickerSet::createNewStickerSet()
  : user_id_()
  , title_()
  , name_()
  , sticker_type_()
  , needs_repainting_()
  , stickers_()
  , source_()
{}

createNewStickerSet::createNewStickerSet(int53 user_id_, string const &title_, string const &name_, object_ptr<StickerType> &&sticker_type_, bool needs_repainting_, array<object_ptr<inputSticker>> &&stickers_, string const &source_)
  : user_id_(user_id_)
  , title_(title_)
  , name_(name_)
  , sticker_type_(std::move(sticker_type_))
  , needs_repainting_(needs_repainting_)
  , stickers_(std::move(stickers_))
  , source_(source_)
{}

const std::int32_t createNewStickerSet::ID;

void createNewStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createNewStickerSet");
    s.store_field("user_id", user_id_);
    s.store_field("title", title_);
    s.store_field("name", name_);
    s.store_object_field("sticker_type", static_cast<const BaseObject *>(sticker_type_.get()));
    s.store_field("needs_repainting", needs_repainting_);
    { s.store_vector_begin("stickers", stickers_.size()); for (const auto &_value : stickers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("source", source_);
    s.store_class_end();
  }
}

deleteChatReplyMarkup::deleteChatReplyMarkup()
  : chat_id_()
  , message_id_()
{}

deleteChatReplyMarkup::deleteChatReplyMarkup(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t deleteChatReplyMarkup::ID;

void deleteChatReplyMarkup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteChatReplyMarkup");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

deleteFile::deleteFile()
  : file_id_()
{}

deleteFile::deleteFile(int32 file_id_)
  : file_id_(file_id_)
{}

const std::int32_t deleteFile::ID;

void deleteFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteFile");
    s.store_field("file_id", file_id_);
    s.store_class_end();
  }
}

deletePassportElement::deletePassportElement()
  : type_()
{}

deletePassportElement::deletePassportElement(object_ptr<PassportElementType> &&type_)
  : type_(std::move(type_))
{}

const std::int32_t deletePassportElement::ID;

void deletePassportElement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deletePassportElement");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

disconnectAllWebsites::disconnectAllWebsites() {
}

const std::int32_t disconnectAllWebsites::ID;

void disconnectAllWebsites::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "disconnectAllWebsites");
    s.store_class_end();
  }
}

editBusinessMessageCaption::editBusinessMessageCaption()
  : business_connection_id_()
  , chat_id_()
  , message_id_()
  , reply_markup_()
  , caption_()
  , show_caption_above_media_()
{}

editBusinessMessageCaption::editBusinessMessageCaption(string const &business_connection_id_, int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<formattedText> &&caption_, bool show_caption_above_media_)
  : business_connection_id_(business_connection_id_)
  , chat_id_(chat_id_)
  , message_id_(message_id_)
  , reply_markup_(std::move(reply_markup_))
  , caption_(std::move(caption_))
  , show_caption_above_media_(show_caption_above_media_)
{}

const std::int32_t editBusinessMessageCaption::ID;

void editBusinessMessageCaption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editBusinessMessageCaption");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_field("show_caption_above_media", show_caption_above_media_);
    s.store_class_end();
  }
}

editBusinessMessageReplyMarkup::editBusinessMessageReplyMarkup()
  : business_connection_id_()
  , chat_id_()
  , message_id_()
  , reply_markup_()
{}

editBusinessMessageReplyMarkup::editBusinessMessageReplyMarkup(string const &business_connection_id_, int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_)
  : business_connection_id_(business_connection_id_)
  , chat_id_(chat_id_)
  , message_id_(message_id_)
  , reply_markup_(std::move(reply_markup_))
{}

const std::int32_t editBusinessMessageReplyMarkup::ID;

void editBusinessMessageReplyMarkup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editBusinessMessageReplyMarkup");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_class_end();
  }
}

editChatInviteLink::editChatInviteLink()
  : chat_id_()
  , invite_link_()
  , name_()
  , expiration_date_()
  , member_limit_()
  , creates_join_request_()
{}

editChatInviteLink::editChatInviteLink(int53 chat_id_, string const &invite_link_, string const &name_, int32 expiration_date_, int32 member_limit_, bool creates_join_request_)
  : chat_id_(chat_id_)
  , invite_link_(invite_link_)
  , name_(name_)
  , expiration_date_(expiration_date_)
  , member_limit_(member_limit_)
  , creates_join_request_(creates_join_request_)
{}

const std::int32_t editChatInviteLink::ID;

void editChatInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editChatInviteLink");
    s.store_field("chat_id", chat_id_);
    s.store_field("invite_link", invite_link_);
    s.store_field("name", name_);
    s.store_field("expiration_date", expiration_date_);
    s.store_field("member_limit", member_limit_);
    s.store_field("creates_join_request", creates_join_request_);
    s.store_class_end();
  }
}

editInlineMessageLiveLocation::editInlineMessageLiveLocation()
  : inline_message_id_()
  , reply_markup_()
  , location_()
  , live_period_()
  , heading_()
  , proximity_alert_radius_()
{}

editInlineMessageLiveLocation::editInlineMessageLiveLocation(string const &inline_message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<location> &&location_, int32 live_period_, int32 heading_, int32 proximity_alert_radius_)
  : inline_message_id_(inline_message_id_)
  , reply_markup_(std::move(reply_markup_))
  , location_(std::move(location_))
  , live_period_(live_period_)
  , heading_(heading_)
  , proximity_alert_radius_(proximity_alert_radius_)
{}

const std::int32_t editInlineMessageLiveLocation::ID;

void editInlineMessageLiveLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editInlineMessageLiveLocation");
    s.store_field("inline_message_id", inline_message_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_field("live_period", live_period_);
    s.store_field("heading", heading_);
    s.store_field("proximity_alert_radius", proximity_alert_radius_);
    s.store_class_end();
  }
}

editMessageCaption::editMessageCaption()
  : chat_id_()
  , message_id_()
  , reply_markup_()
  , caption_()
  , show_caption_above_media_()
{}

editMessageCaption::editMessageCaption(int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<formattedText> &&caption_, bool show_caption_above_media_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , reply_markup_(std::move(reply_markup_))
  , caption_(std::move(caption_))
  , show_caption_above_media_(show_caption_above_media_)
{}

const std::int32_t editMessageCaption::ID;

void editMessageCaption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editMessageCaption");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_field("show_caption_above_media", show_caption_above_media_);
    s.store_class_end();
  }
}

editMessageReplyMarkup::editMessageReplyMarkup()
  : chat_id_()
  , message_id_()
  , reply_markup_()
{}

editMessageReplyMarkup::editMessageReplyMarkup(int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , reply_markup_(std::move(reply_markup_))
{}

const std::int32_t editMessageReplyMarkup::ID;

void editMessageReplyMarkup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editMessageReplyMarkup");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_class_end();
  }
}

editStarSubscription::editStarSubscription()
  : subscription_id_()
  , is_canceled_()
{}

editStarSubscription::editStarSubscription(string const &subscription_id_, bool is_canceled_)
  : subscription_id_(subscription_id_)
  , is_canceled_(is_canceled_)
{}

const std::int32_t editStarSubscription::ID;

void editStarSubscription::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editStarSubscription");
    s.store_field("subscription_id", subscription_id_);
    s.store_field("is_canceled", is_canceled_);
    s.store_class_end();
  }
}

editStory::editStory()
  : story_poster_chat_id_()
  , story_id_()
  , content_()
  , areas_()
  , caption_()
{}

editStory::editStory(int53 story_poster_chat_id_, int32 story_id_, object_ptr<InputStoryContent> &&content_, object_ptr<inputStoryAreas> &&areas_, object_ptr<formattedText> &&caption_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , story_id_(story_id_)
  , content_(std::move(content_))
  , areas_(std::move(areas_))
  , caption_(std::move(caption_))
{}

const std::int32_t editStory::ID;

void editStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editStory");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("story_id", story_id_);
    s.store_object_field("content", static_cast<const BaseObject *>(content_.get()));
    s.store_object_field("areas", static_cast<const BaseObject *>(areas_.get()));
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

getAllStickerEmojis::getAllStickerEmojis()
  : sticker_type_()
  , query_()
  , chat_id_()
  , return_only_main_emoji_()
{}

getAllStickerEmojis::getAllStickerEmojis(object_ptr<StickerType> &&sticker_type_, string const &query_, int53 chat_id_, bool return_only_main_emoji_)
  : sticker_type_(std::move(sticker_type_))
  , query_(query_)
  , chat_id_(chat_id_)
  , return_only_main_emoji_(return_only_main_emoji_)
{}

const std::int32_t getAllStickerEmojis::ID;

void getAllStickerEmojis::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getAllStickerEmojis");
    s.store_object_field("sticker_type", static_cast<const BaseObject *>(sticker_type_.get()));
    s.store_field("query", query_);
    s.store_field("chat_id", chat_id_);
    s.store_field("return_only_main_emoji", return_only_main_emoji_);
    s.store_class_end();
  }
}

getAnimatedEmoji::getAnimatedEmoji()
  : emoji_()
{}

getAnimatedEmoji::getAnimatedEmoji(string const &emoji_)
  : emoji_(emoji_)
{}

const std::int32_t getAnimatedEmoji::ID;

void getAnimatedEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getAnimatedEmoji");
    s.store_field("emoji", emoji_);
    s.store_class_end();
  }
}

getArchiveChatListSettings::getArchiveChatListSettings() {
}

const std::int32_t getArchiveChatListSettings::ID;

void getArchiveChatListSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getArchiveChatListSettings");
    s.store_class_end();
  }
}

getAttachmentMenuBot::getAttachmentMenuBot()
  : bot_user_id_()
{}

getAttachmentMenuBot::getAttachmentMenuBot(int53 bot_user_id_)
  : bot_user_id_(bot_user_id_)
{}

const std::int32_t getAttachmentMenuBot::ID;

void getAttachmentMenuBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getAttachmentMenuBot");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_class_end();
  }
}

getBasicGroup::getBasicGroup()
  : basic_group_id_()
{}

getBasicGroup::getBasicGroup(int53 basic_group_id_)
  : basic_group_id_(basic_group_id_)
{}

const std::int32_t getBasicGroup::ID;

void getBasicGroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getBasicGroup");
    s.store_field("basic_group_id", basic_group_id_);
    s.store_class_end();
  }
}

getBotInfoDescription::getBotInfoDescription()
  : bot_user_id_()
  , language_code_()
{}

getBotInfoDescription::getBotInfoDescription(int53 bot_user_id_, string const &language_code_)
  : bot_user_id_(bot_user_id_)
  , language_code_(language_code_)
{}

const std::int32_t getBotInfoDescription::ID;

void getBotInfoDescription::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getBotInfoDescription");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("language_code", language_code_);
    s.store_class_end();
  }
}

getBotMediaPreviewInfo::getBotMediaPreviewInfo()
  : bot_user_id_()
  , language_code_()
{}

getBotMediaPreviewInfo::getBotMediaPreviewInfo(int53 bot_user_id_, string const &language_code_)
  : bot_user_id_(bot_user_id_)
  , language_code_(language_code_)
{}

const std::int32_t getBotMediaPreviewInfo::ID;

void getBotMediaPreviewInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getBotMediaPreviewInfo");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("language_code", language_code_);
    s.store_class_end();
  }
}

getChatBoostLink::getChatBoostLink()
  : chat_id_()
{}

getChatBoostLink::getChatBoostLink(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t getChatBoostLink::ID;

void getChatBoostLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatBoostLink");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

getChatBoostStatus::getChatBoostStatus()
  : chat_id_()
{}

getChatBoostStatus::getChatBoostStatus(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t getChatBoostStatus::ID;

void getChatBoostStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatBoostStatus");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

getChatFolderChatsToLeave::getChatFolderChatsToLeave()
  : chat_folder_id_()
{}

getChatFolderChatsToLeave::getChatFolderChatsToLeave(int32 chat_folder_id_)
  : chat_folder_id_(chat_folder_id_)
{}

const std::int32_t getChatFolderChatsToLeave::ID;

void getChatFolderChatsToLeave::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatFolderChatsToLeave");
    s.store_field("chat_folder_id", chat_folder_id_);
    s.store_class_end();
  }
}

getChatMessageCalendar::getChatMessageCalendar()
  : chat_id_()
  , topic_id_()
  , filter_()
  , from_message_id_()
{}

getChatMessageCalendar::getChatMessageCalendar(int53 chat_id_, object_ptr<MessageTopic> &&topic_id_, object_ptr<SearchMessagesFilter> &&filter_, int53 from_message_id_)
  : chat_id_(chat_id_)
  , topic_id_(std::move(topic_id_))
  , filter_(std::move(filter_))
  , from_message_id_(from_message_id_)
{}

const std::int32_t getChatMessageCalendar::ID;

void getChatMessageCalendar::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatMessageCalendar");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("topic_id", static_cast<const BaseObject *>(topic_id_.get()));
    s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get()));
    s.store_field("from_message_id", from_message_id_);
    s.store_class_end();
  }
}

getChatPinnedMessage::getChatPinnedMessage()
  : chat_id_()
{}

getChatPinnedMessage::getChatPinnedMessage(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t getChatPinnedMessage::ID;

void getChatPinnedMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatPinnedMessage");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

getChatSparseMessagePositions::getChatSparseMessagePositions()
  : chat_id_()
  , filter_()
  , from_message_id_()
  , limit_()
  , saved_messages_topic_id_()
{}

getChatSparseMessagePositions::getChatSparseMessagePositions(int53 chat_id_, object_ptr<SearchMessagesFilter> &&filter_, int53 from_message_id_, int32 limit_, int53 saved_messages_topic_id_)
  : chat_id_(chat_id_)
  , filter_(std::move(filter_))
  , from_message_id_(from_message_id_)
  , limit_(limit_)
  , saved_messages_topic_id_(saved_messages_topic_id_)
{}

const std::int32_t getChatSparseMessagePositions::ID;

void getChatSparseMessagePositions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatSparseMessagePositions");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get()));
    s.store_field("from_message_id", from_message_id_);
    s.store_field("limit", limit_);
    s.store_field("saved_messages_topic_id", saved_messages_topic_id_);
    s.store_class_end();
  }
}

getChatStatistics::getChatStatistics()
  : chat_id_()
  , is_dark_()
{}

getChatStatistics::getChatStatistics(int53 chat_id_, bool is_dark_)
  : chat_id_(chat_id_)
  , is_dark_(is_dark_)
{}

const std::int32_t getChatStatistics::ID;

void getChatStatistics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatStatistics");
    s.store_field("chat_id", chat_id_);
    s.store_field("is_dark", is_dark_);
    s.store_class_end();
  }
}

getChatsToPostStories::getChatsToPostStories() {
}

const std::int32_t getChatsToPostStories::ID;

void getChatsToPostStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatsToPostStories");
    s.store_class_end();
  }
}

getConnectedAffiliatePrograms::getConnectedAffiliatePrograms()
  : affiliate_()
  , offset_()
  , limit_()
{}

getConnectedAffiliatePrograms::getConnectedAffiliatePrograms(object_ptr<AffiliateType> &&affiliate_, string const &offset_, int32 limit_)
  : affiliate_(std::move(affiliate_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getConnectedAffiliatePrograms::ID;

void getConnectedAffiliatePrograms::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getConnectedAffiliatePrograms");
    s.store_object_field("affiliate", static_cast<const BaseObject *>(affiliate_.get()));
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getConnectedWebsites::getConnectedWebsites() {
}

const std::int32_t getConnectedWebsites::ID;

void getConnectedWebsites::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getConnectedWebsites");
    s.store_class_end();
  }
}

getEmojiReaction::getEmojiReaction()
  : emoji_()
{}

getEmojiReaction::getEmojiReaction(string const &emoji_)
  : emoji_(emoji_)
{}

const std::int32_t getEmojiReaction::ID;

void getEmojiReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getEmojiReaction");
    s.store_field("emoji", emoji_);
    s.store_class_end();
  }
}

getForumTopicDefaultIcons::getForumTopicDefaultIcons() {
}

const std::int32_t getForumTopicDefaultIcons::ID;

void getForumTopicDefaultIcons::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getForumTopicDefaultIcons");
    s.store_class_end();
  }
}

getLanguagePackInfo::getLanguagePackInfo()
  : language_pack_id_()
{}

getLanguagePackInfo::getLanguagePackInfo(string const &language_pack_id_)
  : language_pack_id_(language_pack_id_)
{}

const std::int32_t getLanguagePackInfo::ID;

void getLanguagePackInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getLanguagePackInfo");
    s.store_field("language_pack_id", language_pack_id_);
    s.store_class_end();
  }
}

getLogVerbosityLevel::getLogVerbosityLevel() {
}

const std::int32_t getLogVerbosityLevel::ID;

void getLogVerbosityLevel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getLogVerbosityLevel");
    s.store_class_end();
  }
}

getMessage::getMessage()
  : chat_id_()
  , message_id_()
{}

getMessage::getMessage(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t getMessage::ID;

void getMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMessage");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

getMessageLocally::getMessageLocally()
  : chat_id_()
  , message_id_()
{}

getMessageLocally::getMessageLocally(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t getMessageLocally::ID;

void getMessageLocally::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMessageLocally");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

getOwnedBots::getOwnedBots() {
}

const std::int32_t getOwnedBots::ID;

void getOwnedBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getOwnedBots");
    s.store_class_end();
  }
}

getPaidMessageRevenue::getPaidMessageRevenue()
  : user_id_()
{}

getPaidMessageRevenue::getPaidMessageRevenue(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t getPaidMessageRevenue::ID;

void getPaidMessageRevenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPaidMessageRevenue");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

getRecentStickers::getRecentStickers()
  : is_attached_()
{}

getRecentStickers::getRecentStickers(bool is_attached_)
  : is_attached_(is_attached_)
{}

const std::int32_t getRecentStickers::ID;

void getRecentStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getRecentStickers");
    s.store_field("is_attached", is_attached_);
    s.store_class_end();
  }
}

getStatisticalGraph::getStatisticalGraph()
  : chat_id_()
  , token_()
  , x_()
{}

getStatisticalGraph::getStatisticalGraph(int53 chat_id_, string const &token_, int53 x_)
  : chat_id_(chat_id_)
  , token_(token_)
  , x_(x_)
{}

const std::int32_t getStatisticalGraph::ID;

void getStatisticalGraph::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStatisticalGraph");
    s.store_field("chat_id", chat_id_);
    s.store_field("token", token_);
    s.store_field("x", x_);
    s.store_class_end();
  }
}

getStickerSet::getStickerSet()
  : set_id_()
{}

getStickerSet::getStickerSet(int64 set_id_)
  : set_id_(set_id_)
{}

const std::int32_t getStickerSet::ID;

void getStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStickerSet");
    s.store_field("set_id", set_id_);
    s.store_class_end();
  }
}

getUserProfileAudios::getUserProfileAudios()
  : user_id_()
  , offset_()
  , limit_()
{}

getUserProfileAudios::getUserProfileAudios(int53 user_id_, int32 offset_, int32 limit_)
  : user_id_(user_id_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getUserProfileAudios::ID;

void getUserProfileAudios::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getUserProfileAudios");
    s.store_field("user_id", user_id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getWebPageInstantView::getWebPageInstantView()
  : url_()
  , only_local_()
{}

getWebPageInstantView::getWebPageInstantView(string const &url_, bool only_local_)
  : url_(url_)
  , only_local_(only_local_)
{}

const std::int32_t getWebPageInstantView::ID;

void getWebPageInstantView::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getWebPageInstantView");
    s.store_field("url", url_);
    s.store_field("only_local", only_local_);
    s.store_class_end();
  }
}

joinVideoChat::joinVideoChat()
  : group_call_id_()
  , participant_id_()
  , join_parameters_()
  , invite_hash_()
{}

joinVideoChat::joinVideoChat(int32 group_call_id_, object_ptr<MessageSender> &&participant_id_, object_ptr<groupCallJoinParameters> &&join_parameters_, string const &invite_hash_)
  : group_call_id_(group_call_id_)
  , participant_id_(std::move(participant_id_))
  , join_parameters_(std::move(join_parameters_))
  , invite_hash_(invite_hash_)
{}

const std::int32_t joinVideoChat::ID;

void joinVideoChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "joinVideoChat");
    s.store_field("group_call_id", group_call_id_);
    s.store_object_field("participant_id", static_cast<const BaseObject *>(participant_id_.get()));
    s.store_object_field("join_parameters", static_cast<const BaseObject *>(join_parameters_.get()));
    s.store_field("invite_hash", invite_hash_);
    s.store_class_end();
  }
}

loadChats::loadChats()
  : chat_list_()
  , limit_()
{}

loadChats::loadChats(object_ptr<ChatList> &&chat_list_, int32 limit_)
  : chat_list_(std::move(chat_list_))
  , limit_(limit_)
{}

const std::int32_t loadChats::ID;

void loadChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "loadChats");
    s.store_object_field("chat_list", static_cast<const BaseObject *>(chat_list_.get()));
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

loadDirectMessagesChatTopics::loadDirectMessagesChatTopics()
  : chat_id_()
  , limit_()
{}

loadDirectMessagesChatTopics::loadDirectMessagesChatTopics(int53 chat_id_, int32 limit_)
  : chat_id_(chat_id_)
  , limit_(limit_)
{}

const std::int32_t loadDirectMessagesChatTopics::ID;

void loadDirectMessagesChatTopics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "loadDirectMessagesChatTopics");
    s.store_field("chat_id", chat_id_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

loadSavedMessagesTopics::loadSavedMessagesTopics()
  : limit_()
{}

loadSavedMessagesTopics::loadSavedMessagesTopics(int32 limit_)
  : limit_(limit_)
{}

const std::int32_t loadSavedMessagesTopics::ID;

void loadSavedMessagesTopics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "loadSavedMessagesTopics");
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

postStory::postStory()
  : chat_id_()
  , content_()
  , areas_()
  , caption_()
  , privacy_settings_()
  , album_ids_()
  , active_period_()
  , from_story_full_id_()
  , is_posted_to_chat_page_()
  , protect_content_()
{}

postStory::postStory(int53 chat_id_, object_ptr<InputStoryContent> &&content_, object_ptr<inputStoryAreas> &&areas_, object_ptr<formattedText> &&caption_, object_ptr<StoryPrivacySettings> &&privacy_settings_, array<int32> &&album_ids_, int32 active_period_, object_ptr<storyFullId> &&from_story_full_id_, bool is_posted_to_chat_page_, bool protect_content_)
  : chat_id_(chat_id_)
  , content_(std::move(content_))
  , areas_(std::move(areas_))
  , caption_(std::move(caption_))
  , privacy_settings_(std::move(privacy_settings_))
  , album_ids_(std::move(album_ids_))
  , active_period_(active_period_)
  , from_story_full_id_(std::move(from_story_full_id_))
  , is_posted_to_chat_page_(is_posted_to_chat_page_)
  , protect_content_(protect_content_)
{}

const std::int32_t postStory::ID;

void postStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "postStory");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("content", static_cast<const BaseObject *>(content_.get()));
    s.store_object_field("areas", static_cast<const BaseObject *>(areas_.get()));
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_object_field("privacy_settings", static_cast<const BaseObject *>(privacy_settings_.get()));
    { s.store_vector_begin("album_ids", album_ids_.size()); for (const auto &_value : album_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("active_period", active_period_);
    s.store_object_field("from_story_full_id", static_cast<const BaseObject *>(from_story_full_id_.get()));
    s.store_field("is_posted_to_chat_page", is_posted_to_chat_page_);
    s.store_field("protect_content", protect_content_);
    s.store_class_end();
  }
}

preliminaryUploadFile::preliminaryUploadFile()
  : file_()
  , file_type_()
  , priority_()
{}

preliminaryUploadFile::preliminaryUploadFile(object_ptr<InputFile> &&file_, object_ptr<FileType> &&file_type_, int32 priority_)
  : file_(std::move(file_))
  , file_type_(std::move(file_type_))
  , priority_(priority_)
{}

const std::int32_t preliminaryUploadFile::ID;

void preliminaryUploadFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "preliminaryUploadFile");
    s.store_object_field("file", static_cast<const BaseObject *>(file_.get()));
    s.store_object_field("file_type", static_cast<const BaseObject *>(file_type_.get()));
    s.store_field("priority", priority_);
    s.store_class_end();
  }
}

processChatJoinRequests::processChatJoinRequests()
  : chat_id_()
  , invite_link_()
  , approve_()
{}

processChatJoinRequests::processChatJoinRequests(int53 chat_id_, string const &invite_link_, bool approve_)
  : chat_id_(chat_id_)
  , invite_link_(invite_link_)
  , approve_(approve_)
{}

const std::int32_t processChatJoinRequests::ID;

void processChatJoinRequests::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "processChatJoinRequests");
    s.store_field("chat_id", chat_id_);
    s.store_field("invite_link", invite_link_);
    s.store_field("approve", approve_);
    s.store_class_end();
  }
}

rateSpeechRecognition::rateSpeechRecognition()
  : chat_id_()
  , message_id_()
  , is_good_()
{}

rateSpeechRecognition::rateSpeechRecognition(int53 chat_id_, int53 message_id_, bool is_good_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , is_good_(is_good_)
{}

const std::int32_t rateSpeechRecognition::ID;

void rateSpeechRecognition::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "rateSpeechRecognition");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("is_good", is_good_);
    s.store_class_end();
  }
}

readAllChatReactions::readAllChatReactions()
  : chat_id_()
{}

readAllChatReactions::readAllChatReactions(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t readAllChatReactions::ID;

void readAllChatReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "readAllChatReactions");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

readAllDirectMessagesChatTopicReactions::readAllDirectMessagesChatTopicReactions()
  : chat_id_()
  , topic_id_()
{}

readAllDirectMessagesChatTopicReactions::readAllDirectMessagesChatTopicReactions(int53 chat_id_, int53 topic_id_)
  : chat_id_(chat_id_)
  , topic_id_(topic_id_)
{}

const std::int32_t readAllDirectMessagesChatTopicReactions::ID;

void readAllDirectMessagesChatTopicReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "readAllDirectMessagesChatTopicReactions");
    s.store_field("chat_id", chat_id_);
    s.store_field("topic_id", topic_id_);
    s.store_class_end();
  }
}

removeFavoriteSticker::removeFavoriteSticker()
  : sticker_()
{}

removeFavoriteSticker::removeFavoriteSticker(object_ptr<InputFile> &&sticker_)
  : sticker_(std::move(sticker_))
{}

const std::int32_t removeFavoriteSticker::ID;

void removeFavoriteSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeFavoriteSticker");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

removeSavedNotificationSound::removeSavedNotificationSound()
  : notification_sound_id_()
{}

removeSavedNotificationSound::removeSavedNotificationSound(int64 notification_sound_id_)
  : notification_sound_id_(notification_sound_id_)
{}

const std::int32_t removeSavedNotificationSound::ID;

void removeSavedNotificationSound::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeSavedNotificationSound");
    s.store_field("notification_sound_id", notification_sound_id_);
    s.store_class_end();
  }
}

reportAuthenticationCodeMissing::reportAuthenticationCodeMissing()
  : mobile_network_code_()
{}

reportAuthenticationCodeMissing::reportAuthenticationCodeMissing(string const &mobile_network_code_)
  : mobile_network_code_(mobile_network_code_)
{}

const std::int32_t reportAuthenticationCodeMissing::ID;

void reportAuthenticationCodeMissing::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportAuthenticationCodeMissing");
    s.store_field("mobile_network_code", mobile_network_code_);
    s.store_class_end();
  }
}

reportSupergroupAntiSpamFalsePositive::reportSupergroupAntiSpamFalsePositive()
  : supergroup_id_()
  , message_id_()
{}

reportSupergroupAntiSpamFalsePositive::reportSupergroupAntiSpamFalsePositive(int53 supergroup_id_, int53 message_id_)
  : supergroup_id_(supergroup_id_)
  , message_id_(message_id_)
{}

const std::int32_t reportSupergroupAntiSpamFalsePositive::ID;

void reportSupergroupAntiSpamFalsePositive::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportSupergroupAntiSpamFalsePositive");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

reportSupergroupSpam::reportSupergroupSpam()
  : supergroup_id_()
  , message_ids_()
{}

reportSupergroupSpam::reportSupergroupSpam(int53 supergroup_id_, array<int53> &&message_ids_)
  : supergroup_id_(supergroup_id_)
  , message_ids_(std::move(message_ids_))
{}

const std::int32_t reportSupergroupSpam::ID;

void reportSupergroupSpam::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportSupergroupSpam");
    s.store_field("supergroup_id", supergroup_id_);
    { s.store_vector_begin("message_ids", message_ids_.size()); for (const auto &_value : message_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

reportVideoMessageAdvertisement::reportVideoMessageAdvertisement()
  : advertisement_unique_id_()
  , option_id_()
{}

reportVideoMessageAdvertisement::reportVideoMessageAdvertisement(int53 advertisement_unique_id_, bytes const &option_id_)
  : advertisement_unique_id_(advertisement_unique_id_)
  , option_id_(std::move(option_id_))
{}

const std::int32_t reportVideoMessageAdvertisement::ID;

void reportVideoMessageAdvertisement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportVideoMessageAdvertisement");
    s.store_field("advertisement_unique_id", advertisement_unique_id_);
    s.store_bytes_field("option_id", option_id_);
    s.store_class_end();
  }
}

resendLoginEmailAddressCode::resendLoginEmailAddressCode() {
}

const std::int32_t resendLoginEmailAddressCode::ID;

void resendLoginEmailAddressCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "resendLoginEmailAddressCode");
    s.store_class_end();
  }
}

resendPhoneNumberCode::resendPhoneNumberCode()
  : reason_()
{}

resendPhoneNumberCode::resendPhoneNumberCode(object_ptr<ResendCodeReason> &&reason_)
  : reason_(std::move(reason_))
{}

const std::int32_t resendPhoneNumberCode::ID;

void resendPhoneNumberCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "resendPhoneNumberCode");
    s.store_object_field("reason", static_cast<const BaseObject *>(reason_.get()));
    s.store_class_end();
  }
}

revokeChatInviteLink::revokeChatInviteLink()
  : chat_id_()
  , invite_link_()
{}

revokeChatInviteLink::revokeChatInviteLink(int53 chat_id_, string const &invite_link_)
  : chat_id_(chat_id_)
  , invite_link_(invite_link_)
{}

const std::int32_t revokeChatInviteLink::ID;

void revokeChatInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "revokeChatInviteLink");
    s.store_field("chat_id", chat_id_);
    s.store_field("invite_link", invite_link_);
    s.store_class_end();
  }
}

searchChatMembers::searchChatMembers()
  : chat_id_()
  , query_()
  , limit_()
  , filter_()
{}

searchChatMembers::searchChatMembers(int53 chat_id_, string const &query_, int32 limit_, object_ptr<ChatMembersFilter> &&filter_)
  : chat_id_(chat_id_)
  , query_(query_)
  , limit_(limit_)
  , filter_(std::move(filter_))
{}

const std::int32_t searchChatMembers::ID;

void searchChatMembers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchChatMembers");
    s.store_field("chat_id", chat_id_);
    s.store_field("query", query_);
    s.store_field("limit", limit_);
    s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get()));
    s.store_class_end();
  }
}

searchChats::searchChats()
  : query_()
  , limit_()
{}

searchChats::searchChats(string const &query_, int32 limit_)
  : query_(query_)
  , limit_(limit_)
{}

const std::int32_t searchChats::ID;

void searchChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchChats");
    s.store_field("query", query_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

searchContacts::searchContacts()
  : query_()
  , limit_()
{}

searchContacts::searchContacts(string const &query_, int32 limit_)
  : query_(query_)
  , limit_(limit_)
{}

const std::int32_t searchContacts::ID;

void searchContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchContacts");
    s.store_field("query", query_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

searchFileDownloads::searchFileDownloads()
  : query_()
  , only_active_()
  , only_completed_()
  , offset_()
  , limit_()
{}

searchFileDownloads::searchFileDownloads(string const &query_, bool only_active_, bool only_completed_, string const &offset_, int32 limit_)
  : query_(query_)
  , only_active_(only_active_)
  , only_completed_(only_completed_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t searchFileDownloads::ID;

void searchFileDownloads::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchFileDownloads");
    s.store_field("query", query_);
    s.store_field("only_active", only_active_);
    s.store_field("only_completed", only_completed_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

searchHashtags::searchHashtags()
  : prefix_()
  , limit_()
{}

searchHashtags::searchHashtags(string const &prefix_, int32 limit_)
  : prefix_(prefix_)
  , limit_(limit_)
{}

const std::int32_t searchHashtags::ID;

void searchHashtags::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchHashtags");
    s.store_field("prefix", prefix_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

searchRecentlyFoundChats::searchRecentlyFoundChats()
  : query_()
  , limit_()
{}

searchRecentlyFoundChats::searchRecentlyFoundChats(string const &query_, int32 limit_)
  : query_(query_)
  , limit_(limit_)
{}

const std::int32_t searchRecentlyFoundChats::ID;

void searchRecentlyFoundChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchRecentlyFoundChats");
    s.store_field("query", query_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

setBirthdate::setBirthdate()
  : birthdate_()
{}

setBirthdate::setBirthdate(object_ptr<birthdate> &&birthdate_)
  : birthdate_(std::move(birthdate_))
{}

const std::int32_t setBirthdate::ID;

void setBirthdate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setBirthdate");
    s.store_object_field("birthdate", static_cast<const BaseObject *>(birthdate_.get()));
    s.store_class_end();
  }
}

setChatActiveStoriesList::setChatActiveStoriesList()
  : chat_id_()
  , story_list_()
{}

setChatActiveStoriesList::setChatActiveStoriesList(int53 chat_id_, object_ptr<StoryList> &&story_list_)
  : chat_id_(chat_id_)
  , story_list_(std::move(story_list_))
{}

const std::int32_t setChatActiveStoriesList::ID;

void setChatActiveStoriesList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatActiveStoriesList");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("story_list", static_cast<const BaseObject *>(story_list_.get()));
    s.store_class_end();
  }
}

setChatDiscussionGroup::setChatDiscussionGroup()
  : chat_id_()
  , discussion_chat_id_()
{}

setChatDiscussionGroup::setChatDiscussionGroup(int53 chat_id_, int53 discussion_chat_id_)
  : chat_id_(chat_id_)
  , discussion_chat_id_(discussion_chat_id_)
{}

const std::int32_t setChatDiscussionGroup::ID;

void setChatDiscussionGroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatDiscussionGroup");
    s.store_field("chat_id", chat_id_);
    s.store_field("discussion_chat_id", discussion_chat_id_);
    s.store_class_end();
  }
}

setDefaultMessageAutoDeleteTime::setDefaultMessageAutoDeleteTime()
  : message_auto_delete_time_()
{}

setDefaultMessageAutoDeleteTime::setDefaultMessageAutoDeleteTime(object_ptr<messageAutoDeleteTime> &&message_auto_delete_time_)
  : message_auto_delete_time_(std::move(message_auto_delete_time_))
{}

const std::int32_t setDefaultMessageAutoDeleteTime::ID;

void setDefaultMessageAutoDeleteTime::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setDefaultMessageAutoDeleteTime");
    s.store_object_field("message_auto_delete_time", static_cast<const BaseObject *>(message_auto_delete_time_.get()));
    s.store_class_end();
  }
}

setMainProfileTab::setMainProfileTab()
  : main_profile_tab_()
{}

setMainProfileTab::setMainProfileTab(object_ptr<ProfileTab> &&main_profile_tab_)
  : main_profile_tab_(std::move(main_profile_tab_))
{}

const std::int32_t setMainProfileTab::ID;

void setMainProfileTab::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setMainProfileTab");
    s.store_object_field("main_profile_tab", static_cast<const BaseObject *>(main_profile_tab_.get()));
    s.store_class_end();
  }
}

setMessageReactions::setMessageReactions()
  : chat_id_()
  , message_id_()
  , reaction_types_()
  , is_big_()
{}

setMessageReactions::setMessageReactions(int53 chat_id_, int53 message_id_, array<object_ptr<ReactionType>> &&reaction_types_, bool is_big_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , reaction_types_(std::move(reaction_types_))
  , is_big_(is_big_)
{}

const std::int32_t setMessageReactions::ID;

void setMessageReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setMessageReactions");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    { s.store_vector_begin("reaction_types", reaction_types_.size()); for (const auto &_value : reaction_types_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("is_big", is_big_);
    s.store_class_end();
  }
}

setOption::setOption()
  : name_()
  , value_()
{}

setOption::setOption(string const &name_, object_ptr<OptionValue> &&value_)
  : name_(name_)
  , value_(std::move(value_))
{}

const std::int32_t setOption::ID;

void setOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setOption");
    s.store_field("name", name_);
    s.store_object_field("value", static_cast<const BaseObject *>(value_.get()));
    s.store_class_end();
  }
}

setPassportElementErrors::setPassportElementErrors()
  : user_id_()
  , errors_()
{}

setPassportElementErrors::setPassportElementErrors(int53 user_id_, array<object_ptr<inputPassportElementError>> &&errors_)
  : user_id_(user_id_)
  , errors_(std::move(errors_))
{}

const std::int32_t setPassportElementErrors::ID;

void setPassportElementErrors::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setPassportElementErrors");
    s.store_field("user_id", user_id_);
    { s.store_vector_begin("errors", errors_.size()); for (const auto &_value : errors_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

setPassword::setPassword()
  : old_password_()
  , new_password_()
  , new_hint_()
  , set_recovery_email_address_()
  , new_recovery_email_address_()
{}

setPassword::setPassword(string const &old_password_, string const &new_password_, string const &new_hint_, bool set_recovery_email_address_, string const &new_recovery_email_address_)
  : old_password_(old_password_)
  , new_password_(new_password_)
  , new_hint_(new_hint_)
  , set_recovery_email_address_(set_recovery_email_address_)
  , new_recovery_email_address_(new_recovery_email_address_)
{}

const std::int32_t setPassword::ID;

void setPassword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setPassword");
    s.store_field("old_password", old_password_);
    s.store_field("new_password", new_password_);
    s.store_field("new_hint", new_hint_);
    s.store_field("set_recovery_email_address", set_recovery_email_address_);
    s.store_field("new_recovery_email_address", new_recovery_email_address_);
    s.store_class_end();
  }
}

setPinnedChats::setPinnedChats()
  : chat_list_()
  , chat_ids_()
{}

setPinnedChats::setPinnedChats(object_ptr<ChatList> &&chat_list_, array<int53> &&chat_ids_)
  : chat_list_(std::move(chat_list_))
  , chat_ids_(std::move(chat_ids_))
{}

const std::int32_t setPinnedChats::ID;

void setPinnedChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setPinnedChats");
    s.store_object_field("chat_list", static_cast<const BaseObject *>(chat_list_.get()));
    { s.store_vector_begin("chat_ids", chat_ids_.size()); for (const auto &_value : chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

setRecoveryEmailAddress::setRecoveryEmailAddress()
  : password_()
  , new_recovery_email_address_()
{}

setRecoveryEmailAddress::setRecoveryEmailAddress(string const &password_, string const &new_recovery_email_address_)
  : password_(password_)
  , new_recovery_email_address_(new_recovery_email_address_)
{}

const std::int32_t setRecoveryEmailAddress::ID;

void setRecoveryEmailAddress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setRecoveryEmailAddress");
    s.store_field("password", password_);
    s.store_field("new_recovery_email_address", new_recovery_email_address_);
    s.store_class_end();
  }
}

setSavedMessagesTagLabel::setSavedMessagesTagLabel()
  : tag_()
  , label_()
{}

setSavedMessagesTagLabel::setSavedMessagesTagLabel(object_ptr<ReactionType> &&tag_, string const &label_)
  : tag_(std::move(tag_))
  , label_(label_)
{}

const std::int32_t setSavedMessagesTagLabel::ID;

void setSavedMessagesTagLabel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setSavedMessagesTagLabel");
    s.store_object_field("tag", static_cast<const BaseObject *>(tag_.get()));
    s.store_field("label", label_);
    s.store_class_end();
  }
}

setStickerKeywords::setStickerKeywords()
  : sticker_()
  , keywords_()
{}

setStickerKeywords::setStickerKeywords(object_ptr<InputFile> &&sticker_, array<string> &&keywords_)
  : sticker_(std::move(sticker_))
  , keywords_(std::move(keywords_))
{}

const std::int32_t setStickerKeywords::ID;

void setStickerKeywords::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setStickerKeywords");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    { s.store_vector_begin("keywords", keywords_.size()); for (const auto &_value : keywords_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

setSupergroupStickerSet::setSupergroupStickerSet()
  : supergroup_id_()
  , sticker_set_id_()
{}

setSupergroupStickerSet::setSupergroupStickerSet(int53 supergroup_id_, int64 sticker_set_id_)
  : supergroup_id_(supergroup_id_)
  , sticker_set_id_(sticker_set_id_)
{}

const std::int32_t setSupergroupStickerSet::ID;

void setSupergroupStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setSupergroupStickerSet");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_field("sticker_set_id", sticker_set_id_);
    s.store_class_end();
  }
}

testCallBytes::testCallBytes()
  : x_()
{}

testCallBytes::testCallBytes(bytes const &x_)
  : x_(std::move(x_))
{}

const std::int32_t testCallBytes::ID;

void testCallBytes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testCallBytes");
    s.store_bytes_field("x", x_);
    s.store_class_end();
  }
}

testCallVectorIntObject::testCallVectorIntObject()
  : x_()
{}

testCallVectorIntObject::testCallVectorIntObject(array<object_ptr<testInt>> &&x_)
  : x_(std::move(x_))
{}

const std::int32_t testCallVectorIntObject::ID;

void testCallVectorIntObject::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testCallVectorIntObject");
    { s.store_vector_begin("x", x_.size()); for (const auto &_value : x_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

toggleChatViewAsTopics::toggleChatViewAsTopics()
  : chat_id_()
  , view_as_topics_()
{}

toggleChatViewAsTopics::toggleChatViewAsTopics(int53 chat_id_, bool view_as_topics_)
  : chat_id_(chat_id_)
  , view_as_topics_(view_as_topics_)
{}

const std::int32_t toggleChatViewAsTopics::ID;

void toggleChatViewAsTopics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleChatViewAsTopics");
    s.store_field("chat_id", chat_id_);
    s.store_field("view_as_topics", view_as_topics_);
    s.store_class_end();
  }
}

toggleGiftIsSaved::toggleGiftIsSaved()
  : received_gift_id_()
  , is_saved_()
{}

toggleGiftIsSaved::toggleGiftIsSaved(string const &received_gift_id_, bool is_saved_)
  : received_gift_id_(received_gift_id_)
  , is_saved_(is_saved_)
{}

const std::int32_t toggleGiftIsSaved::ID;

void toggleGiftIsSaved::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleGiftIsSaved");
    s.store_field("received_gift_id", received_gift_id_);
    s.store_field("is_saved", is_saved_);
    s.store_class_end();
  }
}

toggleGroupCallCanSendMessages::toggleGroupCallCanSendMessages()
  : group_call_id_()
  , can_send_messages_()
{}

toggleGroupCallCanSendMessages::toggleGroupCallCanSendMessages(int32 group_call_id_, bool can_send_messages_)
  : group_call_id_(group_call_id_)
  , can_send_messages_(can_send_messages_)
{}

const std::int32_t toggleGroupCallCanSendMessages::ID;

void toggleGroupCallCanSendMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleGroupCallCanSendMessages");
    s.store_field("group_call_id", group_call_id_);
    s.store_field("can_send_messages", can_send_messages_);
    s.store_class_end();
  }
}

toggleGroupCallParticipantIsHandRaised::toggleGroupCallParticipantIsHandRaised()
  : group_call_id_()
  , participant_id_()
  , is_hand_raised_()
{}

toggleGroupCallParticipantIsHandRaised::toggleGroupCallParticipantIsHandRaised(int32 group_call_id_, object_ptr<MessageSender> &&participant_id_, bool is_hand_raised_)
  : group_call_id_(group_call_id_)
  , participant_id_(std::move(participant_id_))
  , is_hand_raised_(is_hand_raised_)
{}

const std::int32_t toggleGroupCallParticipantIsHandRaised::ID;

void toggleGroupCallParticipantIsHandRaised::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleGroupCallParticipantIsHandRaised");
    s.store_field("group_call_id", group_call_id_);
    s.store_object_field("participant_id", static_cast<const BaseObject *>(participant_id_.get()));
    s.store_field("is_hand_raised", is_hand_raised_);
    s.store_class_end();
  }
}

toggleVideoChatEnabledStartNotification::toggleVideoChatEnabledStartNotification()
  : group_call_id_()
  , enabled_start_notification_()
{}

toggleVideoChatEnabledStartNotification::toggleVideoChatEnabledStartNotification(int32 group_call_id_, bool enabled_start_notification_)
  : group_call_id_(group_call_id_)
  , enabled_start_notification_(enabled_start_notification_)
{}

const std::int32_t toggleVideoChatEnabledStartNotification::ID;

void toggleVideoChatEnabledStartNotification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleVideoChatEnabledStartNotification");
    s.store_field("group_call_id", group_call_id_);
    s.store_field("enabled_start_notification", enabled_start_notification_);
    s.store_class_end();
  }
}

transferBusinessAccountStars::transferBusinessAccountStars()
  : business_connection_id_()
  , star_count_()
{}

transferBusinessAccountStars::transferBusinessAccountStars(string const &business_connection_id_, int53 star_count_)
  : business_connection_id_(business_connection_id_)
  , star_count_(star_count_)
{}

const std::int32_t transferBusinessAccountStars::ID;

void transferBusinessAccountStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "transferBusinessAccountStars");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

unpinAllChatMessages::unpinAllChatMessages()
  : chat_id_()
{}

unpinAllChatMessages::unpinAllChatMessages(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t unpinAllChatMessages::ID;

void unpinAllChatMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "unpinAllChatMessages");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

unpinAllForumTopicMessages::unpinAllForumTopicMessages()
  : chat_id_()
  , forum_topic_id_()
{}

unpinAllForumTopicMessages::unpinAllForumTopicMessages(int53 chat_id_, int32 forum_topic_id_)
  : chat_id_(chat_id_)
  , forum_topic_id_(forum_topic_id_)
{}

const std::int32_t unpinAllForumTopicMessages::ID;

void unpinAllForumTopicMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "unpinAllForumTopicMessages");
    s.store_field("chat_id", chat_id_);
    s.store_field("forum_topic_id", forum_topic_id_);
    s.store_class_end();
  }
}

writeGeneratedFilePart::writeGeneratedFilePart()
  : generation_id_()
  , offset_()
  , data_()
{}

writeGeneratedFilePart::writeGeneratedFilePart(int64 generation_id_, int53 offset_, bytes const &data_)
  : generation_id_(generation_id_)
  , offset_(offset_)
  , data_(std::move(data_))
{}

const std::int32_t writeGeneratedFilePart::ID;

void writeGeneratedFilePart::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "writeGeneratedFilePart");
    s.store_field("generation_id", generation_id_);
    s.store_field("offset", offset_);
    s.store_bytes_field("data", data_);
    s.store_class_end();
  }
}
}  // namespace td_api
}  // namespace td
