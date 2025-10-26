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


addedReactions::addedReactions()
  : total_count_()
  , reactions_()
  , next_offset_()
{}

addedReactions::addedReactions(int32 total_count_, array<object_ptr<addedReaction>> &&reactions_, string const &next_offset_)
  : total_count_(total_count_)
  , reactions_(std::move(reactions_))
  , next_offset_(next_offset_)
{}

const std::int32_t addedReactions::ID;

void addedReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addedReactions");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("reactions", reactions_.size()); for (const auto &_value : reactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

ageVerificationParameters::ageVerificationParameters()
  : min_age_()
  , verification_bot_username_()
  , country_()
{}

ageVerificationParameters::ageVerificationParameters(int32 min_age_, string const &verification_bot_username_, string const &country_)
  : min_age_(min_age_)
  , verification_bot_username_(verification_bot_username_)
  , country_(country_)
{}

const std::int32_t ageVerificationParameters::ID;

void ageVerificationParameters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "ageVerificationParameters");
    s.store_field("min_age", min_age_);
    s.store_field("verification_bot_username", verification_bot_username_);
    s.store_field("country", country_);
    s.store_class_end();
  }
}

availableReaction::availableReaction()
  : type_()
  , needs_premium_()
{}

availableReaction::availableReaction(object_ptr<ReactionType> &&type_, bool needs_premium_)
  : type_(std::move(type_))
  , needs_premium_(needs_premium_)
{}

const std::int32_t availableReaction::ID;

void availableReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "availableReaction");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("needs_premium", needs_premium_);
    s.store_class_end();
  }
}

backgroundTypeWallpaper::backgroundTypeWallpaper()
  : is_blurred_()
  , is_moving_()
{}

backgroundTypeWallpaper::backgroundTypeWallpaper(bool is_blurred_, bool is_moving_)
  : is_blurred_(is_blurred_)
  , is_moving_(is_moving_)
{}

const std::int32_t backgroundTypeWallpaper::ID;

void backgroundTypeWallpaper::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "backgroundTypeWallpaper");
    s.store_field("is_blurred", is_blurred_);
    s.store_field("is_moving", is_moving_);
    s.store_class_end();
  }
}

backgroundTypePattern::backgroundTypePattern()
  : fill_()
  , intensity_()
  , is_inverted_()
  , is_moving_()
{}

backgroundTypePattern::backgroundTypePattern(object_ptr<BackgroundFill> &&fill_, int32 intensity_, bool is_inverted_, bool is_moving_)
  : fill_(std::move(fill_))
  , intensity_(intensity_)
  , is_inverted_(is_inverted_)
  , is_moving_(is_moving_)
{}

const std::int32_t backgroundTypePattern::ID;

void backgroundTypePattern::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "backgroundTypePattern");
    s.store_object_field("fill", static_cast<const BaseObject *>(fill_.get()));
    s.store_field("intensity", intensity_);
    s.store_field("is_inverted", is_inverted_);
    s.store_field("is_moving", is_moving_);
    s.store_class_end();
  }
}

backgroundTypeFill::backgroundTypeFill()
  : fill_()
{}

backgroundTypeFill::backgroundTypeFill(object_ptr<BackgroundFill> &&fill_)
  : fill_(std::move(fill_))
{}

const std::int32_t backgroundTypeFill::ID;

void backgroundTypeFill::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "backgroundTypeFill");
    s.store_object_field("fill", static_cast<const BaseObject *>(fill_.get()));
    s.store_class_end();
  }
}

backgroundTypeChatTheme::backgroundTypeChatTheme()
  : theme_name_()
{}

backgroundTypeChatTheme::backgroundTypeChatTheme(string const &theme_name_)
  : theme_name_(theme_name_)
{}

const std::int32_t backgroundTypeChatTheme::ID;

void backgroundTypeChatTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "backgroundTypeChatTheme");
    s.store_field("theme_name", theme_name_);
    s.store_class_end();
  }
}

botCommand::botCommand()
  : command_()
  , description_()
{}

botCommand::botCommand(string const &command_, string const &description_)
  : command_(command_)
  , description_(description_)
{}

const std::int32_t botCommand::ID;

void botCommand::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botCommand");
    s.store_field("command", command_);
    s.store_field("description", description_);
    s.store_class_end();
  }
}

botCommandScopeDefault::botCommandScopeDefault() {
}

const std::int32_t botCommandScopeDefault::ID;

void botCommandScopeDefault::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botCommandScopeDefault");
    s.store_class_end();
  }
}

botCommandScopeAllPrivateChats::botCommandScopeAllPrivateChats() {
}

const std::int32_t botCommandScopeAllPrivateChats::ID;

void botCommandScopeAllPrivateChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botCommandScopeAllPrivateChats");
    s.store_class_end();
  }
}

botCommandScopeAllGroupChats::botCommandScopeAllGroupChats() {
}

const std::int32_t botCommandScopeAllGroupChats::ID;

void botCommandScopeAllGroupChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botCommandScopeAllGroupChats");
    s.store_class_end();
  }
}

botCommandScopeAllChatAdministrators::botCommandScopeAllChatAdministrators() {
}

const std::int32_t botCommandScopeAllChatAdministrators::ID;

void botCommandScopeAllChatAdministrators::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botCommandScopeAllChatAdministrators");
    s.store_class_end();
  }
}

botCommandScopeChat::botCommandScopeChat()
  : chat_id_()
{}

botCommandScopeChat::botCommandScopeChat(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t botCommandScopeChat::ID;

void botCommandScopeChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botCommandScopeChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

botCommandScopeChatAdministrators::botCommandScopeChatAdministrators()
  : chat_id_()
{}

botCommandScopeChatAdministrators::botCommandScopeChatAdministrators(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t botCommandScopeChatAdministrators::ID;

void botCommandScopeChatAdministrators::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botCommandScopeChatAdministrators");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

botCommandScopeChatMember::botCommandScopeChatMember()
  : chat_id_()
  , user_id_()
{}

botCommandScopeChatMember::botCommandScopeChatMember(int53 chat_id_, int53 user_id_)
  : chat_id_(chat_id_)
  , user_id_(user_id_)
{}

const std::int32_t botCommandScopeChatMember::ID;

void botCommandScopeChatMember::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botCommandScopeChatMember");
    s.store_field("chat_id", chat_id_);
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

botMenuButton::botMenuButton()
  : text_()
  , url_()
{}

botMenuButton::botMenuButton(string const &text_, string const &url_)
  : text_(text_)
  , url_(url_)
{}

const std::int32_t botMenuButton::ID;

void botMenuButton::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botMenuButton");
    s.store_field("text", text_);
    s.store_field("url", url_);
    s.store_class_end();
  }
}

builtInThemeClassic::builtInThemeClassic() {
}

const std::int32_t builtInThemeClassic::ID;

void builtInThemeClassic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "builtInThemeClassic");
    s.store_class_end();
  }
}

builtInThemeDay::builtInThemeDay() {
}

const std::int32_t builtInThemeDay::ID;

void builtInThemeDay::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "builtInThemeDay");
    s.store_class_end();
  }
}

builtInThemeNight::builtInThemeNight() {
}

const std::int32_t builtInThemeNight::ID;

void builtInThemeNight::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "builtInThemeNight");
    s.store_class_end();
  }
}

builtInThemeTinted::builtInThemeTinted() {
}

const std::int32_t builtInThemeTinted::ID;

void builtInThemeTinted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "builtInThemeTinted");
    s.store_class_end();
  }
}

builtInThemeArctic::builtInThemeArctic() {
}

const std::int32_t builtInThemeArctic::ID;

void builtInThemeArctic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "builtInThemeArctic");
    s.store_class_end();
  }
}

businessChatLink::businessChatLink()
  : link_()
  , text_()
  , title_()
  , view_count_()
{}

businessChatLink::businessChatLink(string const &link_, object_ptr<formattedText> &&text_, string const &title_, int32 view_count_)
  : link_(link_)
  , text_(std::move(text_))
  , title_(title_)
  , view_count_(view_count_)
{}

const std::int32_t businessChatLink::ID;

void businessChatLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessChatLink");
    s.store_field("link", link_);
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("title", title_);
    s.store_field("view_count", view_count_);
    s.store_class_end();
  }
}

businessMessages::businessMessages()
  : messages_()
{}

businessMessages::businessMessages(array<object_ptr<businessMessage>> &&messages_)
  : messages_(std::move(messages_))
{}

const std::int32_t businessMessages::ID;

void businessMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessMessages");
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

businessOpeningHoursInterval::businessOpeningHoursInterval()
  : start_minute_()
  , end_minute_()
{}

businessOpeningHoursInterval::businessOpeningHoursInterval(int32 start_minute_, int32 end_minute_)
  : start_minute_(start_minute_)
  , end_minute_(end_minute_)
{}

const std::int32_t businessOpeningHoursInterval::ID;

void businessOpeningHoursInterval::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessOpeningHoursInterval");
    s.store_field("start_minute", start_minute_);
    s.store_field("end_minute", end_minute_);
    s.store_class_end();
  }
}

call::call()
  : id_()
  , user_id_()
  , is_outgoing_()
  , is_video_()
  , state_()
{}

call::call(int32 id_, int53 user_id_, bool is_outgoing_, bool is_video_, object_ptr<CallState> &&state_)
  : id_(id_)
  , user_id_(user_id_)
  , is_outgoing_(is_outgoing_)
  , is_video_(is_video_)
  , state_(std::move(state_))
{}

const std::int32_t call::ID;

void call::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "call");
    s.store_field("id", id_);
    s.store_field("user_id", user_id_);
    s.store_field("is_outgoing", is_outgoing_);
    s.store_field("is_video", is_video_);
    s.store_object_field("state", static_cast<const BaseObject *>(state_.get()));
    s.store_class_end();
  }
}

callDiscardReasonEmpty::callDiscardReasonEmpty() {
}

const std::int32_t callDiscardReasonEmpty::ID;

void callDiscardReasonEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callDiscardReasonEmpty");
    s.store_class_end();
  }
}

callDiscardReasonMissed::callDiscardReasonMissed() {
}

const std::int32_t callDiscardReasonMissed::ID;

void callDiscardReasonMissed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callDiscardReasonMissed");
    s.store_class_end();
  }
}

callDiscardReasonDeclined::callDiscardReasonDeclined() {
}

const std::int32_t callDiscardReasonDeclined::ID;

void callDiscardReasonDeclined::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callDiscardReasonDeclined");
    s.store_class_end();
  }
}

callDiscardReasonDisconnected::callDiscardReasonDisconnected() {
}

const std::int32_t callDiscardReasonDisconnected::ID;

void callDiscardReasonDisconnected::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callDiscardReasonDisconnected");
    s.store_class_end();
  }
}

callDiscardReasonHungUp::callDiscardReasonHungUp() {
}

const std::int32_t callDiscardReasonHungUp::ID;

void callDiscardReasonHungUp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callDiscardReasonHungUp");
    s.store_class_end();
  }
}

callDiscardReasonUpgradeToGroupCall::callDiscardReasonUpgradeToGroupCall()
  : invite_link_()
{}

callDiscardReasonUpgradeToGroupCall::callDiscardReasonUpgradeToGroupCall(string const &invite_link_)
  : invite_link_(invite_link_)
{}

const std::int32_t callDiscardReasonUpgradeToGroupCall::ID;

void callDiscardReasonUpgradeToGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callDiscardReasonUpgradeToGroupCall");
    s.store_field("invite_link", invite_link_);
    s.store_class_end();
  }
}

callProtocol::callProtocol()
  : udp_p2p_()
  , udp_reflector_()
  , min_layer_()
  , max_layer_()
  , library_versions_()
{}

callProtocol::callProtocol(bool udp_p2p_, bool udp_reflector_, int32 min_layer_, int32 max_layer_, array<string> &&library_versions_)
  : udp_p2p_(udp_p2p_)
  , udp_reflector_(udp_reflector_)
  , min_layer_(min_layer_)
  , max_layer_(max_layer_)
  , library_versions_(std::move(library_versions_))
{}

const std::int32_t callProtocol::ID;

void callProtocol::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callProtocol");
    s.store_field("udp_p2p", udp_p2p_);
    s.store_field("udp_reflector", udp_reflector_);
    s.store_field("min_layer", min_layer_);
    s.store_field("max_layer", max_layer_);
    { s.store_vector_begin("library_versions", library_versions_.size()); for (const auto &_value : library_versions_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

canPostStoryResultOk::canPostStoryResultOk()
  : story_count_()
{}

canPostStoryResultOk::canPostStoryResultOk(int32 story_count_)
  : story_count_(story_count_)
{}

const std::int32_t canPostStoryResultOk::ID;

void canPostStoryResultOk::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canPostStoryResultOk");
    s.store_field("story_count", story_count_);
    s.store_class_end();
  }
}

canPostStoryResultPremiumNeeded::canPostStoryResultPremiumNeeded() {
}

const std::int32_t canPostStoryResultPremiumNeeded::ID;

void canPostStoryResultPremiumNeeded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canPostStoryResultPremiumNeeded");
    s.store_class_end();
  }
}

canPostStoryResultBoostNeeded::canPostStoryResultBoostNeeded() {
}

const std::int32_t canPostStoryResultBoostNeeded::ID;

void canPostStoryResultBoostNeeded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canPostStoryResultBoostNeeded");
    s.store_class_end();
  }
}

canPostStoryResultActiveStoryLimitExceeded::canPostStoryResultActiveStoryLimitExceeded() {
}

const std::int32_t canPostStoryResultActiveStoryLimitExceeded::ID;

void canPostStoryResultActiveStoryLimitExceeded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canPostStoryResultActiveStoryLimitExceeded");
    s.store_class_end();
  }
}

canPostStoryResultWeeklyLimitExceeded::canPostStoryResultWeeklyLimitExceeded()
  : retry_after_()
{}

canPostStoryResultWeeklyLimitExceeded::canPostStoryResultWeeklyLimitExceeded(int32 retry_after_)
  : retry_after_(retry_after_)
{}

const std::int32_t canPostStoryResultWeeklyLimitExceeded::ID;

void canPostStoryResultWeeklyLimitExceeded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canPostStoryResultWeeklyLimitExceeded");
    s.store_field("retry_after", retry_after_);
    s.store_class_end();
  }
}

canPostStoryResultMonthlyLimitExceeded::canPostStoryResultMonthlyLimitExceeded()
  : retry_after_()
{}

canPostStoryResultMonthlyLimitExceeded::canPostStoryResultMonthlyLimitExceeded(int32 retry_after_)
  : retry_after_(retry_after_)
{}

const std::int32_t canPostStoryResultMonthlyLimitExceeded::ID;

void canPostStoryResultMonthlyLimitExceeded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canPostStoryResultMonthlyLimitExceeded");
    s.store_field("retry_after", retry_after_);
    s.store_class_end();
  }
}

chatAdministrators::chatAdministrators()
  : administrators_()
{}

chatAdministrators::chatAdministrators(array<object_ptr<chatAdministrator>> &&administrators_)
  : administrators_(std::move(administrators_))
{}

const std::int32_t chatAdministrators::ID;

