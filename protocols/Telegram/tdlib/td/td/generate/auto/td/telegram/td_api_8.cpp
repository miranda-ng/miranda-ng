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


affiliateProgramSortOrderProfitability::affiliateProgramSortOrderProfitability() {
}

const std::int32_t affiliateProgramSortOrderProfitability::ID;

void affiliateProgramSortOrderProfitability::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "affiliateProgramSortOrderProfitability");
    s.store_class_end();
  }
}

affiliateProgramSortOrderCreationDate::affiliateProgramSortOrderCreationDate() {
}

const std::int32_t affiliateProgramSortOrderCreationDate::ID;

void affiliateProgramSortOrderCreationDate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "affiliateProgramSortOrderCreationDate");
    s.store_class_end();
  }
}

affiliateProgramSortOrderRevenue::affiliateProgramSortOrderRevenue() {
}

const std::int32_t affiliateProgramSortOrderRevenue::ID;

void affiliateProgramSortOrderRevenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "affiliateProgramSortOrderRevenue");
    s.store_class_end();
  }
}

bankCardActionOpenUrl::bankCardActionOpenUrl()
  : text_()
  , url_()
{}

bankCardActionOpenUrl::bankCardActionOpenUrl(string const &text_, string const &url_)
  : text_(text_)
  , url_(url_)
{}

const std::int32_t bankCardActionOpenUrl::ID;

void bankCardActionOpenUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bankCardActionOpenUrl");
    s.store_field("text", text_);
    s.store_field("url", url_);
    s.store_class_end();
  }
}

birthdate::birthdate()
  : day_()
  , month_()
  , year_()
{}

birthdate::birthdate(int32 day_, int32 month_, int32 year_)
  : day_(day_)
  , month_(month_)
  , year_(year_)
{}

const std::int32_t birthdate::ID;

void birthdate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "birthdate");
    s.store_field("day", day_);
    s.store_field("month", month_);
    s.store_field("year", year_);
    s.store_class_end();
  }
}

blockListMain::blockListMain() {
}

const std::int32_t blockListMain::ID;

void blockListMain::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "blockListMain");
    s.store_class_end();
  }
}

blockListStories::blockListStories() {
}

const std::int32_t blockListStories::ID;

void blockListStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "blockListStories");
    s.store_class_end();
  }
}

botWriteAccessAllowReasonConnectedWebsite::botWriteAccessAllowReasonConnectedWebsite()
  : domain_name_()
{}

botWriteAccessAllowReasonConnectedWebsite::botWriteAccessAllowReasonConnectedWebsite(string const &domain_name_)
  : domain_name_(domain_name_)
{}

const std::int32_t botWriteAccessAllowReasonConnectedWebsite::ID;

void botWriteAccessAllowReasonConnectedWebsite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botWriteAccessAllowReasonConnectedWebsite");
    s.store_field("domain_name", domain_name_);
    s.store_class_end();
  }
}

botWriteAccessAllowReasonAddedToAttachmentMenu::botWriteAccessAllowReasonAddedToAttachmentMenu() {
}

const std::int32_t botWriteAccessAllowReasonAddedToAttachmentMenu::ID;

void botWriteAccessAllowReasonAddedToAttachmentMenu::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botWriteAccessAllowReasonAddedToAttachmentMenu");
    s.store_class_end();
  }
}

botWriteAccessAllowReasonLaunchedWebApp::botWriteAccessAllowReasonLaunchedWebApp()
  : web_app_()
{}

botWriteAccessAllowReasonLaunchedWebApp::botWriteAccessAllowReasonLaunchedWebApp(object_ptr<webApp> &&web_app_)
  : web_app_(std::move(web_app_))
{}

const std::int32_t botWriteAccessAllowReasonLaunchedWebApp::ID;

void botWriteAccessAllowReasonLaunchedWebApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botWriteAccessAllowReasonLaunchedWebApp");
    s.store_object_field("web_app", static_cast<const BaseObject *>(web_app_.get()));
    s.store_class_end();
  }
}

botWriteAccessAllowReasonAcceptedRequest::botWriteAccessAllowReasonAcceptedRequest() {
}

const std::int32_t botWriteAccessAllowReasonAcceptedRequest::ID;

void botWriteAccessAllowReasonAcceptedRequest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botWriteAccessAllowReasonAcceptedRequest");
    s.store_class_end();
  }
}

chatActionBarReportSpam::chatActionBarReportSpam()
  : can_unarchive_()
{}

chatActionBarReportSpam::chatActionBarReportSpam(bool can_unarchive_)
  : can_unarchive_(can_unarchive_)
{}

const std::int32_t chatActionBarReportSpam::ID;

void chatActionBarReportSpam::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionBarReportSpam");
    s.store_field("can_unarchive", can_unarchive_);
    s.store_class_end();
  }
}

chatActionBarInviteMembers::chatActionBarInviteMembers() {
}

const std::int32_t chatActionBarInviteMembers::ID;

void chatActionBarInviteMembers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionBarInviteMembers");
    s.store_class_end();
  }
}

chatActionBarReportAddBlock::chatActionBarReportAddBlock()
  : can_unarchive_()
  , account_info_()
{}

chatActionBarReportAddBlock::chatActionBarReportAddBlock(bool can_unarchive_, object_ptr<accountInfo> &&account_info_)
  : can_unarchive_(can_unarchive_)
  , account_info_(std::move(account_info_))
{}

const std::int32_t chatActionBarReportAddBlock::ID;

void chatActionBarReportAddBlock::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionBarReportAddBlock");
    s.store_field("can_unarchive", can_unarchive_);
    s.store_object_field("account_info", static_cast<const BaseObject *>(account_info_.get()));
    s.store_class_end();
  }
}

chatActionBarAddContact::chatActionBarAddContact() {
}

const std::int32_t chatActionBarAddContact::ID;

void chatActionBarAddContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionBarAddContact");
    s.store_class_end();
  }
}

chatActionBarSharePhoneNumber::chatActionBarSharePhoneNumber() {
}

const std::int32_t chatActionBarSharePhoneNumber::ID;

void chatActionBarSharePhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionBarSharePhoneNumber");
    s.store_class_end();
  }
}

chatActionBarJoinRequest::chatActionBarJoinRequest()
  : title_()
  , is_channel_()
  , request_date_()
{}

chatActionBarJoinRequest::chatActionBarJoinRequest(string const &title_, bool is_channel_, int32 request_date_)
  : title_(title_)
  , is_channel_(is_channel_)
  , request_date_(request_date_)
{}

const std::int32_t chatActionBarJoinRequest::ID;

void chatActionBarJoinRequest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionBarJoinRequest");
    s.store_field("title", title_);
    s.store_field("is_channel", is_channel_);
    s.store_field("request_date", request_date_);
    s.store_class_end();
  }
}

chatBoost::chatBoost()
  : id_()
  , count_()
  , source_()
  , start_date_()
  , expiration_date_()
{}

chatBoost::chatBoost(string const &id_, int32 count_, object_ptr<ChatBoostSource> &&source_, int32 start_date_, int32 expiration_date_)
  : id_(id_)
  , count_(count_)
  , source_(std::move(source_))
  , start_date_(start_date_)
  , expiration_date_(expiration_date_)
{}

const std::int32_t chatBoost::ID;

void chatBoost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatBoost");
    s.store_field("id", id_);
    s.store_field("count", count_);
    s.store_object_field("source", static_cast<const BaseObject *>(source_.get()));
    s.store_field("start_date", start_date_);
    s.store_field("expiration_date", expiration_date_);
    s.store_class_end();
  }
}

chatEventLogFilters::chatEventLogFilters()
  : message_edits_()
  , message_deletions_()
  , message_pins_()
  , member_joins_()
  , member_leaves_()
  , member_invites_()
  , member_promotions_()
  , member_restrictions_()
  , info_changes_()
  , setting_changes_()
  , invite_link_changes_()
  , video_chat_changes_()
  , forum_changes_()
  , subscription_extensions_()
{}

chatEventLogFilters::chatEventLogFilters(bool message_edits_, bool message_deletions_, bool message_pins_, bool member_joins_, bool member_leaves_, bool member_invites_, bool member_promotions_, bool member_restrictions_, bool info_changes_, bool setting_changes_, bool invite_link_changes_, bool video_chat_changes_, bool forum_changes_, bool subscription_extensions_)
  : message_edits_(message_edits_)
  , message_deletions_(message_deletions_)
  , message_pins_(message_pins_)
  , member_joins_(member_joins_)
  , member_leaves_(member_leaves_)
  , member_invites_(member_invites_)
  , member_promotions_(member_promotions_)
  , member_restrictions_(member_restrictions_)
  , info_changes_(info_changes_)
  , setting_changes_(setting_changes_)
  , invite_link_changes_(invite_link_changes_)
  , video_chat_changes_(video_chat_changes_)
  , forum_changes_(forum_changes_)
  , subscription_extensions_(subscription_extensions_)
{}

const std::int32_t chatEventLogFilters::ID;

void chatEventLogFilters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventLogFilters");
    s.store_field("message_edits", message_edits_);
    s.store_field("message_deletions", message_deletions_);
    s.store_field("message_pins", message_pins_);
    s.store_field("member_joins", member_joins_);
    s.store_field("member_leaves", member_leaves_);
    s.store_field("member_invites", member_invites_);
    s.store_field("member_promotions", member_promotions_);
    s.store_field("member_restrictions", member_restrictions_);
    s.store_field("info_changes", info_changes_);
    s.store_field("setting_changes", setting_changes_);
    s.store_field("invite_link_changes", invite_link_changes_);
    s.store_field("video_chat_changes", video_chat_changes_);
    s.store_field("forum_changes", forum_changes_);
    s.store_field("subscription_extensions", subscription_extensions_);
    s.store_class_end();
  }
}

chatInviteLink::chatInviteLink()
  : invite_link_()
  , name_()
  , creator_user_id_()
  , date_()
  , edit_date_()
  , expiration_date_()
  , subscription_pricing_()
  , member_limit_()
  , member_count_()
  , expired_member_count_()
  , pending_join_request_count_()
  , creates_join_request_()
  , is_primary_()
  , is_revoked_()
{}

chatInviteLink::chatInviteLink(string const &invite_link_, string const &name_, int53 creator_user_id_, int32 date_, int32 edit_date_, int32 expiration_date_, object_ptr<starSubscriptionPricing> &&subscription_pricing_, int32 member_limit_, int32 member_count_, int32 expired_member_count_, int32 pending_join_request_count_, bool creates_join_request_, bool is_primary_, bool is_revoked_)
  : invite_link_(invite_link_)
  , name_(name_)
  , creator_user_id_(creator_user_id_)
  , date_(date_)
  , edit_date_(edit_date_)
  , expiration_date_(expiration_date_)
  , subscription_pricing_(std::move(subscription_pricing_))
  , member_limit_(member_limit_)
  , member_count_(member_count_)
  , expired_member_count_(expired_member_count_)
  , pending_join_request_count_(pending_join_request_count_)
  , creates_join_request_(creates_join_request_)
  , is_primary_(is_primary_)
  , is_revoked_(is_revoked_)
{}

const std::int32_t chatInviteLink::ID;

void chatInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatInviteLink");
    s.store_field("invite_link", invite_link_);
    s.store_field("name", name_);
    s.store_field("creator_user_id", creator_user_id_);
    s.store_field("date", date_);
    s.store_field("edit_date", edit_date_);
    s.store_field("expiration_date", expiration_date_);
    s.store_object_field("subscription_pricing", static_cast<const BaseObject *>(subscription_pricing_.get()));
    s.store_field("member_limit", member_limit_);
    s.store_field("member_count", member_count_);
    s.store_field("expired_member_count", expired_member_count_);
    s.store_field("pending_join_request_count", pending_join_request_count_);
    s.store_field("creates_join_request", creates_join_request_);
    s.store_field("is_primary", is_primary_);
    s.store_field("is_revoked", is_revoked_);
    s.store_class_end();
  }
}

chatStatisticsAdministratorActionsInfo::chatStatisticsAdministratorActionsInfo()
  : user_id_()
  , deleted_message_count_()
  , banned_user_count_()
  , restricted_user_count_()
{}

chatStatisticsAdministratorActionsInfo::chatStatisticsAdministratorActionsInfo(int53 user_id_, int32 deleted_message_count_, int32 banned_user_count_, int32 restricted_user_count_)
  : user_id_(user_id_)
  , deleted_message_count_(deleted_message_count_)
  , banned_user_count_(banned_user_count_)
  , restricted_user_count_(restricted_user_count_)
{}

const std::int32_t chatStatisticsAdministratorActionsInfo::ID;

void chatStatisticsAdministratorActionsInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatStatisticsAdministratorActionsInfo");
    s.store_field("user_id", user_id_);
    s.store_field("deleted_message_count", deleted_message_count_);
    s.store_field("banned_user_count", banned_user_count_);
    s.store_field("restricted_user_count", restricted_user_count_);
    s.store_class_end();
  }
}

checklistTask::checklistTask()
  : id_()
  , text_()
  , completed_by_user_id_()
  , completion_date_()
{}

checklistTask::checklistTask(int32 id_, object_ptr<formattedText> &&text_, int53 completed_by_user_id_, int32 completion_date_)
  : id_(id_)
  , text_(std::move(text_))
  , completed_by_user_id_(completed_by_user_id_)
  , completion_date_(completion_date_)
{}

const std::int32_t checklistTask::ID;

void checklistTask::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checklistTask");
    s.store_field("id", id_);
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("completed_by_user_id", completed_by_user_id_);
    s.store_field("completion_date", completion_date_);
    s.store_class_end();
  }
}

collectibleItemTypeUsername::collectibleItemTypeUsername()
  : username_()
{}

collectibleItemTypeUsername::collectibleItemTypeUsername(string const &username_)
  : username_(username_)
{}

const std::int32_t collectibleItemTypeUsername::ID;

void collectibleItemTypeUsername::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "collectibleItemTypeUsername");
    s.store_field("username", username_);
    s.store_class_end();
  }
}

collectibleItemTypePhoneNumber::collectibleItemTypePhoneNumber()
  : phone_number_()
{}

collectibleItemTypePhoneNumber::collectibleItemTypePhoneNumber(string const &phone_number_)
  : phone_number_(phone_number_)
{}

const std::int32_t collectibleItemTypePhoneNumber::ID;

void collectibleItemTypePhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "collectibleItemTypePhoneNumber");
    s.store_field("phone_number", phone_number_);
    s.store_class_end();
  }
}

emailAddressAuthenticationCodeInfo::emailAddressAuthenticationCodeInfo()
  : email_address_pattern_()
  , length_()
{}

emailAddressAuthenticationCodeInfo::emailAddressAuthenticationCodeInfo(string const &email_address_pattern_, int32 length_)
  : email_address_pattern_(email_address_pattern_)
  , length_(length_)
{}

const std::int32_t emailAddressAuthenticationCodeInfo::ID;

void emailAddressAuthenticationCodeInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emailAddressAuthenticationCodeInfo");
    s.store_field("email_address_pattern", email_address_pattern_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

emojis::emojis()
  : emojis_()
{}

emojis::emojis(array<string> &&emojis_)
  : emojis_(std::move(emojis_))
{}

const std::int32_t emojis::ID;

void emojis::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojis");
    { s.store_vector_begin("emojis", emojis_.size()); for (const auto &_value : emojis_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

forumTopicIcon::forumTopicIcon()
  : color_()
  , custom_emoji_id_()
{}

forumTopicIcon::forumTopicIcon(int32 color_, int64 custom_emoji_id_)
  : color_(color_)
  , custom_emoji_id_(custom_emoji_id_)
{}

const std::int32_t forumTopicIcon::ID;

void forumTopicIcon::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "forumTopicIcon");
    s.store_field("color", color_);
    s.store_field("custom_emoji_id", custom_emoji_id_);
    s.store_class_end();
  }
}

foundWebApp::foundWebApp()
  : web_app_()
  , request_write_access_()
  , skip_confirmation_()
{}

foundWebApp::foundWebApp(object_ptr<webApp> &&web_app_, bool request_write_access_, bool skip_confirmation_)
  : web_app_(std::move(web_app_))
  , request_write_access_(request_write_access_)
  , skip_confirmation_(skip_confirmation_)
{}

const std::int32_t foundWebApp::ID;

void foundWebApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "foundWebApp");
    s.store_object_field("web_app", static_cast<const BaseObject *>(web_app_.get()));
    s.store_field("request_write_access", request_write_access_);
    s.store_field("skip_confirmation", skip_confirmation_);
    s.store_class_end();
  }
}

giftChatTheme::giftChatTheme()
  : gift_()
  , light_settings_()
  , dark_settings_()
{}

giftChatTheme::giftChatTheme(object_ptr<upgradedGift> &&gift_, object_ptr<themeSettings> &&light_settings_, object_ptr<themeSettings> &&dark_settings_)
  : gift_(std::move(gift_))
  , light_settings_(std::move(light_settings_))
  , dark_settings_(std::move(dark_settings_))
{}

const std::int32_t giftChatTheme::ID;

void giftChatTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giftChatTheme");
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_object_field("light_settings", static_cast<const BaseObject *>(light_settings_.get()));
    s.store_object_field("dark_settings", static_cast<const BaseObject *>(dark_settings_.get()));
    s.store_class_end();
  }
}

giftCollections::giftCollections()
  : collections_()
{}

giftCollections::giftCollections(array<object_ptr<giftCollection>> &&collections_)
  : collections_(std::move(collections_))
{}

const std::int32_t giftCollections::ID;

void giftCollections::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giftCollections");
    { s.store_vector_begin("collections", collections_.size()); for (const auto &_value : collections_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

groupCallParticipants::groupCallParticipants()
  : total_count_()
  , participant_ids_()
{}

groupCallParticipants::groupCallParticipants(int32 total_count_, array<object_ptr<MessageSender>> &&participant_ids_)
  : total_count_(total_count_)
  , participant_ids_(std::move(participant_ids_))
{}

const std::int32_t groupCallParticipants::ID;

void groupCallParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCallParticipants");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("participant_ids", participant_ids_.size()); for (const auto &_value : participant_ids_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

inlineQueryResultsButtonTypeStartBot::inlineQueryResultsButtonTypeStartBot()
  : parameter_()
{}

inlineQueryResultsButtonTypeStartBot::inlineQueryResultsButtonTypeStartBot(string const &parameter_)
  : parameter_(parameter_)
{}

const std::int32_t inlineQueryResultsButtonTypeStartBot::ID;

void inlineQueryResultsButtonTypeStartBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryResultsButtonTypeStartBot");
    s.store_field("parameter", parameter_);
    s.store_class_end();
  }
}

inlineQueryResultsButtonTypeWebApp::inlineQueryResultsButtonTypeWebApp()
  : url_()
{}

inlineQueryResultsButtonTypeWebApp::inlineQueryResultsButtonTypeWebApp(string const &url_)
  : url_(url_)
{}

const std::int32_t inlineQueryResultsButtonTypeWebApp::ID;

void inlineQueryResultsButtonTypeWebApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryResultsButtonTypeWebApp");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

inputGroupCallLink::inputGroupCallLink()
  : link_()
{}

inputGroupCallLink::inputGroupCallLink(string const &link_)
  : link_(link_)
{}

const std::int32_t inputGroupCallLink::ID;

void inputGroupCallLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputGroupCallLink");
    s.store_field("link", link_);
    s.store_class_end();
  }
}

inputGroupCallMessage::inputGroupCallMessage()
  : chat_id_()
  , message_id_()
{}

inputGroupCallMessage::inputGroupCallMessage(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t inputGroupCallMessage::ID;

void inputGroupCallMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputGroupCallMessage");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

inputSticker::inputSticker()
  : sticker_()
  , format_()
  , emojis_()
  , mask_position_()
  , keywords_()
{}

inputSticker::inputSticker(object_ptr<InputFile> &&sticker_, object_ptr<StickerFormat> &&format_, string const &emojis_, object_ptr<maskPosition> &&mask_position_, array<string> &&keywords_)
  : sticker_(std::move(sticker_))
  , format_(std::move(format_))
  , emojis_(emojis_)
  , mask_position_(std::move(mask_position_))
  , keywords_(std::move(keywords_))
{}

const std::int32_t inputSticker::ID;

void inputSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputSticker");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_object_field("format", static_cast<const BaseObject *>(format_.get()));
    s.store_field("emojis", emojis_);
    s.store_object_field("mask_position", static_cast<const BaseObject *>(mask_position_.get()));
    { s.store_vector_begin("keywords", keywords_.size()); for (const auto &_value : keywords_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

jsonObjectMember::jsonObjectMember()
  : key_()
  , value_()
{}

jsonObjectMember::jsonObjectMember(string const &key_, object_ptr<JsonValue> &&value_)
  : key_(key_)
  , value_(std::move(value_))
{}

const std::int32_t jsonObjectMember::ID;

void jsonObjectMember::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "jsonObjectMember");
    s.store_field("key", key_);
    s.store_object_field("value", static_cast<const BaseObject *>(value_.get()));
    s.store_class_end();
  }
}

jsonValueNull::jsonValueNull() {
}

const std::int32_t jsonValueNull::ID;

void jsonValueNull::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "jsonValueNull");
    s.store_class_end();
  }
}

jsonValueBoolean::jsonValueBoolean()
  : value_()
{}

jsonValueBoolean::jsonValueBoolean(bool value_)
  : value_(value_)
{}

const std::int32_t jsonValueBoolean::ID;

void jsonValueBoolean::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "jsonValueBoolean");
    s.store_field("value", value_);
    s.store_class_end();
  }
}

jsonValueNumber::jsonValueNumber()
  : value_()
{}

jsonValueNumber::jsonValueNumber(double value_)
  : value_(value_)
{}

const std::int32_t jsonValueNumber::ID;

void jsonValueNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "jsonValueNumber");
    s.store_field("value", value_);
    s.store_class_end();
  }
}

jsonValueString::jsonValueString()
  : value_()
{}

jsonValueString::jsonValueString(string const &value_)
  : value_(value_)
{}

const std::int32_t jsonValueString::ID;

void jsonValueString::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "jsonValueString");
    s.store_field("value", value_);
    s.store_class_end();
  }
}

jsonValueArray::jsonValueArray()
  : values_()
{}

jsonValueArray::jsonValueArray(array<object_ptr<JsonValue>> &&values_)
  : values_(std::move(values_))
{}

const std::int32_t jsonValueArray::ID;

void jsonValueArray::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "jsonValueArray");
    { s.store_vector_begin("values", values_.size()); for (const auto &_value : values_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

jsonValueObject::jsonValueObject()
  : members_()
{}

jsonValueObject::jsonValueObject(array<object_ptr<jsonObjectMember>> &&members_)
  : members_(std::move(members_))
{}

const std::int32_t jsonValueObject::ID;

void jsonValueObject::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "jsonValueObject");
    { s.store_vector_begin("members", members_.size()); for (const auto &_value : members_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

languagePackStringValueOrdinary::languagePackStringValueOrdinary()
  : value_()
{}

languagePackStringValueOrdinary::languagePackStringValueOrdinary(string const &value_)
  : value_(value_)
{}

const std::int32_t languagePackStringValueOrdinary::ID;

void languagePackStringValueOrdinary::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "languagePackStringValueOrdinary");
    s.store_field("value", value_);
    s.store_class_end();
  }
}

languagePackStringValuePluralized::languagePackStringValuePluralized()
  : zero_value_()
  , one_value_()
  , two_value_()
  , few_value_()
  , many_value_()
  , other_value_()
{}

languagePackStringValuePluralized::languagePackStringValuePluralized(string const &zero_value_, string const &one_value_, string const &two_value_, string const &few_value_, string const &many_value_, string const &other_value_)
  : zero_value_(zero_value_)
  , one_value_(one_value_)
  , two_value_(two_value_)
  , few_value_(few_value_)
  , many_value_(many_value_)
  , other_value_(other_value_)
{}

const std::int32_t languagePackStringValuePluralized::ID;

void languagePackStringValuePluralized::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "languagePackStringValuePluralized");
    s.store_field("zero_value", zero_value_);
    s.store_field("one_value", one_value_);
    s.store_field("two_value", two_value_);
    s.store_field("few_value", few_value_);
    s.store_field("many_value", many_value_);
    s.store_field("other_value", other_value_);
    s.store_class_end();
  }
}

languagePackStringValueDeleted::languagePackStringValueDeleted() {
}

const std::int32_t languagePackStringValueDeleted::ID;

void languagePackStringValueDeleted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "languagePackStringValueDeleted");
    s.store_class_end();
  }
}

logTags::logTags()
  : tags_()
{}

logTags::logTags(array<string> &&tags_)
  : tags_(std::move(tags_))
{}

const std::int32_t logTags::ID;

void logTags::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "logTags");
    { s.store_vector_begin("tags", tags_.size()); for (const auto &_value : tags_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

message::message()
  : id_()
  , sender_id_()
  , chat_id_()
  , sending_state_()
  , scheduling_state_()
  , is_outgoing_()
  , is_pinned_()
  , is_from_offline_()
  , can_be_saved_()
  , has_timestamped_media_()
  , is_channel_post_()
  , is_paid_star_suggested_post_()
  , is_paid_ton_suggested_post_()
  , contains_unread_mention_()
  , date_()
  , edit_date_()
  , forward_info_()
  , import_info_()
  , interaction_info_()
  , unread_reactions_()
  , fact_check_()
  , suggested_post_info_()
  , reply_to_()
  , topic_id_()
  , self_destruct_type_()
  , self_destruct_in_()
  , auto_delete_in_()
  , via_bot_user_id_()
  , sender_business_bot_user_id_()
  , sender_boost_count_()
  , paid_message_star_count_()
  , author_signature_()
  , media_album_id_()
  , effect_id_()
  , restriction_info_()
  , content_()
  , reply_markup_()
{}

message::message(int53 id_, object_ptr<MessageSender> &&sender_id_, int53 chat_id_, object_ptr<MessageSendingState> &&sending_state_, object_ptr<MessageSchedulingState> &&scheduling_state_, bool is_outgoing_, bool is_pinned_, bool is_from_offline_, bool can_be_saved_, bool has_timestamped_media_, bool is_channel_post_, bool is_paid_star_suggested_post_, bool is_paid_ton_suggested_post_, bool contains_unread_mention_, int32 date_, int32 edit_date_, object_ptr<messageForwardInfo> &&forward_info_, object_ptr<messageImportInfo> &&import_info_, object_ptr<messageInteractionInfo> &&interaction_info_, array<object_ptr<unreadReaction>> &&unread_reactions_, object_ptr<factCheck> &&fact_check_, object_ptr<suggestedPostInfo> &&suggested_post_info_, object_ptr<MessageReplyTo> &&reply_to_, object_ptr<MessageTopic> &&topic_id_, object_ptr<MessageSelfDestructType> &&self_destruct_type_, double self_destruct_in_, double auto_delete_in_, int53 via_bot_user_id_, int53 sender_business_bot_user_id_, int32 sender_boost_count_, int53 paid_message_star_count_, string const &author_signature_, int64 media_album_id_, int64 effect_id_, object_ptr<restrictionInfo> &&restriction_info_, object_ptr<MessageContent> &&content_, object_ptr<ReplyMarkup> &&reply_markup_)
  : id_(id_)
  , sender_id_(std::move(sender_id_))
  , chat_id_(chat_id_)
  , sending_state_(std::move(sending_state_))
  , scheduling_state_(std::move(scheduling_state_))
  , is_outgoing_(is_outgoing_)
  , is_pinned_(is_pinned_)
  , is_from_offline_(is_from_offline_)
  , can_be_saved_(can_be_saved_)
  , has_timestamped_media_(has_timestamped_media_)
  , is_channel_post_(is_channel_post_)
  , is_paid_star_suggested_post_(is_paid_star_suggested_post_)
  , is_paid_ton_suggested_post_(is_paid_ton_suggested_post_)
  , contains_unread_mention_(contains_unread_mention_)
  , date_(date_)
  , edit_date_(edit_date_)
  , forward_info_(std::move(forward_info_))
  , import_info_(std::move(import_info_))
  , interaction_info_(std::move(interaction_info_))
  , unread_reactions_(std::move(unread_reactions_))
  , fact_check_(std::move(fact_check_))
  , suggested_post_info_(std::move(suggested_post_info_))
  , reply_to_(std::move(reply_to_))
  , topic_id_(std::move(topic_id_))
  , self_destruct_type_(std::move(self_destruct_type_))
  , self_destruct_in_(self_destruct_in_)
  , auto_delete_in_(auto_delete_in_)
  , via_bot_user_id_(via_bot_user_id_)
  , sender_business_bot_user_id_(sender_business_bot_user_id_)
  , sender_boost_count_(sender_boost_count_)
  , paid_message_star_count_(paid_message_star_count_)
  , author_signature_(author_signature_)
  , media_album_id_(media_album_id_)
  , effect_id_(effect_id_)
  , restriction_info_(std::move(restriction_info_))
  , content_(std::move(content_))
  , reply_markup_(std::move(reply_markup_))
{}

const std::int32_t message::ID;

void message::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "message");
    s.store_field("id", id_);
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_field("chat_id", chat_id_);
    s.store_object_field("sending_state", static_cast<const BaseObject *>(sending_state_.get()));
    s.store_object_field("scheduling_state", static_cast<const BaseObject *>(scheduling_state_.get()));
    s.store_field("is_outgoing", is_outgoing_);
    s.store_field("is_pinned", is_pinned_);
    s.store_field("is_from_offline", is_from_offline_);
    s.store_field("can_be_saved", can_be_saved_);
    s.store_field("has_timestamped_media", has_timestamped_media_);
    s.store_field("is_channel_post", is_channel_post_);
    s.store_field("is_paid_star_suggested_post", is_paid_star_suggested_post_);
    s.store_field("is_paid_ton_suggested_post", is_paid_ton_suggested_post_);
    s.store_field("contains_unread_mention", contains_unread_mention_);
    s.store_field("date", date_);
    s.store_field("edit_date", edit_date_);
    s.store_object_field("forward_info", static_cast<const BaseObject *>(forward_info_.get()));
    s.store_object_field("import_info", static_cast<const BaseObject *>(import_info_.get()));
    s.store_object_field("interaction_info", static_cast<const BaseObject *>(interaction_info_.get()));
    { s.store_vector_begin("unread_reactions", unread_reactions_.size()); for (const auto &_value : unread_reactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("fact_check", static_cast<const BaseObject *>(fact_check_.get()));
    s.store_object_field("suggested_post_info", static_cast<const BaseObject *>(suggested_post_info_.get()));
    s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get()));
    s.store_object_field("topic_id", static_cast<const BaseObject *>(topic_id_.get()));
    s.store_object_field("self_destruct_type", static_cast<const BaseObject *>(self_destruct_type_.get()));
    s.store_field("self_destruct_in", self_destruct_in_);
    s.store_field("auto_delete_in", auto_delete_in_);
    s.store_field("via_bot_user_id", via_bot_user_id_);
    s.store_field("sender_business_bot_user_id", sender_business_bot_user_id_);
    s.store_field("sender_boost_count", sender_boost_count_);
    s.store_field("paid_message_star_count", paid_message_star_count_);
    s.store_field("author_signature", author_signature_);
    s.store_field("media_album_id", media_album_id_);
    s.store_field("effect_id", effect_id_);
    s.store_object_field("restriction_info", static_cast<const BaseObject *>(restriction_info_.get()));
    s.store_object_field("content", static_cast<const BaseObject *>(content_.get()));
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_class_end();
  }
}