void chatAdministrators::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatAdministrators");
    { s.store_vector_begin("administrators", administrators_.size()); for (const auto &_value : administrators_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatFolderInviteLinks::chatFolderInviteLinks()
  : invite_links_()
{}

chatFolderInviteLinks::chatFolderInviteLinks(array<object_ptr<chatFolderInviteLink>> &&invite_links_)
  : invite_links_(std::move(invite_links_))
{}

const std::int32_t chatFolderInviteLinks::ID;

void chatFolderInviteLinks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatFolderInviteLinks");
    { s.store_vector_begin("invite_links", invite_links_.size()); for (const auto &_value : invite_links_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatListMain::chatListMain() {
}

const std::int32_t chatListMain::ID;

void chatListMain::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatListMain");
    s.store_class_end();
  }
}

chatListArchive::chatListArchive() {
}

const std::int32_t chatListArchive::ID;

void chatListArchive::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatListArchive");
    s.store_class_end();
  }
}

chatListFolder::chatListFolder()
  : chat_folder_id_()
{}

chatListFolder::chatListFolder(int32 chat_folder_id_)
  : chat_folder_id_(chat_folder_id_)
{}

const std::int32_t chatListFolder::ID;

void chatListFolder::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatListFolder");
    s.store_field("chat_folder_id", chat_folder_id_);
    s.store_class_end();
  }
}

chatRevenueTransaction::chatRevenueTransaction()
  : cryptocurrency_()
  , cryptocurrency_amount_()
  , type_()
{}

chatRevenueTransaction::chatRevenueTransaction(string const &cryptocurrency_, int64 cryptocurrency_amount_, object_ptr<ChatRevenueTransactionType> &&type_)
  : cryptocurrency_(cryptocurrency_)
  , cryptocurrency_amount_(cryptocurrency_amount_)
  , type_(std::move(type_))
{}

const std::int32_t chatRevenueTransaction::ID;

void chatRevenueTransaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatRevenueTransaction");
    s.store_field("cryptocurrency", cryptocurrency_);
    s.store_field("cryptocurrency_amount", cryptocurrency_amount_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

checklist::checklist()
  : title_()
  , tasks_()
  , others_can_add_tasks_()
  , can_add_tasks_()
  , others_can_mark_tasks_as_done_()
  , can_mark_tasks_as_done_()
{}

checklist::checklist(object_ptr<formattedText> &&title_, array<object_ptr<checklistTask>> &&tasks_, bool others_can_add_tasks_, bool can_add_tasks_, bool others_can_mark_tasks_as_done_, bool can_mark_tasks_as_done_)
  : title_(std::move(title_))
  , tasks_(std::move(tasks_))
  , others_can_add_tasks_(others_can_add_tasks_)
  , can_add_tasks_(can_add_tasks_)
  , others_can_mark_tasks_as_done_(others_can_mark_tasks_as_done_)
  , can_mark_tasks_as_done_(can_mark_tasks_as_done_)
{}

const std::int32_t checklist::ID;

void checklist::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checklist");
    s.store_object_field("title", static_cast<const BaseObject *>(title_.get()));
    { s.store_vector_begin("tasks", tasks_.size()); for (const auto &_value : tasks_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("others_can_add_tasks", others_can_add_tasks_);
    s.store_field("can_add_tasks", can_add_tasks_);
    s.store_field("others_can_mark_tasks_as_done", others_can_mark_tasks_as_done_);
    s.store_field("can_mark_tasks_as_done", can_mark_tasks_as_done_);
    s.store_class_end();
  }
}

closedVectorPath::closedVectorPath()
  : commands_()
{}

closedVectorPath::closedVectorPath(array<object_ptr<VectorPathCommand>> &&commands_)
  : commands_(std::move(commands_))
{}

const std::int32_t closedVectorPath::ID;

void closedVectorPath::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "closedVectorPath");
    { s.store_vector_begin("commands", commands_.size()); for (const auto &_value : commands_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

connectedAffiliateProgram::connectedAffiliateProgram()
  : url_()
  , bot_user_id_()
  , parameters_()
  , connection_date_()
  , is_disconnected_()
  , user_count_()
  , revenue_star_count_()
{}

connectedAffiliateProgram::connectedAffiliateProgram(string const &url_, int53 bot_user_id_, object_ptr<affiliateProgramParameters> &&parameters_, int32 connection_date_, bool is_disconnected_, int64 user_count_, int64 revenue_star_count_)
  : url_(url_)
  , bot_user_id_(bot_user_id_)
  , parameters_(std::move(parameters_))
  , connection_date_(connection_date_)
  , is_disconnected_(is_disconnected_)
  , user_count_(user_count_)
  , revenue_star_count_(revenue_star_count_)
{}

const std::int32_t connectedAffiliateProgram::ID;

void connectedAffiliateProgram::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "connectedAffiliateProgram");
    s.store_field("url", url_);
    s.store_field("bot_user_id", bot_user_id_);
    s.store_object_field("parameters", static_cast<const BaseObject *>(parameters_.get()));
    s.store_field("connection_date", connection_date_);
    s.store_field("is_disconnected", is_disconnected_);
    s.store_field("user_count", user_count_);
    s.store_field("revenue_star_count", revenue_star_count_);
    s.store_class_end();
  }
}

countries::countries()
  : countries_()
{}

countries::countries(array<object_ptr<countryInfo>> &&countries_)
  : countries_(std::move(countries_))
{}

const std::int32_t countries::ID;

void countries::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "countries");
    { s.store_vector_begin("countries", countries_.size()); for (const auto &_value : countries_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

date::date()
  : day_()
  , month_()
  , year_()
{}

date::date(int32 day_, int32 month_, int32 year_)
  : day_(day_)
  , month_(month_)
  , year_(year_)
{}

const std::int32_t date::ID;

void date::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "date");
    s.store_field("day", day_);
    s.store_field("month", month_);
    s.store_field("year", year_);
    s.store_class_end();
  }
}

deepLinkInfo::deepLinkInfo()
  : text_()
  , need_update_application_()
{}

deepLinkInfo::deepLinkInfo(object_ptr<formattedText> &&text_, bool need_update_application_)
  : text_(std::move(text_))
  , need_update_application_(need_update_application_)
{}

const std::int32_t deepLinkInfo::ID;

void deepLinkInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deepLinkInfo");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("need_update_application", need_update_application_);
    s.store_class_end();
  }
}

emailAddressResetStateAvailable::emailAddressResetStateAvailable()
  : wait_period_()
{}

emailAddressResetStateAvailable::emailAddressResetStateAvailable(int32 wait_period_)
  : wait_period_(wait_period_)
{}

const std::int32_t emailAddressResetStateAvailable::ID;

void emailAddressResetStateAvailable::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emailAddressResetStateAvailable");
    s.store_field("wait_period", wait_period_);
    s.store_class_end();
  }
}

emailAddressResetStatePending::emailAddressResetStatePending()
  : reset_in_()
{}

emailAddressResetStatePending::emailAddressResetStatePending(int32 reset_in_)
  : reset_in_(reset_in_)
{}

const std::int32_t emailAddressResetStatePending::ID;

void emailAddressResetStatePending::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emailAddressResetStatePending");
    s.store_field("reset_in", reset_in_);
    s.store_class_end();
  }
}

emojiKeywords::emojiKeywords()
  : emoji_keywords_()
{}

emojiKeywords::emojiKeywords(array<object_ptr<emojiKeyword>> &&emoji_keywords_)
  : emoji_keywords_(std::move(emoji_keywords_))
{}

const std::int32_t emojiKeywords::ID;

void emojiKeywords::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiKeywords");
    { s.store_vector_begin("emoji_keywords", emoji_keywords_.size()); for (const auto &_value : emoji_keywords_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

emojiStatus::emojiStatus()
  : type_()
  , expiration_date_()
{}

emojiStatus::emojiStatus(object_ptr<EmojiStatusType> &&type_, int32 expiration_date_)
  : type_(std::move(type_))
  , expiration_date_(expiration_date_)
{}

const std::int32_t emojiStatus::ID;

void emojiStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiStatus");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("expiration_date", expiration_date_);
    s.store_class_end();
  }
}

emojiStatuses::emojiStatuses()
  : emoji_statuses_()
{}

emojiStatuses::emojiStatuses(array<object_ptr<emojiStatus>> &&emoji_statuses_)
  : emoji_statuses_(std::move(emoji_statuses_))
{}

const std::int32_t emojiStatuses::ID;

void emojiStatuses::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiStatuses");
    { s.store_vector_begin("emoji_statuses", emoji_statuses_.size()); for (const auto &_value : emoji_statuses_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

encryptedPassportElement::encryptedPassportElement()
  : type_()
  , data_()
  , front_side_()
  , reverse_side_()
  , selfie_()
  , translation_()
  , files_()
  , value_()
  , hash_()
{}

encryptedPassportElement::encryptedPassportElement(object_ptr<PassportElementType> &&type_, bytes const &data_, object_ptr<datedFile> &&front_side_, object_ptr<datedFile> &&reverse_side_, object_ptr<datedFile> &&selfie_, array<object_ptr<datedFile>> &&translation_, array<object_ptr<datedFile>> &&files_, string const &value_, string const &hash_)
  : type_(std::move(type_))
  , data_(std::move(data_))
  , front_side_(std::move(front_side_))
  , reverse_side_(std::move(reverse_side_))
  , selfie_(std::move(selfie_))
  , translation_(std::move(translation_))
  , files_(std::move(files_))
  , value_(value_)
  , hash_(hash_)
{}

const std::int32_t encryptedPassportElement::ID;

void encryptedPassportElement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "encryptedPassportElement");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_bytes_field("data", data_);
    s.store_object_field("front_side", static_cast<const BaseObject *>(front_side_.get()));
    s.store_object_field("reverse_side", static_cast<const BaseObject *>(reverse_side_.get()));
    s.store_object_field("selfie", static_cast<const BaseObject *>(selfie_.get()));
    { s.store_vector_begin("translation", translation_.size()); for (const auto &_value : translation_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("files", files_.size()); for (const auto &_value : files_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("value", value_);
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

factCheck::factCheck()
  : text_()
  , country_code_()
{}

factCheck::factCheck(object_ptr<formattedText> &&text_, string const &country_code_)
  : text_(std::move(text_))
  , country_code_(country_code_)
{}

const std::int32_t factCheck::ID;

void factCheck::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "factCheck");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("country_code", country_code_);
    s.store_class_end();
  }
}

failedToAddMembers::failedToAddMembers()
  : failed_to_add_members_()
{}

failedToAddMembers::failedToAddMembers(array<object_ptr<failedToAddMember>> &&failed_to_add_members_)
  : failed_to_add_members_(std::move(failed_to_add_members_))
{}

const std::int32_t failedToAddMembers::ID;

void failedToAddMembers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "failedToAddMembers");
    { s.store_vector_begin("failed_to_add_members", failed_to_add_members_.size()); for (const auto &_value : failed_to_add_members_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

file::file()
  : id_()
  , size_()
  , expected_size_()
  , local_()
  , remote_()
{}

file::file(int32 id_, int53 size_, int53 expected_size_, object_ptr<localFile> &&local_, object_ptr<remoteFile> &&remote_)
  : id_(id_)
  , size_(size_)
  , expected_size_(expected_size_)
  , local_(std::move(local_))
  , remote_(std::move(remote_))
{}

const std::int32_t file::ID;

void file::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "file");
    s.store_field("id", id_);
    s.store_field("size", size_);
    s.store_field("expected_size", expected_size_);
    s.store_object_field("local", static_cast<const BaseObject *>(local_.get()));
    s.store_object_field("remote", static_cast<const BaseObject *>(remote_.get()));
    s.store_class_end();
  }
}

firebaseAuthenticationSettingsAndroid::firebaseAuthenticationSettingsAndroid() {
}

const std::int32_t firebaseAuthenticationSettingsAndroid::ID;

void firebaseAuthenticationSettingsAndroid::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "firebaseAuthenticationSettingsAndroid");
    s.store_class_end();
  }
}

firebaseAuthenticationSettingsIos::firebaseAuthenticationSettingsIos()
  : device_token_()
  , is_app_sandbox_()
{}

firebaseAuthenticationSettingsIos::firebaseAuthenticationSettingsIos(string const &device_token_, bool is_app_sandbox_)
  : device_token_(device_token_)
  , is_app_sandbox_(is_app_sandbox_)
{}

const std::int32_t firebaseAuthenticationSettingsIos::ID;

void firebaseAuthenticationSettingsIos::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "firebaseAuthenticationSettingsIos");
    s.store_field("device_token", device_token_);
    s.store_field("is_app_sandbox", is_app_sandbox_);
    s.store_class_end();
  }
}

foundChatBoosts::foundChatBoosts()
  : total_count_()
  , boosts_()
  , next_offset_()
{}

foundChatBoosts::foundChatBoosts(int32 total_count_, array<object_ptr<chatBoost>> &&boosts_, string const &next_offset_)
  : total_count_(total_count_)
  , boosts_(std::move(boosts_))
  , next_offset_(next_offset_)
{}

const std::int32_t foundChatBoosts::ID;

void foundChatBoosts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "foundChatBoosts");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("boosts", boosts_.size()); for (const auto &_value : boosts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

foundChatMessages::foundChatMessages()
  : total_count_()
  , messages_()
  , next_from_message_id_()
{}

foundChatMessages::foundChatMessages(int32 total_count_, array<object_ptr<message>> &&messages_, int53 next_from_message_id_)
  : total_count_(total_count_)
  , messages_(std::move(messages_))
  , next_from_message_id_(next_from_message_id_)
{}

const std::int32_t foundChatMessages::ID;

void foundChatMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "foundChatMessages");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("next_from_message_id", next_from_message_id_);
    s.store_class_end();
  }
}

gift::gift()
  : id_()
  , publisher_chat_id_()
  , sticker_()
  , star_count_()
  , default_sell_star_count_()
  , upgrade_star_count_()
  , has_colors_()
  , is_for_birthday_()
  , is_premium_()
  , next_send_date_()
  , user_limits_()
  , overall_limits_()
  , first_send_date_()
  , last_send_date_()
{}

gift::gift(int64 id_, int53 publisher_chat_id_, object_ptr<sticker> &&sticker_, int53 star_count_, int53 default_sell_star_count_, int53 upgrade_star_count_, bool has_colors_, bool is_for_birthday_, bool is_premium_, int32 next_send_date_, object_ptr<giftPurchaseLimits> &&user_limits_, object_ptr<giftPurchaseLimits> &&overall_limits_, int32 first_send_date_, int32 last_send_date_)
  : id_(id_)
  , publisher_chat_id_(publisher_chat_id_)
  , sticker_(std::move(sticker_))
  , star_count_(star_count_)
  , default_sell_star_count_(default_sell_star_count_)
  , upgrade_star_count_(upgrade_star_count_)
  , has_colors_(has_colors_)
  , is_for_birthday_(is_for_birthday_)
  , is_premium_(is_premium_)
  , next_send_date_(next_send_date_)
  , user_limits_(std::move(user_limits_))
  , overall_limits_(std::move(overall_limits_))
  , first_send_date_(first_send_date_)
  , last_send_date_(last_send_date_)
{}

const std::int32_t gift::ID;