messageOriginUser::messageOriginUser()
  : sender_user_id_()
{}

messageOriginUser::messageOriginUser(int53 sender_user_id_)
  : sender_user_id_(sender_user_id_)
{}

const std::int32_t messageOriginUser::ID;

void messageOriginUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageOriginUser");
    s.store_field("sender_user_id", sender_user_id_);
    s.store_class_end();
  }
}

messageOriginHiddenUser::messageOriginHiddenUser()
  : sender_name_()
{}

messageOriginHiddenUser::messageOriginHiddenUser(string const &sender_name_)
  : sender_name_(sender_name_)
{}

const std::int32_t messageOriginHiddenUser::ID;

void messageOriginHiddenUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageOriginHiddenUser");
    s.store_field("sender_name", sender_name_);
    s.store_class_end();
  }
}

messageOriginChat::messageOriginChat()
  : sender_chat_id_()
  , author_signature_()
{}

messageOriginChat::messageOriginChat(int53 sender_chat_id_, string const &author_signature_)
  : sender_chat_id_(sender_chat_id_)
  , author_signature_(author_signature_)
{}

const std::int32_t messageOriginChat::ID;

void messageOriginChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageOriginChat");
    s.store_field("sender_chat_id", sender_chat_id_);
    s.store_field("author_signature", author_signature_);
    s.store_class_end();
  }
}

messageOriginChannel::messageOriginChannel()
  : chat_id_()
  , message_id_()
  , author_signature_()
{}

messageOriginChannel::messageOriginChannel(int53 chat_id_, int53 message_id_, string const &author_signature_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , author_signature_(author_signature_)
{}

const std::int32_t messageOriginChannel::ID;

void messageOriginChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageOriginChannel");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("author_signature", author_signature_);
    s.store_class_end();
  }
}

messageSelfDestructTypeTimer::messageSelfDestructTypeTimer()
  : self_destruct_time_()
{}

messageSelfDestructTypeTimer::messageSelfDestructTypeTimer(int32 self_destruct_time_)
  : self_destruct_time_(self_destruct_time_)
{}

const std::int32_t messageSelfDestructTypeTimer::ID;

void messageSelfDestructTypeTimer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSelfDestructTypeTimer");
    s.store_field("self_destruct_time", self_destruct_time_);
    s.store_class_end();
  }
}

messageSelfDestructTypeImmediately::messageSelfDestructTypeImmediately() {
}

const std::int32_t messageSelfDestructTypeImmediately::ID;

void messageSelfDestructTypeImmediately::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSelfDestructTypeImmediately");
    s.store_class_end();
  }
}

messageSenders::messageSenders()
  : total_count_()
  , senders_()
{}

messageSenders::messageSenders(int32 total_count_, array<object_ptr<MessageSender>> &&senders_)
  : total_count_(total_count_)
  , senders_(std::move(senders_))
{}

const std::int32_t messageSenders::ID;

void messageSenders::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSenders");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("senders", senders_.size()); for (const auto &_value : senders_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messageThreadInfo::messageThreadInfo()
  : chat_id_()
  , message_thread_id_()
  , reply_info_()
  , unread_message_count_()
  , messages_()
  , draft_message_()
{}

messageThreadInfo::messageThreadInfo(int53 chat_id_, int53 message_thread_id_, object_ptr<messageReplyInfo> &&reply_info_, int32 unread_message_count_, array<object_ptr<message>> &&messages_, object_ptr<draftMessage> &&draft_message_)
  : chat_id_(chat_id_)
  , message_thread_id_(message_thread_id_)
  , reply_info_(std::move(reply_info_))
  , unread_message_count_(unread_message_count_)
  , messages_(std::move(messages_))
  , draft_message_(std::move(draft_message_))
{}

const std::int32_t messageThreadInfo::ID;

void messageThreadInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageThreadInfo");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_thread_id", message_thread_id_);
    s.store_object_field("reply_info", static_cast<const BaseObject *>(reply_info_.get()));
    s.store_field("unread_message_count", unread_message_count_);
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("draft_message", static_cast<const BaseObject *>(draft_message_.get()));
    s.store_class_end();
  }
}

pageBlockHorizontalAlignmentLeft::pageBlockHorizontalAlignmentLeft() {
}

const std::int32_t pageBlockHorizontalAlignmentLeft::ID;

void pageBlockHorizontalAlignmentLeft::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockHorizontalAlignmentLeft");
    s.store_class_end();
  }
}

pageBlockHorizontalAlignmentCenter::pageBlockHorizontalAlignmentCenter() {
}

const std::int32_t pageBlockHorizontalAlignmentCenter::ID;

void pageBlockHorizontalAlignmentCenter::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockHorizontalAlignmentCenter");
    s.store_class_end();
  }
}

pageBlockHorizontalAlignmentRight::pageBlockHorizontalAlignmentRight() {
}

const std::int32_t pageBlockHorizontalAlignmentRight::ID;

void pageBlockHorizontalAlignmentRight::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockHorizontalAlignmentRight");
    s.store_class_end();
  }
}

passportAuthorizationForm::passportAuthorizationForm()
  : id_()
  , required_elements_()
  , privacy_policy_url_()
{}

passportAuthorizationForm::passportAuthorizationForm(int32 id_, array<object_ptr<passportRequiredElement>> &&required_elements_, string const &privacy_policy_url_)
  : id_(id_)
  , required_elements_(std::move(required_elements_))
  , privacy_policy_url_(privacy_policy_url_)
{}

const std::int32_t passportAuthorizationForm::ID;

void passportAuthorizationForm::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportAuthorizationForm");
    s.store_field("id", id_);
    { s.store_vector_begin("required_elements", required_elements_.size()); for (const auto &_value : required_elements_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("privacy_policy_url", privacy_policy_url_);
    s.store_class_end();
  }
}

personalDocument::personalDocument()
  : files_()
  , translation_()
{}

personalDocument::personalDocument(array<object_ptr<datedFile>> &&files_, array<object_ptr<datedFile>> &&translation_)
  : files_(std::move(files_))
  , translation_(std::move(translation_))
{}

const std::int32_t personalDocument::ID;

void personalDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "personalDocument");
    { s.store_vector_begin("files", files_.size()); for (const auto &_value : files_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("translation", translation_.size()); for (const auto &_value : translation_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

poll::poll()
  : id_()
  , question_()
  , options_()
  , total_voter_count_()
  , recent_voter_ids_()
  , is_anonymous_()
  , type_()
  , open_period_()
  , close_date_()
  , is_closed_()
{}

poll::poll(int64 id_, object_ptr<formattedText> &&question_, array<object_ptr<pollOption>> &&options_, int32 total_voter_count_, array<object_ptr<MessageSender>> &&recent_voter_ids_, bool is_anonymous_, object_ptr<PollType> &&type_, int32 open_period_, int32 close_date_, bool is_closed_)
  : id_(id_)
  , question_(std::move(question_))
  , options_(std::move(options_))
  , total_voter_count_(total_voter_count_)
  , recent_voter_ids_(std::move(recent_voter_ids_))
  , is_anonymous_(is_anonymous_)
  , type_(std::move(type_))
  , open_period_(open_period_)
  , close_date_(close_date_)
  , is_closed_(is_closed_)
{}

const std::int32_t poll::ID;

void poll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "poll");
    s.store_field("id", id_);
    s.store_object_field("question", static_cast<const BaseObject *>(question_.get()));
    { s.store_vector_begin("options", options_.size()); for (const auto &_value : options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("total_voter_count", total_voter_count_);
    { s.store_vector_begin("recent_voter_ids", recent_voter_ids_.size()); for (const auto &_value : recent_voter_ids_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("is_anonymous", is_anonymous_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("open_period", open_period_);
    s.store_field("close_date", close_date_);
    s.store_field("is_closed", is_closed_);
    s.store_class_end();
  }
}

preparedInlineMessage::preparedInlineMessage()
  : inline_query_id_()
  , result_()
  , chat_types_()
{}

preparedInlineMessage::preparedInlineMessage(int64 inline_query_id_, object_ptr<InlineQueryResult> &&result_, object_ptr<targetChatTypes> &&chat_types_)
  : inline_query_id_(inline_query_id_)
  , result_(std::move(result_))
  , chat_types_(std::move(chat_types_))
{}

const std::int32_t preparedInlineMessage::ID;

void preparedInlineMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "preparedInlineMessage");
    s.store_field("inline_query_id", inline_query_id_);
    s.store_object_field("result", static_cast<const BaseObject *>(result_.get()));
    s.store_object_field("chat_types", static_cast<const BaseObject *>(chat_types_.get()));
    s.store_class_end();
  }
}

reportChatResultOk::reportChatResultOk() {
}

const std::int32_t reportChatResultOk::ID;

void reportChatResultOk::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportChatResultOk");
    s.store_class_end();
  }
}

reportChatResultOptionRequired::reportChatResultOptionRequired()
  : title_()
  , options_()
{}

reportChatResultOptionRequired::reportChatResultOptionRequired(string const &title_, array<object_ptr<reportOption>> &&options_)
  : title_(title_)
  , options_(std::move(options_))
{}

const std::int32_t reportChatResultOptionRequired::ID;

void reportChatResultOptionRequired::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportChatResultOptionRequired");
    s.store_field("title", title_);
    { s.store_vector_begin("options", options_.size()); for (const auto &_value : options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

reportChatResultTextRequired::reportChatResultTextRequired()
  : option_id_()
  , is_optional_()
{}

reportChatResultTextRequired::reportChatResultTextRequired(bytes const &option_id_, bool is_optional_)
  : option_id_(std::move(option_id_))
  , is_optional_(is_optional_)
{}

const std::int32_t reportChatResultTextRequired::ID;

void reportChatResultTextRequired::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportChatResultTextRequired");
    s.store_bytes_field("option_id", option_id_);
    s.store_field("is_optional", is_optional_);
    s.store_class_end();
  }
}

reportChatResultMessagesRequired::reportChatResultMessagesRequired() {
}

const std::int32_t reportChatResultMessagesRequired::ID;

void reportChatResultMessagesRequired::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportChatResultMessagesRequired");
    s.store_class_end();
  }
}

richTextPlain::richTextPlain()
  : text_()
{}

richTextPlain::richTextPlain(string const &text_)
  : text_(text_)
{}

const std::int32_t richTextPlain::ID;

void richTextPlain::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "richTextPlain");
    s.store_field("text", text_);
    s.store_class_end();
  }
}

richTextBold::richTextBold()
  : text_()
{}

richTextBold::richTextBold(object_ptr<RichText> &&text_)
  : text_(std::move(text_))
{}

const std::int32_t richTextBold::ID;

void richTextBold::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "richTextBold");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

richTextItalic::richTextItalic()
  : text_()
{}

richTextItalic::richTextItalic(object_ptr<RichText> &&text_)
  : text_(std::move(text_))
{}

const std::int32_t richTextItalic::ID;

void richTextItalic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "richTextItalic");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

richTextUnderline::richTextUnderline()
  : text_()
{}

richTextUnderline::richTextUnderline(object_ptr<RichText> &&text_)
  : text_(std::move(text_))
{}

const std::int32_t richTextUnderline::ID;

void richTextUnderline::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "richTextUnderline");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

richTextStrikethrough::richTextStrikethrough()
  : text_()
{}

richTextStrikethrough::richTextStrikethrough(object_ptr<RichText> &&text_)
  : text_(std::move(text_))
{}

const std::int32_t richTextStrikethrough::ID;

void richTextStrikethrough::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "richTextStrikethrough");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

richTextFixed::richTextFixed()
  : text_()
{}

richTextFixed::richTextFixed(object_ptr<RichText> &&text_)
  : text_(std::move(text_))
{}

const std::int32_t richTextFixed::ID;

void richTextFixed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "richTextFixed");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

richTextUrl::richTextUrl()
  : text_()
  , url_()
  , is_cached_()
{}

richTextUrl::richTextUrl(object_ptr<RichText> &&text_, string const &url_, bool is_cached_)
  : text_(std::move(text_))
  , url_(url_)
  , is_cached_(is_cached_)
{}

const std::int32_t richTextUrl::ID;

void richTextUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "richTextUrl");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("url", url_);
    s.store_field("is_cached", is_cached_);
    s.store_class_end();
  }
}

richTextEmailAddress::richTextEmailAddress()
  : text_()
  , email_address_()
{}

richTextEmailAddress::richTextEmailAddress(object_ptr<RichText> &&text_, string const &email_address_)
  : text_(std::move(text_))
  , email_address_(email_address_)
{}

const std::int32_t richTextEmailAddress::ID;

void richTextEmailAddress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "richTextEmailAddress");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("email_address", email_address_);
    s.store_class_end();
  }
}

richTextSubscript::richTextSubscript()
  : text_()
{}

richTextSubscript::richTextSubscript(object_ptr<RichText> &&text_)
  : text_(std::move(text_))
{}

const std::int32_t richTextSubscript::ID;

void richTextSubscript::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "richTextSubscript");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

richTextSuperscript::richTextSuperscript()
  : text_()
{}

richTextSuperscript::richTextSuperscript(object_ptr<RichText> &&text_)
  : text_(std::move(text_))
{}

const std::int32_t richTextSuperscript::ID;

void richTextSuperscript::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "richTextSuperscript");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

richTextMarked::richTextMarked()
  : text_()
{}

richTextMarked::richTextMarked(object_ptr<RichText> &&text_)
  : text_(std::move(text_))
{}

const std::int32_t richTextMarked::ID;

void richTextMarked::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "richTextMarked");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

richTextPhoneNumber::richTextPhoneNumber()
  : text_()
  , phone_number_()
{}

richTextPhoneNumber::richTextPhoneNumber(object_ptr<RichText> &&text_, string const &phone_number_)
  : text_(std::move(text_))
  , phone_number_(phone_number_)
{}

const std::int32_t richTextPhoneNumber::ID;

void richTextPhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "richTextPhoneNumber");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("phone_number", phone_number_);
    s.store_class_end();
  }
}

richTextIcon::richTextIcon()
  : document_()
  , width_()
  , height_()
{}

richTextIcon::richTextIcon(object_ptr<document> &&document_, int32 width_, int32 height_)
  : document_(std::move(document_))
  , width_(width_)
  , height_(height_)
{}

const std::int32_t richTextIcon::ID;

void richTextIcon::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "richTextIcon");
    s.store_object_field("document", static_cast<const BaseObject *>(document_.get()));
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_class_end();
  }
}

richTextReference::richTextReference()
  : text_()
  , anchor_name_()
  , url_()
{}

richTextReference::richTextReference(object_ptr<RichText> &&text_, string const &anchor_name_, string const &url_)
  : text_(std::move(text_))
  , anchor_name_(anchor_name_)
  , url_(url_)
{}

const std::int32_t richTextReference::ID;

void richTextReference::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "richTextReference");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("anchor_name", anchor_name_);
    s.store_field("url", url_);
    s.store_class_end();
  }
}

richTextAnchor::richTextAnchor()
  : name_()
{}

richTextAnchor::richTextAnchor(string const &name_)
  : name_(name_)
{}

const std::int32_t richTextAnchor::ID;

void richTextAnchor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "richTextAnchor");
    s.store_field("name", name_);
    s.store_class_end();
  }
}

richTextAnchorLink::richTextAnchorLink()
  : text_()
  , anchor_name_()
  , url_()
{}

richTextAnchorLink::richTextAnchorLink(object_ptr<RichText> &&text_, string const &anchor_name_, string const &url_)
  : text_(std::move(text_))
  , anchor_name_(anchor_name_)
  , url_(url_)
{}

const std::int32_t richTextAnchorLink::ID;

void richTextAnchorLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "richTextAnchorLink");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("anchor_name", anchor_name_);
    s.store_field("url", url_);
    s.store_class_end();
  }
}

richTexts::richTexts()
  : texts_()
{}

richTexts::richTexts(array<object_ptr<RichText>> &&texts_)
  : texts_(std::move(texts_))
{}

const std::int32_t richTexts::ID;

void richTexts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "richTexts");
    { s.store_vector_begin("texts", texts_.size()); for (const auto &_value : texts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

sponsoredChat::sponsoredChat()
  : unique_id_()
  , chat_id_()
  , sponsor_info_()
  , additional_info_()
{}

sponsoredChat::sponsoredChat(int53 unique_id_, int53 chat_id_, string const &sponsor_info_, string const &additional_info_)
  : unique_id_(unique_id_)
  , chat_id_(chat_id_)
  , sponsor_info_(sponsor_info_)
  , additional_info_(additional_info_)
{}

const std::int32_t sponsoredChat::ID;

void sponsoredChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sponsoredChat");
    s.store_field("unique_id", unique_id_);
    s.store_field("chat_id", chat_id_);
    s.store_field("sponsor_info", sponsor_info_);
    s.store_field("additional_info", additional_info_);
    s.store_class_end();
  }
}

starGiveawayWinnerOption::starGiveawayWinnerOption()
  : winner_count_()
  , won_star_count_()
  , is_default_()
{}

starGiveawayWinnerOption::starGiveawayWinnerOption(int32 winner_count_, int53 won_star_count_, bool is_default_)
  : winner_count_(winner_count_)
  , won_star_count_(won_star_count_)
  , is_default_(is_default_)
{}

const std::int32_t starGiveawayWinnerOption::ID;

void starGiveawayWinnerOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starGiveawayWinnerOption");
    s.store_field("winner_count", winner_count_);
    s.store_field("won_star_count", won_star_count_);
    s.store_field("is_default", is_default_);
    s.store_class_end();
  }
}

starTransactions::starTransactions()
  : star_amount_()
  , transactions_()
  , next_offset_()
{}

starTransactions::starTransactions(object_ptr<starAmount> &&star_amount_, array<object_ptr<starTransaction>> &&transactions_, string const &next_offset_)
  : star_amount_(std::move(star_amount_))
  , transactions_(std::move(transactions_))
  , next_offset_(next_offset_)
{}

const std::int32_t starTransactions::ID;

void starTransactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactions");
    s.store_object_field("star_amount", static_cast<const BaseObject *>(star_amount_.get()));
    { s.store_vector_begin("transactions", transactions_.size()); for (const auto &_value : transactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

storageStatisticsByFileType::storageStatisticsByFileType()
  : file_type_()
  , size_()
  , count_()
{}

storageStatisticsByFileType::storageStatisticsByFileType(object_ptr<FileType> &&file_type_, int53 size_, int32 count_)
  : file_type_(std::move(file_type_))
  , size_(size_)
  , count_(count_)
{}

const std::int32_t storageStatisticsByFileType::ID;

void storageStatisticsByFileType::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storageStatisticsByFileType");
    s.store_object_field("file_type", static_cast<const BaseObject *>(file_type_.get()));
    s.store_field("size", size_);
    s.store_field("count", count_);
    s.store_class_end();
  }
}

story::story()
  : id_()
  , poster_chat_id_()
  , poster_id_()
  , date_()
  , is_being_posted_()
  , is_being_edited_()
  , is_edited_()
  , is_posted_to_chat_page_()
  , is_visible_only_for_self_()
  , can_be_added_to_album_()
  , can_be_deleted_()
  , can_be_edited_()
  , can_be_forwarded_()
  , can_be_replied_()
  , can_toggle_is_posted_to_chat_page_()
  , can_get_statistics_()
  , can_get_interactions_()
  , has_expired_viewers_()
  , repost_info_()
  , interaction_info_()
  , chosen_reaction_type_()
  , privacy_settings_()
  , content_()
  , areas_()
  , caption_()
  , album_ids_()
{}

story::story(int32 id_, int53 poster_chat_id_, object_ptr<MessageSender> &&poster_id_, int32 date_, bool is_being_posted_, bool is_being_edited_, bool is_edited_, bool is_posted_to_chat_page_, bool is_visible_only_for_self_, bool can_be_added_to_album_, bool can_be_deleted_, bool can_be_edited_, bool can_be_forwarded_, bool can_be_replied_, bool can_toggle_is_posted_to_chat_page_, bool can_get_statistics_, bool can_get_interactions_, bool has_expired_viewers_, object_ptr<storyRepostInfo> &&repost_info_, object_ptr<storyInteractionInfo> &&interaction_info_, object_ptr<ReactionType> &&chosen_reaction_type_, object_ptr<StoryPrivacySettings> &&privacy_settings_, object_ptr<StoryContent> &&content_, array<object_ptr<storyArea>> &&areas_, object_ptr<formattedText> &&caption_, array<int32> &&album_ids_)
  : id_(id_)
  , poster_chat_id_(poster_chat_id_)
  , poster_id_(std::move(poster_id_))
  , date_(date_)
  , is_being_posted_(is_being_posted_)
  , is_being_edited_(is_being_edited_)
  , is_edited_(is_edited_)
  , is_posted_to_chat_page_(is_posted_to_chat_page_)
  , is_visible_only_for_self_(is_visible_only_for_self_)
  , can_be_added_to_album_(can_be_added_to_album_)
  , can_be_deleted_(can_be_deleted_)
  , can_be_edited_(can_be_edited_)
  , can_be_forwarded_(can_be_forwarded_)
  , can_be_replied_(can_be_replied_)
  , can_toggle_is_posted_to_chat_page_(can_toggle_is_posted_to_chat_page_)
  , can_get_statistics_(can_get_statistics_)
  , can_get_interactions_(can_get_interactions_)
  , has_expired_viewers_(has_expired_viewers_)
  , repost_info_(std::move(repost_info_))
  , interaction_info_(std::move(interaction_info_))
  , chosen_reaction_type_(std::move(chosen_reaction_type_))
  , privacy_settings_(std::move(privacy_settings_))
  , content_(std::move(content_))
  , areas_(std::move(areas_))
  , caption_(std::move(caption_))
  , album_ids_(std::move(album_ids_))
{}

const std::int32_t story::ID;

void story::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "story");
    s.store_field("id", id_);
    s.store_field("poster_chat_id", poster_chat_id_);
    s.store_object_field("poster_id", static_cast<const BaseObject *>(poster_id_.get()));
    s.store_field("date", date_);
    s.store_field("is_being_posted", is_being_posted_);
    s.store_field("is_being_edited", is_being_edited_);
    s.store_field("is_edited", is_edited_);
    s.store_field("is_posted_to_chat_page", is_posted_to_chat_page_);
    s.store_field("is_visible_only_for_self", is_visible_only_for_self_);
    s.store_field("can_be_added_to_album", can_be_added_to_album_);
    s.store_field("can_be_deleted", can_be_deleted_);
    s.store_field("can_be_edited", can_be_edited_);
    s.store_field("can_be_forwarded", can_be_forwarded_);
    s.store_field("can_be_replied", can_be_replied_);
    s.store_field("can_toggle_is_posted_to_chat_page", can_toggle_is_posted_to_chat_page_);
    s.store_field("can_get_statistics", can_get_statistics_);
    s.store_field("can_get_interactions", can_get_interactions_);
    s.store_field("has_expired_viewers", has_expired_viewers_);
    s.store_object_field("repost_info", static_cast<const BaseObject *>(repost_info_.get()));
    s.store_object_field("interaction_info", static_cast<const BaseObject *>(interaction_info_.get()));
    s.store_object_field("chosen_reaction_type", static_cast<const BaseObject *>(chosen_reaction_type_.get()));
    s.store_object_field("privacy_settings", static_cast<const BaseObject *>(privacy_settings_.get()));
    s.store_object_field("content", static_cast<const BaseObject *>(content_.get()));
    { s.store_vector_begin("areas", areas_.size()); for (const auto &_value : areas_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    { s.store_vector_begin("album_ids", album_ids_.size()); for (const auto &_value : album_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

storyAlbums::storyAlbums()
  : albums_()
{}

storyAlbums::storyAlbums(array<object_ptr<storyAlbum>> &&albums_)
  : albums_(std::move(albums_))
{}

const std::int32_t storyAlbums::ID;

void storyAlbums::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyAlbums");
    { s.store_vector_begin("albums", albums_.size()); for (const auto &_value : albums_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

storyInfo::storyInfo()
  : story_id_()
  , date_()
  , is_for_close_friends_()
{}

storyInfo::storyInfo(int32 story_id_, int32 date_, bool is_for_close_friends_)
  : story_id_(story_id_)
  , date_(date_)
  , is_for_close_friends_(is_for_close_friends_)
{}

const std::int32_t storyInfo::ID;

void storyInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyInfo");
    s.store_field("story_id", story_id_);
    s.store_field("date", date_);
    s.store_field("is_for_close_friends", is_for_close_friends_);
    s.store_class_end();
  }
}

suggestedActionEnableArchiveAndMuteNewChats::suggestedActionEnableArchiveAndMuteNewChats() {
}

const std::int32_t suggestedActionEnableArchiveAndMuteNewChats::ID;

void suggestedActionEnableArchiveAndMuteNewChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedActionEnableArchiveAndMuteNewChats");
    s.store_class_end();
  }
}

suggestedActionCheckPassword::suggestedActionCheckPassword() {
}

const std::int32_t suggestedActionCheckPassword::ID;

void suggestedActionCheckPassword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedActionCheckPassword");
    s.store_class_end();
  }
}

suggestedActionCheckPhoneNumber::suggestedActionCheckPhoneNumber() {
}

const std::int32_t suggestedActionCheckPhoneNumber::ID;

void suggestedActionCheckPhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedActionCheckPhoneNumber");
    s.store_class_end();
  }
}

suggestedActionViewChecksHint::suggestedActionViewChecksHint() {
}

const std::int32_t suggestedActionViewChecksHint::ID;

void suggestedActionViewChecksHint::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedActionViewChecksHint");
    s.store_class_end();
  }
}

suggestedActionConvertToBroadcastGroup::suggestedActionConvertToBroadcastGroup()
  : supergroup_id_()
{}

suggestedActionConvertToBroadcastGroup::suggestedActionConvertToBroadcastGroup(int53 supergroup_id_)
  : supergroup_id_(supergroup_id_)
{}

const std::int32_t suggestedActionConvertToBroadcastGroup::ID;

void suggestedActionConvertToBroadcastGroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedActionConvertToBroadcastGroup");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_class_end();
  }
}

suggestedActionSetPassword::suggestedActionSetPassword()
  : authorization_delay_()
{}

suggestedActionSetPassword::suggestedActionSetPassword(int32 authorization_delay_)
  : authorization_delay_(authorization_delay_)
{}

const std::int32_t suggestedActionSetPassword::ID;

void suggestedActionSetPassword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedActionSetPassword");
    s.store_field("authorization_delay", authorization_delay_);
    s.store_class_end();
  }
}

suggestedActionUpgradePremium::suggestedActionUpgradePremium() {
}

const std::int32_t suggestedActionUpgradePremium::ID;

void suggestedActionUpgradePremium::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedActionUpgradePremium");
    s.store_class_end();
  }
}

suggestedActionRestorePremium::suggestedActionRestorePremium() {
}

const std::int32_t suggestedActionRestorePremium::ID;

void suggestedActionRestorePremium::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedActionRestorePremium");
    s.store_class_end();
  }
}

suggestedActionSubscribeToAnnualPremium::suggestedActionSubscribeToAnnualPremium() {
}

const std::int32_t suggestedActionSubscribeToAnnualPremium::ID;

void suggestedActionSubscribeToAnnualPremium::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedActionSubscribeToAnnualPremium");
    s.store_class_end();
  }
}

suggestedActionGiftPremiumForChristmas::suggestedActionGiftPremiumForChristmas() {
}

const std::int32_t suggestedActionGiftPremiumForChristmas::ID;

void suggestedActionGiftPremiumForChristmas::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedActionGiftPremiumForChristmas");
    s.store_class_end();
  }
}

suggestedActionSetBirthdate::suggestedActionSetBirthdate() {
}

const std::int32_t suggestedActionSetBirthdate::ID;

void suggestedActionSetBirthdate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedActionSetBirthdate");
    s.store_class_end();
  }
}

suggestedActionSetProfilePhoto::suggestedActionSetProfilePhoto() {
}

const std::int32_t suggestedActionSetProfilePhoto::ID;

void suggestedActionSetProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedActionSetProfilePhoto");
    s.store_class_end();
  }
}

suggestedActionExtendPremium::suggestedActionExtendPremium()
  : manage_premium_subscription_url_()
{}

suggestedActionExtendPremium::suggestedActionExtendPremium(string const &manage_premium_subscription_url_)
  : manage_premium_subscription_url_(manage_premium_subscription_url_)
{}

const std::int32_t suggestedActionExtendPremium::ID;

void suggestedActionExtendPremium::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedActionExtendPremium");
    s.store_field("manage_premium_subscription_url", manage_premium_subscription_url_);
    s.store_class_end();
  }
}

suggestedActionExtendStarSubscriptions::suggestedActionExtendStarSubscriptions() {
}

const std::int32_t suggestedActionExtendStarSubscriptions::ID;

void suggestedActionExtendStarSubscriptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedActionExtendStarSubscriptions");
    s.store_class_end();
  }
}

suggestedActionCustom::suggestedActionCustom()
  : name_()
  , title_()
  , description_()
  , url_()
{}

suggestedActionCustom::suggestedActionCustom(string const &name_, object_ptr<formattedText> &&title_, object_ptr<formattedText> &&description_, string const &url_)
  : name_(name_)
  , title_(std::move(title_))
  , description_(std::move(description_))
  , url_(url_)
{}

const std::int32_t suggestedActionCustom::ID;

void suggestedActionCustom::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedActionCustom");
    s.store_field("name", name_);
    s.store_object_field("title", static_cast<const BaseObject *>(title_.get()));
    s.store_object_field("description", static_cast<const BaseObject *>(description_.get()));
    s.store_field("url", url_);
    s.store_class_end();
  }
}

tMeUrl::tMeUrl()
  : url_()
  , type_()
{}

tMeUrl::tMeUrl(string const &url_, object_ptr<TMeUrlType> &&type_)
  : url_(url_)
  , type_(std::move(type_))
{}

const std::int32_t tMeUrl::ID;

void tMeUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "tMeUrl");
    s.store_field("url", url_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

temporaryPasswordState::temporaryPasswordState()
  : has_password_()
  , valid_for_()
{}

temporaryPasswordState::temporaryPasswordState(bool has_password_, int32 valid_for_)
  : has_password_(has_password_)
  , valid_for_(valid_for_)
{}

const std::int32_t temporaryPasswordState::ID;

void temporaryPasswordState::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "temporaryPasswordState");
    s.store_field("has_password", has_password_);
    s.store_field("valid_for", valid_for_);
    s.store_class_end();
  }
}

termsOfService::termsOfService()
  : text_()
  , min_user_age_()
  , show_popup_()
{}

termsOfService::termsOfService(object_ptr<formattedText> &&text_, int32 min_user_age_, bool show_popup_)
  : text_(std::move(text_))
  , min_user_age_(min_user_age_)
  , show_popup_(show_popup_)
{}

const std::int32_t termsOfService::ID;

void termsOfService::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "termsOfService");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("min_user_age", min_user_age_);
    s.store_field("show_popup", show_popup_);
    s.store_class_end();
  }
}

testVectorIntObject::testVectorIntObject()
  : value_()
{}

testVectorIntObject::testVectorIntObject(array<object_ptr<testInt>> &&value_)
  : value_(std::move(value_))
{}

const std::int32_t testVectorIntObject::ID;

void testVectorIntObject::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testVectorIntObject");
    { s.store_vector_begin("value", value_.size()); for (const auto &_value : value_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

timeZone::timeZone()
  : id_()
  , name_()
  , utc_time_offset_()
{}

timeZone::timeZone(string const &id_, string const &name_, int32 utc_time_offset_)
  : id_(id_)
  , name_(name_)
  , utc_time_offset_(utc_time_offset_)
{}

const std::int32_t timeZone::ID;

void timeZone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "timeZone");
    s.store_field("id", id_);
    s.store_field("name", name_);
    s.store_field("utc_time_offset", utc_time_offset_);
    s.store_class_end();
  }
}

tonRevenueStatus::tonRevenueStatus()
  : total_amount_()
  , balance_amount_()
  , available_amount_()
  , withdrawal_enabled_()
{}

tonRevenueStatus::tonRevenueStatus(int64 total_amount_, int64 balance_amount_, int64 available_amount_, bool withdrawal_enabled_)
  : total_amount_(total_amount_)
  , balance_amount_(balance_amount_)
  , available_amount_(available_amount_)
  , withdrawal_enabled_(withdrawal_enabled_)
{}

const std::int32_t tonRevenueStatus::ID;

void tonRevenueStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "tonRevenueStatus");
    s.store_field("total_amount", total_amount_);
    s.store_field("balance_amount", balance_amount_);
    s.store_field("available_amount", available_amount_);
    s.store_field("withdrawal_enabled", withdrawal_enabled_);
    s.store_class_end();
  }
}

tonTransaction::tonTransaction()
  : id_()
  , ton_amount_()
  , is_refund_()
  , date_()
  , type_()
{}

tonTransaction::tonTransaction(string const &id_, int53 ton_amount_, bool is_refund_, int32 date_, object_ptr<TonTransactionType> &&type_)
  : id_(id_)
  , ton_amount_(ton_amount_)
  , is_refund_(is_refund_)
  , date_(date_)
  , type_(std::move(type_))
{}

const std::int32_t tonTransaction::ID;

void tonTransaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "tonTransaction");
    s.store_field("id", id_);
    s.store_field("ton_amount", ton_amount_);
    s.store_field("is_refund", is_refund_);
    s.store_field("date", date_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

tonTransactionTypeFragmentDeposit::tonTransactionTypeFragmentDeposit()
  : is_gift_()
  , sticker_()
{}

tonTransactionTypeFragmentDeposit::tonTransactionTypeFragmentDeposit(bool is_gift_, object_ptr<sticker> &&sticker_)
  : is_gift_(is_gift_)
  , sticker_(std::move(sticker_))
{}

const std::int32_t tonTransactionTypeFragmentDeposit::ID;

void tonTransactionTypeFragmentDeposit::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "tonTransactionTypeFragmentDeposit");
    s.store_field("is_gift", is_gift_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

tonTransactionTypeSuggestedPostPayment::tonTransactionTypeSuggestedPostPayment()
  : chat_id_()
{}

tonTransactionTypeSuggestedPostPayment::tonTransactionTypeSuggestedPostPayment(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t tonTransactionTypeSuggestedPostPayment::ID;

void tonTransactionTypeSuggestedPostPayment::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "tonTransactionTypeSuggestedPostPayment");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

tonTransactionTypeUpgradedGiftPurchase::tonTransactionTypeUpgradedGiftPurchase()
  : user_id_()
  , gift_()
{}

tonTransactionTypeUpgradedGiftPurchase::tonTransactionTypeUpgradedGiftPurchase(int53 user_id_, object_ptr<upgradedGift> &&gift_)
  : user_id_(user_id_)
  , gift_(std::move(gift_))
{}

const std::int32_t tonTransactionTypeUpgradedGiftPurchase::ID;

void tonTransactionTypeUpgradedGiftPurchase::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "tonTransactionTypeUpgradedGiftPurchase");
    s.store_field("user_id", user_id_);
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_class_end();
  }
}

tonTransactionTypeUpgradedGiftSale::tonTransactionTypeUpgradedGiftSale()
  : user_id_()
  , gift_()
  , commission_per_mille_()
  , commission_toncoin_amount_()
{}

tonTransactionTypeUpgradedGiftSale::tonTransactionTypeUpgradedGiftSale(int53 user_id_, object_ptr<upgradedGift> &&gift_, int32 commission_per_mille_, int53 commission_toncoin_amount_)
  : user_id_(user_id_)
  , gift_(std::move(gift_))
  , commission_per_mille_(commission_per_mille_)
  , commission_toncoin_amount_(commission_toncoin_amount_)
{}

const std::int32_t tonTransactionTypeUpgradedGiftSale::ID;

void tonTransactionTypeUpgradedGiftSale::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "tonTransactionTypeUpgradedGiftSale");
    s.store_field("user_id", user_id_);
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_field("commission_per_mille", commission_per_mille_);
    s.store_field("commission_toncoin_amount", commission_toncoin_amount_);
    s.store_class_end();
  }
}

tonTransactionTypeUnsupported::tonTransactionTypeUnsupported() {
}

const std::int32_t tonTransactionTypeUnsupported::ID;

void tonTransactionTypeUnsupported::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "tonTransactionTypeUnsupported");
    s.store_class_end();
  }
}

updateAuthorizationState::updateAuthorizationState()
  : authorization_state_()
{}

updateAuthorizationState::updateAuthorizationState(object_ptr<AuthorizationState> &&authorization_state_)
  : authorization_state_(std::move(authorization_state_))
{}

const std::int32_t updateAuthorizationState::ID;

void updateAuthorizationState::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateAuthorizationState");
    s.store_object_field("authorization_state", static_cast<const BaseObject *>(authorization_state_.get()));
    s.store_class_end();
  }
}

updateNewMessage::updateNewMessage()
  : message_()
{}

updateNewMessage::updateNewMessage(object_ptr<message> &&message_)
  : message_(std::move(message_))
{}

const std::int32_t updateNewMessage::ID;

void updateNewMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewMessage");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

updateMessageSendAcknowledged::updateMessageSendAcknowledged()
  : chat_id_()
  , message_id_()
{}

updateMessageSendAcknowledged::updateMessageSendAcknowledged(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t updateMessageSendAcknowledged::ID;

void updateMessageSendAcknowledged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessageSendAcknowledged");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

updateMessageSendSucceeded::updateMessageSendSucceeded()
  : message_()
  , old_message_id_()
{}

updateMessageSendSucceeded::updateMessageSendSucceeded(object_ptr<message> &&message_, int53 old_message_id_)
  : message_(std::move(message_))
  , old_message_id_(old_message_id_)
{}

const std::int32_t updateMessageSendSucceeded::ID;

void updateMessageSendSucceeded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessageSendSucceeded");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_field("old_message_id", old_message_id_);
    s.store_class_end();
  }
}

updateMessageSendFailed::updateMessageSendFailed()
  : message_()
  , old_message_id_()
  , error_()
{}

updateMessageSendFailed::updateMessageSendFailed(object_ptr<message> &&message_, int53 old_message_id_, object_ptr<error> &&error_)
  : message_(std::move(message_))
  , old_message_id_(old_message_id_)
  , error_(std::move(error_))
{}

const std::int32_t updateMessageSendFailed::ID;

void updateMessageSendFailed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessageSendFailed");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_field("old_message_id", old_message_id_);
    s.store_object_field("error", static_cast<const BaseObject *>(error_.get()));
    s.store_class_end();
  }
}

updateMessageContent::updateMessageContent()
  : chat_id_()
  , message_id_()
  , new_content_()
{}

updateMessageContent::updateMessageContent(int53 chat_id_, int53 message_id_, object_ptr<MessageContent> &&new_content_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , new_content_(std::move(new_content_))
{}

const std::int32_t updateMessageContent::ID;

void updateMessageContent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessageContent");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("new_content", static_cast<const BaseObject *>(new_content_.get()));
    s.store_class_end();
  }
}

updateMessageEdited::updateMessageEdited()
  : chat_id_()
  , message_id_()
  , edit_date_()
  , reply_markup_()
{}

updateMessageEdited::updateMessageEdited(int53 chat_id_, int53 message_id_, int32 edit_date_, object_ptr<ReplyMarkup> &&reply_markup_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , edit_date_(edit_date_)
  , reply_markup_(std::move(reply_markup_))
{}

const std::int32_t updateMessageEdited::ID;

void updateMessageEdited::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessageEdited");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("edit_date", edit_date_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_class_end();
  }
}

updateMessageIsPinned::updateMessageIsPinned()
  : chat_id_()
  , message_id_()
  , is_pinned_()
{}

updateMessageIsPinned::updateMessageIsPinned(int53 chat_id_, int53 message_id_, bool is_pinned_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , is_pinned_(is_pinned_)
{}

const std::int32_t updateMessageIsPinned::ID;

void updateMessageIsPinned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessageIsPinned");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

updateMessageInteractionInfo::updateMessageInteractionInfo()
  : chat_id_()
  , message_id_()
  , interaction_info_()
{}

updateMessageInteractionInfo::updateMessageInteractionInfo(int53 chat_id_, int53 message_id_, object_ptr<messageInteractionInfo> &&interaction_info_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , interaction_info_(std::move(interaction_info_))
{}

const std::int32_t updateMessageInteractionInfo::ID;

void updateMessageInteractionInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessageInteractionInfo");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("interaction_info", static_cast<const BaseObject *>(interaction_info_.get()));
    s.store_class_end();
  }
}

updateMessageContentOpened::updateMessageContentOpened()
  : chat_id_()
  , message_id_()
{}

updateMessageContentOpened::updateMessageContentOpened(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t updateMessageContentOpened::ID;

void updateMessageContentOpened::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessageContentOpened");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

updateMessageMentionRead::updateMessageMentionRead()
  : chat_id_()
  , message_id_()
  , unread_mention_count_()
{}

updateMessageMentionRead::updateMessageMentionRead(int53 chat_id_, int53 message_id_, int32 unread_mention_count_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , unread_mention_count_(unread_mention_count_)
{}

const std::int32_t updateMessageMentionRead::ID;

void updateMessageMentionRead::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessageMentionRead");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("unread_mention_count", unread_mention_count_);
    s.store_class_end();
  }
}

updateMessageUnreadReactions::updateMessageUnreadReactions()
  : chat_id_()
  , message_id_()
  , unread_reactions_()
  , unread_reaction_count_()
{}

updateMessageUnreadReactions::updateMessageUnreadReactions(int53 chat_id_, int53 message_id_, array<object_ptr<unreadReaction>> &&unread_reactions_, int32 unread_reaction_count_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , unread_reactions_(std::move(unread_reactions_))
  , unread_reaction_count_(unread_reaction_count_)
{}

const std::int32_t updateMessageUnreadReactions::ID;

void updateMessageUnreadReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessageUnreadReactions");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    { s.store_vector_begin("unread_reactions", unread_reactions_.size()); for (const auto &_value : unread_reactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("unread_reaction_count", unread_reaction_count_);
    s.store_class_end();
  }
}

updateMessageFactCheck::updateMessageFactCheck()
  : chat_id_()
  , message_id_()
  , fact_check_()
{}

updateMessageFactCheck::updateMessageFactCheck(int53 chat_id_, int53 message_id_, object_ptr<factCheck> &&fact_check_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , fact_check_(std::move(fact_check_))
{}

const std::int32_t updateMessageFactCheck::ID;

void updateMessageFactCheck::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessageFactCheck");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("fact_check", static_cast<const BaseObject *>(fact_check_.get()));
    s.store_class_end();
  }
}

updateMessageSuggestedPostInfo::updateMessageSuggestedPostInfo()
  : chat_id_()
  , message_id_()
  , suggested_post_info_()
{}

updateMessageSuggestedPostInfo::updateMessageSuggestedPostInfo(int53 chat_id_, int53 message_id_, object_ptr<suggestedPostInfo> &&suggested_post_info_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , suggested_post_info_(std::move(suggested_post_info_))
{}

const std::int32_t updateMessageSuggestedPostInfo::ID;

void updateMessageSuggestedPostInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessageSuggestedPostInfo");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("suggested_post_info", static_cast<const BaseObject *>(suggested_post_info_.get()));
    s.store_class_end();
  }
}

updateMessageLiveLocationViewed::updateMessageLiveLocationViewed()
  : chat_id_()
  , message_id_()
{}

updateMessageLiveLocationViewed::updateMessageLiveLocationViewed(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t updateMessageLiveLocationViewed::ID;

void updateMessageLiveLocationViewed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessageLiveLocationViewed");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

updateVideoPublished::updateVideoPublished()
  : chat_id_()
  , message_id_()
{}

updateVideoPublished::updateVideoPublished(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t updateVideoPublished::ID;

void updateVideoPublished::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateVideoPublished");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

updateNewChat::updateNewChat()
  : chat_()
{}

updateNewChat::updateNewChat(object_ptr<chat> &&chat_)
  : chat_(std::move(chat_))
{}

const std::int32_t updateNewChat::ID;

void updateNewChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewChat");
    s.store_object_field("chat", static_cast<const BaseObject *>(chat_.get()));
    s.store_class_end();
  }
}

updateChatTitle::updateChatTitle()
  : chat_id_()
  , title_()
{}

updateChatTitle::updateChatTitle(int53 chat_id_, string const &title_)
  : chat_id_(chat_id_)
  , title_(title_)
{}

const std::int32_t updateChatTitle::ID;

void updateChatTitle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatTitle");
    s.store_field("chat_id", chat_id_);
    s.store_field("title", title_);
    s.store_class_end();
  }
}

updateChatPhoto::updateChatPhoto()
  : chat_id_()
  , photo_()
{}

updateChatPhoto::updateChatPhoto(int53 chat_id_, object_ptr<chatPhotoInfo> &&photo_)
  : chat_id_(chat_id_)
  , photo_(std::move(photo_))
{}

const std::int32_t updateChatPhoto::ID;

void updateChatPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatPhoto");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

updateChatAccentColors::updateChatAccentColors()
  : chat_id_()
  , accent_color_id_()
  , background_custom_emoji_id_()
  , upgraded_gift_colors_()
  , profile_accent_color_id_()
  , profile_background_custom_emoji_id_()
{}

updateChatAccentColors::updateChatAccentColors(int53 chat_id_, int32 accent_color_id_, int64 background_custom_emoji_id_, object_ptr<upgradedGiftColors> &&upgraded_gift_colors_, int32 profile_accent_color_id_, int64 profile_background_custom_emoji_id_)
  : chat_id_(chat_id_)
  , accent_color_id_(accent_color_id_)
  , background_custom_emoji_id_(background_custom_emoji_id_)
  , upgraded_gift_colors_(std::move(upgraded_gift_colors_))
  , profile_accent_color_id_(profile_accent_color_id_)
  , profile_background_custom_emoji_id_(profile_background_custom_emoji_id_)
{}

const std::int32_t updateChatAccentColors::ID;

void updateChatAccentColors::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatAccentColors");
    s.store_field("chat_id", chat_id_);
    s.store_field("accent_color_id", accent_color_id_);
    s.store_field("background_custom_emoji_id", background_custom_emoji_id_);
    s.store_object_field("upgraded_gift_colors", static_cast<const BaseObject *>(upgraded_gift_colors_.get()));
    s.store_field("profile_accent_color_id", profile_accent_color_id_);
    s.store_field("profile_background_custom_emoji_id", profile_background_custom_emoji_id_);
    s.store_class_end();
  }
}

updateChatPermissions::updateChatPermissions()
  : chat_id_()
  , permissions_()
{}

updateChatPermissions::updateChatPermissions(int53 chat_id_, object_ptr<chatPermissions> &&permissions_)
  : chat_id_(chat_id_)
  , permissions_(std::move(permissions_))
{}

const std::int32_t updateChatPermissions::ID;

void updateChatPermissions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatPermissions");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("permissions", static_cast<const BaseObject *>(permissions_.get()));
    s.store_class_end();
  }
}

updateChatLastMessage::updateChatLastMessage()
  : chat_id_()
  , last_message_()
  , positions_()
{}

updateChatLastMessage::updateChatLastMessage(int53 chat_id_, object_ptr<message> &&last_message_, array<object_ptr<chatPosition>> &&positions_)
  : chat_id_(chat_id_)
  , last_message_(std::move(last_message_))
  , positions_(std::move(positions_))
{}

const std::int32_t updateChatLastMessage::ID;

void updateChatLastMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatLastMessage");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("last_message", static_cast<const BaseObject *>(last_message_.get()));
    { s.store_vector_begin("positions", positions_.size()); for (const auto &_value : positions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateChatPosition::updateChatPosition()
  : chat_id_()
  , position_()
{}

updateChatPosition::updateChatPosition(int53 chat_id_, object_ptr<chatPosition> &&position_)
  : chat_id_(chat_id_)
  , position_(std::move(position_))
{}

const std::int32_t updateChatPosition::ID;

void updateChatPosition::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatPosition");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("position", static_cast<const BaseObject *>(position_.get()));
    s.store_class_end();
  }
}

updateChatAddedToList::updateChatAddedToList()
  : chat_id_()
  , chat_list_()
{}

updateChatAddedToList::updateChatAddedToList(int53 chat_id_, object_ptr<ChatList> &&chat_list_)
  : chat_id_(chat_id_)
  , chat_list_(std::move(chat_list_))
{}

const std::int32_t updateChatAddedToList::ID;

void updateChatAddedToList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatAddedToList");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("chat_list", static_cast<const BaseObject *>(chat_list_.get()));
    s.store_class_end();
  }
}

updateChatRemovedFromList::updateChatRemovedFromList()
  : chat_id_()
  , chat_list_()
{}

updateChatRemovedFromList::updateChatRemovedFromList(int53 chat_id_, object_ptr<ChatList> &&chat_list_)
  : chat_id_(chat_id_)
  , chat_list_(std::move(chat_list_))
{}

const std::int32_t updateChatRemovedFromList::ID;

void updateChatRemovedFromList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatRemovedFromList");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("chat_list", static_cast<const BaseObject *>(chat_list_.get()));
    s.store_class_end();
  }
}

updateChatReadInbox::updateChatReadInbox()
  : chat_id_()
  , last_read_inbox_message_id_()
  , unread_count_()
{}

updateChatReadInbox::updateChatReadInbox(int53 chat_id_, int53 last_read_inbox_message_id_, int32 unread_count_)
  : chat_id_(chat_id_)
  , last_read_inbox_message_id_(last_read_inbox_message_id_)
  , unread_count_(unread_count_)
{}

const std::int32_t updateChatReadInbox::ID;

void updateChatReadInbox::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatReadInbox");
    s.store_field("chat_id", chat_id_);
    s.store_field("last_read_inbox_message_id", last_read_inbox_message_id_);
    s.store_field("unread_count", unread_count_);
    s.store_class_end();
  }
}

updateChatReadOutbox::updateChatReadOutbox()
  : chat_id_()
  , last_read_outbox_message_id_()
{}

updateChatReadOutbox::updateChatReadOutbox(int53 chat_id_, int53 last_read_outbox_message_id_)
  : chat_id_(chat_id_)
  , last_read_outbox_message_id_(last_read_outbox_message_id_)
{}

const std::int32_t updateChatReadOutbox::ID;

void updateChatReadOutbox::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatReadOutbox");
    s.store_field("chat_id", chat_id_);
    s.store_field("last_read_outbox_message_id", last_read_outbox_message_id_);
    s.store_class_end();
  }
}

updateChatActionBar::updateChatActionBar()
  : chat_id_()
  , action_bar_()
{}

updateChatActionBar::updateChatActionBar(int53 chat_id_, object_ptr<ChatActionBar> &&action_bar_)
  : chat_id_(chat_id_)
  , action_bar_(std::move(action_bar_))
{}

const std::int32_t updateChatActionBar::ID;

void updateChatActionBar::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatActionBar");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("action_bar", static_cast<const BaseObject *>(action_bar_.get()));
    s.store_class_end();
  }
}

updateChatBusinessBotManageBar::updateChatBusinessBotManageBar()
  : chat_id_()
  , business_bot_manage_bar_()
{}

updateChatBusinessBotManageBar::updateChatBusinessBotManageBar(int53 chat_id_, object_ptr<businessBotManageBar> &&business_bot_manage_bar_)
  : chat_id_(chat_id_)
  , business_bot_manage_bar_(std::move(business_bot_manage_bar_))
{}

const std::int32_t updateChatBusinessBotManageBar::ID;

void updateChatBusinessBotManageBar::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatBusinessBotManageBar");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("business_bot_manage_bar", static_cast<const BaseObject *>(business_bot_manage_bar_.get()));
    s.store_class_end();
  }
}

updateChatAvailableReactions::updateChatAvailableReactions()
  : chat_id_()
  , available_reactions_()
{}

updateChatAvailableReactions::updateChatAvailableReactions(int53 chat_id_, object_ptr<ChatAvailableReactions> &&available_reactions_)
  : chat_id_(chat_id_)
  , available_reactions_(std::move(available_reactions_))
{}

const std::int32_t updateChatAvailableReactions::ID;

void updateChatAvailableReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatAvailableReactions");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("available_reactions", static_cast<const BaseObject *>(available_reactions_.get()));
    s.store_class_end();
  }
}

updateChatDraftMessage::updateChatDraftMessage()
  : chat_id_()
  , draft_message_()
  , positions_()
{}

updateChatDraftMessage::updateChatDraftMessage(int53 chat_id_, object_ptr<draftMessage> &&draft_message_, array<object_ptr<chatPosition>> &&positions_)
  : chat_id_(chat_id_)
  , draft_message_(std::move(draft_message_))
  , positions_(std::move(positions_))
{}

const std::int32_t updateChatDraftMessage::ID;

void updateChatDraftMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatDraftMessage");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("draft_message", static_cast<const BaseObject *>(draft_message_.get()));
    { s.store_vector_begin("positions", positions_.size()); for (const auto &_value : positions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateChatEmojiStatus::updateChatEmojiStatus()
  : chat_id_()
  , emoji_status_()
{}

updateChatEmojiStatus::updateChatEmojiStatus(int53 chat_id_, object_ptr<emojiStatus> &&emoji_status_)
  : chat_id_(chat_id_)
  , emoji_status_(std::move(emoji_status_))
{}

const std::int32_t updateChatEmojiStatus::ID;

void updateChatEmojiStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatEmojiStatus");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("emoji_status", static_cast<const BaseObject *>(emoji_status_.get()));
    s.store_class_end();
  }
}

updateChatMessageSender::updateChatMessageSender()
  : chat_id_()
  , message_sender_id_()
{}

updateChatMessageSender::updateChatMessageSender(int53 chat_id_, object_ptr<MessageSender> &&message_sender_id_)
  : chat_id_(chat_id_)
  , message_sender_id_(std::move(message_sender_id_))
{}

const std::int32_t updateChatMessageSender::ID;

void updateChatMessageSender::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatMessageSender");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("message_sender_id", static_cast<const BaseObject *>(message_sender_id_.get()));
    s.store_class_end();
  }
}