void gift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "gift");
    s.store_field("id", id_);
    s.store_field("publisher_chat_id", publisher_chat_id_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_field("star_count", star_count_);
    s.store_field("default_sell_star_count", default_sell_star_count_);
    s.store_field("upgrade_star_count", upgrade_star_count_);
    s.store_field("has_colors", has_colors_);
    s.store_field("is_for_birthday", is_for_birthday_);
    s.store_field("is_premium", is_premium_);
    s.store_field("next_send_date", next_send_date_);
    s.store_object_field("user_limits", static_cast<const BaseObject *>(user_limits_.get()));
    s.store_object_field("overall_limits", static_cast<const BaseObject *>(overall_limits_.get()));
    s.store_field("first_send_date", first_send_date_);
    s.store_field("last_send_date", last_send_date_);
    s.store_class_end();
  }
}

groupCallVideoSourceGroup::groupCallVideoSourceGroup()
  : semantics_()
  , source_ids_()
{}

groupCallVideoSourceGroup::groupCallVideoSourceGroup(string const &semantics_, array<int32> &&source_ids_)
  : semantics_(semantics_)
  , source_ids_(std::move(source_ids_))
{}

const std::int32_t groupCallVideoSourceGroup::ID;

void groupCallVideoSourceGroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCallVideoSourceGroup");
    s.store_field("semantics", semantics_);
    { s.store_vector_begin("source_ids", source_ids_.size()); for (const auto &_value : source_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

inputChecklist::inputChecklist()
  : title_()
  , tasks_()
  , others_can_add_tasks_()
  , others_can_mark_tasks_as_done_()
{}

inputChecklist::inputChecklist(object_ptr<formattedText> &&title_, array<object_ptr<inputChecklistTask>> &&tasks_, bool others_can_add_tasks_, bool others_can_mark_tasks_as_done_)
  : title_(std::move(title_))
  , tasks_(std::move(tasks_))
  , others_can_add_tasks_(others_can_add_tasks_)
  , others_can_mark_tasks_as_done_(others_can_mark_tasks_as_done_)
{}

const std::int32_t inputChecklist::ID;

void inputChecklist::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputChecklist");
    s.store_object_field("title", static_cast<const BaseObject *>(title_.get()));
    { s.store_vector_begin("tasks", tasks_.size()); for (const auto &_value : tasks_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("others_can_add_tasks", others_can_add_tasks_);
    s.store_field("others_can_mark_tasks_as_done", others_can_mark_tasks_as_done_);
    s.store_class_end();
  }
}

inputStoryAreaTypeLocation::inputStoryAreaTypeLocation()
  : location_()
  , address_()
{}

inputStoryAreaTypeLocation::inputStoryAreaTypeLocation(object_ptr<location> &&location_, object_ptr<locationAddress> &&address_)
  : location_(std::move(location_))
  , address_(std::move(address_))
{}

const std::int32_t inputStoryAreaTypeLocation::ID;

void inputStoryAreaTypeLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStoryAreaTypeLocation");
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_object_field("address", static_cast<const BaseObject *>(address_.get()));
    s.store_class_end();
  }
}

inputStoryAreaTypeFoundVenue::inputStoryAreaTypeFoundVenue()
  : query_id_()
  , result_id_()
{}

inputStoryAreaTypeFoundVenue::inputStoryAreaTypeFoundVenue(int64 query_id_, string const &result_id_)
  : query_id_(query_id_)
  , result_id_(result_id_)
{}

const std::int32_t inputStoryAreaTypeFoundVenue::ID;

void inputStoryAreaTypeFoundVenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStoryAreaTypeFoundVenue");
    s.store_field("query_id", query_id_);
    s.store_field("result_id", result_id_);
    s.store_class_end();
  }
}

inputStoryAreaTypePreviousVenue::inputStoryAreaTypePreviousVenue()
  : venue_provider_()
  , venue_id_()
{}

inputStoryAreaTypePreviousVenue::inputStoryAreaTypePreviousVenue(string const &venue_provider_, string const &venue_id_)
  : venue_provider_(venue_provider_)
  , venue_id_(venue_id_)
{}

const std::int32_t inputStoryAreaTypePreviousVenue::ID;

void inputStoryAreaTypePreviousVenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStoryAreaTypePreviousVenue");
    s.store_field("venue_provider", venue_provider_);
    s.store_field("venue_id", venue_id_);
    s.store_class_end();
  }
}

inputStoryAreaTypeSuggestedReaction::inputStoryAreaTypeSuggestedReaction()
  : reaction_type_()
  , is_dark_()
  , is_flipped_()
{}

inputStoryAreaTypeSuggestedReaction::inputStoryAreaTypeSuggestedReaction(object_ptr<ReactionType> &&reaction_type_, bool is_dark_, bool is_flipped_)
  : reaction_type_(std::move(reaction_type_))
  , is_dark_(is_dark_)
  , is_flipped_(is_flipped_)
{}

const std::int32_t inputStoryAreaTypeSuggestedReaction::ID;

void inputStoryAreaTypeSuggestedReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStoryAreaTypeSuggestedReaction");
    s.store_object_field("reaction_type", static_cast<const BaseObject *>(reaction_type_.get()));
    s.store_field("is_dark", is_dark_);
    s.store_field("is_flipped", is_flipped_);
    s.store_class_end();
  }
}

inputStoryAreaTypeMessage::inputStoryAreaTypeMessage()
  : chat_id_()
  , message_id_()
{}

inputStoryAreaTypeMessage::inputStoryAreaTypeMessage(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t inputStoryAreaTypeMessage::ID;

void inputStoryAreaTypeMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStoryAreaTypeMessage");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

inputStoryAreaTypeLink::inputStoryAreaTypeLink()
  : url_()
{}

inputStoryAreaTypeLink::inputStoryAreaTypeLink(string const &url_)
  : url_(url_)
{}

const std::int32_t inputStoryAreaTypeLink::ID;

void inputStoryAreaTypeLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStoryAreaTypeLink");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

inputStoryAreaTypeWeather::inputStoryAreaTypeWeather()
  : temperature_()
  , emoji_()
  , background_color_()
{}

inputStoryAreaTypeWeather::inputStoryAreaTypeWeather(double temperature_, string const &emoji_, int32 background_color_)
  : temperature_(temperature_)
  , emoji_(emoji_)
  , background_color_(background_color_)
{}

const std::int32_t inputStoryAreaTypeWeather::ID;

void inputStoryAreaTypeWeather::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStoryAreaTypeWeather");
    s.store_field("temperature", temperature_);
    s.store_field("emoji", emoji_);
    s.store_field("background_color", background_color_);
    s.store_class_end();
  }
}

inputStoryAreaTypeUpgradedGift::inputStoryAreaTypeUpgradedGift()
  : gift_name_()
{}

inputStoryAreaTypeUpgradedGift::inputStoryAreaTypeUpgradedGift(string const &gift_name_)
  : gift_name_(gift_name_)
{}

const std::int32_t inputStoryAreaTypeUpgradedGift::ID;

void inputStoryAreaTypeUpgradedGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStoryAreaTypeUpgradedGift");
    s.store_field("gift_name", gift_name_);
    s.store_class_end();
  }
}

maskPointForehead::maskPointForehead() {
}

const std::int32_t maskPointForehead::ID;

void maskPointForehead::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "maskPointForehead");
    s.store_class_end();
  }
}

maskPointEyes::maskPointEyes() {
}

const std::int32_t maskPointEyes::ID;

void maskPointEyes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "maskPointEyes");
    s.store_class_end();
  }
}

maskPointMouth::maskPointMouth() {
}

const std::int32_t maskPointMouth::ID;

void maskPointMouth::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "maskPointMouth");
    s.store_class_end();
  }
}

maskPointChin::maskPointChin() {
}

const std::int32_t maskPointChin::ID;

void maskPointChin::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "maskPointChin");
    s.store_class_end();
  }
}

messagePosition::messagePosition()
  : position_()
  , message_id_()
  , date_()
{}

messagePosition::messagePosition(int32 position_, int53 message_id_, int32 date_)
  : position_(position_)
  , message_id_(message_id_)
  , date_(date_)
{}

const std::int32_t messagePosition::ID;

void messagePosition::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messagePosition");
    s.store_field("position", position_);
    s.store_field("message_id", message_id_);
    s.store_field("date", date_);
    s.store_class_end();
  }
}

messageReplyToMessage::messageReplyToMessage()
  : chat_id_()
  , message_id_()
  , quote_()
  , checklist_task_id_()
  , origin_()
  , origin_send_date_()
  , content_()
{}

messageReplyToMessage::messageReplyToMessage(int53 chat_id_, int53 message_id_, object_ptr<textQuote> &&quote_, int32 checklist_task_id_, object_ptr<MessageOrigin> &&origin_, int32 origin_send_date_, object_ptr<MessageContent> &&content_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , quote_(std::move(quote_))
  , checklist_task_id_(checklist_task_id_)
  , origin_(std::move(origin_))
  , origin_send_date_(origin_send_date_)
  , content_(std::move(content_))
{}

const std::int32_t messageReplyToMessage::ID;

void messageReplyToMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageReplyToMessage");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("quote", static_cast<const BaseObject *>(quote_.get()));
    s.store_field("checklist_task_id", checklist_task_id_);
    s.store_object_field("origin", static_cast<const BaseObject *>(origin_.get()));
    s.store_field("origin_send_date", origin_send_date_);
    s.store_object_field("content", static_cast<const BaseObject *>(content_.get()));
    s.store_class_end();
  }
}

messageReplyToStory::messageReplyToStory()
  : story_poster_chat_id_()
  , story_id_()
{}

messageReplyToStory::messageReplyToStory(int53 story_poster_chat_id_, int32 story_id_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , story_id_(story_id_)
{}

const std::int32_t messageReplyToStory::ID;

void messageReplyToStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageReplyToStory");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("story_id", story_id_);
    s.store_class_end();
  }
}

optionValueBoolean::optionValueBoolean()
  : value_()
{}

optionValueBoolean::optionValueBoolean(bool value_)
  : value_(value_)
{}

const std::int32_t optionValueBoolean::ID;

void optionValueBoolean::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "optionValueBoolean");
    s.store_field("value", value_);
    s.store_class_end();
  }
}

optionValueEmpty::optionValueEmpty() {
}

const std::int32_t optionValueEmpty::ID;

void optionValueEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "optionValueEmpty");
    s.store_class_end();
  }
}

optionValueInteger::optionValueInteger()
  : value_()
{}

optionValueInteger::optionValueInteger(int64 value_)
  : value_(value_)
{}

const std::int32_t optionValueInteger::ID;

void optionValueInteger::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "optionValueInteger");
    s.store_field("value", value_);
    s.store_class_end();
  }
}

optionValueString::optionValueString()
  : value_()
{}

optionValueString::optionValueString(string const &value_)
  : value_(value_)
{}

const std::int32_t optionValueString::ID;

void optionValueString::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "optionValueString");
    s.store_field("value", value_);
    s.store_class_end();
  }
}

outline::outline()
  : paths_()
{}

outline::outline(array<object_ptr<closedVectorPath>> &&paths_)
  : paths_(std::move(paths_))
{}

const std::int32_t outline::ID;

void outline::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "outline");
    { s.store_vector_begin("paths", paths_.size()); for (const auto &_value : paths_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

photo::photo()
  : has_stickers_()
  , minithumbnail_()
  , sizes_()
{}

photo::photo(bool has_stickers_, object_ptr<minithumbnail> &&minithumbnail_, array<object_ptr<photoSize>> &&sizes_)
  : has_stickers_(has_stickers_)
  , minithumbnail_(std::move(minithumbnail_))
  , sizes_(std::move(sizes_))
{}

const std::int32_t photo::ID;

void photo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "photo");
    s.store_field("has_stickers", has_stickers_);
    s.store_object_field("minithumbnail", static_cast<const BaseObject *>(minithumbnail_.get()));
    { s.store_vector_begin("sizes", sizes_.size()); for (const auto &_value : sizes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

premiumFeaturePromotionAnimation::premiumFeaturePromotionAnimation()
  : feature_()
  , animation_()
{}

premiumFeaturePromotionAnimation::premiumFeaturePromotionAnimation(object_ptr<PremiumFeature> &&feature_, object_ptr<animation> &&animation_)
  : feature_(std::move(feature_))
  , animation_(std::move(animation_))
{}

const std::int32_t premiumFeaturePromotionAnimation::ID;

void premiumFeaturePromotionAnimation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeaturePromotionAnimation");
    s.store_object_field("feature", static_cast<const BaseObject *>(feature_.get()));
    s.store_object_field("animation", static_cast<const BaseObject *>(animation_.get()));
    s.store_class_end();
  }
}

premiumFeatures::premiumFeatures()
  : features_()
  , limits_()
  , payment_link_()
{}

premiumFeatures::premiumFeatures(array<object_ptr<PremiumFeature>> &&features_, array<object_ptr<premiumLimit>> &&limits_, object_ptr<InternalLinkType> &&payment_link_)
  : features_(std::move(features_))
  , limits_(std::move(limits_))
  , payment_link_(std::move(payment_link_))
{}

const std::int32_t premiumFeatures::ID;

void premiumFeatures::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatures");
    { s.store_vector_begin("features", features_.size()); for (const auto &_value : features_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("limits", limits_.size()); for (const auto &_value : limits_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("payment_link", static_cast<const BaseObject *>(payment_link_.get()));
    s.store_class_end();
  }
}

premiumGiveawayPaymentOptions::premiumGiveawayPaymentOptions()
  : options_()
{}

premiumGiveawayPaymentOptions::premiumGiveawayPaymentOptions(array<object_ptr<premiumGiveawayPaymentOption>> &&options_)
  : options_(std::move(options_))
{}

const std::int32_t premiumGiveawayPaymentOptions::ID;

void premiumGiveawayPaymentOptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumGiveawayPaymentOptions");
    { s.store_vector_begin("options", options_.size()); for (const auto &_value : options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

premiumLimitTypeSupergroupCount::premiumLimitTypeSupergroupCount() {
}

const std::int32_t premiumLimitTypeSupergroupCount::ID;

void premiumLimitTypeSupergroupCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimitTypeSupergroupCount");
    s.store_class_end();
  }
}

premiumLimitTypePinnedChatCount::premiumLimitTypePinnedChatCount() {
}

const std::int32_t premiumLimitTypePinnedChatCount::ID;

void premiumLimitTypePinnedChatCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimitTypePinnedChatCount");
    s.store_class_end();
  }
}

premiumLimitTypeCreatedPublicChatCount::premiumLimitTypeCreatedPublicChatCount() {
}

const std::int32_t premiumLimitTypeCreatedPublicChatCount::ID;

void premiumLimitTypeCreatedPublicChatCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimitTypeCreatedPublicChatCount");
    s.store_class_end();
  }
}

premiumLimitTypeSavedAnimationCount::premiumLimitTypeSavedAnimationCount() {
}

const std::int32_t premiumLimitTypeSavedAnimationCount::ID;

void premiumLimitTypeSavedAnimationCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimitTypeSavedAnimationCount");
    s.store_class_end();
  }
}

premiumLimitTypeFavoriteStickerCount::premiumLimitTypeFavoriteStickerCount() {
}

const std::int32_t premiumLimitTypeFavoriteStickerCount::ID;

void premiumLimitTypeFavoriteStickerCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimitTypeFavoriteStickerCount");
    s.store_class_end();
  }
}

premiumLimitTypeChatFolderCount::premiumLimitTypeChatFolderCount() {
}

const std::int32_t premiumLimitTypeChatFolderCount::ID;

void premiumLimitTypeChatFolderCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimitTypeChatFolderCount");
    s.store_class_end();
  }
}

premiumLimitTypeChatFolderChosenChatCount::premiumLimitTypeChatFolderChosenChatCount() {
}

const std::int32_t premiumLimitTypeChatFolderChosenChatCount::ID;

void premiumLimitTypeChatFolderChosenChatCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimitTypeChatFolderChosenChatCount");
    s.store_class_end();
  }
}

premiumLimitTypePinnedArchivedChatCount::premiumLimitTypePinnedArchivedChatCount() {
}

const std::int32_t premiumLimitTypePinnedArchivedChatCount::ID;

void premiumLimitTypePinnedArchivedChatCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimitTypePinnedArchivedChatCount");
    s.store_class_end();
  }
}

premiumLimitTypePinnedSavedMessagesTopicCount::premiumLimitTypePinnedSavedMessagesTopicCount() {
}

const std::int32_t premiumLimitTypePinnedSavedMessagesTopicCount::ID;

void premiumLimitTypePinnedSavedMessagesTopicCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimitTypePinnedSavedMessagesTopicCount");
    s.store_class_end();
  }
}

premiumLimitTypeCaptionLength::premiumLimitTypeCaptionLength() {
}

const std::int32_t premiumLimitTypeCaptionLength::ID;

void premiumLimitTypeCaptionLength::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimitTypeCaptionLength");
    s.store_class_end();
  }
}

premiumLimitTypeBioLength::premiumLimitTypeBioLength() {
}

const std::int32_t premiumLimitTypeBioLength::ID;

void premiumLimitTypeBioLength::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimitTypeBioLength");
    s.store_class_end();
  }
}

premiumLimitTypeChatFolderInviteLinkCount::premiumLimitTypeChatFolderInviteLinkCount() {
}

const std::int32_t premiumLimitTypeChatFolderInviteLinkCount::ID;

void premiumLimitTypeChatFolderInviteLinkCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimitTypeChatFolderInviteLinkCount");
    s.store_class_end();
  }
}

premiumLimitTypeShareableChatFolderCount::premiumLimitTypeShareableChatFolderCount() {
}

const std::int32_t premiumLimitTypeShareableChatFolderCount::ID;

void premiumLimitTypeShareableChatFolderCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimitTypeShareableChatFolderCount");
    s.store_class_end();
  }
}

premiumLimitTypeActiveStoryCount::premiumLimitTypeActiveStoryCount() {
}

const std::int32_t premiumLimitTypeActiveStoryCount::ID;

void premiumLimitTypeActiveStoryCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimitTypeActiveStoryCount");
    s.store_class_end();
  }
}

premiumLimitTypeWeeklyPostedStoryCount::premiumLimitTypeWeeklyPostedStoryCount() {
}

const std::int32_t premiumLimitTypeWeeklyPostedStoryCount::ID;

void premiumLimitTypeWeeklyPostedStoryCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimitTypeWeeklyPostedStoryCount");
    s.store_class_end();
  }
}

premiumLimitTypeMonthlyPostedStoryCount::premiumLimitTypeMonthlyPostedStoryCount() {
}

const std::int32_t premiumLimitTypeMonthlyPostedStoryCount::ID;

void premiumLimitTypeMonthlyPostedStoryCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimitTypeMonthlyPostedStoryCount");
    s.store_class_end();
  }
}

premiumLimitTypeStoryCaptionLength::premiumLimitTypeStoryCaptionLength() {
}

const std::int32_t premiumLimitTypeStoryCaptionLength::ID;

void premiumLimitTypeStoryCaptionLength::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimitTypeStoryCaptionLength");
    s.store_class_end();
  }
}

premiumLimitTypeStorySuggestedReactionAreaCount::premiumLimitTypeStorySuggestedReactionAreaCount() {
}

const std::int32_t premiumLimitTypeStorySuggestedReactionAreaCount::ID;

void premiumLimitTypeStorySuggestedReactionAreaCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimitTypeStorySuggestedReactionAreaCount");
    s.store_class_end();
  }
}

premiumLimitTypeSimilarChatCount::premiumLimitTypeSimilarChatCount() {
}

const std::int32_t premiumLimitTypeSimilarChatCount::ID;

void premiumLimitTypeSimilarChatCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimitTypeSimilarChatCount");
    s.store_class_end();
  }
}

premiumStoryFeaturePriorityOrder::premiumStoryFeaturePriorityOrder() {
}

const std::int32_t premiumStoryFeaturePriorityOrder::ID;

void premiumStoryFeaturePriorityOrder::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumStoryFeaturePriorityOrder");
    s.store_class_end();
  }
}

premiumStoryFeatureStealthMode::premiumStoryFeatureStealthMode() {
}

const std::int32_t premiumStoryFeatureStealthMode::ID;

void premiumStoryFeatureStealthMode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumStoryFeatureStealthMode");
    s.store_class_end();
  }
}

premiumStoryFeaturePermanentViewsHistory::premiumStoryFeaturePermanentViewsHistory() {
}

const std::int32_t premiumStoryFeaturePermanentViewsHistory::ID;

void premiumStoryFeaturePermanentViewsHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumStoryFeaturePermanentViewsHistory");
    s.store_class_end();
  }
}

premiumStoryFeatureCustomExpirationDuration::premiumStoryFeatureCustomExpirationDuration() {
}

const std::int32_t premiumStoryFeatureCustomExpirationDuration::ID;

void premiumStoryFeatureCustomExpirationDuration::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumStoryFeatureCustomExpirationDuration");
    s.store_class_end();
  }
}

premiumStoryFeatureSaveStories::premiumStoryFeatureSaveStories() {
}

const std::int32_t premiumStoryFeatureSaveStories::ID;

void premiumStoryFeatureSaveStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumStoryFeatureSaveStories");
    s.store_class_end();
  }
}

premiumStoryFeatureLinksAndFormatting::premiumStoryFeatureLinksAndFormatting() {
}

const std::int32_t premiumStoryFeatureLinksAndFormatting::ID;

void premiumStoryFeatureLinksAndFormatting::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumStoryFeatureLinksAndFormatting");
    s.store_class_end();
  }
}

premiumStoryFeatureVideoQuality::premiumStoryFeatureVideoQuality() {
}

const std::int32_t premiumStoryFeatureVideoQuality::ID;

void premiumStoryFeatureVideoQuality::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumStoryFeatureVideoQuality");
    s.store_class_end();
  }
}

profileTabPosts::profileTabPosts() {
}

const std::int32_t profileTabPosts::ID;

void profileTabPosts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "profileTabPosts");
    s.store_class_end();
  }
}

profileTabGifts::profileTabGifts() {
}

const std::int32_t profileTabGifts::ID;

void profileTabGifts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "profileTabGifts");
    s.store_class_end();
  }
}

profileTabMedia::profileTabMedia() {
}

const std::int32_t profileTabMedia::ID;

void profileTabMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "profileTabMedia");
    s.store_class_end();
  }
}

profileTabFiles::profileTabFiles() {
}

const std::int32_t profileTabFiles::ID;

void profileTabFiles::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "profileTabFiles");
    s.store_class_end();
  }
}

profileTabLinks::profileTabLinks() {
}

const std::int32_t profileTabLinks::ID;

void profileTabLinks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "profileTabLinks");
    s.store_class_end();
  }
}

profileTabMusic::profileTabMusic() {
}

const std::int32_t profileTabMusic::ID;

void profileTabMusic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "profileTabMusic");
    s.store_class_end();
  }
}

profileTabVoice::profileTabVoice() {
}

const std::int32_t profileTabVoice::ID;

void profileTabVoice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "profileTabVoice");
    s.store_class_end();
  }
}

profileTabGifs::profileTabGifs() {
}

const std::int32_t profileTabGifs::ID;

void profileTabGifs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "profileTabGifs");
    s.store_class_end();
  }
}

proxyTypeSocks5::proxyTypeSocks5()
  : username_()
  , password_()
{}

proxyTypeSocks5::proxyTypeSocks5(string const &username_, string const &password_)
  : username_(username_)
  , password_(password_)
{}

const std::int32_t proxyTypeSocks5::ID;

void proxyTypeSocks5::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "proxyTypeSocks5");
    s.store_field("username", username_);
    s.store_field("password", password_);
    s.store_class_end();
  }
}

proxyTypeHttp::proxyTypeHttp()
  : username_()
  , password_()
  , http_only_()
{}

proxyTypeHttp::proxyTypeHttp(string const &username_, string const &password_, bool http_only_)
  : username_(username_)
  , password_(password_)
  , http_only_(http_only_)
{}

const std::int32_t proxyTypeHttp::ID;

void proxyTypeHttp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "proxyTypeHttp");
    s.store_field("username", username_);
    s.store_field("password", password_);
    s.store_field("http_only", http_only_);
    s.store_class_end();
  }
}

proxyTypeMtproto::proxyTypeMtproto()
  : secret_()
{}

proxyTypeMtproto::proxyTypeMtproto(string const &secret_)
  : secret_(secret_)
{}

const std::int32_t proxyTypeMtproto::ID;

void proxyTypeMtproto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "proxyTypeMtproto");
    s.store_field("secret", secret_);
    s.store_class_end();
  }
}

reportSponsoredResultOk::reportSponsoredResultOk() {
}

const std::int32_t reportSponsoredResultOk::ID;

void reportSponsoredResultOk::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportSponsoredResultOk");
    s.store_class_end();
  }
}

reportSponsoredResultFailed::reportSponsoredResultFailed() {
}

const std::int32_t reportSponsoredResultFailed::ID;

void reportSponsoredResultFailed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportSponsoredResultFailed");
    s.store_class_end();
  }
}

reportSponsoredResultOptionRequired::reportSponsoredResultOptionRequired()
  : title_()
  , options_()
{}

reportSponsoredResultOptionRequired::reportSponsoredResultOptionRequired(string const &title_, array<object_ptr<reportOption>> &&options_)
  : title_(title_)
  , options_(std::move(options_))
{}

const std::int32_t reportSponsoredResultOptionRequired::ID;

void reportSponsoredResultOptionRequired::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportSponsoredResultOptionRequired");
    s.store_field("title", title_);
    { s.store_vector_begin("options", options_.size()); for (const auto &_value : options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

reportSponsoredResultAdsHidden::reportSponsoredResultAdsHidden() {
}

const std::int32_t reportSponsoredResultAdsHidden::ID;

void reportSponsoredResultAdsHidden::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportSponsoredResultAdsHidden");
    s.store_class_end();
  }
}

reportSponsoredResultPremiumRequired::reportSponsoredResultPremiumRequired() {
}

const std::int32_t reportSponsoredResultPremiumRequired::ID;

void reportSponsoredResultPremiumRequired::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportSponsoredResultPremiumRequired");
    s.store_class_end();
  }
}

resendCodeReasonUserRequest::resendCodeReasonUserRequest() {
}

const std::int32_t resendCodeReasonUserRequest::ID;

void resendCodeReasonUserRequest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "resendCodeReasonUserRequest");
    s.store_class_end();
  }
}

resendCodeReasonVerificationFailed::resendCodeReasonVerificationFailed()
  : error_message_()
{}

resendCodeReasonVerificationFailed::resendCodeReasonVerificationFailed(string const &error_message_)
  : error_message_(error_message_)
{}

const std::int32_t resendCodeReasonVerificationFailed::ID;

void resendCodeReasonVerificationFailed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "resendCodeReasonVerificationFailed");
    s.store_field("error_message", error_message_);
    s.store_class_end();
  }
}

searchMessagesChatTypeFilterPrivate::searchMessagesChatTypeFilterPrivate() {
}

const std::int32_t searchMessagesChatTypeFilterPrivate::ID;

void searchMessagesChatTypeFilterPrivate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesChatTypeFilterPrivate");
    s.store_class_end();
  }
}

searchMessagesChatTypeFilterGroup::searchMessagesChatTypeFilterGroup() {
}

const std::int32_t searchMessagesChatTypeFilterGroup::ID;

void searchMessagesChatTypeFilterGroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesChatTypeFilterGroup");
    s.store_class_end();
  }
}

searchMessagesChatTypeFilterChannel::searchMessagesChatTypeFilterChannel() {
}

const std::int32_t searchMessagesChatTypeFilterChannel::ID;

void searchMessagesChatTypeFilterChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesChatTypeFilterChannel");
    s.store_class_end();
  }
}

sponsoredMessage::sponsoredMessage()
  : message_id_()
  , is_recommended_()
  , can_be_reported_()
  , content_()
  , sponsor_()
  , title_()
  , button_text_()
  , accent_color_id_()
  , background_custom_emoji_id_()
  , additional_info_()
{}

sponsoredMessage::sponsoredMessage(int53 message_id_, bool is_recommended_, bool can_be_reported_, object_ptr<MessageContent> &&content_, object_ptr<advertisementSponsor> &&sponsor_, string const &title_, string const &button_text_, int32 accent_color_id_, int64 background_custom_emoji_id_, string const &additional_info_)
  : message_id_(message_id_)
  , is_recommended_(is_recommended_)
  , can_be_reported_(can_be_reported_)
  , content_(std::move(content_))
  , sponsor_(std::move(sponsor_))
  , title_(title_)
  , button_text_(button_text_)
  , accent_color_id_(accent_color_id_)
  , background_custom_emoji_id_(background_custom_emoji_id_)
  , additional_info_(additional_info_)
{}

const std::int32_t sponsoredMessage::ID;

void sponsoredMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sponsoredMessage");
    s.store_field("message_id", message_id_);
    s.store_field("is_recommended", is_recommended_);
    s.store_field("can_be_reported", can_be_reported_);
    s.store_object_field("content", static_cast<const BaseObject *>(content_.get()));
    s.store_object_field("sponsor", static_cast<const BaseObject *>(sponsor_.get()));
    s.store_field("title", title_);
    s.store_field("button_text", button_text_);
    s.store_field("accent_color_id", accent_color_id_);
    s.store_field("background_custom_emoji_id", background_custom_emoji_id_);
    s.store_field("additional_info", additional_info_);
    s.store_class_end();
  }
}

starPaymentOptions::starPaymentOptions()
  : options_()
{}

starPaymentOptions::starPaymentOptions(array<object_ptr<starPaymentOption>> &&options_)
  : options_(std::move(options_))
{}

const std::int32_t starPaymentOptions::ID;

void starPaymentOptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starPaymentOptions");
    { s.store_vector_begin("options", options_.size()); for (const auto &_value : options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

starTransaction::starTransaction()
  : id_()
  , star_amount_()
  , is_refund_()
  , date_()
  , type_()
{}

starTransaction::starTransaction(string const &id_, object_ptr<starAmount> &&star_amount_, bool is_refund_, int32 date_, object_ptr<StarTransactionType> &&type_)
  : id_(id_)
  , star_amount_(std::move(star_amount_))
  , is_refund_(is_refund_)
  , date_(date_)
  , type_(std::move(type_))
{}

const std::int32_t starTransaction::ID;

void starTransaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransaction");
    s.store_field("id", id_);
    s.store_object_field("star_amount", static_cast<const BaseObject *>(star_amount_.get()));
    s.store_field("is_refund", is_refund_);
    s.store_field("date", date_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

stickerFormatWebp::stickerFormatWebp() {
}

const std::int32_t stickerFormatWebp::ID;

void stickerFormatWebp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickerFormatWebp");
    s.store_class_end();
  }
}

stickerFormatTgs::stickerFormatTgs() {
}

const std::int32_t stickerFormatTgs::ID;

void stickerFormatTgs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickerFormatTgs");
    s.store_class_end();
  }
}