updateChatMessageAutoDeleteTime::updateChatMessageAutoDeleteTime()
  : chat_id_()
  , message_auto_delete_time_()
{}

updateChatMessageAutoDeleteTime::updateChatMessageAutoDeleteTime(int53 chat_id_, int32 message_auto_delete_time_)
  : chat_id_(chat_id_)
  , message_auto_delete_time_(message_auto_delete_time_)
{}

const std::int32_t updateChatMessageAutoDeleteTime::ID;

void updateChatMessageAutoDeleteTime::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatMessageAutoDeleteTime");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_auto_delete_time", message_auto_delete_time_);
    s.store_class_end();
  }
}

updateChatNotificationSettings::updateChatNotificationSettings()
  : chat_id_()
  , notification_settings_()
{}

updateChatNotificationSettings::updateChatNotificationSettings(int53 chat_id_, object_ptr<chatNotificationSettings> &&notification_settings_)
  : chat_id_(chat_id_)
  , notification_settings_(std::move(notification_settings_))
{}

const std::int32_t updateChatNotificationSettings::ID;

void updateChatNotificationSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatNotificationSettings");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("notification_settings", static_cast<const BaseObject *>(notification_settings_.get()));
    s.store_class_end();
  }
}

updateChatPendingJoinRequests::updateChatPendingJoinRequests()
  : chat_id_()
  , pending_join_requests_()
{}

updateChatPendingJoinRequests::updateChatPendingJoinRequests(int53 chat_id_, object_ptr<chatJoinRequestsInfo> &&pending_join_requests_)
  : chat_id_(chat_id_)
  , pending_join_requests_(std::move(pending_join_requests_))
{}

const std::int32_t updateChatPendingJoinRequests::ID;

void updateChatPendingJoinRequests::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatPendingJoinRequests");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("pending_join_requests", static_cast<const BaseObject *>(pending_join_requests_.get()));
    s.store_class_end();
  }
}

updateChatReplyMarkup::updateChatReplyMarkup()
  : chat_id_()
  , reply_markup_message_id_()
{}

updateChatReplyMarkup::updateChatReplyMarkup(int53 chat_id_, int53 reply_markup_message_id_)
  : chat_id_(chat_id_)
  , reply_markup_message_id_(reply_markup_message_id_)
{}

const std::int32_t updateChatReplyMarkup::ID;

void updateChatReplyMarkup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatReplyMarkup");
    s.store_field("chat_id", chat_id_);
    s.store_field("reply_markup_message_id", reply_markup_message_id_);
    s.store_class_end();
  }
}

updateChatBackground::updateChatBackground()
  : chat_id_()
  , background_()
{}

updateChatBackground::updateChatBackground(int53 chat_id_, object_ptr<chatBackground> &&background_)
  : chat_id_(chat_id_)
  , background_(std::move(background_))
{}

const std::int32_t updateChatBackground::ID;

void updateChatBackground::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatBackground");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("background", static_cast<const BaseObject *>(background_.get()));
    s.store_class_end();
  }
}

updateChatTheme::updateChatTheme()
  : chat_id_()
  , theme_()
{}

updateChatTheme::updateChatTheme(int53 chat_id_, object_ptr<ChatTheme> &&theme_)
  : chat_id_(chat_id_)
  , theme_(std::move(theme_))
{}

const std::int32_t updateChatTheme::ID;

void updateChatTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatTheme");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("theme", static_cast<const BaseObject *>(theme_.get()));
    s.store_class_end();
  }
}

updateChatUnreadMentionCount::updateChatUnreadMentionCount()
  : chat_id_()
  , unread_mention_count_()
{}

updateChatUnreadMentionCount::updateChatUnreadMentionCount(int53 chat_id_, int32 unread_mention_count_)
  : chat_id_(chat_id_)
  , unread_mention_count_(unread_mention_count_)
{}

const std::int32_t updateChatUnreadMentionCount::ID;

void updateChatUnreadMentionCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatUnreadMentionCount");
    s.store_field("chat_id", chat_id_);
    s.store_field("unread_mention_count", unread_mention_count_);
    s.store_class_end();
  }
}

updateChatUnreadReactionCount::updateChatUnreadReactionCount()
  : chat_id_()
  , unread_reaction_count_()
{}

updateChatUnreadReactionCount::updateChatUnreadReactionCount(int53 chat_id_, int32 unread_reaction_count_)
  : chat_id_(chat_id_)
  , unread_reaction_count_(unread_reaction_count_)
{}

const std::int32_t updateChatUnreadReactionCount::ID;

void updateChatUnreadReactionCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatUnreadReactionCount");
    s.store_field("chat_id", chat_id_);
    s.store_field("unread_reaction_count", unread_reaction_count_);
    s.store_class_end();
  }
}

updateChatVideoChat::updateChatVideoChat()
  : chat_id_()
  , video_chat_()
{}

updateChatVideoChat::updateChatVideoChat(int53 chat_id_, object_ptr<videoChat> &&video_chat_)
  : chat_id_(chat_id_)
  , video_chat_(std::move(video_chat_))
{}

const std::int32_t updateChatVideoChat::ID;

void updateChatVideoChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatVideoChat");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("video_chat", static_cast<const BaseObject *>(video_chat_.get()));
    s.store_class_end();
  }
}

updateChatDefaultDisableNotification::updateChatDefaultDisableNotification()
  : chat_id_()
  , default_disable_notification_()
{}

updateChatDefaultDisableNotification::updateChatDefaultDisableNotification(int53 chat_id_, bool default_disable_notification_)
  : chat_id_(chat_id_)
  , default_disable_notification_(default_disable_notification_)
{}

const std::int32_t updateChatDefaultDisableNotification::ID;

void updateChatDefaultDisableNotification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatDefaultDisableNotification");
    s.store_field("chat_id", chat_id_);
    s.store_field("default_disable_notification", default_disable_notification_);
    s.store_class_end();
  }
}

updateChatHasProtectedContent::updateChatHasProtectedContent()
  : chat_id_()
  , has_protected_content_()
{}

updateChatHasProtectedContent::updateChatHasProtectedContent(int53 chat_id_, bool has_protected_content_)
  : chat_id_(chat_id_)
  , has_protected_content_(has_protected_content_)
{}

const std::int32_t updateChatHasProtectedContent::ID;

void updateChatHasProtectedContent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatHasProtectedContent");
    s.store_field("chat_id", chat_id_);
    s.store_field("has_protected_content", has_protected_content_);
    s.store_class_end();
  }
}

updateChatIsTranslatable::updateChatIsTranslatable()
  : chat_id_()
  , is_translatable_()
{}

updateChatIsTranslatable::updateChatIsTranslatable(int53 chat_id_, bool is_translatable_)
  : chat_id_(chat_id_)
  , is_translatable_(is_translatable_)
{}

const std::int32_t updateChatIsTranslatable::ID;

void updateChatIsTranslatable::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatIsTranslatable");
    s.store_field("chat_id", chat_id_);
    s.store_field("is_translatable", is_translatable_);
    s.store_class_end();
  }
}

updateChatIsMarkedAsUnread::updateChatIsMarkedAsUnread()
  : chat_id_()
  , is_marked_as_unread_()
{}

updateChatIsMarkedAsUnread::updateChatIsMarkedAsUnread(int53 chat_id_, bool is_marked_as_unread_)
  : chat_id_(chat_id_)
  , is_marked_as_unread_(is_marked_as_unread_)
{}

const std::int32_t updateChatIsMarkedAsUnread::ID;

void updateChatIsMarkedAsUnread::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatIsMarkedAsUnread");
    s.store_field("chat_id", chat_id_);
    s.store_field("is_marked_as_unread", is_marked_as_unread_);
    s.store_class_end();
  }
}

updateChatViewAsTopics::updateChatViewAsTopics()
  : chat_id_()
  , view_as_topics_()
{}

updateChatViewAsTopics::updateChatViewAsTopics(int53 chat_id_, bool view_as_topics_)
  : chat_id_(chat_id_)
  , view_as_topics_(view_as_topics_)
{}

const std::int32_t updateChatViewAsTopics::ID;

void updateChatViewAsTopics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatViewAsTopics");
    s.store_field("chat_id", chat_id_);
    s.store_field("view_as_topics", view_as_topics_);
    s.store_class_end();
  }
}

updateChatBlockList::updateChatBlockList()
  : chat_id_()
  , block_list_()
{}

updateChatBlockList::updateChatBlockList(int53 chat_id_, object_ptr<BlockList> &&block_list_)
  : chat_id_(chat_id_)
  , block_list_(std::move(block_list_))
{}

const std::int32_t updateChatBlockList::ID;

void updateChatBlockList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatBlockList");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("block_list", static_cast<const BaseObject *>(block_list_.get()));
    s.store_class_end();
  }
}

updateChatHasScheduledMessages::updateChatHasScheduledMessages()
  : chat_id_()
  , has_scheduled_messages_()
{}

updateChatHasScheduledMessages::updateChatHasScheduledMessages(int53 chat_id_, bool has_scheduled_messages_)
  : chat_id_(chat_id_)
  , has_scheduled_messages_(has_scheduled_messages_)
{}

const std::int32_t updateChatHasScheduledMessages::ID;

void updateChatHasScheduledMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatHasScheduledMessages");
    s.store_field("chat_id", chat_id_);
    s.store_field("has_scheduled_messages", has_scheduled_messages_);
    s.store_class_end();
  }
}

updateChatFolders::updateChatFolders()
  : chat_folders_()
  , main_chat_list_position_()
  , are_tags_enabled_()
{}

updateChatFolders::updateChatFolders(array<object_ptr<chatFolderInfo>> &&chat_folders_, int32 main_chat_list_position_, bool are_tags_enabled_)
  : chat_folders_(std::move(chat_folders_))
  , main_chat_list_position_(main_chat_list_position_)
  , are_tags_enabled_(are_tags_enabled_)
{}

const std::int32_t updateChatFolders::ID;

void updateChatFolders::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatFolders");
    { s.store_vector_begin("chat_folders", chat_folders_.size()); for (const auto &_value : chat_folders_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("main_chat_list_position", main_chat_list_position_);
    s.store_field("are_tags_enabled", are_tags_enabled_);
    s.store_class_end();
  }
}

updateChatOnlineMemberCount::updateChatOnlineMemberCount()
  : chat_id_()
  , online_member_count_()
{}

updateChatOnlineMemberCount::updateChatOnlineMemberCount(int53 chat_id_, int32 online_member_count_)
  : chat_id_(chat_id_)
  , online_member_count_(online_member_count_)
{}

const std::int32_t updateChatOnlineMemberCount::ID;

void updateChatOnlineMemberCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatOnlineMemberCount");
    s.store_field("chat_id", chat_id_);
    s.store_field("online_member_count", online_member_count_);
    s.store_class_end();
  }
}

updateSavedMessagesTopic::updateSavedMessagesTopic()
  : topic_()
{}

updateSavedMessagesTopic::updateSavedMessagesTopic(object_ptr<savedMessagesTopic> &&topic_)
  : topic_(std::move(topic_))
{}

const std::int32_t updateSavedMessagesTopic::ID;

void updateSavedMessagesTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateSavedMessagesTopic");
    s.store_object_field("topic", static_cast<const BaseObject *>(topic_.get()));
    s.store_class_end();
  }
}

updateSavedMessagesTopicCount::updateSavedMessagesTopicCount()
  : topic_count_()
{}

updateSavedMessagesTopicCount::updateSavedMessagesTopicCount(int32 topic_count_)
  : topic_count_(topic_count_)
{}

const std::int32_t updateSavedMessagesTopicCount::ID;

void updateSavedMessagesTopicCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateSavedMessagesTopicCount");
    s.store_field("topic_count", topic_count_);
    s.store_class_end();
  }
}

updateDirectMessagesChatTopic::updateDirectMessagesChatTopic()
  : topic_()
{}

updateDirectMessagesChatTopic::updateDirectMessagesChatTopic(object_ptr<directMessagesChatTopic> &&topic_)
  : topic_(std::move(topic_))
{}

const std::int32_t updateDirectMessagesChatTopic::ID;

void updateDirectMessagesChatTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateDirectMessagesChatTopic");
    s.store_object_field("topic", static_cast<const BaseObject *>(topic_.get()));
    s.store_class_end();
  }
}

updateTopicMessageCount::updateTopicMessageCount()
  : chat_id_()
  , topic_id_()
  , message_count_()
{}

updateTopicMessageCount::updateTopicMessageCount(int53 chat_id_, object_ptr<MessageTopic> &&topic_id_, int32 message_count_)
  : chat_id_(chat_id_)
  , topic_id_(std::move(topic_id_))
  , message_count_(message_count_)
{}

const std::int32_t updateTopicMessageCount::ID;

void updateTopicMessageCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateTopicMessageCount");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("topic_id", static_cast<const BaseObject *>(topic_id_.get()));
    s.store_field("message_count", message_count_);
    s.store_class_end();
  }
}

updateQuickReplyShortcut::updateQuickReplyShortcut()
  : shortcut_()
{}

updateQuickReplyShortcut::updateQuickReplyShortcut(object_ptr<quickReplyShortcut> &&shortcut_)
  : shortcut_(std::move(shortcut_))
{}

const std::int32_t updateQuickReplyShortcut::ID;

void updateQuickReplyShortcut::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateQuickReplyShortcut");
    s.store_object_field("shortcut", static_cast<const BaseObject *>(shortcut_.get()));
    s.store_class_end();
  }
}

updateQuickReplyShortcutDeleted::updateQuickReplyShortcutDeleted()
  : shortcut_id_()
{}

updateQuickReplyShortcutDeleted::updateQuickReplyShortcutDeleted(int32 shortcut_id_)
  : shortcut_id_(shortcut_id_)
{}

const std::int32_t updateQuickReplyShortcutDeleted::ID;

void updateQuickReplyShortcutDeleted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateQuickReplyShortcutDeleted");
    s.store_field("shortcut_id", shortcut_id_);
    s.store_class_end();
  }
}

updateQuickReplyShortcuts::updateQuickReplyShortcuts()
  : shortcut_ids_()
{}

updateQuickReplyShortcuts::updateQuickReplyShortcuts(array<int32> &&shortcut_ids_)
  : shortcut_ids_(std::move(shortcut_ids_))
{}

const std::int32_t updateQuickReplyShortcuts::ID;

void updateQuickReplyShortcuts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateQuickReplyShortcuts");
    { s.store_vector_begin("shortcut_ids", shortcut_ids_.size()); for (const auto &_value : shortcut_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateQuickReplyShortcutMessages::updateQuickReplyShortcutMessages()
  : shortcut_id_()
  , messages_()
{}

updateQuickReplyShortcutMessages::updateQuickReplyShortcutMessages(int32 shortcut_id_, array<object_ptr<quickReplyMessage>> &&messages_)
  : shortcut_id_(shortcut_id_)
  , messages_(std::move(messages_))
{}

const std::int32_t updateQuickReplyShortcutMessages::ID;

void updateQuickReplyShortcutMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateQuickReplyShortcutMessages");
    s.store_field("shortcut_id", shortcut_id_);
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateForumTopicInfo::updateForumTopicInfo()
  : info_()
{}

updateForumTopicInfo::updateForumTopicInfo(object_ptr<forumTopicInfo> &&info_)
  : info_(std::move(info_))
{}

const std::int32_t updateForumTopicInfo::ID;

void updateForumTopicInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateForumTopicInfo");
    s.store_object_field("info", static_cast<const BaseObject *>(info_.get()));
    s.store_class_end();
  }
}

updateForumTopic::updateForumTopic()
  : chat_id_()
  , forum_topic_id_()
  , is_pinned_()
  , last_read_inbox_message_id_()
  , last_read_outbox_message_id_()
  , unread_mention_count_()
  , unread_reaction_count_()
  , notification_settings_()
{}

updateForumTopic::updateForumTopic(int53 chat_id_, int32 forum_topic_id_, bool is_pinned_, int53 last_read_inbox_message_id_, int53 last_read_outbox_message_id_, int32 unread_mention_count_, int32 unread_reaction_count_, object_ptr<chatNotificationSettings> &&notification_settings_)
  : chat_id_(chat_id_)
  , forum_topic_id_(forum_topic_id_)
  , is_pinned_(is_pinned_)
  , last_read_inbox_message_id_(last_read_inbox_message_id_)
  , last_read_outbox_message_id_(last_read_outbox_message_id_)
  , unread_mention_count_(unread_mention_count_)
  , unread_reaction_count_(unread_reaction_count_)
  , notification_settings_(std::move(notification_settings_))
{}

const std::int32_t updateForumTopic::ID;

void updateForumTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateForumTopic");
    s.store_field("chat_id", chat_id_);
    s.store_field("forum_topic_id", forum_topic_id_);
    s.store_field("is_pinned", is_pinned_);
    s.store_field("last_read_inbox_message_id", last_read_inbox_message_id_);
    s.store_field("last_read_outbox_message_id", last_read_outbox_message_id_);
    s.store_field("unread_mention_count", unread_mention_count_);
    s.store_field("unread_reaction_count", unread_reaction_count_);
    s.store_object_field("notification_settings", static_cast<const BaseObject *>(notification_settings_.get()));
    s.store_class_end();
  }
}

updateScopeNotificationSettings::updateScopeNotificationSettings()
  : scope_()
  , notification_settings_()
{}

updateScopeNotificationSettings::updateScopeNotificationSettings(object_ptr<NotificationSettingsScope> &&scope_, object_ptr<scopeNotificationSettings> &&notification_settings_)
  : scope_(std::move(scope_))
  , notification_settings_(std::move(notification_settings_))
{}

const std::int32_t updateScopeNotificationSettings::ID;

void updateScopeNotificationSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateScopeNotificationSettings");
    s.store_object_field("scope", static_cast<const BaseObject *>(scope_.get()));
    s.store_object_field("notification_settings", static_cast<const BaseObject *>(notification_settings_.get()));
    s.store_class_end();
  }
}

updateReactionNotificationSettings::updateReactionNotificationSettings()
  : notification_settings_()
{}

updateReactionNotificationSettings::updateReactionNotificationSettings(object_ptr<reactionNotificationSettings> &&notification_settings_)
  : notification_settings_(std::move(notification_settings_))
{}

const std::int32_t updateReactionNotificationSettings::ID;

void updateReactionNotificationSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateReactionNotificationSettings");
    s.store_object_field("notification_settings", static_cast<const BaseObject *>(notification_settings_.get()));
    s.store_class_end();
  }
}

updateNotification::updateNotification()
  : notification_group_id_()
  , notification_()
{}

updateNotification::updateNotification(int32 notification_group_id_, object_ptr<notification> &&notification_)
  : notification_group_id_(notification_group_id_)
  , notification_(std::move(notification_))
{}

const std::int32_t updateNotification::ID;

void updateNotification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNotification");
    s.store_field("notification_group_id", notification_group_id_);
    s.store_object_field("notification", static_cast<const BaseObject *>(notification_.get()));
    s.store_class_end();
  }
}

updateNotificationGroup::updateNotificationGroup()
  : notification_group_id_()
  , type_()
  , chat_id_()
  , notification_settings_chat_id_()
  , notification_sound_id_()
  , total_count_()
  , added_notifications_()
  , removed_notification_ids_()
{}

updateNotificationGroup::updateNotificationGroup(int32 notification_group_id_, object_ptr<NotificationGroupType> &&type_, int53 chat_id_, int53 notification_settings_chat_id_, int64 notification_sound_id_, int32 total_count_, array<object_ptr<notification>> &&added_notifications_, array<int32> &&removed_notification_ids_)
  : notification_group_id_(notification_group_id_)
  , type_(std::move(type_))
  , chat_id_(chat_id_)
  , notification_settings_chat_id_(notification_settings_chat_id_)
  , notification_sound_id_(notification_sound_id_)
  , total_count_(total_count_)
  , added_notifications_(std::move(added_notifications_))
  , removed_notification_ids_(std::move(removed_notification_ids_))
{}

const std::int32_t updateNotificationGroup::ID;

void updateNotificationGroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNotificationGroup");
    s.store_field("notification_group_id", notification_group_id_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("chat_id", chat_id_);
    s.store_field("notification_settings_chat_id", notification_settings_chat_id_);
    s.store_field("notification_sound_id", notification_sound_id_);
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("added_notifications", added_notifications_.size()); for (const auto &_value : added_notifications_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("removed_notification_ids", removed_notification_ids_.size()); for (const auto &_value : removed_notification_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateActiveNotifications::updateActiveNotifications()
  : groups_()
{}

updateActiveNotifications::updateActiveNotifications(array<object_ptr<notificationGroup>> &&groups_)
  : groups_(std::move(groups_))
{}

const std::int32_t updateActiveNotifications::ID;

void updateActiveNotifications::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateActiveNotifications");
    { s.store_vector_begin("groups", groups_.size()); for (const auto &_value : groups_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateHavePendingNotifications::updateHavePendingNotifications()
  : have_delayed_notifications_()
  , have_unreceived_notifications_()
{}

updateHavePendingNotifications::updateHavePendingNotifications(bool have_delayed_notifications_, bool have_unreceived_notifications_)
  : have_delayed_notifications_(have_delayed_notifications_)
  , have_unreceived_notifications_(have_unreceived_notifications_)
{}

const std::int32_t updateHavePendingNotifications::ID;

void updateHavePendingNotifications::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateHavePendingNotifications");
    s.store_field("have_delayed_notifications", have_delayed_notifications_);
    s.store_field("have_unreceived_notifications", have_unreceived_notifications_);
    s.store_class_end();
  }
}

updateDeleteMessages::updateDeleteMessages()
  : chat_id_()
  , message_ids_()
  , is_permanent_()
  , from_cache_()
{}

updateDeleteMessages::updateDeleteMessages(int53 chat_id_, array<int53> &&message_ids_, bool is_permanent_, bool from_cache_)
  : chat_id_(chat_id_)
  , message_ids_(std::move(message_ids_))
  , is_permanent_(is_permanent_)
  , from_cache_(from_cache_)
{}

const std::int32_t updateDeleteMessages::ID;

void updateDeleteMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateDeleteMessages");
    s.store_field("chat_id", chat_id_);
    { s.store_vector_begin("message_ids", message_ids_.size()); for (const auto &_value : message_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("is_permanent", is_permanent_);
    s.store_field("from_cache", from_cache_);
    s.store_class_end();
  }
}

updateChatAction::updateChatAction()
  : chat_id_()
  , topic_id_()
  , sender_id_()
  , action_()
{}

updateChatAction::updateChatAction(int53 chat_id_, object_ptr<MessageTopic> &&topic_id_, object_ptr<MessageSender> &&sender_id_, object_ptr<ChatAction> &&action_)
  : chat_id_(chat_id_)
  , topic_id_(std::move(topic_id_))
  , sender_id_(std::move(sender_id_))
  , action_(std::move(action_))
{}

const std::int32_t updateChatAction::ID;

void updateChatAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatAction");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("topic_id", static_cast<const BaseObject *>(topic_id_.get()));
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_object_field("action", static_cast<const BaseObject *>(action_.get()));
    s.store_class_end();
  }
}

updatePendingTextMessage::updatePendingTextMessage()
  : chat_id_()
  , forum_topic_id_()
  , draft_id_()
  , text_()
{}

updatePendingTextMessage::updatePendingTextMessage(int53 chat_id_, int32 forum_topic_id_, int64 draft_id_, object_ptr<formattedText> &&text_)
  : chat_id_(chat_id_)
  , forum_topic_id_(forum_topic_id_)
  , draft_id_(draft_id_)
  , text_(std::move(text_))
{}

const std::int32_t updatePendingTextMessage::ID;

void updatePendingTextMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePendingTextMessage");
    s.store_field("chat_id", chat_id_);
    s.store_field("forum_topic_id", forum_topic_id_);
    s.store_field("draft_id", draft_id_);
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

updateUserStatus::updateUserStatus()
  : user_id_()
  , status_()
{}

updateUserStatus::updateUserStatus(int53 user_id_, object_ptr<UserStatus> &&status_)
  : user_id_(user_id_)
  , status_(std::move(status_))
{}

const std::int32_t updateUserStatus::ID;

void updateUserStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateUserStatus");
    s.store_field("user_id", user_id_);
    s.store_object_field("status", static_cast<const BaseObject *>(status_.get()));
    s.store_class_end();
  }
}

updateUser::updateUser()
  : user_()
{}

updateUser::updateUser(object_ptr<user> &&user_)
  : user_(std::move(user_))
{}

const std::int32_t updateUser::ID;

void updateUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateUser");
    s.store_object_field("user", static_cast<const BaseObject *>(user_.get()));
    s.store_class_end();
  }
}

updateBasicGroup::updateBasicGroup()
  : basic_group_()
{}

updateBasicGroup::updateBasicGroup(object_ptr<basicGroup> &&basic_group_)
  : basic_group_(std::move(basic_group_))
{}

const std::int32_t updateBasicGroup::ID;

void updateBasicGroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBasicGroup");
    s.store_object_field("basic_group", static_cast<const BaseObject *>(basic_group_.get()));
    s.store_class_end();
  }
}

updateSupergroup::updateSupergroup()
  : supergroup_()
{}

updateSupergroup::updateSupergroup(object_ptr<supergroup> &&supergroup_)
  : supergroup_(std::move(supergroup_))
{}

const std::int32_t updateSupergroup::ID;

void updateSupergroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateSupergroup");
    s.store_object_field("supergroup", static_cast<const BaseObject *>(supergroup_.get()));
    s.store_class_end();
  }
}

updateSecretChat::updateSecretChat()
  : secret_chat_()
{}

updateSecretChat::updateSecretChat(object_ptr<secretChat> &&secret_chat_)
  : secret_chat_(std::move(secret_chat_))
{}

const std::int32_t updateSecretChat::ID;

void updateSecretChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateSecretChat");
    s.store_object_field("secret_chat", static_cast<const BaseObject *>(secret_chat_.get()));
    s.store_class_end();
  }
}

updateUserFullInfo::updateUserFullInfo()
  : user_id_()
  , user_full_info_()
{}

updateUserFullInfo::updateUserFullInfo(int53 user_id_, object_ptr<userFullInfo> &&user_full_info_)
  : user_id_(user_id_)
  , user_full_info_(std::move(user_full_info_))
{}

const std::int32_t updateUserFullInfo::ID;

void updateUserFullInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateUserFullInfo");
    s.store_field("user_id", user_id_);
    s.store_object_field("user_full_info", static_cast<const BaseObject *>(user_full_info_.get()));
    s.store_class_end();
  }
}

updateBasicGroupFullInfo::updateBasicGroupFullInfo()
  : basic_group_id_()
  , basic_group_full_info_()
{}

updateBasicGroupFullInfo::updateBasicGroupFullInfo(int53 basic_group_id_, object_ptr<basicGroupFullInfo> &&basic_group_full_info_)
  : basic_group_id_(basic_group_id_)
  , basic_group_full_info_(std::move(basic_group_full_info_))
{}

const std::int32_t updateBasicGroupFullInfo::ID;

void updateBasicGroupFullInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBasicGroupFullInfo");
    s.store_field("basic_group_id", basic_group_id_);
    s.store_object_field("basic_group_full_info", static_cast<const BaseObject *>(basic_group_full_info_.get()));
    s.store_class_end();
  }
}

updateSupergroupFullInfo::updateSupergroupFullInfo()
  : supergroup_id_()
  , supergroup_full_info_()
{}

updateSupergroupFullInfo::updateSupergroupFullInfo(int53 supergroup_id_, object_ptr<supergroupFullInfo> &&supergroup_full_info_)
  : supergroup_id_(supergroup_id_)
  , supergroup_full_info_(std::move(supergroup_full_info_))
{}

const std::int32_t updateSupergroupFullInfo::ID;

void updateSupergroupFullInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateSupergroupFullInfo");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_object_field("supergroup_full_info", static_cast<const BaseObject *>(supergroup_full_info_.get()));
    s.store_class_end();
  }
}

updateServiceNotification::updateServiceNotification()
  : type_()
  , content_()
{}

updateServiceNotification::updateServiceNotification(string const &type_, object_ptr<MessageContent> &&content_)
  : type_(type_)
  , content_(std::move(content_))
{}

const std::int32_t updateServiceNotification::ID;

void updateServiceNotification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateServiceNotification");
    s.store_field("type", type_);
    s.store_object_field("content", static_cast<const BaseObject *>(content_.get()));
    s.store_class_end();
  }
}