stickerFormatWebm::stickerFormatWebm() {
}

const std::int32_t stickerFormatWebm::ID;

void stickerFormatWebm::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickerFormatWebm");
    s.store_class_end();
  }
}

storageStatisticsFast::storageStatisticsFast()
  : files_size_()
  , file_count_()
  , database_size_()
  , language_pack_database_size_()
  , log_size_()
{}

storageStatisticsFast::storageStatisticsFast(int53 files_size_, int32 file_count_, int53 database_size_, int53 language_pack_database_size_, int53 log_size_)
  : files_size_(files_size_)
  , file_count_(file_count_)
  , database_size_(database_size_)
  , language_pack_database_size_(language_pack_database_size_)
  , log_size_(log_size_)
{}

const std::int32_t storageStatisticsFast::ID;

void storageStatisticsFast::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storageStatisticsFast");
    s.store_field("files_size", files_size_);
    s.store_field("file_count", file_count_);
    s.store_field("database_size", database_size_);
    s.store_field("language_pack_database_size", language_pack_database_size_);
    s.store_field("log_size", log_size_);
    s.store_class_end();
  }
}

storyInteractions::storyInteractions()
  : total_count_()
  , total_forward_count_()
  , total_reaction_count_()
  , interactions_()
  , next_offset_()
{}

storyInteractions::storyInteractions(int32 total_count_, int32 total_forward_count_, int32 total_reaction_count_, array<object_ptr<storyInteraction>> &&interactions_, string const &next_offset_)
  : total_count_(total_count_)
  , total_forward_count_(total_forward_count_)
  , total_reaction_count_(total_reaction_count_)
  , interactions_(std::move(interactions_))
  , next_offset_(next_offset_)
{}

const std::int32_t storyInteractions::ID;

void storyInteractions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyInteractions");
    s.store_field("total_count", total_count_);
    s.store_field("total_forward_count", total_forward_count_);
    s.store_field("total_reaction_count", total_reaction_count_);
    { s.store_vector_begin("interactions", interactions_.size()); for (const auto &_value : interactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

storyPrivacySettingsEveryone::storyPrivacySettingsEveryone()
  : except_user_ids_()
{}

storyPrivacySettingsEveryone::storyPrivacySettingsEveryone(array<int53> &&except_user_ids_)
  : except_user_ids_(std::move(except_user_ids_))
{}

const std::int32_t storyPrivacySettingsEveryone::ID;

void storyPrivacySettingsEveryone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyPrivacySettingsEveryone");
    { s.store_vector_begin("except_user_ids", except_user_ids_.size()); for (const auto &_value : except_user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

storyPrivacySettingsContacts::storyPrivacySettingsContacts()
  : except_user_ids_()
{}

storyPrivacySettingsContacts::storyPrivacySettingsContacts(array<int53> &&except_user_ids_)
  : except_user_ids_(std::move(except_user_ids_))
{}

const std::int32_t storyPrivacySettingsContacts::ID;

void storyPrivacySettingsContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyPrivacySettingsContacts");
    { s.store_vector_begin("except_user_ids", except_user_ids_.size()); for (const auto &_value : except_user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

storyPrivacySettingsCloseFriends::storyPrivacySettingsCloseFriends() {
}

const std::int32_t storyPrivacySettingsCloseFriends::ID;

void storyPrivacySettingsCloseFriends::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyPrivacySettingsCloseFriends");
    s.store_class_end();
  }
}

storyPrivacySettingsSelectedUsers::storyPrivacySettingsSelectedUsers()
  : user_ids_()
{}

storyPrivacySettingsSelectedUsers::storyPrivacySettingsSelectedUsers(array<int53> &&user_ids_)
  : user_ids_(std::move(user_ids_))
{}

const std::int32_t storyPrivacySettingsSelectedUsers::ID;

void storyPrivacySettingsSelectedUsers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyPrivacySettingsSelectedUsers");
    { s.store_vector_begin("user_ids", user_ids_.size()); for (const auto &_value : user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

suggestedPostPriceStar::suggestedPostPriceStar()
  : star_count_()
{}

suggestedPostPriceStar::suggestedPostPriceStar(int53 star_count_)
  : star_count_(star_count_)
{}

const std::int32_t suggestedPostPriceStar::ID;

void suggestedPostPriceStar::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedPostPriceStar");
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

suggestedPostPriceTon::suggestedPostPriceTon()
  : toncoin_cent_count_()
{}

suggestedPostPriceTon::suggestedPostPriceTon(int53 toncoin_cent_count_)
  : toncoin_cent_count_(toncoin_cent_count_)
{}

const std::int32_t suggestedPostPriceTon::ID;

void suggestedPostPriceTon::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedPostPriceTon");
    s.store_field("toncoin_cent_count", toncoin_cent_count_);
    s.store_class_end();
  }
}

testVectorInt::testVectorInt()
  : value_()
{}

testVectorInt::testVectorInt(array<int32> &&value_)
  : value_(std::move(value_))
{}

const std::int32_t testVectorInt::ID;

void testVectorInt::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testVectorInt");
    { s.store_vector_begin("value", value_.size()); for (const auto &_value : value_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

textEntities::textEntities()
  : entities_()
{}

textEntities::textEntities(array<object_ptr<textEntity>> &&entities_)
  : entities_(std::move(entities_))
{}

const std::int32_t textEntities::ID;

void textEntities::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntities");
    { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

tonRevenueStatistics::tonRevenueStatistics()
  : revenue_by_day_graph_()
  , status_()
  , usd_rate_()
{}

tonRevenueStatistics::tonRevenueStatistics(object_ptr<StatisticalGraph> &&revenue_by_day_graph_, object_ptr<tonRevenueStatus> &&status_, double usd_rate_)
  : revenue_by_day_graph_(std::move(revenue_by_day_graph_))
  , status_(std::move(status_))
  , usd_rate_(usd_rate_)
{}

const std::int32_t tonRevenueStatistics::ID;

void tonRevenueStatistics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "tonRevenueStatistics");
    s.store_object_field("revenue_by_day_graph", static_cast<const BaseObject *>(revenue_by_day_graph_.get()));
    s.store_object_field("status", static_cast<const BaseObject *>(status_.get()));
    s.store_field("usd_rate", usd_rate_);
    s.store_class_end();
  }
}

unreadReaction::unreadReaction()
  : type_()
  , sender_id_()
  , is_big_()
{}

unreadReaction::unreadReaction(object_ptr<ReactionType> &&type_, object_ptr<MessageSender> &&sender_id_, bool is_big_)
  : type_(std::move(type_))
  , sender_id_(std::move(sender_id_))
  , is_big_(is_big_)
{}

const std::int32_t unreadReaction::ID;

void unreadReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "unreadReaction");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_field("is_big", is_big_);
    s.store_class_end();
  }
}

upgradedGiftBackdrop::upgradedGiftBackdrop()
  : id_()
  , name_()
  , colors_()
  , rarity_per_mille_()
{}

upgradedGiftBackdrop::upgradedGiftBackdrop(int32 id_, string const &name_, object_ptr<upgradedGiftBackdropColors> &&colors_, int32 rarity_per_mille_)
  : id_(id_)
  , name_(name_)
  , colors_(std::move(colors_))
  , rarity_per_mille_(rarity_per_mille_)
{}

const std::int32_t upgradedGiftBackdrop::ID;

void upgradedGiftBackdrop::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradedGiftBackdrop");
    s.store_field("id", id_);
    s.store_field("name", name_);
    s.store_object_field("colors", static_cast<const BaseObject *>(colors_.get()));
    s.store_field("rarity_per_mille", rarity_per_mille_);
    s.store_class_end();
  }
}

upgradedGiftBackdropColors::upgradedGiftBackdropColors()
  : center_color_()
  , edge_color_()
  , symbol_color_()
  , text_color_()
{}

upgradedGiftBackdropColors::upgradedGiftBackdropColors(int32 center_color_, int32 edge_color_, int32 symbol_color_, int32 text_color_)
  : center_color_(center_color_)
  , edge_color_(edge_color_)
  , symbol_color_(symbol_color_)
  , text_color_(text_color_)
{}

const std::int32_t upgradedGiftBackdropColors::ID;

void upgradedGiftBackdropColors::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradedGiftBackdropColors");
    s.store_field("center_color", center_color_);
    s.store_field("edge_color", edge_color_);
    s.store_field("symbol_color", symbol_color_);
    s.store_field("text_color", text_color_);
    s.store_class_end();
  }
}

upgradedGiftColors::upgradedGiftColors()
  : id_()
  , model_custom_emoji_id_()
  , symbol_custom_emoji_id_()
  , light_theme_accent_color_()
  , light_theme_colors_()
  , dark_theme_accent_color_()
  , dark_theme_colors_()
{}

upgradedGiftColors::upgradedGiftColors(int64 id_, int64 model_custom_emoji_id_, int64 symbol_custom_emoji_id_, int32 light_theme_accent_color_, array<int32> &&light_theme_colors_, int32 dark_theme_accent_color_, array<int32> &&dark_theme_colors_)
  : id_(id_)
  , model_custom_emoji_id_(model_custom_emoji_id_)
  , symbol_custom_emoji_id_(symbol_custom_emoji_id_)
  , light_theme_accent_color_(light_theme_accent_color_)
  , light_theme_colors_(std::move(light_theme_colors_))
  , dark_theme_accent_color_(dark_theme_accent_color_)
  , dark_theme_colors_(std::move(dark_theme_colors_))
{}

const std::int32_t upgradedGiftColors::ID;

void upgradedGiftColors::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradedGiftColors");
    s.store_field("id", id_);
    s.store_field("model_custom_emoji_id", model_custom_emoji_id_);
    s.store_field("symbol_custom_emoji_id", symbol_custom_emoji_id_);
    s.store_field("light_theme_accent_color", light_theme_accent_color_);
    { s.store_vector_begin("light_theme_colors", light_theme_colors_.size()); for (const auto &_value : light_theme_colors_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("dark_theme_accent_color", dark_theme_accent_color_);
    { s.store_vector_begin("dark_theme_colors", dark_theme_colors_.size()); for (const auto &_value : dark_theme_colors_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

upgradedGiftSymbol::upgradedGiftSymbol()
  : name_()
  , sticker_()
  , rarity_per_mille_()
{}

upgradedGiftSymbol::upgradedGiftSymbol(string const &name_, object_ptr<sticker> &&sticker_, int32 rarity_per_mille_)
  : name_(name_)
  , sticker_(std::move(sticker_))
  , rarity_per_mille_(rarity_per_mille_)
{}

const std::int32_t upgradedGiftSymbol::ID;

void upgradedGiftSymbol::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradedGiftSymbol");
    s.store_field("name", name_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_field("rarity_per_mille", rarity_per_mille_);
    s.store_class_end();
  }
}

userSupportInfo::userSupportInfo()
  : message_()
  , author_()
  , date_()
{}

userSupportInfo::userSupportInfo(object_ptr<formattedText> &&message_, string const &author_, int32 date_)
  : message_(std::move(message_))
  , author_(author_)
  , date_(date_)
{}

const std::int32_t userSupportInfo::ID;

void userSupportInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userSupportInfo");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_field("author", author_);
    s.store_field("date", date_);
    s.store_class_end();
  }
}

addCustomServerLanguagePack::addCustomServerLanguagePack()
  : language_pack_id_()
{}

addCustomServerLanguagePack::addCustomServerLanguagePack(string const &language_pack_id_)
  : language_pack_id_(language_pack_id_)
{}

const std::int32_t addCustomServerLanguagePack::ID;

void addCustomServerLanguagePack::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addCustomServerLanguagePack");
    s.store_field("language_pack_id", language_pack_id_);
    s.store_class_end();
  }
}

addLogMessage::addLogMessage()
  : verbosity_level_()
  , text_()
{}

addLogMessage::addLogMessage(int32 verbosity_level_, string const &text_)
  : verbosity_level_(verbosity_level_)
  , text_(text_)
{}

const std::int32_t addLogMessage::ID;

void addLogMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addLogMessage");
    s.store_field("verbosity_level", verbosity_level_);
    s.store_field("text", text_);
    s.store_class_end();
  }
}

addPendingPaidMessageReaction::addPendingPaidMessageReaction()
  : chat_id_()
  , message_id_()
  , star_count_()
  , type_()
{}

addPendingPaidMessageReaction::addPendingPaidMessageReaction(int53 chat_id_, int53 message_id_, int53 star_count_, object_ptr<PaidReactionType> &&type_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , star_count_(star_count_)
  , type_(std::move(type_))
{}

const std::int32_t addPendingPaidMessageReaction::ID;

void addPendingPaidMessageReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addPendingPaidMessageReaction");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("star_count", star_count_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

addProfileAudio::addProfileAudio()
  : file_id_()
{}

addProfileAudio::addProfileAudio(int32 file_id_)
  : file_id_(file_id_)
{}

const std::int32_t addProfileAudio::ID;

void addProfileAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addProfileAudio");
    s.store_field("file_id", file_id_);
    s.store_class_end();
  }
}

addStoryAlbumStories::addStoryAlbumStories()
  : chat_id_()
  , story_album_id_()
  , story_ids_()
{}

addStoryAlbumStories::addStoryAlbumStories(int53 chat_id_, int32 story_album_id_, array<int32> &&story_ids_)
  : chat_id_(chat_id_)
  , story_album_id_(story_album_id_)
  , story_ids_(std::move(story_ids_))
{}

const std::int32_t addStoryAlbumStories::ID;

void addStoryAlbumStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addStoryAlbumStories");
    s.store_field("chat_id", chat_id_);
    s.store_field("story_album_id", story_album_id_);
    { s.store_vector_begin("story_ids", story_ids_.size()); for (const auto &_value : story_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

canTransferOwnership::canTransferOwnership() {
}

const std::int32_t canTransferOwnership::ID;

void canTransferOwnership::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canTransferOwnership");
    s.store_class_end();
  }
}

cancelDownloadFile::cancelDownloadFile()
  : file_id_()
  , only_if_pending_()
{}

cancelDownloadFile::cancelDownloadFile(int32 file_id_, bool only_if_pending_)
  : file_id_(file_id_)
  , only_if_pending_(only_if_pending_)
{}

const std::int32_t cancelDownloadFile::ID;

void cancelDownloadFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "cancelDownloadFile");
    s.store_field("file_id", file_id_);
    s.store_field("only_if_pending", only_if_pending_);
    s.store_class_end();
  }
}

cancelPasswordReset::cancelPasswordReset() {
}

const std::int32_t cancelPasswordReset::ID;

void cancelPasswordReset::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "cancelPasswordReset");
    s.store_class_end();
  }
}

closeSecretChat::closeSecretChat()
  : secret_chat_id_()
{}

closeSecretChat::closeSecretChat(int32 secret_chat_id_)
  : secret_chat_id_(secret_chat_id_)
{}

const std::int32_t closeSecretChat::ID;

void closeSecretChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "closeSecretChat");
    s.store_field("secret_chat_id", secret_chat_id_);
    s.store_class_end();
  }
}

confirmSession::confirmSession()
  : session_id_()
{}

confirmSession::confirmSession(int64 session_id_)
  : session_id_(session_id_)
{}

const std::int32_t confirmSession::ID;

void confirmSession::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "confirmSession");
    s.store_field("session_id", session_id_);
    s.store_class_end();
  }
}

createChatFolder::createChatFolder()
  : folder_()
{}

createChatFolder::createChatFolder(object_ptr<chatFolder> &&folder_)
  : folder_(std::move(folder_))
{}

const std::int32_t createChatFolder::ID;

void createChatFolder::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createChatFolder");
    s.store_object_field("folder", static_cast<const BaseObject *>(folder_.get()));
    s.store_class_end();
  }
}

createChatSubscriptionInviteLink::createChatSubscriptionInviteLink()
  : chat_id_()
  , name_()
  , subscription_pricing_()
{}

createChatSubscriptionInviteLink::createChatSubscriptionInviteLink(int53 chat_id_, string const &name_, object_ptr<starSubscriptionPricing> &&subscription_pricing_)
  : chat_id_(chat_id_)
  , name_(name_)
  , subscription_pricing_(std::move(subscription_pricing_))
{}

const std::int32_t createChatSubscriptionInviteLink::ID;

void createChatSubscriptionInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createChatSubscriptionInviteLink");
    s.store_field("chat_id", chat_id_);
    s.store_field("name", name_);
    s.store_object_field("subscription_pricing", static_cast<const BaseObject *>(subscription_pricing_.get()));
    s.store_class_end();
  }
}

declineGroupCallInvitation::declineGroupCallInvitation()
  : chat_id_()
  , message_id_()
{}

declineGroupCallInvitation::declineGroupCallInvitation(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t declineGroupCallInvitation::ID;

void declineGroupCallInvitation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "declineGroupCallInvitation");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

deleteAllCallMessages::deleteAllCallMessages()
  : revoke_()
{}

deleteAllCallMessages::deleteAllCallMessages(bool revoke_)
  : revoke_(revoke_)
{}

const std::int32_t deleteAllCallMessages::ID;

void deleteAllCallMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteAllCallMessages");
    s.store_field("revoke", revoke_);
    s.store_class_end();
  }
}

deleteRevokedChatInviteLink::deleteRevokedChatInviteLink()
  : chat_id_()
  , invite_link_()
{}

deleteRevokedChatInviteLink::deleteRevokedChatInviteLink(int53 chat_id_, string const &invite_link_)
  : chat_id_(chat_id_)
  , invite_link_(invite_link_)
{}

const std::int32_t deleteRevokedChatInviteLink::ID;

void deleteRevokedChatInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteRevokedChatInviteLink");
    s.store_field("chat_id", chat_id_);
    s.store_field("invite_link", invite_link_);
    s.store_class_end();
  }
}

deleteSavedOrderInfo::deleteSavedOrderInfo() {
}

const std::int32_t deleteSavedOrderInfo::ID;

void deleteSavedOrderInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteSavedOrderInfo");
    s.store_class_end();
  }
}

destroy::destroy() {
}

const std::int32_t destroy::ID;

void destroy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "destroy");
    s.store_class_end();
  }
}

editBusinessChatLink::editBusinessChatLink()
  : link_()
  , link_info_()
{}

editBusinessChatLink::editBusinessChatLink(string const &link_, object_ptr<inputBusinessChatLink> &&link_info_)
  : link_(link_)
  , link_info_(std::move(link_info_))
{}

const std::int32_t editBusinessChatLink::ID;

void editBusinessChatLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editBusinessChatLink");
    s.store_field("link", link_);
    s.store_object_field("link_info", static_cast<const BaseObject *>(link_info_.get()));
    s.store_class_end();
  }
}

editBusinessMessageLiveLocation::editBusinessMessageLiveLocation()
  : business_connection_id_()
  , chat_id_()
  , message_id_()
  , reply_markup_()
  , location_()
  , live_period_()
  , heading_()
  , proximity_alert_radius_()
{}

editBusinessMessageLiveLocation::editBusinessMessageLiveLocation(string const &business_connection_id_, int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<location> &&location_, int32 live_period_, int32 heading_, int32 proximity_alert_radius_)
  : business_connection_id_(business_connection_id_)
  , chat_id_(chat_id_)
  , message_id_(message_id_)
  , reply_markup_(std::move(reply_markup_))
  , location_(std::move(location_))
  , live_period_(live_period_)
  , heading_(heading_)
  , proximity_alert_radius_(proximity_alert_radius_)
{}

const std::int32_t editBusinessMessageLiveLocation::ID;

void editBusinessMessageLiveLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editBusinessMessageLiveLocation");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_field("live_period", live_period_);
    s.store_field("heading", heading_);
    s.store_field("proximity_alert_radius", proximity_alert_radius_);
    s.store_class_end();
  }
}

editChatSubscriptionInviteLink::editChatSubscriptionInviteLink()
  : chat_id_()
  , invite_link_()
  , name_()
{}

editChatSubscriptionInviteLink::editChatSubscriptionInviteLink(int53 chat_id_, string const &invite_link_, string const &name_)
  : chat_id_(chat_id_)
  , invite_link_(invite_link_)
  , name_(name_)
{}

const std::int32_t editChatSubscriptionInviteLink::ID;

void editChatSubscriptionInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editChatSubscriptionInviteLink");
    s.store_field("chat_id", chat_id_);
    s.store_field("invite_link", invite_link_);
    s.store_field("name", name_);
    s.store_class_end();
  }
}

editInlineMessageReplyMarkup::editInlineMessageReplyMarkup()
  : inline_message_id_()
  , reply_markup_()
{}

editInlineMessageReplyMarkup::editInlineMessageReplyMarkup(string const &inline_message_id_, object_ptr<ReplyMarkup> &&reply_markup_)
  : inline_message_id_(inline_message_id_)
  , reply_markup_(std::move(reply_markup_))
{}

const std::int32_t editInlineMessageReplyMarkup::ID;

void editInlineMessageReplyMarkup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editInlineMessageReplyMarkup");
    s.store_field("inline_message_id", inline_message_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_class_end();
  }
}

editMessageLiveLocation::editMessageLiveLocation()
  : chat_id_()
  , message_id_()
  , reply_markup_()
  , location_()
  , live_period_()
  , heading_()
  , proximity_alert_radius_()
{}

editMessageLiveLocation::editMessageLiveLocation(int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<location> &&location_, int32 live_period_, int32 heading_, int32 proximity_alert_radius_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , reply_markup_(std::move(reply_markup_))
  , location_(std::move(location_))
  , live_period_(live_period_)
  , heading_(heading_)
  , proximity_alert_radius_(proximity_alert_radius_)
{}

const std::int32_t editMessageLiveLocation::ID;

void editMessageLiveLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editMessageLiveLocation");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_field("live_period", live_period_);
    s.store_field("heading", heading_);
    s.store_field("proximity_alert_radius", proximity_alert_radius_);
    s.store_class_end();
  }
}

editStoryCover::editStoryCover()
  : story_poster_chat_id_()
  , story_id_()
  , cover_frame_timestamp_()
{}

editStoryCover::editStoryCover(int53 story_poster_chat_id_, int32 story_id_, double cover_frame_timestamp_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , story_id_(story_id_)
  , cover_frame_timestamp_(cover_frame_timestamp_)
{}

const std::int32_t editStoryCover::ID;

void editStoryCover::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editStoryCover");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("story_id", story_id_);
    s.store_field("cover_frame_timestamp", cover_frame_timestamp_);
    s.store_class_end();
  }
}

encryptGroupCallData::encryptGroupCallData()
  : group_call_id_()
  , data_channel_()
  , data_()
  , unencrypted_prefix_size_()
{}

encryptGroupCallData::encryptGroupCallData(int32 group_call_id_, object_ptr<GroupCallDataChannel> &&data_channel_, bytes const &data_, int32 unencrypted_prefix_size_)
  : group_call_id_(group_call_id_)
  , data_channel_(std::move(data_channel_))
  , data_(std::move(data_))
  , unencrypted_prefix_size_(unencrypted_prefix_size_)
{}

const std::int32_t encryptGroupCallData::ID;

void encryptGroupCallData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "encryptGroupCallData");
    s.store_field("group_call_id", group_call_id_);
    s.store_object_field("data_channel", static_cast<const BaseObject *>(data_channel_.get()));
    s.store_bytes_field("data", data_);
    s.store_field("unencrypted_prefix_size", unencrypted_prefix_size_);
    s.store_class_end();
  }
}

getAvailableGifts::getAvailableGifts() {
}

const std::int32_t getAvailableGifts::ID;

void getAvailableGifts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getAvailableGifts");
    s.store_class_end();
  }
}

getBusinessConnection::getBusinessConnection()
  : connection_id_()
{}

getBusinessConnection::getBusinessConnection(string const &connection_id_)
  : connection_id_(connection_id_)
{}

const std::int32_t getBusinessConnection::ID;

void getBusinessConnection::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getBusinessConnection");
    s.store_field("connection_id", connection_id_);
    s.store_class_end();
  }
}

getChatEventLog::getChatEventLog()
  : chat_id_()
  , query_()
  , from_event_id_()
  , limit_()
  , filters_()
  , user_ids_()
{}

getChatEventLog::getChatEventLog(int53 chat_id_, string const &query_, int64 from_event_id_, int32 limit_, object_ptr<chatEventLogFilters> &&filters_, array<int53> &&user_ids_)
  : chat_id_(chat_id_)
  , query_(query_)
  , from_event_id_(from_event_id_)
  , limit_(limit_)
  , filters_(std::move(filters_))
  , user_ids_(std::move(user_ids_))
{}

const std::int32_t getChatEventLog::ID;

void getChatEventLog::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatEventLog");
    s.store_field("chat_id", chat_id_);
    s.store_field("query", query_);
    s.store_field("from_event_id", from_event_id_);
    s.store_field("limit", limit_);
    s.store_object_field("filters", static_cast<const BaseObject *>(filters_.get()));
    { s.store_vector_begin("user_ids", user_ids_.size()); for (const auto &_value : user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

getChatJoinRequests::getChatJoinRequests()
  : chat_id_()
  , invite_link_()
  , query_()
  , offset_request_()
  , limit_()
{}

getChatJoinRequests::getChatJoinRequests(int53 chat_id_, string const &invite_link_, string const &query_, object_ptr<chatJoinRequest> &&offset_request_, int32 limit_)
  : chat_id_(chat_id_)
  , invite_link_(invite_link_)
  , query_(query_)
  , offset_request_(std::move(offset_request_))
  , limit_(limit_)
{}

const std::int32_t getChatJoinRequests::ID;

void getChatJoinRequests::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatJoinRequests");
    s.store_field("chat_id", chat_id_);
    s.store_field("invite_link", invite_link_);
    s.store_field("query", query_);
    s.store_object_field("offset_request", static_cast<const BaseObject *>(offset_request_.get()));
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getChatSimilarChats::getChatSimilarChats()
  : chat_id_()
{}

getChatSimilarChats::getChatSimilarChats(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t getChatSimilarChats::ID;

void getChatSimilarChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatSimilarChats");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

getChatStoryInteractions::getChatStoryInteractions()
  : story_poster_chat_id_()
  , story_id_()
  , reaction_type_()
  , prefer_forwards_()
  , offset_()
  , limit_()
{}

getChatStoryInteractions::getChatStoryInteractions(int53 story_poster_chat_id_, int32 story_id_, object_ptr<ReactionType> &&reaction_type_, bool prefer_forwards_, string const &offset_, int32 limit_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , story_id_(story_id_)
  , reaction_type_(std::move(reaction_type_))
  , prefer_forwards_(prefer_forwards_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getChatStoryInteractions::ID;

void getChatStoryInteractions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatStoryInteractions");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("story_id", story_id_);
    s.store_object_field("reaction_type", static_cast<const BaseObject *>(reaction_type_.get()));
    s.store_field("prefer_forwards", prefer_forwards_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getCountries::getCountries() {
}

const std::int32_t getCountries::ID;

void getCountries::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getCountries");
    s.store_class_end();
  }
}

getCurrentState::getCurrentState() {
}

const std::int32_t getCurrentState::ID;

void getCurrentState::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getCurrentState");
    s.store_class_end();
  }
}

getDeepLinkInfo::getDeepLinkInfo()
  : link_()
{}

getDeepLinkInfo::getDeepLinkInfo(string const &link_)
  : link_(link_)
{}

const std::int32_t getDeepLinkInfo::ID;

void getDeepLinkInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getDeepLinkInfo");
    s.store_field("link", link_);
    s.store_class_end();
  }
}

getDefaultChatPhotoCustomEmojiStickers::getDefaultChatPhotoCustomEmojiStickers() {
}

const std::int32_t getDefaultChatPhotoCustomEmojiStickers::ID;

void getDefaultChatPhotoCustomEmojiStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getDefaultChatPhotoCustomEmojiStickers");
    s.store_class_end();
  }
}

getDirectMessagesChatTopicHistory::getDirectMessagesChatTopicHistory()
  : chat_id_()
  , topic_id_()
  , from_message_id_()
  , offset_()
  , limit_()
{}

getDirectMessagesChatTopicHistory::getDirectMessagesChatTopicHistory(int53 chat_id_, int53 topic_id_, int53 from_message_id_, int32 offset_, int32 limit_)
  : chat_id_(chat_id_)
  , topic_id_(topic_id_)
  , from_message_id_(from_message_id_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getDirectMessagesChatTopicHistory::ID;

void getDirectMessagesChatTopicHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getDirectMessagesChatTopicHistory");
    s.store_field("chat_id", chat_id_);
    s.store_field("topic_id", topic_id_);
    s.store_field("from_message_id", from_message_id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getForumTopicLink::getForumTopicLink()
  : chat_id_()
  , forum_topic_id_()
{}

getForumTopicLink::getForumTopicLink(int53 chat_id_, int32 forum_topic_id_)
  : chat_id_(chat_id_)
  , forum_topic_id_(forum_topic_id_)
{}

const std::int32_t getForumTopicLink::ID;

void getForumTopicLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getForumTopicLink");
    s.store_field("chat_id", chat_id_);
    s.store_field("forum_topic_id", forum_topic_id_);
    s.store_class_end();
  }
}

getInactiveSupergroupChats::getInactiveSupergroupChats() {
}

const std::int32_t getInactiveSupergroupChats::ID;

void getInactiveSupergroupChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getInactiveSupergroupChats");
    s.store_class_end();
  }
}

getMessageAuthor::getMessageAuthor()
  : chat_id_()
  , message_id_()
{}