updateFile::updateFile()
  : file_()
{}

updateFile::updateFile(object_ptr<file> &&file_)
  : file_(std::move(file_))
{}

const std::int32_t updateFile::ID;

void updateFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateFile");
    s.store_object_field("file", static_cast<const BaseObject *>(file_.get()));
    s.store_class_end();
  }
}

updateFileGenerationStart::updateFileGenerationStart()
  : generation_id_()
  , original_path_()
  , destination_path_()
  , conversion_()
{}

updateFileGenerationStart::updateFileGenerationStart(int64 generation_id_, string const &original_path_, string const &destination_path_, string const &conversion_)
  : generation_id_(generation_id_)
  , original_path_(original_path_)
  , destination_path_(destination_path_)
  , conversion_(conversion_)
{}

const std::int32_t updateFileGenerationStart::ID;

void updateFileGenerationStart::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateFileGenerationStart");
    s.store_field("generation_id", generation_id_);
    s.store_field("original_path", original_path_);
    s.store_field("destination_path", destination_path_);
    s.store_field("conversion", conversion_);
    s.store_class_end();
  }
}

updateFileGenerationStop::updateFileGenerationStop()
  : generation_id_()
{}

updateFileGenerationStop::updateFileGenerationStop(int64 generation_id_)
  : generation_id_(generation_id_)
{}

const std::int32_t updateFileGenerationStop::ID;

void updateFileGenerationStop::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateFileGenerationStop");
    s.store_field("generation_id", generation_id_);
    s.store_class_end();
  }
}

updateFileDownloads::updateFileDownloads()
  : total_size_()
  , total_count_()
  , downloaded_size_()
{}

updateFileDownloads::updateFileDownloads(int53 total_size_, int32 total_count_, int53 downloaded_size_)
  : total_size_(total_size_)
  , total_count_(total_count_)
  , downloaded_size_(downloaded_size_)
{}

const std::int32_t updateFileDownloads::ID;

void updateFileDownloads::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateFileDownloads");
    s.store_field("total_size", total_size_);
    s.store_field("total_count", total_count_);
    s.store_field("downloaded_size", downloaded_size_);
    s.store_class_end();
  }
}

updateFileAddedToDownloads::updateFileAddedToDownloads()
  : file_download_()
  , counts_()
{}

updateFileAddedToDownloads::updateFileAddedToDownloads(object_ptr<fileDownload> &&file_download_, object_ptr<downloadedFileCounts> &&counts_)
  : file_download_(std::move(file_download_))
  , counts_(std::move(counts_))
{}

const std::int32_t updateFileAddedToDownloads::ID;

void updateFileAddedToDownloads::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateFileAddedToDownloads");
    s.store_object_field("file_download", static_cast<const BaseObject *>(file_download_.get()));
    s.store_object_field("counts", static_cast<const BaseObject *>(counts_.get()));
    s.store_class_end();
  }
}

updateFileDownload::updateFileDownload()
  : file_id_()
  , complete_date_()
  , is_paused_()
  , counts_()
{}

updateFileDownload::updateFileDownload(int32 file_id_, int32 complete_date_, bool is_paused_, object_ptr<downloadedFileCounts> &&counts_)
  : file_id_(file_id_)
  , complete_date_(complete_date_)
  , is_paused_(is_paused_)
  , counts_(std::move(counts_))
{}

const std::int32_t updateFileDownload::ID;

void updateFileDownload::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateFileDownload");
    s.store_field("file_id", file_id_);
    s.store_field("complete_date", complete_date_);
    s.store_field("is_paused", is_paused_);
    s.store_object_field("counts", static_cast<const BaseObject *>(counts_.get()));
    s.store_class_end();
  }
}

updateFileRemovedFromDownloads::updateFileRemovedFromDownloads()
  : file_id_()
  , counts_()
{}

updateFileRemovedFromDownloads::updateFileRemovedFromDownloads(int32 file_id_, object_ptr<downloadedFileCounts> &&counts_)
  : file_id_(file_id_)
  , counts_(std::move(counts_))
{}

const std::int32_t updateFileRemovedFromDownloads::ID;

void updateFileRemovedFromDownloads::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateFileRemovedFromDownloads");
    s.store_field("file_id", file_id_);
    s.store_object_field("counts", static_cast<const BaseObject *>(counts_.get()));
    s.store_class_end();
  }
}

updateApplicationVerificationRequired::updateApplicationVerificationRequired()
  : verification_id_()
  , nonce_()
  , cloud_project_number_()
{}

updateApplicationVerificationRequired::updateApplicationVerificationRequired(int53 verification_id_, string const &nonce_, int64 cloud_project_number_)
  : verification_id_(verification_id_)
  , nonce_(nonce_)
  , cloud_project_number_(cloud_project_number_)
{}

const std::int32_t updateApplicationVerificationRequired::ID;

void updateApplicationVerificationRequired::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateApplicationVerificationRequired");
    s.store_field("verification_id", verification_id_);
    s.store_field("nonce", nonce_);
    s.store_field("cloud_project_number", cloud_project_number_);
    s.store_class_end();
  }
}

updateApplicationRecaptchaVerificationRequired::updateApplicationRecaptchaVerificationRequired()
  : verification_id_()
  , action_()
  , recaptcha_key_id_()
{}

updateApplicationRecaptchaVerificationRequired::updateApplicationRecaptchaVerificationRequired(int53 verification_id_, string const &action_, string const &recaptcha_key_id_)
  : verification_id_(verification_id_)
  , action_(action_)
  , recaptcha_key_id_(recaptcha_key_id_)
{}

const std::int32_t updateApplicationRecaptchaVerificationRequired::ID;

void updateApplicationRecaptchaVerificationRequired::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateApplicationRecaptchaVerificationRequired");
    s.store_field("verification_id", verification_id_);
    s.store_field("action", action_);
    s.store_field("recaptcha_key_id", recaptcha_key_id_);
    s.store_class_end();
  }
}

updateCall::updateCall()
  : call_()
{}

updateCall::updateCall(object_ptr<call> &&call_)
  : call_(std::move(call_))
{}

const std::int32_t updateCall::ID;

void updateCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateCall");
    s.store_object_field("call", static_cast<const BaseObject *>(call_.get()));
    s.store_class_end();
  }
}

updateGroupCall::updateGroupCall()
  : group_call_()
{}

updateGroupCall::updateGroupCall(object_ptr<groupCall> &&group_call_)
  : group_call_(std::move(group_call_))
{}

const std::int32_t updateGroupCall::ID;

void updateGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateGroupCall");
    s.store_object_field("group_call", static_cast<const BaseObject *>(group_call_.get()));
    s.store_class_end();
  }
}

updateGroupCallParticipant::updateGroupCallParticipant()
  : group_call_id_()
  , participant_()
{}

updateGroupCallParticipant::updateGroupCallParticipant(int32 group_call_id_, object_ptr<groupCallParticipant> &&participant_)
  : group_call_id_(group_call_id_)
  , participant_(std::move(participant_))
{}

const std::int32_t updateGroupCallParticipant::ID;

void updateGroupCallParticipant::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateGroupCallParticipant");
    s.store_field("group_call_id", group_call_id_);
    s.store_object_field("participant", static_cast<const BaseObject *>(participant_.get()));
    s.store_class_end();
  }
}

updateGroupCallParticipants::updateGroupCallParticipants()
  : group_call_id_()
  , participant_user_ids_()
{}

updateGroupCallParticipants::updateGroupCallParticipants(int32 group_call_id_, array<int64> &&participant_user_ids_)
  : group_call_id_(group_call_id_)
  , participant_user_ids_(std::move(participant_user_ids_))
{}

const std::int32_t updateGroupCallParticipants::ID;

void updateGroupCallParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateGroupCallParticipants");
    s.store_field("group_call_id", group_call_id_);
    { s.store_vector_begin("participant_user_ids", participant_user_ids_.size()); for (const auto &_value : participant_user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateGroupCallVerificationState::updateGroupCallVerificationState()
  : group_call_id_()
  , generation_()
  , emojis_()
{}

updateGroupCallVerificationState::updateGroupCallVerificationState(int32 group_call_id_, int32 generation_, array<string> &&emojis_)
  : group_call_id_(group_call_id_)
  , generation_(generation_)
  , emojis_(std::move(emojis_))
{}

const std::int32_t updateGroupCallVerificationState::ID;

void updateGroupCallVerificationState::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateGroupCallVerificationState");
    s.store_field("group_call_id", group_call_id_);
    s.store_field("generation", generation_);
    { s.store_vector_begin("emojis", emojis_.size()); for (const auto &_value : emojis_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateGroupCallNewMessage::updateGroupCallNewMessage()
  : group_call_id_()
  , sender_id_()
  , text_()
{}

updateGroupCallNewMessage::updateGroupCallNewMessage(int32 group_call_id_, object_ptr<MessageSender> &&sender_id_, object_ptr<formattedText> &&text_)
  : group_call_id_(group_call_id_)
  , sender_id_(std::move(sender_id_))
  , text_(std::move(text_))
{}

const std::int32_t updateGroupCallNewMessage::ID;

void updateGroupCallNewMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateGroupCallNewMessage");
    s.store_field("group_call_id", group_call_id_);
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

updateNewCallSignalingData::updateNewCallSignalingData()
  : call_id_()
  , data_()
{}

updateNewCallSignalingData::updateNewCallSignalingData(int32 call_id_, bytes const &data_)
  : call_id_(call_id_)
  , data_(std::move(data_))
{}

const std::int32_t updateNewCallSignalingData::ID;

void updateNewCallSignalingData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewCallSignalingData");
    s.store_field("call_id", call_id_);
    s.store_bytes_field("data", data_);
    s.store_class_end();
  }
}

updateUserPrivacySettingRules::updateUserPrivacySettingRules()
  : setting_()
  , rules_()
{}

updateUserPrivacySettingRules::updateUserPrivacySettingRules(object_ptr<UserPrivacySetting> &&setting_, object_ptr<userPrivacySettingRules> &&rules_)
  : setting_(std::move(setting_))
  , rules_(std::move(rules_))
{}

const std::int32_t updateUserPrivacySettingRules::ID;

void updateUserPrivacySettingRules::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateUserPrivacySettingRules");
    s.store_object_field("setting", static_cast<const BaseObject *>(setting_.get()));
    s.store_object_field("rules", static_cast<const BaseObject *>(rules_.get()));
    s.store_class_end();
  }
}

updateUnreadMessageCount::updateUnreadMessageCount()
  : chat_list_()
  , unread_count_()
  , unread_unmuted_count_()
{}

updateUnreadMessageCount::updateUnreadMessageCount(object_ptr<ChatList> &&chat_list_, int32 unread_count_, int32 unread_unmuted_count_)
  : chat_list_(std::move(chat_list_))
  , unread_count_(unread_count_)
  , unread_unmuted_count_(unread_unmuted_count_)
{}

const std::int32_t updateUnreadMessageCount::ID;

void updateUnreadMessageCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateUnreadMessageCount");
    s.store_object_field("chat_list", static_cast<const BaseObject *>(chat_list_.get()));
    s.store_field("unread_count", unread_count_);
    s.store_field("unread_unmuted_count", unread_unmuted_count_);
    s.store_class_end();
  }
}

updateUnreadChatCount::updateUnreadChatCount()
  : chat_list_()
  , total_count_()
  , unread_count_()
  , unread_unmuted_count_()
  , marked_as_unread_count_()
  , marked_as_unread_unmuted_count_()
{}

updateUnreadChatCount::updateUnreadChatCount(object_ptr<ChatList> &&chat_list_, int32 total_count_, int32 unread_count_, int32 unread_unmuted_count_, int32 marked_as_unread_count_, int32 marked_as_unread_unmuted_count_)
  : chat_list_(std::move(chat_list_))
  , total_count_(total_count_)
  , unread_count_(unread_count_)
  , unread_unmuted_count_(unread_unmuted_count_)
  , marked_as_unread_count_(marked_as_unread_count_)
  , marked_as_unread_unmuted_count_(marked_as_unread_unmuted_count_)
{}

const std::int32_t updateUnreadChatCount::ID;

void updateUnreadChatCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateUnreadChatCount");
    s.store_object_field("chat_list", static_cast<const BaseObject *>(chat_list_.get()));
    s.store_field("total_count", total_count_);
    s.store_field("unread_count", unread_count_);
    s.store_field("unread_unmuted_count", unread_unmuted_count_);
    s.store_field("marked_as_unread_count", marked_as_unread_count_);
    s.store_field("marked_as_unread_unmuted_count", marked_as_unread_unmuted_count_);
    s.store_class_end();
  }
}

updateStory::updateStory()
  : story_()
{}

updateStory::updateStory(object_ptr<story> &&story_)
  : story_(std::move(story_))
{}

const std::int32_t updateStory::ID;

void updateStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateStory");
    s.store_object_field("story", static_cast<const BaseObject *>(story_.get()));
    s.store_class_end();
  }
}

updateStoryDeleted::updateStoryDeleted()
  : story_poster_chat_id_()
  , story_id_()
{}

updateStoryDeleted::updateStoryDeleted(int53 story_poster_chat_id_, int32 story_id_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , story_id_(story_id_)
{}

const std::int32_t updateStoryDeleted::ID;

void updateStoryDeleted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateStoryDeleted");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("story_id", story_id_);
    s.store_class_end();
  }
}

updateStoryPostSucceeded::updateStoryPostSucceeded()
  : story_()
  , old_story_id_()
{}

updateStoryPostSucceeded::updateStoryPostSucceeded(object_ptr<story> &&story_, int32 old_story_id_)
  : story_(std::move(story_))
  , old_story_id_(old_story_id_)
{}

const std::int32_t updateStoryPostSucceeded::ID;

void updateStoryPostSucceeded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateStoryPostSucceeded");
    s.store_object_field("story", static_cast<const BaseObject *>(story_.get()));
    s.store_field("old_story_id", old_story_id_);
    s.store_class_end();
  }
}

updateStoryPostFailed::updateStoryPostFailed()
  : story_()
  , error_()
  , error_type_()
{}

updateStoryPostFailed::updateStoryPostFailed(object_ptr<story> &&story_, object_ptr<error> &&error_, object_ptr<CanPostStoryResult> &&error_type_)
  : story_(std::move(story_))
  , error_(std::move(error_))
  , error_type_(std::move(error_type_))
{}

const std::int32_t updateStoryPostFailed::ID;

void updateStoryPostFailed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateStoryPostFailed");
    s.store_object_field("story", static_cast<const BaseObject *>(story_.get()));
    s.store_object_field("error", static_cast<const BaseObject *>(error_.get()));
    s.store_object_field("error_type", static_cast<const BaseObject *>(error_type_.get()));
    s.store_class_end();
  }
}

updateChatActiveStories::updateChatActiveStories()
  : active_stories_()
{}

updateChatActiveStories::updateChatActiveStories(object_ptr<chatActiveStories> &&active_stories_)
  : active_stories_(std::move(active_stories_))
{}

const std::int32_t updateChatActiveStories::ID;

void updateChatActiveStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatActiveStories");
    s.store_object_field("active_stories", static_cast<const BaseObject *>(active_stories_.get()));
    s.store_class_end();
  }
}

updateStoryListChatCount::updateStoryListChatCount()
  : story_list_()
  , chat_count_()
{}

updateStoryListChatCount::updateStoryListChatCount(object_ptr<StoryList> &&story_list_, int32 chat_count_)
  : story_list_(std::move(story_list_))
  , chat_count_(chat_count_)
{}

const std::int32_t updateStoryListChatCount::ID;

void updateStoryListChatCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateStoryListChatCount");
    s.store_object_field("story_list", static_cast<const BaseObject *>(story_list_.get()));
    s.store_field("chat_count", chat_count_);
    s.store_class_end();
  }
}

updateStoryStealthMode::updateStoryStealthMode()
  : active_until_date_()
  , cooldown_until_date_()
{}

updateStoryStealthMode::updateStoryStealthMode(int32 active_until_date_, int32 cooldown_until_date_)
  : active_until_date_(active_until_date_)
  , cooldown_until_date_(cooldown_until_date_)
{}

const std::int32_t updateStoryStealthMode::ID;

void updateStoryStealthMode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateStoryStealthMode");
    s.store_field("active_until_date", active_until_date_);
    s.store_field("cooldown_until_date", cooldown_until_date_);
    s.store_class_end();
  }
}

updateOption::updateOption()
  : name_()
  , value_()
{}

updateOption::updateOption(string const &name_, object_ptr<OptionValue> &&value_)
  : name_(name_)
  , value_(std::move(value_))
{}

const std::int32_t updateOption::ID;

void updateOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateOption");
    s.store_field("name", name_);
    s.store_object_field("value", static_cast<const BaseObject *>(value_.get()));
    s.store_class_end();
  }
}

updateStickerSet::updateStickerSet()
  : sticker_set_()
{}

updateStickerSet::updateStickerSet(object_ptr<stickerSet> &&sticker_set_)
  : sticker_set_(std::move(sticker_set_))
{}

const std::int32_t updateStickerSet::ID;

void updateStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateStickerSet");
    s.store_object_field("sticker_set", static_cast<const BaseObject *>(sticker_set_.get()));
    s.store_class_end();
  }
}

updateInstalledStickerSets::updateInstalledStickerSets()
  : sticker_type_()
  , sticker_set_ids_()
{}

updateInstalledStickerSets::updateInstalledStickerSets(object_ptr<StickerType> &&sticker_type_, array<int64> &&sticker_set_ids_)
  : sticker_type_(std::move(sticker_type_))
  , sticker_set_ids_(std::move(sticker_set_ids_))
{}

const std::int32_t updateInstalledStickerSets::ID;

void updateInstalledStickerSets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateInstalledStickerSets");
    s.store_object_field("sticker_type", static_cast<const BaseObject *>(sticker_type_.get()));
    { s.store_vector_begin("sticker_set_ids", sticker_set_ids_.size()); for (const auto &_value : sticker_set_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateTrendingStickerSets::updateTrendingStickerSets()
  : sticker_type_()
  , sticker_sets_()
{}

updateTrendingStickerSets::updateTrendingStickerSets(object_ptr<StickerType> &&sticker_type_, object_ptr<trendingStickerSets> &&sticker_sets_)
  : sticker_type_(std::move(sticker_type_))
  , sticker_sets_(std::move(sticker_sets_))
{}

const std::int32_t updateTrendingStickerSets::ID;

void updateTrendingStickerSets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateTrendingStickerSets");
    s.store_object_field("sticker_type", static_cast<const BaseObject *>(sticker_type_.get()));
    s.store_object_field("sticker_sets", static_cast<const BaseObject *>(sticker_sets_.get()));
    s.store_class_end();
  }
}

updateRecentStickers::updateRecentStickers()
  : is_attached_()
  , sticker_ids_()
{}

updateRecentStickers::updateRecentStickers(bool is_attached_, array<int32> &&sticker_ids_)
  : is_attached_(is_attached_)
  , sticker_ids_(std::move(sticker_ids_))
{}

const std::int32_t updateRecentStickers::ID;

void updateRecentStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateRecentStickers");
    s.store_field("is_attached", is_attached_);
    { s.store_vector_begin("sticker_ids", sticker_ids_.size()); for (const auto &_value : sticker_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateFavoriteStickers::updateFavoriteStickers()
  : sticker_ids_()
{}

updateFavoriteStickers::updateFavoriteStickers(array<int32> &&sticker_ids_)
  : sticker_ids_(std::move(sticker_ids_))
{}

const std::int32_t updateFavoriteStickers::ID;

void updateFavoriteStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateFavoriteStickers");
    { s.store_vector_begin("sticker_ids", sticker_ids_.size()); for (const auto &_value : sticker_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateSavedAnimations::updateSavedAnimations()
  : animation_ids_()
{}

updateSavedAnimations::updateSavedAnimations(array<int32> &&animation_ids_)
  : animation_ids_(std::move(animation_ids_))
{}

const std::int32_t updateSavedAnimations::ID;

void updateSavedAnimations::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateSavedAnimations");
    { s.store_vector_begin("animation_ids", animation_ids_.size()); for (const auto &_value : animation_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateSavedNotificationSounds::updateSavedNotificationSounds()
  : notification_sound_ids_()
{}

updateSavedNotificationSounds::updateSavedNotificationSounds(array<int64> &&notification_sound_ids_)
  : notification_sound_ids_(std::move(notification_sound_ids_))
{}

const std::int32_t updateSavedNotificationSounds::ID;

void updateSavedNotificationSounds::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateSavedNotificationSounds");
    { s.store_vector_begin("notification_sound_ids", notification_sound_ids_.size()); for (const auto &_value : notification_sound_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateDefaultBackground::updateDefaultBackground()
  : for_dark_theme_()
  , background_()
{}

updateDefaultBackground::updateDefaultBackground(bool for_dark_theme_, object_ptr<background> &&background_)
  : for_dark_theme_(for_dark_theme_)
  , background_(std::move(background_))
{}

const std::int32_t updateDefaultBackground::ID;

void updateDefaultBackground::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateDefaultBackground");
    s.store_field("for_dark_theme", for_dark_theme_);
    s.store_object_field("background", static_cast<const BaseObject *>(background_.get()));
    s.store_class_end();
  }
}

updateEmojiChatThemes::updateEmojiChatThemes()
  : chat_themes_()
{}

updateEmojiChatThemes::updateEmojiChatThemes(array<object_ptr<emojiChatTheme>> &&chat_themes_)
  : chat_themes_(std::move(chat_themes_))
{}

const std::int32_t updateEmojiChatThemes::ID;

void updateEmojiChatThemes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateEmojiChatThemes");
    { s.store_vector_begin("chat_themes", chat_themes_.size()); for (const auto &_value : chat_themes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateAccentColors::updateAccentColors()
  : colors_()
  , available_accent_color_ids_()
{}

updateAccentColors::updateAccentColors(array<object_ptr<accentColor>> &&colors_, array<int32> &&available_accent_color_ids_)
  : colors_(std::move(colors_))
  , available_accent_color_ids_(std::move(available_accent_color_ids_))
{}

const std::int32_t updateAccentColors::ID;

void updateAccentColors::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateAccentColors");
    { s.store_vector_begin("colors", colors_.size()); for (const auto &_value : colors_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("available_accent_color_ids", available_accent_color_ids_.size()); for (const auto &_value : available_accent_color_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateProfileAccentColors::updateProfileAccentColors()
  : colors_()
  , available_accent_color_ids_()
{}

updateProfileAccentColors::updateProfileAccentColors(array<object_ptr<profileAccentColor>> &&colors_, array<int32> &&available_accent_color_ids_)
  : colors_(std::move(colors_))
  , available_accent_color_ids_(std::move(available_accent_color_ids_))
{}

const std::int32_t updateProfileAccentColors::ID;

void updateProfileAccentColors::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateProfileAccentColors");
    { s.store_vector_begin("colors", colors_.size()); for (const auto &_value : colors_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("available_accent_color_ids", available_accent_color_ids_.size()); for (const auto &_value : available_accent_color_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateLanguagePackStrings::updateLanguagePackStrings()
  : localization_target_()
  , language_pack_id_()
  , strings_()
{}

updateLanguagePackStrings::updateLanguagePackStrings(string const &localization_target_, string const &language_pack_id_, array<object_ptr<languagePackString>> &&strings_)
  : localization_target_(localization_target_)
  , language_pack_id_(language_pack_id_)
  , strings_(std::move(strings_))
{}

const std::int32_t updateLanguagePackStrings::ID;

void updateLanguagePackStrings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateLanguagePackStrings");
    s.store_field("localization_target", localization_target_);
    s.store_field("language_pack_id", language_pack_id_);
    { s.store_vector_begin("strings", strings_.size()); for (const auto &_value : strings_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateConnectionState::updateConnectionState()
  : state_()
{}

updateConnectionState::updateConnectionState(object_ptr<ConnectionState> &&state_)
  : state_(std::move(state_))
{}

const std::int32_t updateConnectionState::ID;

void updateConnectionState::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateConnectionState");
    s.store_object_field("state", static_cast<const BaseObject *>(state_.get()));
    s.store_class_end();
  }
}

updateFreezeState::updateFreezeState()
  : is_frozen_()
  , freezing_date_()
  , deletion_date_()
  , appeal_link_()
{}

updateFreezeState::updateFreezeState(bool is_frozen_, int32 freezing_date_, int32 deletion_date_, string const &appeal_link_)
  : is_frozen_(is_frozen_)
  , freezing_date_(freezing_date_)
  , deletion_date_(deletion_date_)
  , appeal_link_(appeal_link_)
{}

const std::int32_t updateFreezeState::ID;

void updateFreezeState::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateFreezeState");
    s.store_field("is_frozen", is_frozen_);
    s.store_field("freezing_date", freezing_date_);
    s.store_field("deletion_date", deletion_date_);
    s.store_field("appeal_link", appeal_link_);
    s.store_class_end();
  }
}

updateAgeVerificationParameters::updateAgeVerificationParameters()
  : parameters_()
{}

updateAgeVerificationParameters::updateAgeVerificationParameters(object_ptr<ageVerificationParameters> &&parameters_)
  : parameters_(std::move(parameters_))
{}

const std::int32_t updateAgeVerificationParameters::ID;

void updateAgeVerificationParameters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateAgeVerificationParameters");
    s.store_object_field("parameters", static_cast<const BaseObject *>(parameters_.get()));
    s.store_class_end();
  }
}

updateTermsOfService::updateTermsOfService()
  : terms_of_service_id_()
  , terms_of_service_()
{}

updateTermsOfService::updateTermsOfService(string const &terms_of_service_id_, object_ptr<termsOfService> &&terms_of_service_)
  : terms_of_service_id_(terms_of_service_id_)
  , terms_of_service_(std::move(terms_of_service_))
{}

const std::int32_t updateTermsOfService::ID;

void updateTermsOfService::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateTermsOfService");
    s.store_field("terms_of_service_id", terms_of_service_id_);
    s.store_object_field("terms_of_service", static_cast<const BaseObject *>(terms_of_service_.get()));
    s.store_class_end();
  }
}

updateUnconfirmedSession::updateUnconfirmedSession()
  : session_()
{}

updateUnconfirmedSession::updateUnconfirmedSession(object_ptr<unconfirmedSession> &&session_)
  : session_(std::move(session_))
{}

const std::int32_t updateUnconfirmedSession::ID;

void updateUnconfirmedSession::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateUnconfirmedSession");
    s.store_object_field("session", static_cast<const BaseObject *>(session_.get()));
    s.store_class_end();
  }
}

updateAttachmentMenuBots::updateAttachmentMenuBots()
  : bots_()
{}

updateAttachmentMenuBots::updateAttachmentMenuBots(array<object_ptr<attachmentMenuBot>> &&bots_)
  : bots_(std::move(bots_))
{}

const std::int32_t updateAttachmentMenuBots::ID;

void updateAttachmentMenuBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateAttachmentMenuBots");
    { s.store_vector_begin("bots", bots_.size()); for (const auto &_value : bots_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateWebAppMessageSent::updateWebAppMessageSent()
  : web_app_launch_id_()
{}

updateWebAppMessageSent::updateWebAppMessageSent(int64 web_app_launch_id_)
  : web_app_launch_id_(web_app_launch_id_)
{}

const std::int32_t updateWebAppMessageSent::ID;

void updateWebAppMessageSent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateWebAppMessageSent");
    s.store_field("web_app_launch_id", web_app_launch_id_);
    s.store_class_end();
  }
}

updateActiveEmojiReactions::updateActiveEmojiReactions()
  : emojis_()
{}

updateActiveEmojiReactions::updateActiveEmojiReactions(array<string> &&emojis_)
  : emojis_(std::move(emojis_))
{}

const std::int32_t updateActiveEmojiReactions::ID;

void updateActiveEmojiReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateActiveEmojiReactions");
    { s.store_vector_begin("emojis", emojis_.size()); for (const auto &_value : emojis_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateAvailableMessageEffects::updateAvailableMessageEffects()
  : reaction_effect_ids_()
  , sticker_effect_ids_()
{}

updateAvailableMessageEffects::updateAvailableMessageEffects(array<int64> &&reaction_effect_ids_, array<int64> &&sticker_effect_ids_)
  : reaction_effect_ids_(std::move(reaction_effect_ids_))
  , sticker_effect_ids_(std::move(sticker_effect_ids_))
{}

const std::int32_t updateAvailableMessageEffects::ID;

void updateAvailableMessageEffects::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateAvailableMessageEffects");
    { s.store_vector_begin("reaction_effect_ids", reaction_effect_ids_.size()); for (const auto &_value : reaction_effect_ids_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("sticker_effect_ids", sticker_effect_ids_.size()); for (const auto &_value : sticker_effect_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateDefaultReactionType::updateDefaultReactionType()
  : reaction_type_()
{}

updateDefaultReactionType::updateDefaultReactionType(object_ptr<ReactionType> &&reaction_type_)
  : reaction_type_(std::move(reaction_type_))
{}

const std::int32_t updateDefaultReactionType::ID;

void updateDefaultReactionType::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateDefaultReactionType");
    s.store_object_field("reaction_type", static_cast<const BaseObject *>(reaction_type_.get()));
    s.store_class_end();
  }
}

updateDefaultPaidReactionType::updateDefaultPaidReactionType()
  : type_()
{}

updateDefaultPaidReactionType::updateDefaultPaidReactionType(object_ptr<PaidReactionType> &&type_)
  : type_(std::move(type_))
{}

const std::int32_t updateDefaultPaidReactionType::ID;

void updateDefaultPaidReactionType::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateDefaultPaidReactionType");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

updateSavedMessagesTags::updateSavedMessagesTags()
  : saved_messages_topic_id_()
  , tags_()
{}

updateSavedMessagesTags::updateSavedMessagesTags(int53 saved_messages_topic_id_, object_ptr<savedMessagesTags> &&tags_)
  : saved_messages_topic_id_(saved_messages_topic_id_)
  , tags_(std::move(tags_))
{}

const std::int32_t updateSavedMessagesTags::ID;

void updateSavedMessagesTags::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateSavedMessagesTags");
    s.store_field("saved_messages_topic_id", saved_messages_topic_id_);
    s.store_object_field("tags", static_cast<const BaseObject *>(tags_.get()));
    s.store_class_end();
  }
}

updateActiveLiveLocationMessages::updateActiveLiveLocationMessages()
  : messages_()
{}

updateActiveLiveLocationMessages::updateActiveLiveLocationMessages(array<object_ptr<message>> &&messages_)
  : messages_(std::move(messages_))
{}

const std::int32_t updateActiveLiveLocationMessages::ID;

void updateActiveLiveLocationMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateActiveLiveLocationMessages");
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateOwnedStarCount::updateOwnedStarCount()
  : star_amount_()
{}

updateOwnedStarCount::updateOwnedStarCount(object_ptr<starAmount> &&star_amount_)
  : star_amount_(std::move(star_amount_))
{}

const std::int32_t updateOwnedStarCount::ID;

void updateOwnedStarCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateOwnedStarCount");
    s.store_object_field("star_amount", static_cast<const BaseObject *>(star_amount_.get()));
    s.store_class_end();
  }
}

updateOwnedTonCount::updateOwnedTonCount()
  : ton_amount_()
{}

updateOwnedTonCount::updateOwnedTonCount(int53 ton_amount_)
  : ton_amount_(ton_amount_)
{}

const std::int32_t updateOwnedTonCount::ID;

void updateOwnedTonCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateOwnedTonCount");
    s.store_field("ton_amount", ton_amount_);
    s.store_class_end();
  }
}

updateChatRevenueAmount::updateChatRevenueAmount()
  : chat_id_()
  , revenue_amount_()
{}

updateChatRevenueAmount::updateChatRevenueAmount(int53 chat_id_, object_ptr<chatRevenueAmount> &&revenue_amount_)
  : chat_id_(chat_id_)
  , revenue_amount_(std::move(revenue_amount_))
{}

const std::int32_t updateChatRevenueAmount::ID;

void updateChatRevenueAmount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatRevenueAmount");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("revenue_amount", static_cast<const BaseObject *>(revenue_amount_.get()));
    s.store_class_end();
  }
}

updateStarRevenueStatus::updateStarRevenueStatus()
  : owner_id_()
  , status_()
{}

updateStarRevenueStatus::updateStarRevenueStatus(object_ptr<MessageSender> &&owner_id_, object_ptr<starRevenueStatus> &&status_)
  : owner_id_(std::move(owner_id_))
  , status_(std::move(status_))
{}

const std::int32_t updateStarRevenueStatus::ID;

void updateStarRevenueStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateStarRevenueStatus");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_object_field("status", static_cast<const BaseObject *>(status_.get()));
    s.store_class_end();
  }
}