getMessageAuthor::getMessageAuthor(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t getMessageAuthor::ID;

void getMessageAuthor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMessageAuthor");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

getMessageEffect::getMessageEffect()
  : effect_id_()
{}

getMessageEffect::getMessageEffect(int64 effect_id_)
  : effect_id_(effect_id_)
{}

const std::int32_t getMessageEffect::ID;

void getMessageEffect::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMessageEffect");
    s.store_field("effect_id", effect_id_);
    s.store_class_end();
  }
}

getMessageProperties::getMessageProperties()
  : chat_id_()
  , message_id_()
{}

getMessageProperties::getMessageProperties(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t getMessageProperties::ID;

void getMessageProperties::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMessageProperties");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

getMessageStatistics::getMessageStatistics()
  : chat_id_()
  , message_id_()
  , is_dark_()
{}

getMessageStatistics::getMessageStatistics(int53 chat_id_, int53 message_id_, bool is_dark_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , is_dark_(is_dark_)
{}

const std::int32_t getMessageStatistics::ID;

void getMessageStatistics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMessageStatistics");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("is_dark", is_dark_);
    s.store_class_end();
  }
}

getPremiumInfoSticker::getPremiumInfoSticker()
  : month_count_()
{}

getPremiumInfoSticker::getPremiumInfoSticker(int32 month_count_)
  : month_count_(month_count_)
{}

const std::int32_t getPremiumInfoSticker::ID;

void getPremiumInfoSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPremiumInfoSticker");
    s.store_field("month_count", month_count_);
    s.store_class_end();
  }
}

getProxyLink::getProxyLink()
  : proxy_id_()
{}

getProxyLink::getProxyLink(int32 proxy_id_)
  : proxy_id_(proxy_id_)
{}

const std::int32_t getProxyLink::ID;

void getProxyLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getProxyLink");
    s.store_field("proxy_id", proxy_id_);
    s.store_class_end();
  }
}

getRecentInlineBots::getRecentInlineBots() {
}

const std::int32_t getRecentInlineBots::ID;

void getRecentInlineBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getRecentInlineBots");
    s.store_class_end();
  }
}

getSavedAnimations::getSavedAnimations() {
}

const std::int32_t getSavedAnimations::ID;

void getSavedAnimations::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getSavedAnimations");
    s.store_class_end();
  }
}

getSavedMessagesTopicMessageByDate::getSavedMessagesTopicMessageByDate()
  : saved_messages_topic_id_()
  , date_()
{}

getSavedMessagesTopicMessageByDate::getSavedMessagesTopicMessageByDate(int53 saved_messages_topic_id_, int32 date_)
  : saved_messages_topic_id_(saved_messages_topic_id_)
  , date_(date_)
{}

const std::int32_t getSavedMessagesTopicMessageByDate::ID;

void getSavedMessagesTopicMessageByDate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getSavedMessagesTopicMessageByDate");
    s.store_field("saved_messages_topic_id", saved_messages_topic_id_);
    s.store_field("date", date_);
    s.store_class_end();
  }
}

getSecretChat::getSecretChat()
  : secret_chat_id_()
{}

getSecretChat::getSecretChat(int32 secret_chat_id_)
  : secret_chat_id_(secret_chat_id_)
{}

const std::int32_t getSecretChat::ID;

void getSecretChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getSecretChat");
    s.store_field("secret_chat_id", secret_chat_id_);
    s.store_class_end();
  }
}

getStarPaymentOptions::getStarPaymentOptions() {
}

const std::int32_t getStarPaymentOptions::ID;

void getStarPaymentOptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStarPaymentOptions");
    s.store_class_end();
  }
}

getStarSubscriptions::getStarSubscriptions()
  : only_expiring_()
  , offset_()
{}

getStarSubscriptions::getStarSubscriptions(bool only_expiring_, string const &offset_)
  : only_expiring_(only_expiring_)
  , offset_(offset_)
{}

const std::int32_t getStarSubscriptions::ID;

void getStarSubscriptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStarSubscriptions");
    s.store_field("only_expiring", only_expiring_);
    s.store_field("offset", offset_);
    s.store_class_end();
  }
}

getStarWithdrawalUrl::getStarWithdrawalUrl()
  : owner_id_()
  , star_count_()
  , password_()
{}

getStarWithdrawalUrl::getStarWithdrawalUrl(object_ptr<MessageSender> &&owner_id_, int53 star_count_, string const &password_)
  : owner_id_(std::move(owner_id_))
  , star_count_(star_count_)
  , password_(password_)
{}

const std::int32_t getStarWithdrawalUrl::ID;

void getStarWithdrawalUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStarWithdrawalUrl");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_field("star_count", star_count_);
    s.store_field("password", password_);
    s.store_class_end();
  }
}

getStorageStatisticsFast::getStorageStatisticsFast() {
}

const std::int32_t getStorageStatisticsFast::ID;

void getStorageStatisticsFast::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStorageStatisticsFast");
    s.store_class_end();
  }
}

getStoryAvailableReactions::getStoryAvailableReactions()
  : row_size_()
{}

getStoryAvailableReactions::getStoryAvailableReactions(int32 row_size_)
  : row_size_(row_size_)
{}

const std::int32_t getStoryAvailableReactions::ID;

void getStoryAvailableReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStoryAvailableReactions");
    s.store_field("row_size", row_size_);
    s.store_class_end();
  }
}

getStoryStatistics::getStoryStatistics()
  : chat_id_()
  , story_id_()
  , is_dark_()
{}

getStoryStatistics::getStoryStatistics(int53 chat_id_, int32 story_id_, bool is_dark_)
  : chat_id_(chat_id_)
  , story_id_(story_id_)
  , is_dark_(is_dark_)
{}

const std::int32_t getStoryStatistics::ID;

void getStoryStatistics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStoryStatistics");
    s.store_field("chat_id", chat_id_);
    s.store_field("story_id", story_id_);
    s.store_field("is_dark", is_dark_);
    s.store_class_end();
  }
}

getSupergroup::getSupergroup()
  : supergroup_id_()
{}

getSupergroup::getSupergroup(int53 supergroup_id_)
  : supergroup_id_(supergroup_id_)
{}

const std::int32_t getSupergroup::ID;

void getSupergroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getSupergroup");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_class_end();
  }
}

getThemeParametersJsonString::getThemeParametersJsonString()
  : theme_()
{}

getThemeParametersJsonString::getThemeParametersJsonString(object_ptr<themeParameters> &&theme_)
  : theme_(std::move(theme_))
{}

const std::int32_t getThemeParametersJsonString::ID;

void getThemeParametersJsonString::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getThemeParametersJsonString");
    s.store_object_field("theme", static_cast<const BaseObject *>(theme_.get()));
    s.store_class_end();
  }
}

getTimeZones::getTimeZones() {
}

const std::int32_t getTimeZones::ID;

void getTimeZones::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getTimeZones");
    s.store_class_end();
  }
}

getVideoMessageAdvertisements::getVideoMessageAdvertisements()
  : chat_id_()
  , message_id_()
{}

getVideoMessageAdvertisements::getVideoMessageAdvertisements(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t getVideoMessageAdvertisements::ID;

void getVideoMessageAdvertisements::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getVideoMessageAdvertisements");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

joinChat::joinChat()
  : chat_id_()
{}

joinChat::joinChat(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t joinChat::ID;

void joinChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "joinChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

processPushNotification::processPushNotification()
  : payload_()
{}

processPushNotification::processPushNotification(string const &payload_)
  : payload_(payload_)
{}

const std::int32_t processPushNotification::ID;

void processPushNotification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "processPushNotification");
    s.store_field("payload", payload_);
    s.store_class_end();
  }
}

readAllForumTopicMentions::readAllForumTopicMentions()
  : chat_id_()
  , forum_topic_id_()
{}

readAllForumTopicMentions::readAllForumTopicMentions(int53 chat_id_, int32 forum_topic_id_)
  : chat_id_(chat_id_)
  , forum_topic_id_(forum_topic_id_)
{}

const std::int32_t readAllForumTopicMentions::ID;

void readAllForumTopicMentions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "readAllForumTopicMentions");
    s.store_field("chat_id", chat_id_);
    s.store_field("forum_topic_id", forum_topic_id_);
    s.store_class_end();
  }
}

readChatList::readChatList()
  : chat_list_()
{}

readChatList::readChatList(object_ptr<ChatList> &&chat_list_)
  : chat_list_(std::move(chat_list_))
{}

const std::int32_t readChatList::ID;

void readChatList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "readChatList");
    s.store_object_field("chat_list", static_cast<const BaseObject *>(chat_list_.get()));
    s.store_class_end();
  }
}

removeMessageSenderBotVerification::removeMessageSenderBotVerification()
  : bot_user_id_()
  , verified_id_()
{}

removeMessageSenderBotVerification::removeMessageSenderBotVerification(int53 bot_user_id_, object_ptr<MessageSender> &&verified_id_)
  : bot_user_id_(bot_user_id_)
  , verified_id_(std::move(verified_id_))
{}

const std::int32_t removeMessageSenderBotVerification::ID;

void removeMessageSenderBotVerification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeMessageSenderBotVerification");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_object_field("verified_id", static_cast<const BaseObject *>(verified_id_.get()));
    s.store_class_end();
  }
}

removeRecentlyFoundChat::removeRecentlyFoundChat()
  : chat_id_()
{}

removeRecentlyFoundChat::removeRecentlyFoundChat(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t removeRecentlyFoundChat::ID;

void removeRecentlyFoundChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeRecentlyFoundChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

reorderChatFolders::reorderChatFolders()
  : chat_folder_ids_()
  , main_chat_list_position_()
{}

reorderChatFolders::reorderChatFolders(array<int32> &&chat_folder_ids_, int32 main_chat_list_position_)
  : chat_folder_ids_(std::move(chat_folder_ids_))
  , main_chat_list_position_(main_chat_list_position_)
{}

const std::int32_t reorderChatFolders::ID;

void reorderChatFolders::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reorderChatFolders");
    { s.store_vector_begin("chat_folder_ids", chat_folder_ids_.size()); for (const auto &_value : chat_folder_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("main_chat_list_position", main_chat_list_position_);
    s.store_class_end();
  }
}

reorderGiftCollectionGifts::reorderGiftCollectionGifts()
  : owner_id_()
  , collection_id_()
  , received_gift_ids_()
{}

reorderGiftCollectionGifts::reorderGiftCollectionGifts(object_ptr<MessageSender> &&owner_id_, int32 collection_id_, array<string> &&received_gift_ids_)
  : owner_id_(std::move(owner_id_))
  , collection_id_(collection_id_)
  , received_gift_ids_(std::move(received_gift_ids_))
{}

const std::int32_t reorderGiftCollectionGifts::ID;

void reorderGiftCollectionGifts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reorderGiftCollectionGifts");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_field("collection_id", collection_id_);
    { s.store_vector_begin("received_gift_ids", received_gift_ids_.size()); for (const auto &_value : received_gift_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

reportChat::reportChat()
  : chat_id_()
  , option_id_()
  , message_ids_()
  , text_()
{}

reportChat::reportChat(int53 chat_id_, bytes const &option_id_, array<int53> &&message_ids_, string const &text_)
  : chat_id_(chat_id_)
  , option_id_(std::move(option_id_))
  , message_ids_(std::move(message_ids_))
  , text_(text_)
{}

const std::int32_t reportChat::ID;

void reportChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportChat");
    s.store_field("chat_id", chat_id_);
    s.store_bytes_field("option_id", option_id_);
    { s.store_vector_begin("message_ids", message_ids_.size()); for (const auto &_value : message_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("text", text_);
    s.store_class_end();
  }
}

reportChatSponsoredMessage::reportChatSponsoredMessage()
  : chat_id_()
  , message_id_()
  , option_id_()
{}

reportChatSponsoredMessage::reportChatSponsoredMessage(int53 chat_id_, int53 message_id_, bytes const &option_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , option_id_(std::move(option_id_))
{}

const std::int32_t reportChatSponsoredMessage::ID;

void reportChatSponsoredMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportChatSponsoredMessage");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_bytes_field("option_id", option_id_);
    s.store_class_end();
  }
}

resetPassword::resetPassword() {
}

const std::int32_t resetPassword::ID;

void resetPassword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "resetPassword");
    s.store_class_end();
  }
}

searchChatMessages::searchChatMessages()
  : chat_id_()
  , topic_id_()
  , query_()
  , sender_id_()
  , from_message_id_()
  , offset_()
  , limit_()
  , filter_()
{}

searchChatMessages::searchChatMessages(int53 chat_id_, object_ptr<MessageTopic> &&topic_id_, string const &query_, object_ptr<MessageSender> &&sender_id_, int53 from_message_id_, int32 offset_, int32 limit_, object_ptr<SearchMessagesFilter> &&filter_)
  : chat_id_(chat_id_)
  , topic_id_(std::move(topic_id_))
  , query_(query_)
  , sender_id_(std::move(sender_id_))
  , from_message_id_(from_message_id_)
  , offset_(offset_)
  , limit_(limit_)
  , filter_(std::move(filter_))
{}

const std::int32_t searchChatMessages::ID;

void searchChatMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchChatMessages");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("topic_id", static_cast<const BaseObject *>(topic_id_.get()));
    s.store_field("query", query_);
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_field("from_message_id", from_message_id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get()));
    s.store_class_end();
  }
}

searchChatRecentLocationMessages::searchChatRecentLocationMessages()
  : chat_id_()
  , limit_()
{}

searchChatRecentLocationMessages::searchChatRecentLocationMessages(int53 chat_id_, int32 limit_)
  : chat_id_(chat_id_)
  , limit_(limit_)
{}

const std::int32_t searchChatRecentLocationMessages::ID;

void searchChatRecentLocationMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchChatRecentLocationMessages");
    s.store_field("chat_id", chat_id_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

searchInstalledStickerSets::searchInstalledStickerSets()
  : sticker_type_()
  , query_()
  , limit_()
{}

searchInstalledStickerSets::searchInstalledStickerSets(object_ptr<StickerType> &&sticker_type_, string const &query_, int32 limit_)
  : sticker_type_(std::move(sticker_type_))
  , query_(query_)
  , limit_(limit_)
{}

const std::int32_t searchInstalledStickerSets::ID;

void searchInstalledStickerSets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchInstalledStickerSets");
    s.store_object_field("sticker_type", static_cast<const BaseObject *>(sticker_type_.get()));
    s.store_field("query", query_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

searchMessages::searchMessages()
  : chat_list_()
  , query_()
  , offset_()
  , limit_()
  , filter_()
  , chat_type_filter_()
  , min_date_()
  , max_date_()
{}

searchMessages::searchMessages(object_ptr<ChatList> &&chat_list_, string const &query_, string const &offset_, int32 limit_, object_ptr<SearchMessagesFilter> &&filter_, object_ptr<SearchMessagesChatTypeFilter> &&chat_type_filter_, int32 min_date_, int32 max_date_)
  : chat_list_(std::move(chat_list_))
  , query_(query_)
  , offset_(offset_)
  , limit_(limit_)
  , filter_(std::move(filter_))
  , chat_type_filter_(std::move(chat_type_filter_))
  , min_date_(min_date_)
  , max_date_(max_date_)
{}

const std::int32_t searchMessages::ID;

void searchMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessages");
    s.store_object_field("chat_list", static_cast<const BaseObject *>(chat_list_.get()));
    s.store_field("query", query_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get()));
    s.store_object_field("chat_type_filter", static_cast<const BaseObject *>(chat_type_filter_.get()));
    s.store_field("min_date", min_date_);
    s.store_field("max_date", max_date_);
    s.store_class_end();
  }
}