updateTonRevenueStatus::updateTonRevenueStatus()
  : status_()
{}

updateTonRevenueStatus::updateTonRevenueStatus(object_ptr<tonRevenueStatus> &&status_)
  : status_(std::move(status_))
{}

const std::int32_t updateTonRevenueStatus::ID;

void updateTonRevenueStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateTonRevenueStatus");
    s.store_object_field("status", static_cast<const BaseObject *>(status_.get()));
    s.store_class_end();
  }
}

updateSpeechRecognitionTrial::updateSpeechRecognitionTrial()
  : max_media_duration_()
  , weekly_count_()
  , left_count_()
  , next_reset_date_()
{}

updateSpeechRecognitionTrial::updateSpeechRecognitionTrial(int32 max_media_duration_, int32 weekly_count_, int32 left_count_, int32 next_reset_date_)
  : max_media_duration_(max_media_duration_)
  , weekly_count_(weekly_count_)
  , left_count_(left_count_)
  , next_reset_date_(next_reset_date_)
{}

const std::int32_t updateSpeechRecognitionTrial::ID;

void updateSpeechRecognitionTrial::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateSpeechRecognitionTrial");
    s.store_field("max_media_duration", max_media_duration_);
    s.store_field("weekly_count", weekly_count_);
    s.store_field("left_count", left_count_);
    s.store_field("next_reset_date", next_reset_date_);
    s.store_class_end();
  }
}

updateDiceEmojis::updateDiceEmojis()
  : emojis_()
{}

updateDiceEmojis::updateDiceEmojis(array<string> &&emojis_)
  : emojis_(std::move(emojis_))
{}

const std::int32_t updateDiceEmojis::ID;

void updateDiceEmojis::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateDiceEmojis");
    { s.store_vector_begin("emojis", emojis_.size()); for (const auto &_value : emojis_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateAnimatedEmojiMessageClicked::updateAnimatedEmojiMessageClicked()
  : chat_id_()
  , message_id_()
  , sticker_()
{}

updateAnimatedEmojiMessageClicked::updateAnimatedEmojiMessageClicked(int53 chat_id_, int53 message_id_, object_ptr<sticker> &&sticker_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , sticker_(std::move(sticker_))
{}

const std::int32_t updateAnimatedEmojiMessageClicked::ID;

void updateAnimatedEmojiMessageClicked::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateAnimatedEmojiMessageClicked");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

updateAnimationSearchParameters::updateAnimationSearchParameters()
  : provider_()
  , emojis_()
{}

updateAnimationSearchParameters::updateAnimationSearchParameters(string const &provider_, array<string> &&emojis_)
  : provider_(provider_)
  , emojis_(std::move(emojis_))
{}

const std::int32_t updateAnimationSearchParameters::ID;

void updateAnimationSearchParameters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateAnimationSearchParameters");
    s.store_field("provider", provider_);
    { s.store_vector_begin("emojis", emojis_.size()); for (const auto &_value : emojis_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateSuggestedActions::updateSuggestedActions()
  : added_actions_()
  , removed_actions_()
{}

updateSuggestedActions::updateSuggestedActions(array<object_ptr<SuggestedAction>> &&added_actions_, array<object_ptr<SuggestedAction>> &&removed_actions_)
  : added_actions_(std::move(added_actions_))
  , removed_actions_(std::move(removed_actions_))
{}

const std::int32_t updateSuggestedActions::ID;

void updateSuggestedActions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateSuggestedActions");
    { s.store_vector_begin("added_actions", added_actions_.size()); for (const auto &_value : added_actions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("removed_actions", removed_actions_.size()); for (const auto &_value : removed_actions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateSpeedLimitNotification::updateSpeedLimitNotification()
  : is_upload_()
{}

updateSpeedLimitNotification::updateSpeedLimitNotification(bool is_upload_)
  : is_upload_(is_upload_)
{}

const std::int32_t updateSpeedLimitNotification::ID;

void updateSpeedLimitNotification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateSpeedLimitNotification");
    s.store_field("is_upload", is_upload_);
    s.store_class_end();
  }
}

updateContactCloseBirthdays::updateContactCloseBirthdays()
  : close_birthday_users_()
{}

updateContactCloseBirthdays::updateContactCloseBirthdays(array<object_ptr<closeBirthdayUser>> &&close_birthday_users_)
  : close_birthday_users_(std::move(close_birthday_users_))
{}

const std::int32_t updateContactCloseBirthdays::ID;

void updateContactCloseBirthdays::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateContactCloseBirthdays");
    { s.store_vector_begin("close_birthday_users", close_birthday_users_.size()); for (const auto &_value : close_birthday_users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateAutosaveSettings::updateAutosaveSettings()
  : scope_()
  , settings_()
{}

updateAutosaveSettings::updateAutosaveSettings(object_ptr<AutosaveSettingsScope> &&scope_, object_ptr<scopeAutosaveSettings> &&settings_)
  : scope_(std::move(scope_))
  , settings_(std::move(settings_))
{}

const std::int32_t updateAutosaveSettings::ID;

void updateAutosaveSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateAutosaveSettings");
    s.store_object_field("scope", static_cast<const BaseObject *>(scope_.get()));
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

updateBusinessConnection::updateBusinessConnection()
  : connection_()
{}

updateBusinessConnection::updateBusinessConnection(object_ptr<businessConnection> &&connection_)
  : connection_(std::move(connection_))
{}

const std::int32_t updateBusinessConnection::ID;

void updateBusinessConnection::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBusinessConnection");
    s.store_object_field("connection", static_cast<const BaseObject *>(connection_.get()));
    s.store_class_end();
  }
}

updateNewBusinessMessage::updateNewBusinessMessage()
  : connection_id_()
  , message_()
{}

updateNewBusinessMessage::updateNewBusinessMessage(string const &connection_id_, object_ptr<businessMessage> &&message_)
  : connection_id_(connection_id_)
  , message_(std::move(message_))
{}

const std::int32_t updateNewBusinessMessage::ID;

void updateNewBusinessMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewBusinessMessage");
    s.store_field("connection_id", connection_id_);
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

updateBusinessMessageEdited::updateBusinessMessageEdited()
  : connection_id_()
  , message_()
{}

updateBusinessMessageEdited::updateBusinessMessageEdited(string const &connection_id_, object_ptr<businessMessage> &&message_)
  : connection_id_(connection_id_)
  , message_(std::move(message_))
{}

const std::int32_t updateBusinessMessageEdited::ID;

void updateBusinessMessageEdited::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBusinessMessageEdited");
    s.store_field("connection_id", connection_id_);
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

updateBusinessMessagesDeleted::updateBusinessMessagesDeleted()
  : connection_id_()
  , chat_id_()
  , message_ids_()
{}

updateBusinessMessagesDeleted::updateBusinessMessagesDeleted(string const &connection_id_, int53 chat_id_, array<int53> &&message_ids_)
  : connection_id_(connection_id_)
  , chat_id_(chat_id_)
  , message_ids_(std::move(message_ids_))
{}

const std::int32_t updateBusinessMessagesDeleted::ID;

void updateBusinessMessagesDeleted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateBusinessMessagesDeleted");
    s.store_field("connection_id", connection_id_);
    s.store_field("chat_id", chat_id_);
    { s.store_vector_begin("message_ids", message_ids_.size()); for (const auto &_value : message_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateNewInlineQuery::updateNewInlineQuery()
  : id_()
  , sender_user_id_()
  , user_location_()
  , chat_type_()
  , query_()
  , offset_()
{}

updateNewInlineQuery::updateNewInlineQuery(int64 id_, int53 sender_user_id_, object_ptr<location> &&user_location_, object_ptr<ChatType> &&chat_type_, string const &query_, string const &offset_)
  : id_(id_)
  , sender_user_id_(sender_user_id_)
  , user_location_(std::move(user_location_))
  , chat_type_(std::move(chat_type_))
  , query_(query_)
  , offset_(offset_)
{}

const std::int32_t updateNewInlineQuery::ID;

void updateNewInlineQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewInlineQuery");
    s.store_field("id", id_);
    s.store_field("sender_user_id", sender_user_id_);
    s.store_object_field("user_location", static_cast<const BaseObject *>(user_location_.get()));
    s.store_object_field("chat_type", static_cast<const BaseObject *>(chat_type_.get()));
    s.store_field("query", query_);
    s.store_field("offset", offset_);
    s.store_class_end();
  }
}

updateNewChosenInlineResult::updateNewChosenInlineResult()
  : sender_user_id_()
  , user_location_()
  , query_()
  , result_id_()
  , inline_message_id_()
{}

updateNewChosenInlineResult::updateNewChosenInlineResult(int53 sender_user_id_, object_ptr<location> &&user_location_, string const &query_, string const &result_id_, string const &inline_message_id_)
  : sender_user_id_(sender_user_id_)
  , user_location_(std::move(user_location_))
  , query_(query_)
  , result_id_(result_id_)
  , inline_message_id_(inline_message_id_)
{}

const std::int32_t updateNewChosenInlineResult::ID;

void updateNewChosenInlineResult::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewChosenInlineResult");
    s.store_field("sender_user_id", sender_user_id_);
    s.store_object_field("user_location", static_cast<const BaseObject *>(user_location_.get()));
    s.store_field("query", query_);
    s.store_field("result_id", result_id_);
    s.store_field("inline_message_id", inline_message_id_);
    s.store_class_end();
  }
}

updateNewCallbackQuery::updateNewCallbackQuery()
  : id_()
  , sender_user_id_()
  , chat_id_()
  , message_id_()
  , chat_instance_()
  , payload_()
{}

updateNewCallbackQuery::updateNewCallbackQuery(int64 id_, int53 sender_user_id_, int53 chat_id_, int53 message_id_, int64 chat_instance_, object_ptr<CallbackQueryPayload> &&payload_)
  : id_(id_)
  , sender_user_id_(sender_user_id_)
  , chat_id_(chat_id_)
  , message_id_(message_id_)
  , chat_instance_(chat_instance_)
  , payload_(std::move(payload_))
{}

const std::int32_t updateNewCallbackQuery::ID;

void updateNewCallbackQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewCallbackQuery");
    s.store_field("id", id_);
    s.store_field("sender_user_id", sender_user_id_);
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("chat_instance", chat_instance_);
    s.store_object_field("payload", static_cast<const BaseObject *>(payload_.get()));
    s.store_class_end();
  }
}

updateNewInlineCallbackQuery::updateNewInlineCallbackQuery()
  : id_()
  , sender_user_id_()
  , inline_message_id_()
  , chat_instance_()
  , payload_()
{}

updateNewInlineCallbackQuery::updateNewInlineCallbackQuery(int64 id_, int53 sender_user_id_, string const &inline_message_id_, int64 chat_instance_, object_ptr<CallbackQueryPayload> &&payload_)
  : id_(id_)
  , sender_user_id_(sender_user_id_)
  , inline_message_id_(inline_message_id_)
  , chat_instance_(chat_instance_)
  , payload_(std::move(payload_))
{}

const std::int32_t updateNewInlineCallbackQuery::ID;

void updateNewInlineCallbackQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewInlineCallbackQuery");
    s.store_field("id", id_);
    s.store_field("sender_user_id", sender_user_id_);
    s.store_field("inline_message_id", inline_message_id_);
    s.store_field("chat_instance", chat_instance_);
    s.store_object_field("payload", static_cast<const BaseObject *>(payload_.get()));
    s.store_class_end();
  }
}

updateNewBusinessCallbackQuery::updateNewBusinessCallbackQuery()
  : id_()
  , sender_user_id_()
  , connection_id_()
  , message_()
  , chat_instance_()
  , payload_()
{}

updateNewBusinessCallbackQuery::updateNewBusinessCallbackQuery(int64 id_, int53 sender_user_id_, string const &connection_id_, object_ptr<businessMessage> &&message_, int64 chat_instance_, object_ptr<CallbackQueryPayload> &&payload_)
  : id_(id_)
  , sender_user_id_(sender_user_id_)
  , connection_id_(connection_id_)
  , message_(std::move(message_))
  , chat_instance_(chat_instance_)
  , payload_(std::move(payload_))
{}

const std::int32_t updateNewBusinessCallbackQuery::ID;

void updateNewBusinessCallbackQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewBusinessCallbackQuery");
    s.store_field("id", id_);
    s.store_field("sender_user_id", sender_user_id_);
    s.store_field("connection_id", connection_id_);
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_field("chat_instance", chat_instance_);
    s.store_object_field("payload", static_cast<const BaseObject *>(payload_.get()));
    s.store_class_end();
  }
}

updateNewShippingQuery::updateNewShippingQuery()
  : id_()
  , sender_user_id_()
  , invoice_payload_()
  , shipping_address_()
{}

updateNewShippingQuery::updateNewShippingQuery(int64 id_, int53 sender_user_id_, string const &invoice_payload_, object_ptr<address> &&shipping_address_)
  : id_(id_)
  , sender_user_id_(sender_user_id_)
  , invoice_payload_(invoice_payload_)
  , shipping_address_(std::move(shipping_address_))
{}

const std::int32_t updateNewShippingQuery::ID;

void updateNewShippingQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewShippingQuery");
    s.store_field("id", id_);
    s.store_field("sender_user_id", sender_user_id_);
    s.store_field("invoice_payload", invoice_payload_);
    s.store_object_field("shipping_address", static_cast<const BaseObject *>(shipping_address_.get()));
    s.store_class_end();
  }
}

updateNewPreCheckoutQuery::updateNewPreCheckoutQuery()
  : id_()
  , sender_user_id_()
  , currency_()
  , total_amount_()
  , invoice_payload_()
  , shipping_option_id_()
  , order_info_()
{}

updateNewPreCheckoutQuery::updateNewPreCheckoutQuery(int64 id_, int53 sender_user_id_, string const &currency_, int53 total_amount_, bytes const &invoice_payload_, string const &shipping_option_id_, object_ptr<orderInfo> &&order_info_)
  : id_(id_)
  , sender_user_id_(sender_user_id_)
  , currency_(currency_)
  , total_amount_(total_amount_)
  , invoice_payload_(std::move(invoice_payload_))
  , shipping_option_id_(shipping_option_id_)
  , order_info_(std::move(order_info_))
{}

const std::int32_t updateNewPreCheckoutQuery::ID;

void updateNewPreCheckoutQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewPreCheckoutQuery");
    s.store_field("id", id_);
    s.store_field("sender_user_id", sender_user_id_);
    s.store_field("currency", currency_);
    s.store_field("total_amount", total_amount_);
    s.store_bytes_field("invoice_payload", invoice_payload_);
    s.store_field("shipping_option_id", shipping_option_id_);
    s.store_object_field("order_info", static_cast<const BaseObject *>(order_info_.get()));
    s.store_class_end();
  }
}

updateNewCustomEvent::updateNewCustomEvent()
  : event_()
{}

updateNewCustomEvent::updateNewCustomEvent(string const &event_)
  : event_(event_)
{}

const std::int32_t updateNewCustomEvent::ID;

void updateNewCustomEvent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewCustomEvent");
    s.store_field("event", event_);
    s.store_class_end();
  }
}

updateNewCustomQuery::updateNewCustomQuery()
  : id_()
  , data_()
  , timeout_()
{}

updateNewCustomQuery::updateNewCustomQuery(int64 id_, string const &data_, int32 timeout_)
  : id_(id_)
  , data_(data_)
  , timeout_(timeout_)
{}

const std::int32_t updateNewCustomQuery::ID;

void updateNewCustomQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewCustomQuery");
    s.store_field("id", id_);
    s.store_field("data", data_);
    s.store_field("timeout", timeout_);
    s.store_class_end();
  }
}

updatePoll::updatePoll()
  : poll_()
{}

updatePoll::updatePoll(object_ptr<poll> &&poll_)
  : poll_(std::move(poll_))
{}

const std::int32_t updatePoll::ID;

void updatePoll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePoll");
    s.store_object_field("poll", static_cast<const BaseObject *>(poll_.get()));
    s.store_class_end();
  }
}

updatePollAnswer::updatePollAnswer()
  : poll_id_()
  , voter_id_()
  , option_ids_()
{}

updatePollAnswer::updatePollAnswer(int64 poll_id_, object_ptr<MessageSender> &&voter_id_, array<int32> &&option_ids_)
  : poll_id_(poll_id_)
  , voter_id_(std::move(voter_id_))
  , option_ids_(std::move(option_ids_))
{}

const std::int32_t updatePollAnswer::ID;

void updatePollAnswer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePollAnswer");
    s.store_field("poll_id", poll_id_);
    s.store_object_field("voter_id", static_cast<const BaseObject *>(voter_id_.get()));
    { s.store_vector_begin("option_ids", option_ids_.size()); for (const auto &_value : option_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateChatMember::updateChatMember()
  : chat_id_()
  , actor_user_id_()
  , date_()
  , invite_link_()
  , via_join_request_()
  , via_chat_folder_invite_link_()
  , old_chat_member_()
  , new_chat_member_()
{}

updateChatMember::updateChatMember(int53 chat_id_, int53 actor_user_id_, int32 date_, object_ptr<chatInviteLink> &&invite_link_, bool via_join_request_, bool via_chat_folder_invite_link_, object_ptr<chatMember> &&old_chat_member_, object_ptr<chatMember> &&new_chat_member_)
  : chat_id_(chat_id_)
  , actor_user_id_(actor_user_id_)
  , date_(date_)
  , invite_link_(std::move(invite_link_))
  , via_join_request_(via_join_request_)
  , via_chat_folder_invite_link_(via_chat_folder_invite_link_)
  , old_chat_member_(std::move(old_chat_member_))
  , new_chat_member_(std::move(new_chat_member_))
{}

const std::int32_t updateChatMember::ID;

void updateChatMember::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatMember");
    s.store_field("chat_id", chat_id_);
    s.store_field("actor_user_id", actor_user_id_);
    s.store_field("date", date_);
    s.store_object_field("invite_link", static_cast<const BaseObject *>(invite_link_.get()));
    s.store_field("via_join_request", via_join_request_);
    s.store_field("via_chat_folder_invite_link", via_chat_folder_invite_link_);
    s.store_object_field("old_chat_member", static_cast<const BaseObject *>(old_chat_member_.get()));
    s.store_object_field("new_chat_member", static_cast<const BaseObject *>(new_chat_member_.get()));
    s.store_class_end();
  }
}

updateNewChatJoinRequest::updateNewChatJoinRequest()
  : chat_id_()
  , request_()
  , user_chat_id_()
  , invite_link_()
{}

updateNewChatJoinRequest::updateNewChatJoinRequest(int53 chat_id_, object_ptr<chatJoinRequest> &&request_, int53 user_chat_id_, object_ptr<chatInviteLink> &&invite_link_)
  : chat_id_(chat_id_)
  , request_(std::move(request_))
  , user_chat_id_(user_chat_id_)
  , invite_link_(std::move(invite_link_))
{}

const std::int32_t updateNewChatJoinRequest::ID;

void updateNewChatJoinRequest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateNewChatJoinRequest");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("request", static_cast<const BaseObject *>(request_.get()));
    s.store_field("user_chat_id", user_chat_id_);
    s.store_object_field("invite_link", static_cast<const BaseObject *>(invite_link_.get()));
    s.store_class_end();
  }
}

updateChatBoost::updateChatBoost()
  : chat_id_()
  , boost_()
{}

updateChatBoost::updateChatBoost(int53 chat_id_, object_ptr<chatBoost> &&boost_)
  : chat_id_(chat_id_)
  , boost_(std::move(boost_))
{}

const std::int32_t updateChatBoost::ID;

void updateChatBoost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateChatBoost");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("boost", static_cast<const BaseObject *>(boost_.get()));
    s.store_class_end();
  }
}

updateMessageReaction::updateMessageReaction()
  : chat_id_()
  , message_id_()
  , actor_id_()
  , date_()
  , old_reaction_types_()
  , new_reaction_types_()
{}

updateMessageReaction::updateMessageReaction(int53 chat_id_, int53 message_id_, object_ptr<MessageSender> &&actor_id_, int32 date_, array<object_ptr<ReactionType>> &&old_reaction_types_, array<object_ptr<ReactionType>> &&new_reaction_types_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , actor_id_(std::move(actor_id_))
  , date_(date_)
  , old_reaction_types_(std::move(old_reaction_types_))
  , new_reaction_types_(std::move(new_reaction_types_))
{}

const std::int32_t updateMessageReaction::ID;

void updateMessageReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessageReaction");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("actor_id", static_cast<const BaseObject *>(actor_id_.get()));
    s.store_field("date", date_);
    { s.store_vector_begin("old_reaction_types", old_reaction_types_.size()); for (const auto &_value : old_reaction_types_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("new_reaction_types", new_reaction_types_.size()); for (const auto &_value : new_reaction_types_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

updateMessageReactions::updateMessageReactions()
  : chat_id_()
  , message_id_()
  , date_()
  , reactions_()
{}

updateMessageReactions::updateMessageReactions(int53 chat_id_, int53 message_id_, int32 date_, array<object_ptr<messageReaction>> &&reactions_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , date_(date_)
  , reactions_(std::move(reactions_))
{}

const std::int32_t updateMessageReactions::ID;

void updateMessageReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updateMessageReactions");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("date", date_);
    { s.store_vector_begin("reactions", reactions_.size()); for (const auto &_value : reactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

updatePaidMediaPurchased::updatePaidMediaPurchased()
  : user_id_()
  , payload_()
{}

updatePaidMediaPurchased::updatePaidMediaPurchased(int53 user_id_, string const &payload_)
  : user_id_(user_id_)
  , payload_(payload_)
{}

const std::int32_t updatePaidMediaPurchased::ID;

void updatePaidMediaPurchased::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updatePaidMediaPurchased");
    s.store_field("user_id", user_id_);
    s.store_field("payload", payload_);
    s.store_class_end();
  }
}

upgradedGiftAttributeIdModel::upgradedGiftAttributeIdModel()
  : sticker_id_()
{}

upgradedGiftAttributeIdModel::upgradedGiftAttributeIdModel(int64 sticker_id_)
  : sticker_id_(sticker_id_)
{}

const std::int32_t upgradedGiftAttributeIdModel::ID;

void upgradedGiftAttributeIdModel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradedGiftAttributeIdModel");
    s.store_field("sticker_id", sticker_id_);
    s.store_class_end();
  }
}

upgradedGiftAttributeIdSymbol::upgradedGiftAttributeIdSymbol()
  : sticker_id_()
{}

upgradedGiftAttributeIdSymbol::upgradedGiftAttributeIdSymbol(int64 sticker_id_)
  : sticker_id_(sticker_id_)
{}

const std::int32_t upgradedGiftAttributeIdSymbol::ID;

void upgradedGiftAttributeIdSymbol::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradedGiftAttributeIdSymbol");
    s.store_field("sticker_id", sticker_id_);
    s.store_class_end();
  }
}

upgradedGiftAttributeIdBackdrop::upgradedGiftAttributeIdBackdrop()
  : backdrop_id_()
{}

upgradedGiftAttributeIdBackdrop::upgradedGiftAttributeIdBackdrop(int32 backdrop_id_)
  : backdrop_id_(backdrop_id_)
{}

const std::int32_t upgradedGiftAttributeIdBackdrop::ID;

void upgradedGiftAttributeIdBackdrop::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradedGiftAttributeIdBackdrop");
    s.store_field("backdrop_id", backdrop_id_);
    s.store_class_end();
  }
}

userPrivacySettingRuleAllowAll::userPrivacySettingRuleAllowAll() {
}

const std::int32_t userPrivacySettingRuleAllowAll::ID;

void userPrivacySettingRuleAllowAll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingRuleAllowAll");
    s.store_class_end();
  }
}

userPrivacySettingRuleAllowContacts::userPrivacySettingRuleAllowContacts() {
}

const std::int32_t userPrivacySettingRuleAllowContacts::ID;

void userPrivacySettingRuleAllowContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingRuleAllowContacts");
    s.store_class_end();
  }
}

userPrivacySettingRuleAllowBots::userPrivacySettingRuleAllowBots() {
}

const std::int32_t userPrivacySettingRuleAllowBots::ID;

void userPrivacySettingRuleAllowBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingRuleAllowBots");
    s.store_class_end();
  }
}

userPrivacySettingRuleAllowPremiumUsers::userPrivacySettingRuleAllowPremiumUsers() {
}

const std::int32_t userPrivacySettingRuleAllowPremiumUsers::ID;

void userPrivacySettingRuleAllowPremiumUsers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingRuleAllowPremiumUsers");
    s.store_class_end();
  }
}

userPrivacySettingRuleAllowUsers::userPrivacySettingRuleAllowUsers()
  : user_ids_()
{}

userPrivacySettingRuleAllowUsers::userPrivacySettingRuleAllowUsers(array<int53> &&user_ids_)
  : user_ids_(std::move(user_ids_))
{}

const std::int32_t userPrivacySettingRuleAllowUsers::ID;

void userPrivacySettingRuleAllowUsers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingRuleAllowUsers");
    { s.store_vector_begin("user_ids", user_ids_.size()); for (const auto &_value : user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

userPrivacySettingRuleAllowChatMembers::userPrivacySettingRuleAllowChatMembers()
  : chat_ids_()
{}

userPrivacySettingRuleAllowChatMembers::userPrivacySettingRuleAllowChatMembers(array<int53> &&chat_ids_)
  : chat_ids_(std::move(chat_ids_))
{}

const std::int32_t userPrivacySettingRuleAllowChatMembers::ID;

void userPrivacySettingRuleAllowChatMembers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingRuleAllowChatMembers");
    { s.store_vector_begin("chat_ids", chat_ids_.size()); for (const auto &_value : chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

userPrivacySettingRuleRestrictAll::userPrivacySettingRuleRestrictAll() {
}

const std::int32_t userPrivacySettingRuleRestrictAll::ID;

void userPrivacySettingRuleRestrictAll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingRuleRestrictAll");
    s.store_class_end();
  }
}

userPrivacySettingRuleRestrictContacts::userPrivacySettingRuleRestrictContacts() {
}

const std::int32_t userPrivacySettingRuleRestrictContacts::ID;

void userPrivacySettingRuleRestrictContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingRuleRestrictContacts");
    s.store_class_end();
  }
}

userPrivacySettingRuleRestrictBots::userPrivacySettingRuleRestrictBots() {
}

const std::int32_t userPrivacySettingRuleRestrictBots::ID;

void userPrivacySettingRuleRestrictBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingRuleRestrictBots");
    s.store_class_end();
  }
}

userPrivacySettingRuleRestrictUsers::userPrivacySettingRuleRestrictUsers()
  : user_ids_()
{}

userPrivacySettingRuleRestrictUsers::userPrivacySettingRuleRestrictUsers(array<int53> &&user_ids_)
  : user_ids_(std::move(user_ids_))
{}

const std::int32_t userPrivacySettingRuleRestrictUsers::ID;

void userPrivacySettingRuleRestrictUsers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingRuleRestrictUsers");
    { s.store_vector_begin("user_ids", user_ids_.size()); for (const auto &_value : user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

userPrivacySettingRuleRestrictChatMembers::userPrivacySettingRuleRestrictChatMembers()
  : chat_ids_()
{}

userPrivacySettingRuleRestrictChatMembers::userPrivacySettingRuleRestrictChatMembers(array<int53> &&chat_ids_)
  : chat_ids_(std::move(chat_ids_))
{}

const std::int32_t userPrivacySettingRuleRestrictChatMembers::ID;

void userPrivacySettingRuleRestrictChatMembers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingRuleRestrictChatMembers");
    { s.store_vector_begin("chat_ids", chat_ids_.size()); for (const auto &_value : chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

usernames::usernames()
  : active_usernames_()
  , disabled_usernames_()
  , editable_username_()
{}

usernames::usernames(array<string> &&active_usernames_, array<string> &&disabled_usernames_, string const &editable_username_)
  : active_usernames_(std::move(active_usernames_))
  , disabled_usernames_(std::move(disabled_usernames_))
  , editable_username_(editable_username_)
{}

const std::int32_t usernames::ID;

void usernames::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "usernames");
    { s.store_vector_begin("active_usernames", active_usernames_.size()); for (const auto &_value : active_usernames_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("disabled_usernames", disabled_usernames_.size()); for (const auto &_value : disabled_usernames_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("editable_username", editable_username_);
    s.store_class_end();
  }
}

webApp::webApp()
  : short_name_()
  , title_()
  , description_()
  , photo_()
  , animation_()
{}

webApp::webApp(string const &short_name_, string const &title_, string const &description_, object_ptr<photo> &&photo_, object_ptr<animation> &&animation_)
  : short_name_(short_name_)
  , title_(title_)
  , description_(description_)
  , photo_(std::move(photo_))
  , animation_(std::move(animation_))
{}

const std::int32_t webApp::ID;

void webApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webApp");
    s.store_field("short_name", short_name_);
    s.store_field("title", title_);
    s.store_field("description", description_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_object_field("animation", static_cast<const BaseObject *>(animation_.get()));
    s.store_class_end();
  }
}

webPageInstantView::webPageInstantView()
  : page_blocks_()
  , view_count_()
  , version_()
  , is_rtl_()
  , is_full_()
  , feedback_link_()
{}

webPageInstantView::webPageInstantView(array<object_ptr<PageBlock>> &&page_blocks_, int32 view_count_, int32 version_, bool is_rtl_, bool is_full_, object_ptr<InternalLinkType> &&feedback_link_)
  : page_blocks_(std::move(page_blocks_))
  , view_count_(view_count_)
  , version_(version_)
  , is_rtl_(is_rtl_)
  , is_full_(is_full_)
  , feedback_link_(std::move(feedback_link_))
{}

const std::int32_t webPageInstantView::ID;

void webPageInstantView::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webPageInstantView");
    { s.store_vector_begin("page_blocks", page_blocks_.size()); for (const auto &_value : page_blocks_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("view_count", view_count_);
    s.store_field("version", version_);
    s.store_field("is_rtl", is_rtl_);
    s.store_field("is_full", is_full_);
    s.store_object_field("feedback_link", static_cast<const BaseObject *>(feedback_link_.get()));
    s.store_class_end();
  }
}

addChatMember::addChatMember()
  : chat_id_()
  , user_id_()
  , forward_limit_()
{}

addChatMember::addChatMember(int53 chat_id_, int53 user_id_, int32 forward_limit_)
  : chat_id_(chat_id_)
  , user_id_(user_id_)
  , forward_limit_(forward_limit_)
{}

const std::int32_t addChatMember::ID;

void addChatMember::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addChatMember");
    s.store_field("chat_id", chat_id_);
    s.store_field("user_id", user_id_);
    s.store_field("forward_limit", forward_limit_);
    s.store_class_end();
  }
}

addChecklistTasks::addChecklistTasks()
  : chat_id_()
  , message_id_()
  , tasks_()
{}

addChecklistTasks::addChecklistTasks(int53 chat_id_, int53 message_id_, array<object_ptr<inputChecklistTask>> &&tasks_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , tasks_(std::move(tasks_))
{}

const std::int32_t addChecklistTasks::ID;

void addChecklistTasks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addChecklistTasks");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    { s.store_vector_begin("tasks", tasks_.size()); for (const auto &_value : tasks_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

addLocalMessage::addLocalMessage()
  : chat_id_()
  , sender_id_()
  , reply_to_()
  , disable_notification_()
  , input_message_content_()
{}

addLocalMessage::addLocalMessage(int53 chat_id_, object_ptr<MessageSender> &&sender_id_, object_ptr<InputMessageReplyTo> &&reply_to_, bool disable_notification_, object_ptr<InputMessageContent> &&input_message_content_)
  : chat_id_(chat_id_)
  , sender_id_(std::move(sender_id_))
  , reply_to_(std::move(reply_to_))
  , disable_notification_(disable_notification_)
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t addLocalMessage::ID;

void addLocalMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addLocalMessage");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get()));
    s.store_field("disable_notification", disable_notification_);
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

addQuickReplyShortcutMessage::addQuickReplyShortcutMessage()
  : shortcut_name_()
  , reply_to_message_id_()
  , input_message_content_()
{}

addQuickReplyShortcutMessage::addQuickReplyShortcutMessage(string const &shortcut_name_, int53 reply_to_message_id_, object_ptr<InputMessageContent> &&input_message_content_)
  : shortcut_name_(shortcut_name_)
  , reply_to_message_id_(reply_to_message_id_)
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t addQuickReplyShortcutMessage::ID;

void addQuickReplyShortcutMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addQuickReplyShortcutMessage");
    s.store_field("shortcut_name", shortcut_name_);
    s.store_field("reply_to_message_id", reply_to_message_id_);
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

allowBotToSendMessages::allowBotToSendMessages()
  : bot_user_id_()
{}

allowBotToSendMessages::allowBotToSendMessages(int53 bot_user_id_)
  : bot_user_id_(bot_user_id_)
{}

const std::int32_t allowBotToSendMessages::ID;

void allowBotToSendMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "allowBotToSendMessages");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_class_end();
  }
}

answerCallbackQuery::answerCallbackQuery()
  : callback_query_id_()
  , text_()
  , show_alert_()
  , url_()
  , cache_time_()
{}

answerCallbackQuery::answerCallbackQuery(int64 callback_query_id_, string const &text_, bool show_alert_, string const &url_, int32 cache_time_)
  : callback_query_id_(callback_query_id_)
  , text_(text_)
  , show_alert_(show_alert_)
  , url_(url_)
  , cache_time_(cache_time_)
{}

const std::int32_t answerCallbackQuery::ID;

void answerCallbackQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "answerCallbackQuery");
    s.store_field("callback_query_id", callback_query_id_);
    s.store_field("text", text_);
    s.store_field("show_alert", show_alert_);
    s.store_field("url", url_);
    s.store_field("cache_time", cache_time_);
    s.store_class_end();
  }
}

canSendMessageToUser::canSendMessageToUser()
  : user_id_()
  , only_local_()
{}

canSendMessageToUser::canSendMessageToUser(int53 user_id_, bool only_local_)
  : user_id_(user_id_)
  , only_local_(only_local_)
{}

const std::int32_t canSendMessageToUser::ID;

void canSendMessageToUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canSendMessageToUser");
    s.store_field("user_id", user_id_);
    s.store_field("only_local", only_local_);
    s.store_class_end();
  }
}

checkAuthenticationCode::checkAuthenticationCode()
  : code_()
{}

checkAuthenticationCode::checkAuthenticationCode(string const &code_)
  : code_(code_)
{}

const std::int32_t checkAuthenticationCode::ID;

void checkAuthenticationCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkAuthenticationCode");
    s.store_field("code", code_);
    s.store_class_end();
  }
}

checkWebAppFileDownload::checkWebAppFileDownload()
  : bot_user_id_()
  , file_name_()
  , url_()
{}

checkWebAppFileDownload::checkWebAppFileDownload(int53 bot_user_id_, string const &file_name_, string const &url_)
  : bot_user_id_(bot_user_id_)
  , file_name_(file_name_)
  , url_(url_)
{}

const std::int32_t checkWebAppFileDownload::ID;

void checkWebAppFileDownload::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkWebAppFileDownload");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("file_name", file_name_);
    s.store_field("url", url_);
    s.store_class_end();
  }
}

commitPendingPaidMessageReactions::commitPendingPaidMessageReactions()
  : chat_id_()
  , message_id_()
{}

commitPendingPaidMessageReactions::commitPendingPaidMessageReactions(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t commitPendingPaidMessageReactions::ID;

void commitPendingPaidMessageReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "commitPendingPaidMessageReactions");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

connectAffiliateProgram::connectAffiliateProgram()
  : affiliate_()
  , bot_user_id_()
{}

connectAffiliateProgram::connectAffiliateProgram(object_ptr<AffiliateType> &&affiliate_, int53 bot_user_id_)
  : affiliate_(std::move(affiliate_))
  , bot_user_id_(bot_user_id_)
{}

const std::int32_t connectAffiliateProgram::ID;

void connectAffiliateProgram::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "connectAffiliateProgram");
    s.store_object_field("affiliate", static_cast<const BaseObject *>(affiliate_.get()));
    s.store_field("bot_user_id", bot_user_id_);
    s.store_class_end();
  }
}

createNewBasicGroupChat::createNewBasicGroupChat()
  : user_ids_()
  , title_()
  , message_auto_delete_time_()
{}

createNewBasicGroupChat::createNewBasicGroupChat(array<int53> &&user_ids_, string const &title_, int32 message_auto_delete_time_)
  : user_ids_(std::move(user_ids_))
  , title_(title_)
  , message_auto_delete_time_(message_auto_delete_time_)
{}

const std::int32_t createNewBasicGroupChat::ID;

void createNewBasicGroupChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createNewBasicGroupChat");
    { s.store_vector_begin("user_ids", user_ids_.size()); for (const auto &_value : user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("title", title_);
    s.store_field("message_auto_delete_time", message_auto_delete_time_);
    s.store_class_end();
  }
}

createPrivateChat::createPrivateChat()
  : user_id_()
  , force_()
{}

createPrivateChat::createPrivateChat(int53 user_id_, bool force_)
  : user_id_(user_id_)
  , force_(force_)
{}

const std::int32_t createPrivateChat::ID;

void createPrivateChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createPrivateChat");
    s.store_field("user_id", user_id_);
    s.store_field("force", force_);
    s.store_class_end();
  }
}

deleteChat::deleteChat()
  : chat_id_()
{}

deleteChat::deleteChat(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t deleteChat::ID;

void deleteChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

deleteChatHistory::deleteChatHistory()
  : chat_id_()
  , remove_from_chat_list_()
  , revoke_()
{}

deleteChatHistory::deleteChatHistory(int53 chat_id_, bool remove_from_chat_list_, bool revoke_)
  : chat_id_(chat_id_)
  , remove_from_chat_list_(remove_from_chat_list_)
  , revoke_(revoke_)
{}

const std::int32_t deleteChatHistory::ID;

void deleteChatHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteChatHistory");
    s.store_field("chat_id", chat_id_);
    s.store_field("remove_from_chat_list", remove_from_chat_list_);
    s.store_field("revoke", revoke_);
    s.store_class_end();
  }
}

deleteChatMessagesBySender::deleteChatMessagesBySender()
  : chat_id_()
  , sender_id_()
{}

deleteChatMessagesBySender::deleteChatMessagesBySender(int53 chat_id_, object_ptr<MessageSender> &&sender_id_)
  : chat_id_(chat_id_)
  , sender_id_(std::move(sender_id_))
{}

const std::int32_t deleteChatMessagesBySender::ID;

void deleteChatMessagesBySender::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteChatMessagesBySender");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_class_end();
  }
}

deleteCommands::deleteCommands()
  : scope_()
  , language_code_()
{}

deleteCommands::deleteCommands(object_ptr<BotCommandScope> &&scope_, string const &language_code_)
  : scope_(std::move(scope_))
  , language_code_(language_code_)
{}

const std::int32_t deleteCommands::ID;

void deleteCommands::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteCommands");
    s.store_object_field("scope", static_cast<const BaseObject *>(scope_.get()));
    s.store_field("language_code", language_code_);
    s.store_class_end();
  }
}

deleteForumTopic::deleteForumTopic()
  : chat_id_()
  , forum_topic_id_()
{}

deleteForumTopic::deleteForumTopic(int53 chat_id_, int32 forum_topic_id_)
  : chat_id_(chat_id_)
  , forum_topic_id_(forum_topic_id_)
{}

const std::int32_t deleteForumTopic::ID;

void deleteForumTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteForumTopic");
    s.store_field("chat_id", chat_id_);
    s.store_field("forum_topic_id", forum_topic_id_);
    s.store_class_end();
  }
}

deleteGiftCollection::deleteGiftCollection()
  : owner_id_()
  , collection_id_()
{}

deleteGiftCollection::deleteGiftCollection(object_ptr<MessageSender> &&owner_id_, int32 collection_id_)
  : owner_id_(std::move(owner_id_))
  , collection_id_(collection_id_)
{}

const std::int32_t deleteGiftCollection::ID;

void deleteGiftCollection::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteGiftCollection");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_field("collection_id", collection_id_);
    s.store_class_end();
  }
}

editBotMediaPreview::editBotMediaPreview()
  : bot_user_id_()
  , language_code_()
  , file_id_()
  , content_()
{}

editBotMediaPreview::editBotMediaPreview(int53 bot_user_id_, string const &language_code_, int32 file_id_, object_ptr<InputStoryContent> &&content_)
  : bot_user_id_(bot_user_id_)
  , language_code_(language_code_)
  , file_id_(file_id_)
  , content_(std::move(content_))
{}

const std::int32_t editBotMediaPreview::ID;

void editBotMediaPreview::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editBotMediaPreview");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("language_code", language_code_);
    s.store_field("file_id", file_id_);
    s.store_object_field("content", static_cast<const BaseObject *>(content_.get()));
    s.store_class_end();
  }
}

editMessageMedia::editMessageMedia()
  : chat_id_()
  , message_id_()
  , reply_markup_()
  , input_message_content_()
{}

editMessageMedia::editMessageMedia(int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , reply_markup_(std::move(reply_markup_))
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t editMessageMedia::ID;

void editMessageMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editMessageMedia");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

getAvailableChatBoostSlots::getAvailableChatBoostSlots() {
}

const std::int32_t getAvailableChatBoostSlots::ID;

void getAvailableChatBoostSlots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getAvailableChatBoostSlots");
    s.store_class_end();
  }
}

getChatBoostLinkInfo::getChatBoostLinkInfo()
  : url_()
{}

getChatBoostLinkInfo::getChatBoostLinkInfo(string const &url_)
  : url_(url_)
{}

const std::int32_t getChatBoostLinkInfo::ID;

void getChatBoostLinkInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatBoostLinkInfo");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

getChatInviteLinkCounts::getChatInviteLinkCounts()
  : chat_id_()
{}

getChatInviteLinkCounts::getChatInviteLinkCounts(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t getChatInviteLinkCounts::ID;

void getChatInviteLinkCounts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatInviteLinkCounts");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

getChatListsToAddChat::getChatListsToAddChat()
  : chat_id_()
{}

getChatListsToAddChat::getChatListsToAddChat(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t getChatListsToAddChat::ID;

void getChatListsToAddChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatListsToAddChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

getChatRevenueStatistics::getChatRevenueStatistics()
  : chat_id_()
  , is_dark_()
{}

getChatRevenueStatistics::getChatRevenueStatistics(int53 chat_id_, bool is_dark_)
  : chat_id_(chat_id_)
  , is_dark_(is_dark_)
{}

const std::int32_t getChatRevenueStatistics::ID;

void getChatRevenueStatistics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatRevenueStatistics");
    s.store_field("chat_id", chat_id_);
    s.store_field("is_dark", is_dark_);
    s.store_class_end();
  }
}

getCollectibleItemInfo::getCollectibleItemInfo()
  : type_()
{}

getCollectibleItemInfo::getCollectibleItemInfo(object_ptr<CollectibleItemType> &&type_)
  : type_(std::move(type_))
{}

const std::int32_t getCollectibleItemInfo::ID;

void getCollectibleItemInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getCollectibleItemInfo");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

getDefaultChatEmojiStatuses::getDefaultChatEmojiStatuses() {
}

const std::int32_t getDefaultChatEmojiStatuses::ID;

void getDefaultChatEmojiStatuses::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getDefaultChatEmojiStatuses");
    s.store_class_end();
  }
}

getDefaultProfilePhotoCustomEmojiStickers::getDefaultProfilePhotoCustomEmojiStickers() {
}

const std::int32_t getDefaultProfilePhotoCustomEmojiStickers::ID;

void getDefaultProfilePhotoCustomEmojiStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getDefaultProfilePhotoCustomEmojiStickers");
    s.store_class_end();
  }
}

getFileExtension::getFileExtension()
  : mime_type_()
{}

getFileExtension::getFileExtension(string const &mime_type_)
  : mime_type_(mime_type_)
{}

const std::int32_t getFileExtension::ID;

void getFileExtension::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getFileExtension");
    s.store_field("mime_type", mime_type_);
    s.store_class_end();
  }
}

getGroupCall::getGroupCall()
  : group_call_id_()
{}

getGroupCall::getGroupCall(int32 group_call_id_)
  : group_call_id_(group_call_id_)
{}

const std::int32_t getGroupCall::ID;

void getGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getGroupCall");
    s.store_field("group_call_id", group_call_id_);
    s.store_class_end();
  }
}

getImportedContactCount::getImportedContactCount() {
}

const std::int32_t getImportedContactCount::ID;

void getImportedContactCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getImportedContactCount");
    s.store_class_end();
  }
}

getInlineQueryResults::getInlineQueryResults()
  : bot_user_id_()
  , chat_id_()
  , user_location_()
  , query_()
  , offset_()
{}

getInlineQueryResults::getInlineQueryResults(int53 bot_user_id_, int53 chat_id_, object_ptr<location> &&user_location_, string const &query_, string const &offset_)
  : bot_user_id_(bot_user_id_)
  , chat_id_(chat_id_)
  , user_location_(std::move(user_location_))
  , query_(query_)
  , offset_(offset_)
{}

const std::int32_t getInlineQueryResults::ID;

void getInlineQueryResults::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getInlineQueryResults");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("chat_id", chat_id_);
    s.store_object_field("user_location", static_cast<const BaseObject *>(user_location_.get()));
    s.store_field("query", query_);
    s.store_field("offset", offset_);
    s.store_class_end();
  }
}

getJsonString::getJsonString()
  : json_value_()
{}

getJsonString::getJsonString(object_ptr<JsonValue> &&json_value_)
  : json_value_(std::move(json_value_))
{}

const std::int32_t getJsonString::ID;

void getJsonString::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getJsonString");
    s.store_object_field("json_value", static_cast<const BaseObject *>(json_value_.get()));
    s.store_class_end();
  }
}

getMessageFileType::getMessageFileType()
  : message_file_head_()
{}

getMessageFileType::getMessageFileType(string const &message_file_head_)
  : message_file_head_(message_file_head_)
{}

const std::int32_t getMessageFileType::ID;

void getMessageFileType::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMessageFileType");
    s.store_field("message_file_head", message_file_head_);
    s.store_class_end();
  }
}

getMessageLink::getMessageLink()
  : chat_id_()
  , message_id_()
  , media_timestamp_()
  , for_album_()
  , in_message_thread_()
{}

getMessageLink::getMessageLink(int53 chat_id_, int53 message_id_, int32 media_timestamp_, bool for_album_, bool in_message_thread_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , media_timestamp_(media_timestamp_)
  , for_album_(for_album_)
  , in_message_thread_(in_message_thread_)
{}

const std::int32_t getMessageLink::ID;

void getMessageLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMessageLink");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("media_timestamp", media_timestamp_);
    s.store_field("for_album", for_album_);
    s.store_field("in_message_thread", in_message_thread_);
    s.store_class_end();
  }
}

getOption::getOption()
  : name_()
{}

getOption::getOption(string const &name_)
  : name_(name_)
{}

const std::int32_t getOption::ID;

void getOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getOption");
    s.store_field("name", name_);
    s.store_class_end();
  }
}

getSavedOrderInfo::getSavedOrderInfo() {
}

const std::int32_t getSavedOrderInfo::ID;

void getSavedOrderInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getSavedOrderInfo");
    s.store_class_end();
  }
}

getStarAdAccountUrl::getStarAdAccountUrl()
  : owner_id_()
{}

getStarAdAccountUrl::getStarAdAccountUrl(object_ptr<MessageSender> &&owner_id_)
  : owner_id_(std::move(owner_id_))
{}

const std::int32_t getStarAdAccountUrl::ID;

void getStarAdAccountUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStarAdAccountUrl");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_class_end();
  }
}

getStory::getStory()
  : story_poster_chat_id_()
  , story_id_()
  , only_local_()
{}

getStory::getStory(int53 story_poster_chat_id_, int32 story_id_, bool only_local_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , story_id_(story_id_)
  , only_local_(only_local_)
{}

const std::int32_t getStory::ID;

void getStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStory");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("story_id", story_id_);
    s.store_field("only_local", only_local_);
    s.store_class_end();
  }
}

getTemporaryPasswordState::getTemporaryPasswordState() {
}

const std::int32_t getTemporaryPasswordState::ID;

void getTemporaryPasswordState::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getTemporaryPasswordState");
    s.store_class_end();
  }
}

getThemedEmojiStatuses::getThemedEmojiStatuses() {
}

const std::int32_t getThemedEmojiStatuses::ID;

void getThemedEmojiStatuses::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getThemedEmojiStatuses");
    s.store_class_end();
  }
}

getTonWithdrawalUrl::getTonWithdrawalUrl()
  : password_()
{}

getTonWithdrawalUrl::getTonWithdrawalUrl(string const &password_)
  : password_(password_)
{}

const std::int32_t getTonWithdrawalUrl::ID;

void getTonWithdrawalUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getTonWithdrawalUrl");
    s.store_field("password", password_);
    s.store_class_end();
  }
}

getUpgradedGiftValueInfo::getUpgradedGiftValueInfo()
  : name_()
{}

getUpgradedGiftValueInfo::getUpgradedGiftValueInfo(string const &name_)
  : name_(name_)
{}

const std::int32_t getUpgradedGiftValueInfo::ID;

void getUpgradedGiftValueInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getUpgradedGiftValueInfo");
    s.store_field("name", name_);
    s.store_class_end();
  }
}

getUserLink::getUserLink() {
}

const std::int32_t getUserLink::ID;

void getUserLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getUserLink");
    s.store_class_end();
  }
}

getVideoChatRtmpUrl::getVideoChatRtmpUrl()
  : chat_id_()
{}

getVideoChatRtmpUrl::getVideoChatRtmpUrl(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t getVideoChatRtmpUrl::ID;

void getVideoChatRtmpUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getVideoChatRtmpUrl");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

hideSuggestedAction::hideSuggestedAction()
  : action_()
{}

hideSuggestedAction::hideSuggestedAction(object_ptr<SuggestedAction> &&action_)
  : action_(std::move(action_))
{}

const std::int32_t hideSuggestedAction::ID;

void hideSuggestedAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "hideSuggestedAction");
    s.store_object_field("action", static_cast<const BaseObject *>(action_.get()));
    s.store_class_end();
  }
}

importMessages::importMessages()
  : chat_id_()
  , message_file_()
  , attached_files_()
{}

importMessages::importMessages(int53 chat_id_, object_ptr<InputFile> &&message_file_, array<object_ptr<InputFile>> &&attached_files_)
  : chat_id_(chat_id_)
  , message_file_(std::move(message_file_))
  , attached_files_(std::move(attached_files_))
{}

const std::int32_t importMessages::ID;

void importMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "importMessages");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("message_file", static_cast<const BaseObject *>(message_file_.get()));
    { s.store_vector_begin("attached_files", attached_files_.size()); for (const auto &_value : attached_files_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

joinGroupCall::joinGroupCall()
  : input_group_call_()
  , join_parameters_()
{}

joinGroupCall::joinGroupCall(object_ptr<InputGroupCall> &&input_group_call_, object_ptr<groupCallJoinParameters> &&join_parameters_)
  : input_group_call_(std::move(input_group_call_))
  , join_parameters_(std::move(join_parameters_))
{}

const std::int32_t joinGroupCall::ID;

void joinGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "joinGroupCall");
    s.store_object_field("input_group_call", static_cast<const BaseObject *>(input_group_call_.get()));
    s.store_object_field("join_parameters", static_cast<const BaseObject *>(join_parameters_.get()));
    s.store_class_end();
  }
}

launchPrepaidGiveaway::launchPrepaidGiveaway()
  : giveaway_id_()
  , parameters_()
  , winner_count_()
  , star_count_()
{}

launchPrepaidGiveaway::launchPrepaidGiveaway(int64 giveaway_id_, object_ptr<giveawayParameters> &&parameters_, int32 winner_count_, int53 star_count_)
  : giveaway_id_(giveaway_id_)
  , parameters_(std::move(parameters_))
  , winner_count_(winner_count_)
  , star_count_(star_count_)
{}

const std::int32_t launchPrepaidGiveaway::ID;

void launchPrepaidGiveaway::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "launchPrepaidGiveaway");
    s.store_field("giveaway_id", giveaway_id_);
    s.store_object_field("parameters", static_cast<const BaseObject *>(parameters_.get()));
    s.store_field("winner_count", winner_count_);
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

loadQuickReplyShortcutMessages::loadQuickReplyShortcutMessages()
  : shortcut_id_()
{}

loadQuickReplyShortcutMessages::loadQuickReplyShortcutMessages(int32 shortcut_id_)
  : shortcut_id_(shortcut_id_)
{}