searchPublicStoriesByVenue::searchPublicStoriesByVenue()
  : venue_provider_()
  , venue_id_()
  , offset_()
  , limit_()
{}

searchPublicStoriesByVenue::searchPublicStoriesByVenue(string const &venue_provider_, string const &venue_id_, string const &offset_, int32 limit_)
  : venue_provider_(venue_provider_)
  , venue_id_(venue_id_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t searchPublicStoriesByVenue::ID;

void searchPublicStoriesByVenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchPublicStoriesByVenue");
    s.store_field("venue_provider", venue_provider_);
    s.store_field("venue_id", venue_id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

searchQuote::searchQuote()
  : text_()
  , quote_()
  , quote_position_()
{}

searchQuote::searchQuote(object_ptr<formattedText> &&text_, object_ptr<formattedText> &&quote_, int32 quote_position_)
  : text_(std::move(text_))
  , quote_(std::move(quote_))
  , quote_position_(quote_position_)
{}

const std::int32_t searchQuote::ID;

void searchQuote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchQuote");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_object_field("quote", static_cast<const BaseObject *>(quote_.get()));
    s.store_field("quote_position", quote_position_);
    s.store_class_end();
  }
}

sendCustomRequest::sendCustomRequest()
  : method_()
  , parameters_()
{}

sendCustomRequest::sendCustomRequest(string const &method_, string const &parameters_)
  : method_(method_)
  , parameters_(parameters_)
{}

const std::int32_t sendCustomRequest::ID;

void sendCustomRequest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendCustomRequest");
    s.store_field("method", method_);
    s.store_field("parameters", parameters_);
    s.store_class_end();
  }
}

sendPhoneNumberCode::sendPhoneNumberCode()
  : phone_number_()
  , settings_()
  , type_()
{}

sendPhoneNumberCode::sendPhoneNumberCode(string const &phone_number_, object_ptr<phoneNumberAuthenticationSettings> &&settings_, object_ptr<PhoneNumberCodeType> &&type_)
  : phone_number_(phone_number_)
  , settings_(std::move(settings_))
  , type_(std::move(type_))
{}

const std::int32_t sendPhoneNumberCode::ID;

void sendPhoneNumberCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendPhoneNumberCode");
    s.store_field("phone_number", phone_number_);
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

setApplicationVerificationToken::setApplicationVerificationToken()
  : verification_id_()
  , token_()
{}

setApplicationVerificationToken::setApplicationVerificationToken(int53 verification_id_, string const &token_)
  : verification_id_(verification_id_)
  , token_(token_)
{}

const std::int32_t setApplicationVerificationToken::ID;

void setApplicationVerificationToken::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setApplicationVerificationToken");
    s.store_field("verification_id", verification_id_);
    s.store_field("token", token_);
    s.store_class_end();
  }
}

setAuthenticationPremiumPurchaseTransaction::setAuthenticationPremiumPurchaseTransaction()
  : transaction_()
  , is_restore_()
  , currency_()
  , amount_()
{}

setAuthenticationPremiumPurchaseTransaction::setAuthenticationPremiumPurchaseTransaction(object_ptr<StoreTransaction> &&transaction_, bool is_restore_, string const &currency_, int53 amount_)
  : transaction_(std::move(transaction_))
  , is_restore_(is_restore_)
  , currency_(currency_)
  , amount_(amount_)
{}

const std::int32_t setAuthenticationPremiumPurchaseTransaction::ID;

void setAuthenticationPremiumPurchaseTransaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setAuthenticationPremiumPurchaseTransaction");
    s.store_object_field("transaction", static_cast<const BaseObject *>(transaction_.get()));
    s.store_field("is_restore", is_restore_);
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_class_end();
  }
}

setAutoDownloadSettings::setAutoDownloadSettings()
  : settings_()
  , type_()
{}

setAutoDownloadSettings::setAutoDownloadSettings(object_ptr<autoDownloadSettings> &&settings_, object_ptr<NetworkType> &&type_)
  : settings_(std::move(settings_))
  , type_(std::move(type_))
{}

const std::int32_t setAutoDownloadSettings::ID;

void setAutoDownloadSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setAutoDownloadSettings");
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

setBusinessAccountName::setBusinessAccountName()
  : business_connection_id_()
  , first_name_()
  , last_name_()
{}

setBusinessAccountName::setBusinessAccountName(string const &business_connection_id_, string const &first_name_, string const &last_name_)
  : business_connection_id_(business_connection_id_)
  , first_name_(first_name_)
  , last_name_(last_name_)
{}

const std::int32_t setBusinessAccountName::ID;

void setBusinessAccountName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setBusinessAccountName");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_field("first_name", first_name_);
    s.store_field("last_name", last_name_);
    s.store_class_end();
  }
}

setBusinessAwayMessageSettings::setBusinessAwayMessageSettings()
  : away_message_settings_()
{}

setBusinessAwayMessageSettings::setBusinessAwayMessageSettings(object_ptr<businessAwayMessageSettings> &&away_message_settings_)
  : away_message_settings_(std::move(away_message_settings_))
{}

const std::int32_t setBusinessAwayMessageSettings::ID;

void setBusinessAwayMessageSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setBusinessAwayMessageSettings");
    s.store_object_field("away_message_settings", static_cast<const BaseObject *>(away_message_settings_.get()));
    s.store_class_end();
  }
}

setBusinessConnectedBot::setBusinessConnectedBot()
  : bot_()
{}

setBusinessConnectedBot::setBusinessConnectedBot(object_ptr<businessConnectedBot> &&bot_)
  : bot_(std::move(bot_))
{}

const std::int32_t setBusinessConnectedBot::ID;

void setBusinessConnectedBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setBusinessConnectedBot");
    s.store_object_field("bot", static_cast<const BaseObject *>(bot_.get()));
    s.store_class_end();
  }
}

setChatDescription::setChatDescription()
  : chat_id_()
  , description_()
{}

setChatDescription::setChatDescription(int53 chat_id_, string const &description_)
  : chat_id_(chat_id_)
  , description_(description_)
{}

const std::int32_t setChatDescription::ID;

void setChatDescription::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatDescription");
    s.store_field("chat_id", chat_id_);
    s.store_field("description", description_);
    s.store_class_end();
  }
}

setChatDraftMessage::setChatDraftMessage()
  : chat_id_()
  , topic_id_()
  , draft_message_()
{}

setChatDraftMessage::setChatDraftMessage(int53 chat_id_, object_ptr<MessageTopic> &&topic_id_, object_ptr<draftMessage> &&draft_message_)
  : chat_id_(chat_id_)
  , topic_id_(std::move(topic_id_))
  , draft_message_(std::move(draft_message_))
{}

const std::int32_t setChatDraftMessage::ID;

void setChatDraftMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatDraftMessage");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("topic_id", static_cast<const BaseObject *>(topic_id_.get()));
    s.store_object_field("draft_message", static_cast<const BaseObject *>(draft_message_.get()));
    s.store_class_end();
  }
}

setChatMessageAutoDeleteTime::setChatMessageAutoDeleteTime()
  : chat_id_()
  , message_auto_delete_time_()
{}

setChatMessageAutoDeleteTime::setChatMessageAutoDeleteTime(int53 chat_id_, int32 message_auto_delete_time_)
  : chat_id_(chat_id_)
  , message_auto_delete_time_(message_auto_delete_time_)
{}

const std::int32_t setChatMessageAutoDeleteTime::ID;

void setChatMessageAutoDeleteTime::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatMessageAutoDeleteTime");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_auto_delete_time", message_auto_delete_time_);
    s.store_class_end();
  }
}

setGroupCallParticipantIsSpeaking::setGroupCallParticipantIsSpeaking()
  : group_call_id_()
  , audio_source_()
  , is_speaking_()
{}

setGroupCallParticipantIsSpeaking::setGroupCallParticipantIsSpeaking(int32 group_call_id_, int32 audio_source_, bool is_speaking_)
  : group_call_id_(group_call_id_)
  , audio_source_(audio_source_)
  , is_speaking_(is_speaking_)
{}

const std::int32_t setGroupCallParticipantIsSpeaking::ID;

void setGroupCallParticipantIsSpeaking::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setGroupCallParticipantIsSpeaking");
    s.store_field("group_call_id", group_call_id_);
    s.store_field("audio_source", audio_source_);
    s.store_field("is_speaking", is_speaking_);
    s.store_class_end();
  }
}

setPaidMessageReactionType::setPaidMessageReactionType()
  : chat_id_()
  , message_id_()
  , type_()
{}

setPaidMessageReactionType::setPaidMessageReactionType(int53 chat_id_, int53 message_id_, object_ptr<PaidReactionType> &&type_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , type_(std::move(type_))
{}

const std::int32_t setPaidMessageReactionType::ID;

void setPaidMessageReactionType::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setPaidMessageReactionType");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

setSupergroupUsername::setSupergroupUsername()
  : supergroup_id_()
  , username_()
{}

setSupergroupUsername::setSupergroupUsername(int53 supergroup_id_, string const &username_)
  : supergroup_id_(supergroup_id_)
  , username_(username_)
{}

const std::int32_t setSupergroupUsername::ID;

void setSupergroupUsername::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setSupergroupUsername");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_field("username", username_);
    s.store_class_end();
  }
}

setUserPrivacySettingRules::setUserPrivacySettingRules()
  : setting_()
  , rules_()
{}

setUserPrivacySettingRules::setUserPrivacySettingRules(object_ptr<UserPrivacySetting> &&setting_, object_ptr<userPrivacySettingRules> &&rules_)
  : setting_(std::move(setting_))
  , rules_(std::move(rules_))
{}

const std::int32_t setUserPrivacySettingRules::ID;

void setUserPrivacySettingRules::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setUserPrivacySettingRules");
    s.store_object_field("setting", static_cast<const BaseObject *>(setting_.get()));
    s.store_object_field("rules", static_cast<const BaseObject *>(rules_.get()));
    s.store_class_end();
  }
}

startScheduledVideoChat::startScheduledVideoChat()
  : group_call_id_()
{}

startScheduledVideoChat::startScheduledVideoChat(int32 group_call_id_)
  : group_call_id_(group_call_id_)
{}

const std::int32_t startScheduledVideoChat::ID;

void startScheduledVideoChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "startScheduledVideoChat");
    s.store_field("group_call_id", group_call_id_);
    s.store_class_end();
  }
}

toggleBotCanManageEmojiStatus::toggleBotCanManageEmojiStatus()
  : bot_user_id_()
  , can_manage_emoji_status_()
{}

toggleBotCanManageEmojiStatus::toggleBotCanManageEmojiStatus(int53 bot_user_id_, bool can_manage_emoji_status_)
  : bot_user_id_(bot_user_id_)
  , can_manage_emoji_status_(can_manage_emoji_status_)
{}

const std::int32_t toggleBotCanManageEmojiStatus::ID;

void toggleBotCanManageEmojiStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleBotCanManageEmojiStatus");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("can_manage_emoji_status", can_manage_emoji_status_);
    s.store_class_end();
  }
}

toggleChatDefaultDisableNotification::toggleChatDefaultDisableNotification()
  : chat_id_()
  , default_disable_notification_()
{}

toggleChatDefaultDisableNotification::toggleChatDefaultDisableNotification(int53 chat_id_, bool default_disable_notification_)
  : chat_id_(chat_id_)
  , default_disable_notification_(default_disable_notification_)
{}

const std::int32_t toggleChatDefaultDisableNotification::ID;

void toggleChatDefaultDisableNotification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleChatDefaultDisableNotification");
    s.store_field("chat_id", chat_id_);
    s.store_field("default_disable_notification", default_disable_notification_);
    s.store_class_end();
  }
}

toggleSavedMessagesTopicIsPinned::toggleSavedMessagesTopicIsPinned()
  : saved_messages_topic_id_()
  , is_pinned_()
{}

toggleSavedMessagesTopicIsPinned::toggleSavedMessagesTopicIsPinned(int53 saved_messages_topic_id_, bool is_pinned_)
  : saved_messages_topic_id_(saved_messages_topic_id_)
  , is_pinned_(is_pinned_)
{}

const std::int32_t toggleSavedMessagesTopicIsPinned::ID;

void toggleSavedMessagesTopicIsPinned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleSavedMessagesTopicIsPinned");
    s.store_field("saved_messages_topic_id", saved_messages_topic_id_);
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

toggleStoryIsPostedToChatPage::toggleStoryIsPostedToChatPage()
  : story_poster_chat_id_()
  , story_id_()
  , is_posted_to_chat_page_()
{}

toggleStoryIsPostedToChatPage::toggleStoryIsPostedToChatPage(int53 story_poster_chat_id_, int32 story_id_, bool is_posted_to_chat_page_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , story_id_(story_id_)
  , is_posted_to_chat_page_(is_posted_to_chat_page_)
{}

const std::int32_t toggleStoryIsPostedToChatPage::ID;

void toggleStoryIsPostedToChatPage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleStoryIsPostedToChatPage");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("story_id", story_id_);
    s.store_field("is_posted_to_chat_page", is_posted_to_chat_page_);
    s.store_class_end();
  }
}

toggleSupergroupHasAggressiveAntiSpamEnabled::toggleSupergroupHasAggressiveAntiSpamEnabled()
  : supergroup_id_()
  , has_aggressive_anti_spam_enabled_()
{}

toggleSupergroupHasAggressiveAntiSpamEnabled::toggleSupergroupHasAggressiveAntiSpamEnabled(int53 supergroup_id_, bool has_aggressive_anti_spam_enabled_)
  : supergroup_id_(supergroup_id_)
  , has_aggressive_anti_spam_enabled_(has_aggressive_anti_spam_enabled_)
{}

const std::int32_t toggleSupergroupHasAggressiveAntiSpamEnabled::ID;

void toggleSupergroupHasAggressiveAntiSpamEnabled::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleSupergroupHasAggressiveAntiSpamEnabled");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_field("has_aggressive_anti_spam_enabled", has_aggressive_anti_spam_enabled_);
    s.store_class_end();
  }
}

toggleSupergroupIsBroadcastGroup::toggleSupergroupIsBroadcastGroup()
  : supergroup_id_()
{}

toggleSupergroupIsBroadcastGroup::toggleSupergroupIsBroadcastGroup(int53 supergroup_id_)
  : supergroup_id_(supergroup_id_)
{}

const std::int32_t toggleSupergroupIsBroadcastGroup::ID;

void toggleSupergroupIsBroadcastGroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleSupergroupIsBroadcastGroup");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_class_end();
  }
}

viewPremiumFeature::viewPremiumFeature()
  : feature_()
{}

viewPremiumFeature::viewPremiumFeature(object_ptr<PremiumFeature> &&feature_)
  : feature_(std::move(feature_))
{}

const std::int32_t viewPremiumFeature::ID;

void viewPremiumFeature::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "viewPremiumFeature");
    s.store_object_field("feature", static_cast<const BaseObject *>(feature_.get()));
    s.store_class_end();
  }
}
}  // namespace td_api
}  // namespace td