const std::int32_t loadQuickReplyShortcutMessages::ID;

void loadQuickReplyShortcutMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "loadQuickReplyShortcutMessages");
    s.store_field("shortcut_id", shortcut_id_);
    s.store_class_end();
  }
}

openChatSimilarChat::openChatSimilarChat()
  : chat_id_()
  , opened_chat_id_()
{}

openChatSimilarChat::openChatSimilarChat(int53 chat_id_, int53 opened_chat_id_)
  : chat_id_(chat_id_)
  , opened_chat_id_(opened_chat_id_)
{}

const std::int32_t openChatSimilarChat::ID;

void openChatSimilarChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "openChatSimilarChat");
    s.store_field("chat_id", chat_id_);
    s.store_field("opened_chat_id", opened_chat_id_);
    s.store_class_end();
  }
}

openStory::openStory()
  : story_poster_chat_id_()
  , story_id_()
{}

openStory::openStory(int53 story_poster_chat_id_, int32 story_id_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , story_id_(story_id_)
{}

const std::int32_t openStory::ID;

void openStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "openStory");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("story_id", story_id_);
    s.store_class_end();
  }
}

readdQuickReplyShortcutMessages::readdQuickReplyShortcutMessages()
  : shortcut_name_()
  , message_ids_()
{}

readdQuickReplyShortcutMessages::readdQuickReplyShortcutMessages(string const &shortcut_name_, array<int53> &&message_ids_)
  : shortcut_name_(shortcut_name_)
  , message_ids_(std::move(message_ids_))
{}

const std::int32_t readdQuickReplyShortcutMessages::ID;

void readdQuickReplyShortcutMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "readdQuickReplyShortcutMessages");
    s.store_field("shortcut_name", shortcut_name_);
    { s.store_vector_begin("message_ids", message_ids_.size()); for (const auto &_value : message_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

recoverPassword::recoverPassword()
  : recovery_code_()
  , new_password_()
  , new_hint_()
{}

recoverPassword::recoverPassword(string const &recovery_code_, string const &new_password_, string const &new_hint_)
  : recovery_code_(recovery_code_)
  , new_password_(new_password_)
  , new_hint_(new_hint_)
{}

const std::int32_t recoverPassword::ID;

void recoverPassword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "recoverPassword");
    s.store_field("recovery_code", recovery_code_);
    s.store_field("new_password", new_password_);
    s.store_field("new_hint", new_hint_);
    s.store_class_end();
  }
}

registerDevice::registerDevice()
  : device_token_()
  , other_user_ids_()
{}

registerDevice::registerDevice(object_ptr<DeviceToken> &&device_token_, array<int53> &&other_user_ids_)
  : device_token_(std::move(device_token_))
  , other_user_ids_(std::move(other_user_ids_))
{}

const std::int32_t registerDevice::ID;

void registerDevice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "registerDevice");
    s.store_object_field("device_token", static_cast<const BaseObject *>(device_token_.get()));
    { s.store_vector_begin("other_user_ids", other_user_ids_.size()); for (const auto &_value : other_user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

removeAllFilesFromDownloads::removeAllFilesFromDownloads()
  : only_active_()
  , only_completed_()
  , delete_from_cache_()
{}

removeAllFilesFromDownloads::removeAllFilesFromDownloads(bool only_active_, bool only_completed_, bool delete_from_cache_)
  : only_active_(only_active_)
  , only_completed_(only_completed_)
  , delete_from_cache_(delete_from_cache_)
{}

const std::int32_t removeAllFilesFromDownloads::ID;

void removeAllFilesFromDownloads::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeAllFilesFromDownloads");
    s.store_field("only_active", only_active_);
    s.store_field("only_completed", only_completed_);
    s.store_field("delete_from_cache", delete_from_cache_);
    s.store_class_end();
  }
}

removeNotification::removeNotification()
  : notification_group_id_()
  , notification_id_()
{}

removeNotification::removeNotification(int32 notification_group_id_, int32 notification_id_)
  : notification_group_id_(notification_group_id_)
  , notification_id_(notification_id_)
{}

const std::int32_t removeNotification::ID;

void removeNotification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeNotification");
    s.store_field("notification_group_id", notification_group_id_);
    s.store_field("notification_id", notification_id_);
    s.store_class_end();
  }
}

reorderBotActiveUsernames::reorderBotActiveUsernames()
  : bot_user_id_()
  , usernames_()
{}

reorderBotActiveUsernames::reorderBotActiveUsernames(int53 bot_user_id_, array<string> &&usernames_)
  : bot_user_id_(bot_user_id_)
  , usernames_(std::move(usernames_))
{}

const std::int32_t reorderBotActiveUsernames::ID;

void reorderBotActiveUsernames::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reorderBotActiveUsernames");
    s.store_field("bot_user_id", bot_user_id_);
    { s.store_vector_begin("usernames", usernames_.size()); for (const auto &_value : usernames_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

reportSponsoredChat::reportSponsoredChat()
  : sponsored_chat_unique_id_()
  , option_id_()
{}

reportSponsoredChat::reportSponsoredChat(int53 sponsored_chat_unique_id_, bytes const &option_id_)
  : sponsored_chat_unique_id_(sponsored_chat_unique_id_)
  , option_id_(std::move(option_id_))
{}

const std::int32_t reportSponsoredChat::ID;

void reportSponsoredChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportSponsoredChat");
    s.store_field("sponsored_chat_unique_id", sponsored_chat_unique_id_);
    s.store_bytes_field("option_id", option_id_);
    s.store_class_end();
  }
}

requestAuthenticationPasswordRecovery::requestAuthenticationPasswordRecovery() {
}

const std::int32_t requestAuthenticationPasswordRecovery::ID;

void requestAuthenticationPasswordRecovery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "requestAuthenticationPasswordRecovery");
    s.store_class_end();
  }
}

resendMessages::resendMessages()
  : chat_id_()
  , message_ids_()
  , quote_()
  , paid_message_star_count_()
{}

resendMessages::resendMessages(int53 chat_id_, array<int53> &&message_ids_, object_ptr<inputTextQuote> &&quote_, int53 paid_message_star_count_)
  : chat_id_(chat_id_)
  , message_ids_(std::move(message_ids_))
  , quote_(std::move(quote_))
  , paid_message_star_count_(paid_message_star_count_)
{}

const std::int32_t resendMessages::ID;

void resendMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "resendMessages");
    s.store_field("chat_id", chat_id_);
    { s.store_vector_begin("message_ids", message_ids_.size()); for (const auto &_value : message_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_object_field("quote", static_cast<const BaseObject *>(quote_.get()));
    s.store_field("paid_message_star_count", paid_message_star_count_);
    s.store_class_end();
  }
}

resetAuthenticationEmailAddress::resetAuthenticationEmailAddress() {
}

const std::int32_t resetAuthenticationEmailAddress::ID;

void resetAuthenticationEmailAddress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "resetAuthenticationEmailAddress");
    s.store_class_end();
  }
}

reuseStarSubscription::reuseStarSubscription()
  : subscription_id_()
{}

reuseStarSubscription::reuseStarSubscription(string const &subscription_id_)
  : subscription_id_(subscription_id_)
{}

const std::int32_t reuseStarSubscription::ID;

void reuseStarSubscription::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reuseStarSubscription");
    s.store_field("subscription_id", subscription_id_);
    s.store_class_end();
  }
}

saveApplicationLogEvent::saveApplicationLogEvent()
  : type_()
  , chat_id_()
  , data_()
{}

saveApplicationLogEvent::saveApplicationLogEvent(string const &type_, int53 chat_id_, object_ptr<JsonValue> &&data_)
  : type_(type_)
  , chat_id_(chat_id_)
  , data_(std::move(data_))
{}

const std::int32_t saveApplicationLogEvent::ID;

void saveApplicationLogEvent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "saveApplicationLogEvent");
    s.store_field("type", type_);
    s.store_field("chat_id", chat_id_);
    s.store_object_field("data", static_cast<const BaseObject *>(data_.get()));
    s.store_class_end();
  }
}

searchEmojis::searchEmojis()
  : text_()
  , input_language_codes_()
{}

searchEmojis::searchEmojis(string const &text_, array<string> &&input_language_codes_)
  : text_(text_)
  , input_language_codes_(std::move(input_language_codes_))
{}

const std::int32_t searchEmojis::ID;

void searchEmojis::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchEmojis");
    s.store_field("text", text_);
    { s.store_vector_begin("input_language_codes", input_language_codes_.size()); for (const auto &_value : input_language_codes_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

searchOutgoingDocumentMessages::searchOutgoingDocumentMessages()
  : query_()
  , limit_()
{}

searchOutgoingDocumentMessages::searchOutgoingDocumentMessages(string const &query_, int32 limit_)
  : query_(query_)
  , limit_(limit_)
{}

const std::int32_t searchOutgoingDocumentMessages::ID;

void searchOutgoingDocumentMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchOutgoingDocumentMessages");
    s.store_field("query", query_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

searchPublicStoriesByTag::searchPublicStoriesByTag()
  : story_poster_chat_id_()
  , tag_()
  , offset_()
  , limit_()
{}

searchPublicStoriesByTag::searchPublicStoriesByTag(int53 story_poster_chat_id_, string const &tag_, string const &offset_, int32 limit_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , tag_(tag_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t searchPublicStoriesByTag::ID;

void searchPublicStoriesByTag::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchPublicStoriesByTag");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("tag", tag_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

searchStickerSet::searchStickerSet()
  : name_()
  , ignore_cache_()
{}

searchStickerSet::searchStickerSet(string const &name_, bool ignore_cache_)
  : name_(name_)
  , ignore_cache_(ignore_cache_)
{}

const std::int32_t searchStickerSet::ID;

void searchStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchStickerSet");
    s.store_field("name", name_);
    s.store_field("ignore_cache", ignore_cache_);
    s.store_class_end();
  }
}

searchWebApp::searchWebApp()
  : bot_user_id_()
  , web_app_short_name_()
{}

searchWebApp::searchWebApp(int53 bot_user_id_, string const &web_app_short_name_)
  : bot_user_id_(bot_user_id_)
  , web_app_short_name_(web_app_short_name_)
{}

const std::int32_t searchWebApp::ID;

void searchWebApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchWebApp");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("web_app_short_name", web_app_short_name_);
    s.store_class_end();
  }
}

sendBusinessMessage::sendBusinessMessage()
  : business_connection_id_()
  , chat_id_()
  , reply_to_()
  , disable_notification_()
  , protect_content_()
  , effect_id_()
  , reply_markup_()
  , input_message_content_()
{}

sendBusinessMessage::sendBusinessMessage(string const &business_connection_id_, int53 chat_id_, object_ptr<InputMessageReplyTo> &&reply_to_, bool disable_notification_, bool protect_content_, int64 effect_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_)
  : business_connection_id_(business_connection_id_)
  , chat_id_(chat_id_)
  , reply_to_(std::move(reply_to_))
  , disable_notification_(disable_notification_)
  , protect_content_(protect_content_)
  , effect_id_(effect_id_)
  , reply_markup_(std::move(reply_markup_))
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t sendBusinessMessage::ID;

void sendBusinessMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendBusinessMessage");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_field("chat_id", chat_id_);
    s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get()));
    s.store_field("disable_notification", disable_notification_);
    s.store_field("protect_content", protect_content_);
    s.store_field("effect_id", effect_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

sendCallRating::sendCallRating()
  : call_id_()
  , rating_()
  , comment_()
  , problems_()
{}

sendCallRating::sendCallRating(int32 call_id_, int32 rating_, string const &comment_, array<object_ptr<CallProblem>> &&problems_)
  : call_id_(call_id_)
  , rating_(rating_)
  , comment_(comment_)
  , problems_(std::move(problems_))
{}

const std::int32_t sendCallRating::ID;

void sendCallRating::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendCallRating");
    s.store_field("call_id", call_id_);
    s.store_field("rating", rating_);
    s.store_field("comment", comment_);
    { s.store_vector_begin("problems", problems_.size()); for (const auto &_value : problems_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

sendPassportAuthorizationForm::sendPassportAuthorizationForm()
  : authorization_form_id_()
  , types_()
{}

sendPassportAuthorizationForm::sendPassportAuthorizationForm(int32 authorization_form_id_, array<object_ptr<PassportElementType>> &&types_)
  : authorization_form_id_(authorization_form_id_)
  , types_(std::move(types_))
{}

const std::int32_t sendPassportAuthorizationForm::ID;

void sendPassportAuthorizationForm::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendPassportAuthorizationForm");
    s.store_field("authorization_form_id", authorization_form_id_);
    { s.store_vector_begin("types", types_.size()); for (const auto &_value : types_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

sendWebAppData::sendWebAppData()
  : bot_user_id_()
  , button_text_()
  , data_()
{}

sendWebAppData::sendWebAppData(int53 bot_user_id_, string const &button_text_, string const &data_)
  : bot_user_id_(bot_user_id_)
  , button_text_(button_text_)
  , data_(data_)
{}

const std::int32_t sendWebAppData::ID;

void sendWebAppData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendWebAppData");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("button_text", button_text_);
    s.store_field("data", data_);
    s.store_class_end();
  }
}

setAccentColor::setAccentColor()
  : accent_color_id_()
  , background_custom_emoji_id_()
{}

setAccentColor::setAccentColor(int32 accent_color_id_, int64 background_custom_emoji_id_)
  : accent_color_id_(accent_color_id_)
  , background_custom_emoji_id_(background_custom_emoji_id_)
{}

const std::int32_t setAccentColor::ID;

void setAccentColor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setAccentColor");
    s.store_field("accent_color_id", accent_color_id_);
    s.store_field("background_custom_emoji_id", background_custom_emoji_id_);
    s.store_class_end();
  }
}

setAuthenticationPhoneNumber::setAuthenticationPhoneNumber()
  : phone_number_()
  , settings_()
{}

setAuthenticationPhoneNumber::setAuthenticationPhoneNumber(string const &phone_number_, object_ptr<phoneNumberAuthenticationSettings> &&settings_)
  : phone_number_(phone_number_)
  , settings_(std::move(settings_))
{}

const std::int32_t setAuthenticationPhoneNumber::ID;

void setAuthenticationPhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setAuthenticationPhoneNumber");
    s.store_field("phone_number", phone_number_);
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

setBusinessAccountBio::setBusinessAccountBio()
  : business_connection_id_()
  , bio_()
{}

setBusinessAccountBio::setBusinessAccountBio(string const &business_connection_id_, string const &bio_)
  : business_connection_id_(business_connection_id_)
  , bio_(bio_)
{}

const std::int32_t setBusinessAccountBio::ID;

void setBusinessAccountBio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setBusinessAccountBio");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_field("bio", bio_);
    s.store_class_end();
  }
}

setBusinessLocation::setBusinessLocation()
  : location_()
{}

setBusinessLocation::setBusinessLocation(object_ptr<businessLocation> &&location_)
  : location_(std::move(location_))
{}

const std::int32_t setBusinessLocation::ID;

void setBusinessLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setBusinessLocation");
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_class_end();
  }
}

setBusinessOpeningHours::setBusinessOpeningHours()
  : opening_hours_()
{}

setBusinessOpeningHours::setBusinessOpeningHours(object_ptr<businessOpeningHours> &&opening_hours_)
  : opening_hours_(std::move(opening_hours_))
{}

const std::int32_t setBusinessOpeningHours::ID;

void setBusinessOpeningHours::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setBusinessOpeningHours");
    s.store_object_field("opening_hours", static_cast<const BaseObject *>(opening_hours_.get()));
    s.store_class_end();
  }
}

setChatLocation::setChatLocation()
  : chat_id_()
  , location_()
{}

setChatLocation::setChatLocation(int53 chat_id_, object_ptr<chatLocation> &&location_)
  : chat_id_(chat_id_)
  , location_(std::move(location_))
{}

const std::int32_t setChatLocation::ID;

void setChatLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatLocation");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_class_end();
  }
}

setChatPinnedStories::setChatPinnedStories()
  : chat_id_()
  , story_ids_()
{}

setChatPinnedStories::setChatPinnedStories(int53 chat_id_, array<int32> &&story_ids_)
  : chat_id_(chat_id_)
  , story_ids_(std::move(story_ids_))
{}

const std::int32_t setChatPinnedStories::ID;

void setChatPinnedStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatPinnedStories");
    s.store_field("chat_id", chat_id_);
    { s.store_vector_begin("story_ids", story_ids_.size()); for (const auto &_value : story_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

setDatabaseEncryptionKey::setDatabaseEncryptionKey()
  : new_encryption_key_()
{}

setDatabaseEncryptionKey::setDatabaseEncryptionKey(bytes const &new_encryption_key_)
  : new_encryption_key_(std::move(new_encryption_key_))
{}

const std::int32_t setDatabaseEncryptionKey::ID;

void setDatabaseEncryptionKey::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setDatabaseEncryptionKey");
    s.store_bytes_field("new_encryption_key", new_encryption_key_);
    s.store_class_end();
  }
}

setDirectMessagesChatTopicIsMarkedAsUnread::setDirectMessagesChatTopicIsMarkedAsUnread()
  : chat_id_()
  , topic_id_()
  , is_marked_as_unread_()
{}

setDirectMessagesChatTopicIsMarkedAsUnread::setDirectMessagesChatTopicIsMarkedAsUnread(int53 chat_id_, int53 topic_id_, bool is_marked_as_unread_)
  : chat_id_(chat_id_)
  , topic_id_(topic_id_)
  , is_marked_as_unread_(is_marked_as_unread_)
{}

const std::int32_t setDirectMessagesChatTopicIsMarkedAsUnread::ID;

void setDirectMessagesChatTopicIsMarkedAsUnread::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setDirectMessagesChatTopicIsMarkedAsUnread");
    s.store_field("chat_id", chat_id_);
    s.store_field("topic_id", topic_id_);
    s.store_field("is_marked_as_unread", is_marked_as_unread_);
    s.store_class_end();
  }
}

setMenuButton::setMenuButton()
  : user_id_()
  , menu_button_()
{}

setMenuButton::setMenuButton(int53 user_id_, object_ptr<botMenuButton> &&menu_button_)
  : user_id_(user_id_)
  , menu_button_(std::move(menu_button_))
{}

const std::int32_t setMenuButton::ID;

void setMenuButton::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setMenuButton");
    s.store_field("user_id", user_id_);
    s.store_object_field("menu_button", static_cast<const BaseObject *>(menu_button_.get()));
    s.store_class_end();
  }
}

setNewChatPrivacySettings::setNewChatPrivacySettings()
  : settings_()
{}

setNewChatPrivacySettings::setNewChatPrivacySettings(object_ptr<newChatPrivacySettings> &&settings_)
  : settings_(std::move(settings_))
{}

const std::int32_t setNewChatPrivacySettings::ID;

void setNewChatPrivacySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setNewChatPrivacySettings");
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

setPinnedForumTopics::setPinnedForumTopics()
  : chat_id_()
  , forum_topic_ids_()
{}

setPinnedForumTopics::setPinnedForumTopics(int53 chat_id_, array<int32> &&forum_topic_ids_)
  : chat_id_(chat_id_)
  , forum_topic_ids_(std::move(forum_topic_ids_))
{}

const std::int32_t setPinnedForumTopics::ID;

void setPinnedForumTopics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setPinnedForumTopics");
    s.store_field("chat_id", chat_id_);
    { s.store_vector_begin("forum_topic_ids", forum_topic_ids_.size()); for (const auto &_value : forum_topic_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

setReadDatePrivacySettings::setReadDatePrivacySettings()
  : settings_()
{}

setReadDatePrivacySettings::setReadDatePrivacySettings(object_ptr<readDatePrivacySettings> &&settings_)
  : settings_(std::move(settings_))
{}

const std::int32_t setReadDatePrivacySettings::ID;

void setReadDatePrivacySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setReadDatePrivacySettings");
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

setStickerSetThumbnail::setStickerSetThumbnail()
  : user_id_()
  , name_()
  , thumbnail_()
  , format_()
{}

setStickerSetThumbnail::setStickerSetThumbnail(int53 user_id_, string const &name_, object_ptr<InputFile> &&thumbnail_, object_ptr<StickerFormat> &&format_)
  : user_id_(user_id_)
  , name_(name_)
  , thumbnail_(std::move(thumbnail_))
  , format_(std::move(format_))
{}

const std::int32_t setStickerSetThumbnail::ID;

void setStickerSetThumbnail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setStickerSetThumbnail");
    s.store_field("user_id", user_id_);
    s.store_field("name", name_);
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_object_field("format", static_cast<const BaseObject *>(format_.get()));
    s.store_class_end();
  }
}

setStoryAlbumName::setStoryAlbumName()
  : chat_id_()
  , story_album_id_()
  , name_()
{}

setStoryAlbumName::setStoryAlbumName(int53 chat_id_, int32 story_album_id_, string const &name_)
  : chat_id_(chat_id_)
  , story_album_id_(story_album_id_)
  , name_(name_)
{}

const std::int32_t setStoryAlbumName::ID;

void setStoryAlbumName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setStoryAlbumName");
    s.store_field("chat_id", chat_id_);
    s.store_field("story_album_id", story_album_id_);
    s.store_field("name", name_);
    s.store_class_end();
  }
}

setTdlibParameters::setTdlibParameters()
  : use_test_dc_()
  , database_directory_()
  , files_directory_()
  , database_encryption_key_()
  , use_file_database_()
  , use_chat_info_database_()
  , use_message_database_()
  , use_secret_chats_()
  , api_id_()
  , api_hash_()
  , system_language_code_()
  , device_model_()
  , system_version_()
  , application_version_()
{}

setTdlibParameters::setTdlibParameters(bool use_test_dc_, string const &database_directory_, string const &files_directory_, bytes const &database_encryption_key_, bool use_file_database_, bool use_chat_info_database_, bool use_message_database_, bool use_secret_chats_, int32 api_id_, string const &api_hash_, string const &system_language_code_, string const &device_model_, string const &system_version_, string const &application_version_)
  : use_test_dc_(use_test_dc_)
  , database_directory_(database_directory_)
  , files_directory_(files_directory_)
  , database_encryption_key_(std::move(database_encryption_key_))
  , use_file_database_(use_file_database_)
  , use_chat_info_database_(use_chat_info_database_)
  , use_message_database_(use_message_database_)
  , use_secret_chats_(use_secret_chats_)
  , api_id_(api_id_)
  , api_hash_(api_hash_)
  , system_language_code_(system_language_code_)
  , device_model_(device_model_)
  , system_version_(system_version_)
  , application_version_(application_version_)
{}

const std::int32_t setTdlibParameters::ID;

void setTdlibParameters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setTdlibParameters");
    s.store_field("use_test_dc", use_test_dc_);
    s.store_field("database_directory", database_directory_);
    s.store_field("files_directory", files_directory_);
    s.store_bytes_field("database_encryption_key", database_encryption_key_);
    s.store_field("use_file_database", use_file_database_);
    s.store_field("use_chat_info_database", use_chat_info_database_);
    s.store_field("use_message_database", use_message_database_);
    s.store_field("use_secret_chats", use_secret_chats_);
    s.store_field("api_id", api_id_);
    s.store_field("api_hash", api_hash_);
    s.store_field("system_language_code", system_language_code_);
    s.store_field("device_model", device_model_);
    s.store_field("system_version", system_version_);
    s.store_field("application_version", application_version_);
    s.store_class_end();
  }
}

startGroupCallScreenSharing::startGroupCallScreenSharing()
  : group_call_id_()
  , audio_source_id_()
  , payload_()
{}

startGroupCallScreenSharing::startGroupCallScreenSharing(int32 group_call_id_, int32 audio_source_id_, string const &payload_)
  : group_call_id_(group_call_id_)
  , audio_source_id_(audio_source_id_)
  , payload_(payload_)
{}

const std::int32_t startGroupCallScreenSharing::ID;

void startGroupCallScreenSharing::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "startGroupCallScreenSharing");
    s.store_field("group_call_id", group_call_id_);
    s.store_field("audio_source_id", audio_source_id_);
    s.store_field("payload", payload_);
    s.store_class_end();
  }
}

stopPoll::stopPoll()
  : chat_id_()
  , message_id_()
  , reply_markup_()
{}

stopPoll::stopPoll(int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , reply_markup_(std::move(reply_markup_))
{}

const std::int32_t stopPoll::ID;

void stopPoll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stopPoll");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_class_end();
  }
}

synchronizeLanguagePack::synchronizeLanguagePack()
  : language_pack_id_()
{}

synchronizeLanguagePack::synchronizeLanguagePack(string const &language_pack_id_)
  : language_pack_id_(language_pack_id_)
{}

const std::int32_t synchronizeLanguagePack::ID;

void synchronizeLanguagePack::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "synchronizeLanguagePack");
    s.store_field("language_pack_id", language_pack_id_);
    s.store_class_end();
  }
}

testCallEmpty::testCallEmpty() {
}

const std::int32_t testCallEmpty::ID;

void testCallEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testCallEmpty");
    s.store_class_end();
  }
}

testCallString::testCallString()
  : x_()
{}

testCallString::testCallString(string const &x_)
  : x_(x_)
{}

const std::int32_t testCallString::ID;

void testCallString::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testCallString");
    s.store_field("x", x_);
    s.store_class_end();
  }
}

toggleDirectMessagesChatTopicCanSendUnpaidMessages::toggleDirectMessagesChatTopicCanSendUnpaidMessages()
  : chat_id_()
  , topic_id_()
  , can_send_unpaid_messages_()
  , refund_payments_()
{}

toggleDirectMessagesChatTopicCanSendUnpaidMessages::toggleDirectMessagesChatTopicCanSendUnpaidMessages(int53 chat_id_, int53 topic_id_, bool can_send_unpaid_messages_, bool refund_payments_)
  : chat_id_(chat_id_)
  , topic_id_(topic_id_)
  , can_send_unpaid_messages_(can_send_unpaid_messages_)
  , refund_payments_(refund_payments_)
{}

const std::int32_t toggleDirectMessagesChatTopicCanSendUnpaidMessages::ID;

void toggleDirectMessagesChatTopicCanSendUnpaidMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleDirectMessagesChatTopicCanSendUnpaidMessages");
    s.store_field("chat_id", chat_id_);
    s.store_field("topic_id", topic_id_);
    s.store_field("can_send_unpaid_messages", can_send_unpaid_messages_);
    s.store_field("refund_payments", refund_payments_);
    s.store_class_end();
  }
}

toggleSupergroupCanHaveSponsoredMessages::toggleSupergroupCanHaveSponsoredMessages()
  : supergroup_id_()
  , can_have_sponsored_messages_()
{}

toggleSupergroupCanHaveSponsoredMessages::toggleSupergroupCanHaveSponsoredMessages(int53 supergroup_id_, bool can_have_sponsored_messages_)
  : supergroup_id_(supergroup_id_)
  , can_have_sponsored_messages_(can_have_sponsored_messages_)
{}

const std::int32_t toggleSupergroupCanHaveSponsoredMessages::ID;

void toggleSupergroupCanHaveSponsoredMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleSupergroupCanHaveSponsoredMessages");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_field("can_have_sponsored_messages", can_have_sponsored_messages_);
    s.store_class_end();
  }
}

toggleSupergroupHasHiddenMembers::toggleSupergroupHasHiddenMembers()
  : supergroup_id_()
  , has_hidden_members_()
{}

toggleSupergroupHasHiddenMembers::toggleSupergroupHasHiddenMembers(int53 supergroup_id_, bool has_hidden_members_)
  : supergroup_id_(supergroup_id_)
  , has_hidden_members_(has_hidden_members_)
{}

const std::int32_t toggleSupergroupHasHiddenMembers::ID;

void toggleSupergroupHasHiddenMembers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleSupergroupHasHiddenMembers");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_field("has_hidden_members", has_hidden_members_);
    s.store_class_end();
  }
}

toggleSupergroupSignMessages::toggleSupergroupSignMessages()
  : supergroup_id_()
  , sign_messages_()
  , show_message_sender_()
{}

toggleSupergroupSignMessages::toggleSupergroupSignMessages(int53 supergroup_id_, bool sign_messages_, bool show_message_sender_)
  : supergroup_id_(supergroup_id_)
  , sign_messages_(sign_messages_)
  , show_message_sender_(show_message_sender_)
{}

const std::int32_t toggleSupergroupSignMessages::ID;

void toggleSupergroupSignMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleSupergroupSignMessages");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_field("sign_messages", sign_messages_);
    s.store_field("show_message_sender", show_message_sender_);
    s.store_class_end();
  }
}
}  // namespace td_api
}  // namespace td
