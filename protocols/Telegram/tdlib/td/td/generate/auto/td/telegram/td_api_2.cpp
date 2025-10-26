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


accountInfo::accountInfo()
  : registration_month_()
  , registration_year_()
  , phone_number_country_code_()
  , last_name_change_date_()
  , last_photo_change_date_()
{}

accountInfo::accountInfo(int32 registration_month_, int32 registration_year_, string const &phone_number_country_code_, int32 last_name_change_date_, int32 last_photo_change_date_)
  : registration_month_(registration_month_)
  , registration_year_(registration_year_)
  , phone_number_country_code_(phone_number_country_code_)
  , last_name_change_date_(last_name_change_date_)
  , last_photo_change_date_(last_photo_change_date_)
{}

const std::int32_t accountInfo::ID;

void accountInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "accountInfo");
    s.store_field("registration_month", registration_month_);
    s.store_field("registration_year", registration_year_);
    s.store_field("phone_number_country_code", phone_number_country_code_);
    s.store_field("last_name_change_date", last_name_change_date_);
    s.store_field("last_photo_change_date", last_photo_change_date_);
    s.store_class_end();
  }
}

advertisementSponsor::advertisementSponsor()
  : url_()
  , photo_()
  , info_()
{}

advertisementSponsor::advertisementSponsor(string const &url_, object_ptr<photo> &&photo_, string const &info_)
  : url_(url_)
  , photo_(std::move(photo_))
  , info_(info_)
{}

const std::int32_t advertisementSponsor::ID;

void advertisementSponsor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "advertisementSponsor");
    s.store_field("url", url_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_field("info", info_);
    s.store_class_end();
  }
}

affiliateProgramInfo::affiliateProgramInfo()
  : parameters_()
  , end_date_()
  , daily_revenue_per_user_amount_()
{}

affiliateProgramInfo::affiliateProgramInfo(object_ptr<affiliateProgramParameters> &&parameters_, int32 end_date_, object_ptr<starAmount> &&daily_revenue_per_user_amount_)
  : parameters_(std::move(parameters_))
  , end_date_(end_date_)
  , daily_revenue_per_user_amount_(std::move(daily_revenue_per_user_amount_))
{}

const std::int32_t affiliateProgramInfo::ID;

void affiliateProgramInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "affiliateProgramInfo");
    s.store_object_field("parameters", static_cast<const BaseObject *>(parameters_.get()));
    s.store_field("end_date", end_date_);
    s.store_object_field("daily_revenue_per_user_amount", static_cast<const BaseObject *>(daily_revenue_per_user_amount_.get()));
    s.store_class_end();
  }
}

authenticationCodeInfo::authenticationCodeInfo()
  : phone_number_()
  , type_()
  , next_type_()
  , timeout_()
{}

authenticationCodeInfo::authenticationCodeInfo(string const &phone_number_, object_ptr<AuthenticationCodeType> &&type_, object_ptr<AuthenticationCodeType> &&next_type_, int32 timeout_)
  : phone_number_(phone_number_)
  , type_(std::move(type_))
  , next_type_(std::move(next_type_))
  , timeout_(timeout_)
{}

const std::int32_t authenticationCodeInfo::ID;

void authenticationCodeInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authenticationCodeInfo");
    s.store_field("phone_number", phone_number_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_object_field("next_type", static_cast<const BaseObject *>(next_type_.get()));
    s.store_field("timeout", timeout_);
    s.store_class_end();
  }
}

availableReactions::availableReactions()
  : top_reactions_()
  , recent_reactions_()
  , popular_reactions_()
  , allow_custom_emoji_()
  , are_tags_()
  , unavailability_reason_()
{}

availableReactions::availableReactions(array<object_ptr<availableReaction>> &&top_reactions_, array<object_ptr<availableReaction>> &&recent_reactions_, array<object_ptr<availableReaction>> &&popular_reactions_, bool allow_custom_emoji_, bool are_tags_, object_ptr<ReactionUnavailabilityReason> &&unavailability_reason_)
  : top_reactions_(std::move(top_reactions_))
  , recent_reactions_(std::move(recent_reactions_))
  , popular_reactions_(std::move(popular_reactions_))
  , allow_custom_emoji_(allow_custom_emoji_)
  , are_tags_(are_tags_)
  , unavailability_reason_(std::move(unavailability_reason_))
{}

const std::int32_t availableReactions::ID;

void availableReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "availableReactions");
    { s.store_vector_begin("top_reactions", top_reactions_.size()); for (const auto &_value : top_reactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("recent_reactions", recent_reactions_.size()); for (const auto &_value : recent_reactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("popular_reactions", popular_reactions_.size()); for (const auto &_value : popular_reactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("allow_custom_emoji", allow_custom_emoji_);
    s.store_field("are_tags", are_tags_);
    s.store_object_field("unavailability_reason", static_cast<const BaseObject *>(unavailability_reason_.get()));
    s.store_class_end();
  }
}

botMediaPreview::botMediaPreview()
  : date_()
  , content_()
{}

botMediaPreview::botMediaPreview(int32 date_, object_ptr<StoryContent> &&content_)
  : date_(date_)
  , content_(std::move(content_))
{}

const std::int32_t botMediaPreview::ID;

void botMediaPreview::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botMediaPreview");
    s.store_field("date", date_);
    s.store_object_field("content", static_cast<const BaseObject *>(content_.get()));
    s.store_class_end();
  }
}

businessFeaturePromotionAnimation::businessFeaturePromotionAnimation()
  : feature_()
  , animation_()
{}

businessFeaturePromotionAnimation::businessFeaturePromotionAnimation(object_ptr<BusinessFeature> &&feature_, object_ptr<animation> &&animation_)
  : feature_(std::move(feature_))
  , animation_(std::move(animation_))
{}

const std::int32_t businessFeaturePromotionAnimation::ID;

void businessFeaturePromotionAnimation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessFeaturePromotionAnimation");
    s.store_object_field("feature", static_cast<const BaseObject *>(feature_.get()));
    s.store_object_field("animation", static_cast<const BaseObject *>(animation_.get()));
    s.store_class_end();
  }
}

callbackQueryPayloadData::callbackQueryPayloadData()
  : data_()
{}

callbackQueryPayloadData::callbackQueryPayloadData(bytes const &data_)
  : data_(std::move(data_))
{}

const std::int32_t callbackQueryPayloadData::ID;

void callbackQueryPayloadData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callbackQueryPayloadData");
    s.store_bytes_field("data", data_);
    s.store_class_end();
  }
}

callbackQueryPayloadDataWithPassword::callbackQueryPayloadDataWithPassword()
  : password_()
  , data_()
{}

callbackQueryPayloadDataWithPassword::callbackQueryPayloadDataWithPassword(string const &password_, bytes const &data_)
  : password_(password_)
  , data_(std::move(data_))
{}

const std::int32_t callbackQueryPayloadDataWithPassword::ID;

void callbackQueryPayloadDataWithPassword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callbackQueryPayloadDataWithPassword");
    s.store_field("password", password_);
    s.store_bytes_field("data", data_);
    s.store_class_end();
  }
}

callbackQueryPayloadGame::callbackQueryPayloadGame()
  : game_short_name_()
{}

callbackQueryPayloadGame::callbackQueryPayloadGame(string const &game_short_name_)
  : game_short_name_(game_short_name_)
{}

const std::int32_t callbackQueryPayloadGame::ID;

void callbackQueryPayloadGame::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callbackQueryPayloadGame");
    s.store_field("game_short_name", game_short_name_);
    s.store_class_end();
  }
}

canTransferOwnershipResultOk::canTransferOwnershipResultOk() {
}

const std::int32_t canTransferOwnershipResultOk::ID;

void canTransferOwnershipResultOk::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canTransferOwnershipResultOk");
    s.store_class_end();
  }
}

canTransferOwnershipResultPasswordNeeded::canTransferOwnershipResultPasswordNeeded() {
}

const std::int32_t canTransferOwnershipResultPasswordNeeded::ID;

void canTransferOwnershipResultPasswordNeeded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canTransferOwnershipResultPasswordNeeded");
    s.store_class_end();
  }
}

canTransferOwnershipResultPasswordTooFresh::canTransferOwnershipResultPasswordTooFresh()
  : retry_after_()
{}

canTransferOwnershipResultPasswordTooFresh::canTransferOwnershipResultPasswordTooFresh(int32 retry_after_)
  : retry_after_(retry_after_)
{}

const std::int32_t canTransferOwnershipResultPasswordTooFresh::ID;

void canTransferOwnershipResultPasswordTooFresh::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canTransferOwnershipResultPasswordTooFresh");
    s.store_field("retry_after", retry_after_);
    s.store_class_end();
  }
}

canTransferOwnershipResultSessionTooFresh::canTransferOwnershipResultSessionTooFresh()
  : retry_after_()
{}

canTransferOwnershipResultSessionTooFresh::canTransferOwnershipResultSessionTooFresh(int32 retry_after_)
  : retry_after_(retry_after_)
{}

const std::int32_t canTransferOwnershipResultSessionTooFresh::ID;

void canTransferOwnershipResultSessionTooFresh::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canTransferOwnershipResultSessionTooFresh");
    s.store_field("retry_after", retry_after_);
    s.store_class_end();
  }
}

chatActiveStories::chatActiveStories()
  : chat_id_()
  , list_()
  , order_()
  , can_be_archived_()
  , max_read_story_id_()
  , stories_()
{}

chatActiveStories::chatActiveStories(int53 chat_id_, object_ptr<StoryList> &&list_, int53 order_, bool can_be_archived_, int32 max_read_story_id_, array<object_ptr<storyInfo>> &&stories_)
  : chat_id_(chat_id_)
  , list_(std::move(list_))
  , order_(order_)
  , can_be_archived_(can_be_archived_)
  , max_read_story_id_(max_read_story_id_)
  , stories_(std::move(stories_))
{}

const std::int32_t chatActiveStories::ID;

void chatActiveStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActiveStories");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("list", static_cast<const BaseObject *>(list_.get()));
    s.store_field("order", order_);
    s.store_field("can_be_archived", can_be_archived_);
    s.store_field("max_read_story_id", max_read_story_id_);
    { s.store_vector_begin("stories", stories_.size()); for (const auto &_value : stories_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatBoostLink::chatBoostLink()
  : link_()
  , is_public_()
{}

chatBoostLink::chatBoostLink(string const &link_, bool is_public_)
  : link_(link_)
  , is_public_(is_public_)
{}

const std::int32_t chatBoostLink::ID;

void chatBoostLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatBoostLink");
    s.store_field("link", link_);
    s.store_field("is_public", is_public_);
    s.store_class_end();
  }
}

chatBoostSlot::chatBoostSlot()
  : slot_id_()
  , currently_boosted_chat_id_()
  , start_date_()
  , expiration_date_()
  , cooldown_until_date_()
{}

chatBoostSlot::chatBoostSlot(int32 slot_id_, int53 currently_boosted_chat_id_, int32 start_date_, int32 expiration_date_, int32 cooldown_until_date_)
  : slot_id_(slot_id_)
  , currently_boosted_chat_id_(currently_boosted_chat_id_)
  , start_date_(start_date_)
  , expiration_date_(expiration_date_)
  , cooldown_until_date_(cooldown_until_date_)
{}

const std::int32_t chatBoostSlot::ID;

void chatBoostSlot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatBoostSlot");
    s.store_field("slot_id", slot_id_);
    s.store_field("currently_boosted_chat_id", currently_boosted_chat_id_);
    s.store_field("start_date", start_date_);
    s.store_field("expiration_date", expiration_date_);
    s.store_field("cooldown_until_date", cooldown_until_date_);
    s.store_class_end();
  }
}

chatFolderName::chatFolderName()
  : text_()
  , animate_custom_emoji_()
{}

chatFolderName::chatFolderName(object_ptr<formattedText> &&text_, bool animate_custom_emoji_)
  : text_(std::move(text_))
  , animate_custom_emoji_(animate_custom_emoji_)
{}

const std::int32_t chatFolderName::ID;

void chatFolderName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatFolderName");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("animate_custom_emoji", animate_custom_emoji_);
    s.store_class_end();
  }
}

chatInviteLinkCounts::chatInviteLinkCounts()
  : invite_link_counts_()
{}

chatInviteLinkCounts::chatInviteLinkCounts(array<object_ptr<chatInviteLinkCount>> &&invite_link_counts_)
  : invite_link_counts_(std::move(invite_link_counts_))
{}

const std::int32_t chatInviteLinkCounts::ID;

void chatInviteLinkCounts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatInviteLinkCounts");
    { s.store_vector_begin("invite_link_counts", invite_link_counts_.size()); for (const auto &_value : invite_link_counts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatLists::chatLists()
  : chat_lists_()
{}

chatLists::chatLists(array<object_ptr<ChatList>> &&chat_lists_)
  : chat_lists_(std::move(chat_lists_))
{}

const std::int32_t chatLists::ID;

void chatLists::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatLists");
    { s.store_vector_begin("chat_lists", chat_lists_.size()); for (const auto &_value : chat_lists_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatLocation::chatLocation()
  : location_()
  , address_()
{}

chatLocation::chatLocation(object_ptr<location> &&location_, string const &address_)
  : location_(std::move(location_))
  , address_(address_)
{}

const std::int32_t chatLocation::ID;

void chatLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatLocation");
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_field("address", address_);
    s.store_class_end();
  }
}

chatRevenueStatistics::chatRevenueStatistics()
  : revenue_by_hour_graph_()
  , revenue_graph_()
  , revenue_amount_()
  , usd_rate_()
{}

chatRevenueStatistics::chatRevenueStatistics(object_ptr<StatisticalGraph> &&revenue_by_hour_graph_, object_ptr<StatisticalGraph> &&revenue_graph_, object_ptr<chatRevenueAmount> &&revenue_amount_, double usd_rate_)
  : revenue_by_hour_graph_(std::move(revenue_by_hour_graph_))
  , revenue_graph_(std::move(revenue_graph_))
  , revenue_amount_(std::move(revenue_amount_))
  , usd_rate_(usd_rate_)
{}

const std::int32_t chatRevenueStatistics::ID;

void chatRevenueStatistics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatRevenueStatistics");
    s.store_object_field("revenue_by_hour_graph", static_cast<const BaseObject *>(revenue_by_hour_graph_.get()));
    s.store_object_field("revenue_graph", static_cast<const BaseObject *>(revenue_graph_.get()));
    s.store_object_field("revenue_amount", static_cast<const BaseObject *>(revenue_amount_.get()));
    s.store_field("usd_rate", usd_rate_);
    s.store_class_end();
  }
}

chatSourceMtprotoProxy::chatSourceMtprotoProxy() {
}

const std::int32_t chatSourceMtprotoProxy::ID;

void chatSourceMtprotoProxy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatSourceMtprotoProxy");
    s.store_class_end();
  }
}

chatSourcePublicServiceAnnouncement::chatSourcePublicServiceAnnouncement()
  : type_()
  , text_()
{}

chatSourcePublicServiceAnnouncement::chatSourcePublicServiceAnnouncement(string const &type_, string const &text_)
  : type_(type_)
  , text_(text_)
{}

const std::int32_t chatSourcePublicServiceAnnouncement::ID;

void chatSourcePublicServiceAnnouncement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatSourcePublicServiceAnnouncement");
    s.store_field("type", type_);
    s.store_field("text", text_);
    s.store_class_end();
  }
}

chatStatisticsSupergroup::chatStatisticsSupergroup()
  : period_()
  , member_count_()
  , message_count_()
  , viewer_count_()
  , sender_count_()
  , member_count_graph_()
  , join_graph_()
  , join_by_source_graph_()
  , language_graph_()
  , message_content_graph_()
  , action_graph_()
  , day_graph_()
  , week_graph_()
  , top_senders_()
  , top_administrators_()
  , top_inviters_()
{}

chatStatisticsSupergroup::chatStatisticsSupergroup(object_ptr<dateRange> &&period_, object_ptr<statisticalValue> &&member_count_, object_ptr<statisticalValue> &&message_count_, object_ptr<statisticalValue> &&viewer_count_, object_ptr<statisticalValue> &&sender_count_, object_ptr<StatisticalGraph> &&member_count_graph_, object_ptr<StatisticalGraph> &&join_graph_, object_ptr<StatisticalGraph> &&join_by_source_graph_, object_ptr<StatisticalGraph> &&language_graph_, object_ptr<StatisticalGraph> &&message_content_graph_, object_ptr<StatisticalGraph> &&action_graph_, object_ptr<StatisticalGraph> &&day_graph_, object_ptr<StatisticalGraph> &&week_graph_, array<object_ptr<chatStatisticsMessageSenderInfo>> &&top_senders_, array<object_ptr<chatStatisticsAdministratorActionsInfo>> &&top_administrators_, array<object_ptr<chatStatisticsInviterInfo>> &&top_inviters_)
  : period_(std::move(period_))
  , member_count_(std::move(member_count_))
  , message_count_(std::move(message_count_))
  , viewer_count_(std::move(viewer_count_))
  , sender_count_(std::move(sender_count_))
  , member_count_graph_(std::move(member_count_graph_))
  , join_graph_(std::move(join_graph_))
  , join_by_source_graph_(std::move(join_by_source_graph_))
  , language_graph_(std::move(language_graph_))
  , message_content_graph_(std::move(message_content_graph_))
  , action_graph_(std::move(action_graph_))
  , day_graph_(std::move(day_graph_))
  , week_graph_(std::move(week_graph_))
  , top_senders_(std::move(top_senders_))
  , top_administrators_(std::move(top_administrators_))
  , top_inviters_(std::move(top_inviters_))
{}

const std::int32_t chatStatisticsSupergroup::ID;

void chatStatisticsSupergroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatStatisticsSupergroup");
    s.store_object_field("period", static_cast<const BaseObject *>(period_.get()));
    s.store_object_field("member_count", static_cast<const BaseObject *>(member_count_.get()));
    s.store_object_field("message_count", static_cast<const BaseObject *>(message_count_.get()));
    s.store_object_field("viewer_count", static_cast<const BaseObject *>(viewer_count_.get()));
    s.store_object_field("sender_count", static_cast<const BaseObject *>(sender_count_.get()));
    s.store_object_field("member_count_graph", static_cast<const BaseObject *>(member_count_graph_.get()));
    s.store_object_field("join_graph", static_cast<const BaseObject *>(join_graph_.get()));
    s.store_object_field("join_by_source_graph", static_cast<const BaseObject *>(join_by_source_graph_.get()));
    s.store_object_field("language_graph", static_cast<const BaseObject *>(language_graph_.get()));
    s.store_object_field("message_content_graph", static_cast<const BaseObject *>(message_content_graph_.get()));
    s.store_object_field("action_graph", static_cast<const BaseObject *>(action_graph_.get()));
    s.store_object_field("day_graph", static_cast<const BaseObject *>(day_graph_.get()));
    s.store_object_field("week_graph", static_cast<const BaseObject *>(week_graph_.get()));
    { s.store_vector_begin("top_senders", top_senders_.size()); for (const auto &_value : top_senders_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("top_administrators", top_administrators_.size()); for (const auto &_value : top_administrators_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("top_inviters", top_inviters_.size()); for (const auto &_value : top_inviters_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatStatisticsChannel::chatStatisticsChannel()
  : period_()
  , member_count_()
  , mean_message_view_count_()
  , mean_message_share_count_()
  , mean_message_reaction_count_()
  , mean_story_view_count_()
  , mean_story_share_count_()
  , mean_story_reaction_count_()
  , enabled_notifications_percentage_()
  , member_count_graph_()
  , join_graph_()
  , mute_graph_()
  , view_count_by_hour_graph_()
  , view_count_by_source_graph_()
  , join_by_source_graph_()
  , language_graph_()
  , message_interaction_graph_()
  , message_reaction_graph_()
  , story_interaction_graph_()
  , story_reaction_graph_()
  , instant_view_interaction_graph_()
  , recent_interactions_()
{}

chatStatisticsChannel::chatStatisticsChannel(object_ptr<dateRange> &&period_, object_ptr<statisticalValue> &&member_count_, object_ptr<statisticalValue> &&mean_message_view_count_, object_ptr<statisticalValue> &&mean_message_share_count_, object_ptr<statisticalValue> &&mean_message_reaction_count_, object_ptr<statisticalValue> &&mean_story_view_count_, object_ptr<statisticalValue> &&mean_story_share_count_, object_ptr<statisticalValue> &&mean_story_reaction_count_, double enabled_notifications_percentage_, object_ptr<StatisticalGraph> &&member_count_graph_, object_ptr<StatisticalGraph> &&join_graph_, object_ptr<StatisticalGraph> &&mute_graph_, object_ptr<StatisticalGraph> &&view_count_by_hour_graph_, object_ptr<StatisticalGraph> &&view_count_by_source_graph_, object_ptr<StatisticalGraph> &&join_by_source_graph_, object_ptr<StatisticalGraph> &&language_graph_, object_ptr<StatisticalGraph> &&message_interaction_graph_, object_ptr<StatisticalGraph> &&message_reaction_graph_, object_ptr<StatisticalGraph> &&story_interaction_graph_, object_ptr<StatisticalGraph> &&story_reaction_graph_, object_ptr<StatisticalGraph> &&instant_view_interaction_graph_, array<object_ptr<chatStatisticsInteractionInfo>> &&recent_interactions_)
  : period_(std::move(period_))
  , member_count_(std::move(member_count_))
  , mean_message_view_count_(std::move(mean_message_view_count_))
  , mean_message_share_count_(std::move(mean_message_share_count_))
  , mean_message_reaction_count_(std::move(mean_message_reaction_count_))
  , mean_story_view_count_(std::move(mean_story_view_count_))
  , mean_story_share_count_(std::move(mean_story_share_count_))
  , mean_story_reaction_count_(std::move(mean_story_reaction_count_))
  , enabled_notifications_percentage_(enabled_notifications_percentage_)
  , member_count_graph_(std::move(member_count_graph_))
  , join_graph_(std::move(join_graph_))
  , mute_graph_(std::move(mute_graph_))
  , view_count_by_hour_graph_(std::move(view_count_by_hour_graph_))
  , view_count_by_source_graph_(std::move(view_count_by_source_graph_))
  , join_by_source_graph_(std::move(join_by_source_graph_))
  , language_graph_(std::move(language_graph_))
  , message_interaction_graph_(std::move(message_interaction_graph_))
  , message_reaction_graph_(std::move(message_reaction_graph_))
  , story_interaction_graph_(std::move(story_interaction_graph_))
  , story_reaction_graph_(std::move(story_reaction_graph_))
  , instant_view_interaction_graph_(std::move(instant_view_interaction_graph_))
  , recent_interactions_(std::move(recent_interactions_))
{}

const std::int32_t chatStatisticsChannel::ID;

void chatStatisticsChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatStatisticsChannel");
    s.store_object_field("period", static_cast<const BaseObject *>(period_.get()));
    s.store_object_field("member_count", static_cast<const BaseObject *>(member_count_.get()));
    s.store_object_field("mean_message_view_count", static_cast<const BaseObject *>(mean_message_view_count_.get()));
    s.store_object_field("mean_message_share_count", static_cast<const BaseObject *>(mean_message_share_count_.get()));
    s.store_object_field("mean_message_reaction_count", static_cast<const BaseObject *>(mean_message_reaction_count_.get()));
    s.store_object_field("mean_story_view_count", static_cast<const BaseObject *>(mean_story_view_count_.get()));
    s.store_object_field("mean_story_share_count", static_cast<const BaseObject *>(mean_story_share_count_.get()));
    s.store_object_field("mean_story_reaction_count", static_cast<const BaseObject *>(mean_story_reaction_count_.get()));
    s.store_field("enabled_notifications_percentage", enabled_notifications_percentage_);
    s.store_object_field("member_count_graph", static_cast<const BaseObject *>(member_count_graph_.get()));
    s.store_object_field("join_graph", static_cast<const BaseObject *>(join_graph_.get()));
    s.store_object_field("mute_graph", static_cast<const BaseObject *>(mute_graph_.get()));
    s.store_object_field("view_count_by_hour_graph", static_cast<const BaseObject *>(view_count_by_hour_graph_.get()));
    s.store_object_field("view_count_by_source_graph", static_cast<const BaseObject *>(view_count_by_source_graph_.get()));
    s.store_object_field("join_by_source_graph", static_cast<const BaseObject *>(join_by_source_graph_.get()));
    s.store_object_field("language_graph", static_cast<const BaseObject *>(language_graph_.get()));
    s.store_object_field("message_interaction_graph", static_cast<const BaseObject *>(message_interaction_graph_.get()));
    s.store_object_field("message_reaction_graph", static_cast<const BaseObject *>(message_reaction_graph_.get()));
    s.store_object_field("story_interaction_graph", static_cast<const BaseObject *>(story_interaction_graph_.get()));
    s.store_object_field("story_reaction_graph", static_cast<const BaseObject *>(story_reaction_graph_.get()));
    s.store_object_field("instant_view_interaction_graph", static_cast<const BaseObject *>(instant_view_interaction_graph_.get()));
    { s.store_vector_begin("recent_interactions", recent_interactions_.size()); for (const auto &_value : recent_interactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

currentWeather::currentWeather()
  : temperature_()
  , emoji_()
{}

currentWeather::currentWeather(double temperature_, string const &emoji_)
  : temperature_(temperature_)
  , emoji_(emoji_)
{}

const std::int32_t currentWeather::ID;

void currentWeather::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "currentWeather");
    s.store_field("temperature", temperature_);
    s.store_field("emoji", emoji_);
    s.store_class_end();
  }
}

dateRange::dateRange()
  : start_date_()
  , end_date_()
{}

dateRange::dateRange(int32 start_date_, int32 end_date_)
  : start_date_(start_date_)
  , end_date_(end_date_)
{}

const std::int32_t dateRange::ID;

void dateRange::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "dateRange");
    s.store_field("start_date", start_date_);
    s.store_field("end_date", end_date_);
    s.store_class_end();
  }
}

deviceTokenFirebaseCloudMessaging::deviceTokenFirebaseCloudMessaging()
  : token_()
  , encrypt_()
{}

deviceTokenFirebaseCloudMessaging::deviceTokenFirebaseCloudMessaging(string const &token_, bool encrypt_)
  : token_(token_)
  , encrypt_(encrypt_)
{}

const std::int32_t deviceTokenFirebaseCloudMessaging::ID;

void deviceTokenFirebaseCloudMessaging::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deviceTokenFirebaseCloudMessaging");
    s.store_field("token", token_);
    s.store_field("encrypt", encrypt_);
    s.store_class_end();
  }
}

deviceTokenApplePush::deviceTokenApplePush()
  : device_token_()
  , is_app_sandbox_()
{}

deviceTokenApplePush::deviceTokenApplePush(string const &device_token_, bool is_app_sandbox_)
  : device_token_(device_token_)
  , is_app_sandbox_(is_app_sandbox_)
{}

const std::int32_t deviceTokenApplePush::ID;

void deviceTokenApplePush::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deviceTokenApplePush");
    s.store_field("device_token", device_token_);
    s.store_field("is_app_sandbox", is_app_sandbox_);
    s.store_class_end();
  }
}

deviceTokenApplePushVoIP::deviceTokenApplePushVoIP()
  : device_token_()
  , is_app_sandbox_()
  , encrypt_()
{}

deviceTokenApplePushVoIP::deviceTokenApplePushVoIP(string const &device_token_, bool is_app_sandbox_, bool encrypt_)
  : device_token_(device_token_)
  , is_app_sandbox_(is_app_sandbox_)
  , encrypt_(encrypt_)
{}

const std::int32_t deviceTokenApplePushVoIP::ID;

void deviceTokenApplePushVoIP::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deviceTokenApplePushVoIP");
    s.store_field("device_token", device_token_);
    s.store_field("is_app_sandbox", is_app_sandbox_);
    s.store_field("encrypt", encrypt_);
    s.store_class_end();
  }
}

deviceTokenWindowsPush::deviceTokenWindowsPush()
  : access_token_()
{}

deviceTokenWindowsPush::deviceTokenWindowsPush(string const &access_token_)
  : access_token_(access_token_)
{}

const std::int32_t deviceTokenWindowsPush::ID;

void deviceTokenWindowsPush::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deviceTokenWindowsPush");
    s.store_field("access_token", access_token_);
    s.store_class_end();
  }
}

deviceTokenMicrosoftPush::deviceTokenMicrosoftPush()
  : channel_uri_()
{}

deviceTokenMicrosoftPush::deviceTokenMicrosoftPush(string const &channel_uri_)
  : channel_uri_(channel_uri_)
{}

const std::int32_t deviceTokenMicrosoftPush::ID;

void deviceTokenMicrosoftPush::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deviceTokenMicrosoftPush");
    s.store_field("channel_uri", channel_uri_);
    s.store_class_end();
  }
}

deviceTokenMicrosoftPushVoIP::deviceTokenMicrosoftPushVoIP()
  : channel_uri_()
{}

deviceTokenMicrosoftPushVoIP::deviceTokenMicrosoftPushVoIP(string const &channel_uri_)
  : channel_uri_(channel_uri_)
{}

const std::int32_t deviceTokenMicrosoftPushVoIP::ID;

void deviceTokenMicrosoftPushVoIP::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deviceTokenMicrosoftPushVoIP");
    s.store_field("channel_uri", channel_uri_);
    s.store_class_end();
  }
}

deviceTokenWebPush::deviceTokenWebPush()
  : endpoint_()
  , p256dh_base64url_()
  , auth_base64url_()
{}

deviceTokenWebPush::deviceTokenWebPush(string const &endpoint_, string const &p256dh_base64url_, string const &auth_base64url_)
  : endpoint_(endpoint_)
  , p256dh_base64url_(p256dh_base64url_)
  , auth_base64url_(auth_base64url_)
{}

const std::int32_t deviceTokenWebPush::ID;

void deviceTokenWebPush::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deviceTokenWebPush");
    s.store_field("endpoint", endpoint_);
    s.store_field("p256dh_base64url", p256dh_base64url_);
    s.store_field("auth_base64url", auth_base64url_);
    s.store_class_end();
  }
}

deviceTokenSimplePush::deviceTokenSimplePush()
  : endpoint_()
{}

deviceTokenSimplePush::deviceTokenSimplePush(string const &endpoint_)
  : endpoint_(endpoint_)
{}

const std::int32_t deviceTokenSimplePush::ID;

void deviceTokenSimplePush::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deviceTokenSimplePush");
    s.store_field("endpoint", endpoint_);
    s.store_class_end();
  }
}

deviceTokenUbuntuPush::deviceTokenUbuntuPush()
  : token_()
{}

deviceTokenUbuntuPush::deviceTokenUbuntuPush(string const &token_)
  : token_(token_)
{}

const std::int32_t deviceTokenUbuntuPush::ID;

void deviceTokenUbuntuPush::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deviceTokenUbuntuPush");
    s.store_field("token", token_);
    s.store_class_end();
  }
}

deviceTokenBlackBerryPush::deviceTokenBlackBerryPush()
  : token_()
{}

deviceTokenBlackBerryPush::deviceTokenBlackBerryPush(string const &token_)
  : token_(token_)
{}

const std::int32_t deviceTokenBlackBerryPush::ID;

void deviceTokenBlackBerryPush::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deviceTokenBlackBerryPush");
    s.store_field("token", token_);
    s.store_class_end();
  }
}

deviceTokenTizenPush::deviceTokenTizenPush()
  : reg_id_()
{}

deviceTokenTizenPush::deviceTokenTizenPush(string const &reg_id_)
  : reg_id_(reg_id_)
{}

const std::int32_t deviceTokenTizenPush::ID;

void deviceTokenTizenPush::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deviceTokenTizenPush");
    s.store_field("reg_id", reg_id_);
    s.store_class_end();
  }
}

deviceTokenHuaweiPush::deviceTokenHuaweiPush()
  : token_()
  , encrypt_()
{}

deviceTokenHuaweiPush::deviceTokenHuaweiPush(string const &token_, bool encrypt_)
  : token_(token_)
  , encrypt_(encrypt_)
{}

const std::int32_t deviceTokenHuaweiPush::ID;

void deviceTokenHuaweiPush::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deviceTokenHuaweiPush");
    s.store_field("token", token_);
    s.store_field("encrypt", encrypt_);
    s.store_class_end();
  }
}

downloadedFileCounts::downloadedFileCounts()
  : active_count_()
  , paused_count_()
  , completed_count_()
{}

downloadedFileCounts::downloadedFileCounts(int32 active_count_, int32 paused_count_, int32 completed_count_)
  : active_count_(active_count_)
  , paused_count_(paused_count_)
  , completed_count_(completed_count_)
{}

const std::int32_t downloadedFileCounts::ID;

void downloadedFileCounts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "downloadedFileCounts");
    s.store_field("active_count", active_count_);
    s.store_field("paused_count", paused_count_);
    s.store_field("completed_count", completed_count_);
    s.store_class_end();
  }
}

emojiKeyword::emojiKeyword()
  : emoji_()
  , keyword_()
{}

emojiKeyword::emojiKeyword(string const &emoji_, string const &keyword_)
  : emoji_(emoji_)
  , keyword_(keyword_)
{}

const std::int32_t emojiKeyword::ID;

void emojiKeyword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiKeyword");
    s.store_field("emoji", emoji_);
    s.store_field("keyword", keyword_);
    s.store_class_end();
  }
}

foundAffiliateProgram::foundAffiliateProgram()
  : bot_user_id_()
  , info_()
{}

foundAffiliateProgram::foundAffiliateProgram(int53 bot_user_id_, object_ptr<affiliateProgramInfo> &&info_)
  : bot_user_id_(bot_user_id_)
  , info_(std::move(info_))
{}

const std::int32_t foundAffiliateProgram::ID;

void foundAffiliateProgram::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "foundAffiliateProgram");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_object_field("info", static_cast<const BaseObject *>(info_.get()));
    s.store_class_end();
  }
}

foundFileDownloads::foundFileDownloads()
  : total_counts_()
  , files_()
  , next_offset_()
{}

foundFileDownloads::foundFileDownloads(object_ptr<downloadedFileCounts> &&total_counts_, array<object_ptr<fileDownload>> &&files_, string const &next_offset_)
  : total_counts_(std::move(total_counts_))
  , files_(std::move(files_))
  , next_offset_(next_offset_)
{}

const std::int32_t foundFileDownloads::ID;

void foundFileDownloads::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "foundFileDownloads");
    s.store_object_field("total_counts", static_cast<const BaseObject *>(total_counts_.get()));
    { s.store_vector_begin("files", files_.size()); for (const auto &_value : files_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

giftResaleResultOk::giftResaleResultOk() {
}

const std::int32_t giftResaleResultOk::ID;

void giftResaleResultOk::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giftResaleResultOk");
    s.store_class_end();
  }
}

giftResaleResultPriceIncreased::giftResaleResultPriceIncreased()
  : price_()
{}

giftResaleResultPriceIncreased::giftResaleResultPriceIncreased(object_ptr<GiftResalePrice> &&price_)
  : price_(std::move(price_))
{}

const std::int32_t giftResaleResultPriceIncreased::ID;

void giftResaleResultPriceIncreased::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giftResaleResultPriceIncreased");
    s.store_object_field("price", static_cast<const BaseObject *>(price_.get()));
    s.store_class_end();
  }
}

giftUpgradePrice::giftUpgradePrice()
  : date_()
  , star_count_()
{}

giftUpgradePrice::giftUpgradePrice(int32 date_, int53 star_count_)
  : date_(date_)
  , star_count_(star_count_)
{}

const std::int32_t giftUpgradePrice::ID;

void giftUpgradePrice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giftUpgradePrice");
    s.store_field("date", date_);
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

groupCallVideoQualityThumbnail::groupCallVideoQualityThumbnail() {
}

const std::int32_t groupCallVideoQualityThumbnail::ID;

void groupCallVideoQualityThumbnail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCallVideoQualityThumbnail");
    s.store_class_end();
  }
}

groupCallVideoQualityMedium::groupCallVideoQualityMedium() {
}

const std::int32_t groupCallVideoQualityMedium::ID;

void groupCallVideoQualityMedium::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCallVideoQualityMedium");
    s.store_class_end();
  }
}

groupCallVideoQualityFull::groupCallVideoQualityFull() {
}

const std::int32_t groupCallVideoQualityFull::ID;

void groupCallVideoQualityFull::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCallVideoQualityFull");
    s.store_class_end();
  }
}

hashtags::hashtags()
  : hashtags_()
{}

hashtags::hashtags(array<string> &&hashtags_)
  : hashtags_(std::move(hashtags_))
{}

const std::int32_t hashtags::ID;

void hashtags::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "hashtags");
    { s.store_vector_begin("hashtags", hashtags_.size()); for (const auto &_value : hashtags_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

inputBusinessStartPage::inputBusinessStartPage()
  : title_()
  , message_()
  , sticker_()
{}

inputBusinessStartPage::inputBusinessStartPage(string const &title_, string const &message_, object_ptr<InputFile> &&sticker_)
  : title_(title_)
  , message_(message_)
  , sticker_(std::move(sticker_))
{}

const std::int32_t inputBusinessStartPage::ID;

void inputBusinessStartPage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBusinessStartPage");
    s.store_field("title", title_);
    s.store_field("message", message_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

inputInvoiceMessage::inputInvoiceMessage()
  : chat_id_()
  , message_id_()
{}

inputInvoiceMessage::inputInvoiceMessage(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t inputInvoiceMessage::ID;

void inputInvoiceMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInvoiceMessage");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

inputInvoiceName::inputInvoiceName()
  : name_()
{}

inputInvoiceName::inputInvoiceName(string const &name_)
  : name_(name_)
{}

const std::int32_t inputInvoiceName::ID;

void inputInvoiceName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInvoiceName");
    s.store_field("name", name_);
    s.store_class_end();
  }
}

inputInvoiceTelegram::inputInvoiceTelegram()
  : purpose_()
{}

inputInvoiceTelegram::inputInvoiceTelegram(object_ptr<TelegramPaymentPurpose> &&purpose_)
  : purpose_(std::move(purpose_))
{}

const std::int32_t inputInvoiceTelegram::ID;

void inputInvoiceTelegram::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInvoiceTelegram");
    s.store_object_field("purpose", static_cast<const BaseObject *>(purpose_.get()));
    s.store_class_end();
  }
}

inputStoryAreas::inputStoryAreas()
  : areas_()
{}

inputStoryAreas::inputStoryAreas(array<object_ptr<inputStoryArea>> &&areas_)
  : areas_(std::move(areas_))
{}

const std::int32_t inputStoryAreas::ID;

void inputStoryAreas::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStoryAreas");
    { s.store_vector_begin("areas", areas_.size()); for (const auto &_value : areas_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

inviteLinkChatTypeBasicGroup::inviteLinkChatTypeBasicGroup() {
}

const std::int32_t inviteLinkChatTypeBasicGroup::ID;

void inviteLinkChatTypeBasicGroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inviteLinkChatTypeBasicGroup");
    s.store_class_end();
  }
}

inviteLinkChatTypeSupergroup::inviteLinkChatTypeSupergroup() {
}

const std::int32_t inviteLinkChatTypeSupergroup::ID;

void inviteLinkChatTypeSupergroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inviteLinkChatTypeSupergroup");
    s.store_class_end();
  }
}

inviteLinkChatTypeChannel::inviteLinkChatTypeChannel() {
}

const std::int32_t inviteLinkChatTypeChannel::ID;

void inviteLinkChatTypeChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inviteLinkChatTypeChannel");
    s.store_class_end();
  }
}

languagePackStrings::languagePackStrings()
  : strings_()
{}

languagePackStrings::languagePackStrings(array<object_ptr<languagePackString>> &&strings_)
  : strings_(std::move(strings_))
{}

const std::int32_t languagePackStrings::ID;

void languagePackStrings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "languagePackStrings");
    { s.store_vector_begin("strings", strings_.size()); for (const auto &_value : strings_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

linkPreviewAlbumMediaPhoto::linkPreviewAlbumMediaPhoto()
  : photo_()
{}

linkPreviewAlbumMediaPhoto::linkPreviewAlbumMediaPhoto(object_ptr<photo> &&photo_)
  : photo_(std::move(photo_))
{}

const std::int32_t linkPreviewAlbumMediaPhoto::ID;

void linkPreviewAlbumMediaPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewAlbumMediaPhoto");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

linkPreviewAlbumMediaVideo::linkPreviewAlbumMediaVideo()
  : video_()
{}

linkPreviewAlbumMediaVideo::linkPreviewAlbumMediaVideo(object_ptr<video> &&video_)
  : video_(std::move(video_))
{}

const std::int32_t linkPreviewAlbumMediaVideo::ID;

void linkPreviewAlbumMediaVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewAlbumMediaVideo");
    s.store_object_field("video", static_cast<const BaseObject *>(video_.get()));
    s.store_class_end();
  }
}

locationAddress::locationAddress()
  : country_code_()
  , state_()
  , city_()
  , street_()
{}

locationAddress::locationAddress(string const &country_code_, string const &state_, string const &city_, string const &street_)
  : country_code_(country_code_)
  , state_(state_)
  , city_(city_)
  , street_(street_)
{}

const std::int32_t locationAddress::ID;

void locationAddress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "locationAddress");
    s.store_field("country_code", country_code_);
    s.store_field("state", state_);
    s.store_field("city", city_);
    s.store_field("street", street_);
    s.store_class_end();
  }
}

maskPosition::maskPosition()
  : point_()
  , x_shift_()
  , y_shift_()
  , scale_()
{}

maskPosition::maskPosition(object_ptr<MaskPoint> &&point_, double x_shift_, double y_shift_, double scale_)
  : point_(std::move(point_))
  , x_shift_(x_shift_)
  , y_shift_(y_shift_)
  , scale_(scale_)
{}

const std::int32_t maskPosition::ID;

void maskPosition::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "maskPosition");
    s.store_object_field("point", static_cast<const BaseObject *>(point_.get()));
    s.store_field("x_shift", x_shift_);
    s.store_field("y_shift", y_shift_);
    s.store_field("scale", scale_);
    s.store_class_end();
  }
}

messageCalendarDay::messageCalendarDay()
  : total_count_()
  , message_()
{}

messageCalendarDay::messageCalendarDay(int32 total_count_, object_ptr<message> &&message_)
  : total_count_(total_count_)
  , message_(std::move(message_))
{}

const std::int32_t messageCalendarDay::ID;

void messageCalendarDay::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageCalendarDay");
    s.store_field("total_count", total_count_);
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

messageSendOptions::messageSendOptions()
  : suggested_post_info_()
  , disable_notification_()
  , from_background_()
  , protect_content_()
  , allow_paid_broadcast_()
  , paid_message_star_count_()
  , update_order_of_installed_sticker_sets_()
  , scheduling_state_()
  , effect_id_()
  , sending_id_()
  , only_preview_()
{}

messageSendOptions::messageSendOptions(object_ptr<inputSuggestedPostInfo> &&suggested_post_info_, bool disable_notification_, bool from_background_, bool protect_content_, bool allow_paid_broadcast_, int53 paid_message_star_count_, bool update_order_of_installed_sticker_sets_, object_ptr<MessageSchedulingState> &&scheduling_state_, int64 effect_id_, int32 sending_id_, bool only_preview_)
  : suggested_post_info_(std::move(suggested_post_info_))
  , disable_notification_(disable_notification_)
  , from_background_(from_background_)
  , protect_content_(protect_content_)
  , allow_paid_broadcast_(allow_paid_broadcast_)
  , paid_message_star_count_(paid_message_star_count_)
  , update_order_of_installed_sticker_sets_(update_order_of_installed_sticker_sets_)
  , scheduling_state_(std::move(scheduling_state_))
  , effect_id_(effect_id_)
  , sending_id_(sending_id_)
  , only_preview_(only_preview_)
{}

const std::int32_t messageSendOptions::ID;

void messageSendOptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSendOptions");
    s.store_object_field("suggested_post_info", static_cast<const BaseObject *>(suggested_post_info_.get()));
    s.store_field("disable_notification", disable_notification_);
    s.store_field("from_background", from_background_);
    s.store_field("protect_content", protect_content_);
    s.store_field("allow_paid_broadcast", allow_paid_broadcast_);
    s.store_field("paid_message_star_count", paid_message_star_count_);
    s.store_field("update_order_of_installed_sticker_sets", update_order_of_installed_sticker_sets_);
    s.store_object_field("scheduling_state", static_cast<const BaseObject *>(scheduling_state_.get()));
    s.store_field("effect_id", effect_id_);
    s.store_field("sending_id", sending_id_);
    s.store_field("only_preview", only_preview_);
    s.store_class_end();
  }
}

newChatPrivacySettings::newChatPrivacySettings()
  : allow_new_chats_from_unknown_users_()
  , incoming_paid_message_star_count_()
{}

newChatPrivacySettings::newChatPrivacySettings(bool allow_new_chats_from_unknown_users_, int53 incoming_paid_message_star_count_)
  : allow_new_chats_from_unknown_users_(allow_new_chats_from_unknown_users_)
  , incoming_paid_message_star_count_(incoming_paid_message_star_count_)
{}

const std::int32_t newChatPrivacySettings::ID;

void newChatPrivacySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "newChatPrivacySettings");
    s.store_field("allow_new_chats_from_unknown_users", allow_new_chats_from_unknown_users_);
    s.store_field("incoming_paid_message_star_count", incoming_paid_message_star_count_);
    s.store_class_end();
  }
}

notification::notification()
  : id_()
  , date_()
  , is_silent_()
  , type_()
{}

notification::notification(int32 id_, int32 date_, bool is_silent_, object_ptr<NotificationType> &&type_)
  : id_(id_)
  , date_(date_)
  , is_silent_(is_silent_)
  , type_(std::move(type_))
{}

const std::int32_t notification::ID;

void notification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notification");
    s.store_field("id", id_);
    s.store_field("date", date_);
    s.store_field("is_silent", is_silent_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

notificationTypeNewMessage::notificationTypeNewMessage()
  : message_()
  , show_preview_()
{}

notificationTypeNewMessage::notificationTypeNewMessage(object_ptr<message> &&message_, bool show_preview_)
  : message_(std::move(message_))
  , show_preview_(show_preview_)
{}

const std::int32_t notificationTypeNewMessage::ID;

void notificationTypeNewMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notificationTypeNewMessage");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_field("show_preview", show_preview_);
    s.store_class_end();
  }
}

notificationTypeNewSecretChat::notificationTypeNewSecretChat() {
}

const std::int32_t notificationTypeNewSecretChat::ID;

void notificationTypeNewSecretChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notificationTypeNewSecretChat");
    s.store_class_end();
  }
}

notificationTypeNewCall::notificationTypeNewCall()
  : call_id_()
{}

notificationTypeNewCall::notificationTypeNewCall(int32 call_id_)
  : call_id_(call_id_)
{}

const std::int32_t notificationTypeNewCall::ID;

void notificationTypeNewCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notificationTypeNewCall");
    s.store_field("call_id", call_id_);
    s.store_class_end();
  }
}

notificationTypeNewPushMessage::notificationTypeNewPushMessage()
  : message_id_()
  , sender_id_()
  , sender_name_()
  , is_outgoing_()
  , content_()
{}

notificationTypeNewPushMessage::notificationTypeNewPushMessage(int53 message_id_, object_ptr<MessageSender> &&sender_id_, string const &sender_name_, bool is_outgoing_, object_ptr<PushMessageContent> &&content_)
  : message_id_(message_id_)
  , sender_id_(std::move(sender_id_))
  , sender_name_(sender_name_)
  , is_outgoing_(is_outgoing_)
  , content_(std::move(content_))
{}

const std::int32_t notificationTypeNewPushMessage::ID;

void notificationTypeNewPushMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notificationTypeNewPushMessage");
    s.store_field("message_id", message_id_);
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_field("sender_name", sender_name_);
    s.store_field("is_outgoing", is_outgoing_);
    s.store_object_field("content", static_cast<const BaseObject *>(content_.get()));
    s.store_class_end();
  }
}

paymentOption::paymentOption()
  : title_()
  , url_()
{}

paymentOption::paymentOption(string const &title_, string const &url_)
  : title_(title_)
  , url_(url_)
{}

const std::int32_t paymentOption::ID;

void paymentOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paymentOption");
    s.store_field("title", title_);
    s.store_field("url", url_);
    s.store_class_end();
  }
}

paymentResult::paymentResult()
  : success_()
  , verification_url_()
{}

paymentResult::paymentResult(bool success_, string const &verification_url_)
  : success_(success_)
  , verification_url_(verification_url_)
{}

const std::int32_t paymentResult::ID;

void paymentResult::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paymentResult");
    s.store_field("success", success_);
    s.store_field("verification_url", verification_url_);
    s.store_class_end();
  }
}

phoneNumberInfo::phoneNumberInfo()
  : country_()
  , country_calling_code_()
  , formatted_phone_number_()
  , is_anonymous_()
{}

phoneNumberInfo::phoneNumberInfo(object_ptr<countryInfo> &&country_, string const &country_calling_code_, string const &formatted_phone_number_, bool is_anonymous_)
  : country_(std::move(country_))
  , country_calling_code_(country_calling_code_)
  , formatted_phone_number_(formatted_phone_number_)
  , is_anonymous_(is_anonymous_)
{}

const std::int32_t phoneNumberInfo::ID;

void phoneNumberInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phoneNumberInfo");
    s.store_object_field("country", static_cast<const BaseObject *>(country_.get()));
    s.store_field("country_calling_code", country_calling_code_);
    s.store_field("formatted_phone_number", formatted_phone_number_);
    s.store_field("is_anonymous", is_anonymous_);
    s.store_class_end();
  }
}

premiumFeatureIncreasedLimits::premiumFeatureIncreasedLimits() {
}

const std::int32_t premiumFeatureIncreasedLimits::ID;

void premiumFeatureIncreasedLimits::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureIncreasedLimits");
    s.store_class_end();
  }
}

premiumFeatureIncreasedUploadFileSize::premiumFeatureIncreasedUploadFileSize() {
}

const std::int32_t premiumFeatureIncreasedUploadFileSize::ID;

void premiumFeatureIncreasedUploadFileSize::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureIncreasedUploadFileSize");
    s.store_class_end();
  }
}

premiumFeatureImprovedDownloadSpeed::premiumFeatureImprovedDownloadSpeed() {
}

const std::int32_t premiumFeatureImprovedDownloadSpeed::ID;

void premiumFeatureImprovedDownloadSpeed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureImprovedDownloadSpeed");
    s.store_class_end();
  }
}

premiumFeatureVoiceRecognition::premiumFeatureVoiceRecognition() {
}

const std::int32_t premiumFeatureVoiceRecognition::ID;

void premiumFeatureVoiceRecognition::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureVoiceRecognition");
    s.store_class_end();
  }
}

premiumFeatureDisabledAds::premiumFeatureDisabledAds() {
}

const std::int32_t premiumFeatureDisabledAds::ID;

void premiumFeatureDisabledAds::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureDisabledAds");
    s.store_class_end();
  }
}

premiumFeatureUniqueReactions::premiumFeatureUniqueReactions() {
}

const std::int32_t premiumFeatureUniqueReactions::ID;

void premiumFeatureUniqueReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureUniqueReactions");
    s.store_class_end();
  }
}

premiumFeatureUniqueStickers::premiumFeatureUniqueStickers() {
}

const std::int32_t premiumFeatureUniqueStickers::ID;

void premiumFeatureUniqueStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureUniqueStickers");
    s.store_class_end();
  }
}

premiumFeatureCustomEmoji::premiumFeatureCustomEmoji() {
}

const std::int32_t premiumFeatureCustomEmoji::ID;

void premiumFeatureCustomEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureCustomEmoji");
    s.store_class_end();
  }
}

premiumFeatureAdvancedChatManagement::premiumFeatureAdvancedChatManagement() {
}

const std::int32_t premiumFeatureAdvancedChatManagement::ID;

void premiumFeatureAdvancedChatManagement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureAdvancedChatManagement");
    s.store_class_end();
  }
}

premiumFeatureProfileBadge::premiumFeatureProfileBadge() {
}

const std::int32_t premiumFeatureProfileBadge::ID;

void premiumFeatureProfileBadge::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureProfileBadge");
    s.store_class_end();
  }
}

premiumFeatureEmojiStatus::premiumFeatureEmojiStatus() {
}

const std::int32_t premiumFeatureEmojiStatus::ID;

void premiumFeatureEmojiStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureEmojiStatus");
    s.store_class_end();
  }
}

premiumFeatureAnimatedProfilePhoto::premiumFeatureAnimatedProfilePhoto() {
}

const std::int32_t premiumFeatureAnimatedProfilePhoto::ID;

void premiumFeatureAnimatedProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureAnimatedProfilePhoto");
    s.store_class_end();
  }
}

premiumFeatureForumTopicIcon::premiumFeatureForumTopicIcon() {
}

const std::int32_t premiumFeatureForumTopicIcon::ID;

void premiumFeatureForumTopicIcon::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureForumTopicIcon");
    s.store_class_end();
  }
}

premiumFeatureAppIcons::premiumFeatureAppIcons() {
}

const std::int32_t premiumFeatureAppIcons::ID;

void premiumFeatureAppIcons::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureAppIcons");
    s.store_class_end();
  }
}

premiumFeatureRealTimeChatTranslation::premiumFeatureRealTimeChatTranslation() {
}

const std::int32_t premiumFeatureRealTimeChatTranslation::ID;

void premiumFeatureRealTimeChatTranslation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureRealTimeChatTranslation");
    s.store_class_end();
  }
}

premiumFeatureUpgradedStories::premiumFeatureUpgradedStories() {
}

const std::int32_t premiumFeatureUpgradedStories::ID;

void premiumFeatureUpgradedStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureUpgradedStories");
    s.store_class_end();
  }
}

premiumFeatureChatBoost::premiumFeatureChatBoost() {
}

const std::int32_t premiumFeatureChatBoost::ID;

void premiumFeatureChatBoost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureChatBoost");
    s.store_class_end();
  }
}

premiumFeatureAccentColor::premiumFeatureAccentColor() {
}

const std::int32_t premiumFeatureAccentColor::ID;

void premiumFeatureAccentColor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureAccentColor");
    s.store_class_end();
  }
}

premiumFeatureBackgroundForBoth::premiumFeatureBackgroundForBoth() {
}

const std::int32_t premiumFeatureBackgroundForBoth::ID;

void premiumFeatureBackgroundForBoth::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureBackgroundForBoth");
    s.store_class_end();
  }
}

premiumFeatureSavedMessagesTags::premiumFeatureSavedMessagesTags() {
}

const std::int32_t premiumFeatureSavedMessagesTags::ID;

void premiumFeatureSavedMessagesTags::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureSavedMessagesTags");
    s.store_class_end();
  }
}

premiumFeatureMessagePrivacy::premiumFeatureMessagePrivacy() {
}

const std::int32_t premiumFeatureMessagePrivacy::ID;

void premiumFeatureMessagePrivacy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureMessagePrivacy");
    s.store_class_end();
  }
}

premiumFeatureLastSeenTimes::premiumFeatureLastSeenTimes() {
}

const std::int32_t premiumFeatureLastSeenTimes::ID;

void premiumFeatureLastSeenTimes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureLastSeenTimes");
    s.store_class_end();
  }
}

premiumFeatureBusiness::premiumFeatureBusiness() {
}

const std::int32_t premiumFeatureBusiness::ID;

void premiumFeatureBusiness::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureBusiness");
    s.store_class_end();
  }
}

premiumFeatureMessageEffects::premiumFeatureMessageEffects() {
}

const std::int32_t premiumFeatureMessageEffects::ID;

void premiumFeatureMessageEffects::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureMessageEffects");
    s.store_class_end();
  }
}

premiumFeatureChecklists::premiumFeatureChecklists() {
}

const std::int32_t premiumFeatureChecklists::ID;

void premiumFeatureChecklists::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumFeatureChecklists");
    s.store_class_end();
  }
}

profileAccentColors::profileAccentColors()
  : palette_colors_()
  , background_colors_()
  , story_colors_()
{}

profileAccentColors::profileAccentColors(array<int32> &&palette_colors_, array<int32> &&background_colors_, array<int32> &&story_colors_)
  : palette_colors_(std::move(palette_colors_))
  , background_colors_(std::move(background_colors_))
  , story_colors_(std::move(story_colors_))
{}

const std::int32_t profileAccentColors::ID;

void profileAccentColors::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "profileAccentColors");
    { s.store_vector_begin("palette_colors", palette_colors_.size()); for (const auto &_value : palette_colors_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("background_colors", background_colors_.size()); for (const auto &_value : background_colors_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("story_colors", story_colors_.size()); for (const auto &_value : story_colors_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

quickReplyMessages::quickReplyMessages()
  : messages_()
{}

quickReplyMessages::quickReplyMessages(array<object_ptr<quickReplyMessage>> &&messages_)
  : messages_(std::move(messages_))
{}

const std::int32_t quickReplyMessages::ID;

void quickReplyMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "quickReplyMessages");
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

readDatePrivacySettings::readDatePrivacySettings()
  : show_read_date_()
{}

readDatePrivacySettings::readDatePrivacySettings(bool show_read_date_)
  : show_read_date_(show_read_date_)
{}

const std::int32_t readDatePrivacySettings::ID;

void readDatePrivacySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "readDatePrivacySettings");
    s.store_field("show_read_date", show_read_date_);
    s.store_class_end();
  }
}

recommendedChatFolder::recommendedChatFolder()
  : folder_()
  , description_()
{}

recommendedChatFolder::recommendedChatFolder(object_ptr<chatFolder> &&folder_, string const &description_)
  : folder_(std::move(folder_))
  , description_(description_)
{}

const std::int32_t recommendedChatFolder::ID;

void recommendedChatFolder::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "recommendedChatFolder");
    s.store_object_field("folder", static_cast<const BaseObject *>(folder_.get()));
    s.store_field("description", description_);
    s.store_class_end();
  }
}

resetPasswordResultOk::resetPasswordResultOk() {
}

const std::int32_t resetPasswordResultOk::ID;

void resetPasswordResultOk::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "resetPasswordResultOk");
    s.store_class_end();
  }
}

resetPasswordResultPending::resetPasswordResultPending()
  : pending_reset_date_()
{}

resetPasswordResultPending::resetPasswordResultPending(int32 pending_reset_date_)
  : pending_reset_date_(pending_reset_date_)
{}

const std::int32_t resetPasswordResultPending::ID;

void resetPasswordResultPending::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "resetPasswordResultPending");
    s.store_field("pending_reset_date", pending_reset_date_);
    s.store_class_end();
  }
}

resetPasswordResultDeclined::resetPasswordResultDeclined()
  : retry_date_()
{}

resetPasswordResultDeclined::resetPasswordResultDeclined(int32 retry_date_)
  : retry_date_(retry_date_)
{}

const std::int32_t resetPasswordResultDeclined::ID;

void resetPasswordResultDeclined::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "resetPasswordResultDeclined");
    s.store_field("retry_date", retry_date_);
    s.store_class_end();
  }
}

secretChatStatePending::secretChatStatePending() {
}

const std::int32_t secretChatStatePending::ID;

void secretChatStatePending::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secretChatStatePending");
    s.store_class_end();
  }
}

secretChatStateReady::secretChatStateReady() {
}

const std::int32_t secretChatStateReady::ID;

void secretChatStateReady::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secretChatStateReady");
    s.store_class_end();
  }
}

secretChatStateClosed::secretChatStateClosed() {
}

const std::int32_t secretChatStateClosed::ID;

void secretChatStateClosed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secretChatStateClosed");
    s.store_class_end();
  }
}

sessions::sessions()
  : sessions_()
  , inactive_session_ttl_days_()
{}

sessions::sessions(array<object_ptr<session>> &&sessions_, int32 inactive_session_ttl_days_)
  : sessions_(std::move(sessions_))
  , inactive_session_ttl_days_(inactive_session_ttl_days_)
{}

const std::int32_t sessions::ID;

void sessions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sessions");
    { s.store_vector_begin("sessions", sessions_.size()); for (const auto &_value : sessions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("inactive_session_ttl_days", inactive_session_ttl_days_);
    s.store_class_end();
  }
}

shippingOption::shippingOption()
  : id_()
  , title_()
  , price_parts_()
{}

shippingOption::shippingOption(string const &id_, string const &title_, array<object_ptr<labeledPricePart>> &&price_parts_)
  : id_(id_)
  , title_(title_)
  , price_parts_(std::move(price_parts_))
{}

const std::int32_t shippingOption::ID;

void shippingOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "shippingOption");
    s.store_field("id", id_);
    s.store_field("title", title_);
    { s.store_vector_begin("price_parts", price_parts_.size()); for (const auto &_value : price_parts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

starRevenueStatistics::starRevenueStatistics()
  : revenue_by_day_graph_()
  , status_()
  , usd_rate_()
{}

starRevenueStatistics::starRevenueStatistics(object_ptr<StatisticalGraph> &&revenue_by_day_graph_, object_ptr<starRevenueStatus> &&status_, double usd_rate_)
  : revenue_by_day_graph_(std::move(revenue_by_day_graph_))
  , status_(std::move(status_))
  , usd_rate_(usd_rate_)
{}

const std::int32_t starRevenueStatistics::ID;

void starRevenueStatistics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starRevenueStatistics");
    s.store_object_field("revenue_by_day_graph", static_cast<const BaseObject *>(revenue_by_day_graph_.get()));
    s.store_object_field("status", static_cast<const BaseObject *>(status_.get()));
    s.store_field("usd_rate", usd_rate_);
    s.store_class_end();
  }
}

starSubscriptionTypeChannel::starSubscriptionTypeChannel()
  : can_reuse_()
  , invite_link_()
{}

starSubscriptionTypeChannel::starSubscriptionTypeChannel(bool can_reuse_, string const &invite_link_)
  : can_reuse_(can_reuse_)
  , invite_link_(invite_link_)
{}

const std::int32_t starSubscriptionTypeChannel::ID;

void starSubscriptionTypeChannel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starSubscriptionTypeChannel");
    s.store_field("can_reuse", can_reuse_);
    s.store_field("invite_link", invite_link_);
    s.store_class_end();
  }
}

starSubscriptionTypeBot::starSubscriptionTypeBot()
  : is_canceled_by_bot_()
  , title_()
  , photo_()
  , invoice_link_()
{}

starSubscriptionTypeBot::starSubscriptionTypeBot(bool is_canceled_by_bot_, string const &title_, object_ptr<photo> &&photo_, string const &invoice_link_)
  : is_canceled_by_bot_(is_canceled_by_bot_)
  , title_(title_)
  , photo_(std::move(photo_))
  , invoice_link_(invoice_link_)
{}

const std::int32_t starSubscriptionTypeBot::ID;

void starSubscriptionTypeBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starSubscriptionTypeBot");
    s.store_field("is_canceled_by_bot", is_canceled_by_bot_);
    s.store_field("title", title_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_field("invoice_link", invoice_link_);
    s.store_class_end();
  }
}

sticker::sticker()
  : id_()
  , set_id_()
  , width_()
  , height_()
  , emoji_()
  , format_()
  , full_type_()
  , thumbnail_()
  , sticker_()
{}

sticker::sticker(int64 id_, int64 set_id_, int32 width_, int32 height_, string const &emoji_, object_ptr<StickerFormat> &&format_, object_ptr<StickerFullType> &&full_type_, object_ptr<thumbnail> &&thumbnail_, object_ptr<file> &&sticker_)
  : id_(id_)
  , set_id_(set_id_)
  , width_(width_)
  , height_(height_)
  , emoji_(emoji_)
  , format_(std::move(format_))
  , full_type_(std::move(full_type_))
  , thumbnail_(std::move(thumbnail_))
  , sticker_(std::move(sticker_))
{}

const std::int32_t sticker::ID;

void sticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sticker");
    s.store_field("id", id_);
    s.store_field("set_id", set_id_);
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_field("emoji", emoji_);
    s.store_object_field("format", static_cast<const BaseObject *>(format_.get()));
    s.store_object_field("full_type", static_cast<const BaseObject *>(full_type_.get()));
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

storageStatisticsByChat::storageStatisticsByChat()
  : chat_id_()
  , size_()
  , count_()
  , by_file_type_()
{}

storageStatisticsByChat::storageStatisticsByChat(int53 chat_id_, int53 size_, int32 count_, array<object_ptr<storageStatisticsByFileType>> &&by_file_type_)
  : chat_id_(chat_id_)
  , size_(size_)
  , count_(count_)
  , by_file_type_(std::move(by_file_type_))
{}

const std::int32_t storageStatisticsByChat::ID;

void storageStatisticsByChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storageStatisticsByChat");
    s.store_field("chat_id", chat_id_);
    s.store_field("size", size_);
    s.store_field("count", count_);
    { s.store_vector_begin("by_file_type", by_file_type_.size()); for (const auto &_value : by_file_type_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

storyAreaPosition::storyAreaPosition()
  : x_percentage_()
  , y_percentage_()
  , width_percentage_()
  , height_percentage_()
  , rotation_angle_()
  , corner_radius_percentage_()
{}

storyAreaPosition::storyAreaPosition(double x_percentage_, double y_percentage_, double width_percentage_, double height_percentage_, double rotation_angle_, double corner_radius_percentage_)
  : x_percentage_(x_percentage_)
  , y_percentage_(y_percentage_)
  , width_percentage_(width_percentage_)
  , height_percentage_(height_percentage_)
  , rotation_angle_(rotation_angle_)
  , corner_radius_percentage_(corner_radius_percentage_)
{}

const std::int32_t storyAreaPosition::ID;

void storyAreaPosition::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyAreaPosition");
    s.store_field("x_percentage", x_percentage_);
    s.store_field("y_percentage", y_percentage_);
    s.store_field("width_percentage", width_percentage_);
    s.store_field("height_percentage", height_percentage_);
    s.store_field("rotation_angle", rotation_angle_);
    s.store_field("corner_radius_percentage", corner_radius_percentage_);
    s.store_class_end();
  }
}

storyFullId::storyFullId()
  : poster_chat_id_()
  , story_id_()
{}

storyFullId::storyFullId(int53 poster_chat_id_, int32 story_id_)
  : poster_chat_id_(poster_chat_id_)
  , story_id_(story_id_)
{}

const std::int32_t storyFullId::ID;

void storyFullId::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyFullId");
    s.store_field("poster_chat_id", poster_chat_id_);
    s.store_field("story_id", story_id_);
    s.store_class_end();
  }
}

storyInteraction::storyInteraction()
  : actor_id_()
  , interaction_date_()
  , block_list_()
  , type_()
{}

storyInteraction::storyInteraction(object_ptr<MessageSender> &&actor_id_, int32 interaction_date_, object_ptr<BlockList> &&block_list_, object_ptr<StoryInteractionType> &&type_)
  : actor_id_(std::move(actor_id_))
  , interaction_date_(interaction_date_)
  , block_list_(std::move(block_list_))
  , type_(std::move(type_))
{}

const std::int32_t storyInteraction::ID;

void storyInteraction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyInteraction");
    s.store_object_field("actor_id", static_cast<const BaseObject *>(actor_id_.get()));
    s.store_field("interaction_date", interaction_date_);
    s.store_object_field("block_list", static_cast<const BaseObject *>(block_list_.get()));
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

storyInteractionInfo::storyInteractionInfo()
  : view_count_()
  , forward_count_()
  , reaction_count_()
  , recent_viewer_user_ids_()
{}

storyInteractionInfo::storyInteractionInfo(int32 view_count_, int32 forward_count_, int32 reaction_count_, array<int53> &&recent_viewer_user_ids_)
  : view_count_(view_count_)
  , forward_count_(forward_count_)
  , reaction_count_(reaction_count_)
  , recent_viewer_user_ids_(std::move(recent_viewer_user_ids_))
{}

const std::int32_t storyInteractionInfo::ID;

void storyInteractionInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyInteractionInfo");
    s.store_field("view_count", view_count_);
    s.store_field("forward_count", forward_count_);
    s.store_field("reaction_count", reaction_count_);
    { s.store_vector_begin("recent_viewer_user_ids", recent_viewer_user_ids_.size()); for (const auto &_value : recent_viewer_user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

storyOriginPublicStory::storyOriginPublicStory()
  : chat_id_()
  , story_id_()
{}

storyOriginPublicStory::storyOriginPublicStory(int53 chat_id_, int32 story_id_)
  : chat_id_(chat_id_)
  , story_id_(story_id_)
{}

const std::int32_t storyOriginPublicStory::ID;

void storyOriginPublicStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyOriginPublicStory");
    s.store_field("chat_id", chat_id_);
    s.store_field("story_id", story_id_);
    s.store_class_end();
  }
}

storyOriginHiddenUser::storyOriginHiddenUser()
  : poster_name_()
{}

storyOriginHiddenUser::storyOriginHiddenUser(string const &poster_name_)
  : poster_name_(poster_name_)
{}

const std::int32_t storyOriginHiddenUser::ID;

void storyOriginHiddenUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyOriginHiddenUser");
    s.store_field("poster_name", poster_name_);
    s.store_class_end();
  }
}

supergroupMembersFilterRecent::supergroupMembersFilterRecent() {
}

const std::int32_t supergroupMembersFilterRecent::ID;

void supergroupMembersFilterRecent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "supergroupMembersFilterRecent");
    s.store_class_end();
  }
}

supergroupMembersFilterContacts::supergroupMembersFilterContacts()
  : query_()
{}

supergroupMembersFilterContacts::supergroupMembersFilterContacts(string const &query_)
  : query_(query_)
{}

const std::int32_t supergroupMembersFilterContacts::ID;

void supergroupMembersFilterContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "supergroupMembersFilterContacts");
    s.store_field("query", query_);
    s.store_class_end();
  }
}

supergroupMembersFilterAdministrators::supergroupMembersFilterAdministrators() {
}

const std::int32_t supergroupMembersFilterAdministrators::ID;

void supergroupMembersFilterAdministrators::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "supergroupMembersFilterAdministrators");
    s.store_class_end();
  }
}

supergroupMembersFilterSearch::supergroupMembersFilterSearch()
  : query_()
{}

supergroupMembersFilterSearch::supergroupMembersFilterSearch(string const &query_)
  : query_(query_)
{}

const std::int32_t supergroupMembersFilterSearch::ID;

void supergroupMembersFilterSearch::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "supergroupMembersFilterSearch");
    s.store_field("query", query_);
    s.store_class_end();
  }
}

supergroupMembersFilterRestricted::supergroupMembersFilterRestricted()
  : query_()
{}

supergroupMembersFilterRestricted::supergroupMembersFilterRestricted(string const &query_)
  : query_(query_)
{}

const std::int32_t supergroupMembersFilterRestricted::ID;

void supergroupMembersFilterRestricted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "supergroupMembersFilterRestricted");
    s.store_field("query", query_);
    s.store_class_end();
  }
}

supergroupMembersFilterBanned::supergroupMembersFilterBanned()
  : query_()
{}

supergroupMembersFilterBanned::supergroupMembersFilterBanned(string const &query_)
  : query_(query_)
{}

const std::int32_t supergroupMembersFilterBanned::ID;

void supergroupMembersFilterBanned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "supergroupMembersFilterBanned");
    s.store_field("query", query_);
    s.store_class_end();
  }
}

supergroupMembersFilterMention::supergroupMembersFilterMention()
  : query_()
  , topic_id_()
{}

supergroupMembersFilterMention::supergroupMembersFilterMention(string const &query_, object_ptr<MessageTopic> &&topic_id_)
  : query_(query_)
  , topic_id_(std::move(topic_id_))
{}

const std::int32_t supergroupMembersFilterMention::ID;

void supergroupMembersFilterMention::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "supergroupMembersFilterMention");
    s.store_field("query", query_);
    s.store_object_field("topic_id", static_cast<const BaseObject *>(topic_id_.get()));
    s.store_class_end();
  }
}

supergroupMembersFilterBots::supergroupMembersFilterBots() {
}

const std::int32_t supergroupMembersFilterBots::ID;

void supergroupMembersFilterBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "supergroupMembersFilterBots");
    s.store_class_end();
  }
}

testBytes::testBytes()
  : value_()
{}

testBytes::testBytes(bytes const &value_)
  : value_(std::move(value_))
{}

const std::int32_t testBytes::ID;

void testBytes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testBytes");
    s.store_bytes_field("value", value_);
    s.store_class_end();
  }
}

textParseModeMarkdown::textParseModeMarkdown()
  : version_()
{}

textParseModeMarkdown::textParseModeMarkdown(int32 version_)
  : version_(version_)
{}

const std::int32_t textParseModeMarkdown::ID;

void textParseModeMarkdown::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textParseModeMarkdown");
    s.store_field("version", version_);
    s.store_class_end();
  }
}

textParseModeHTML::textParseModeHTML() {
}

const std::int32_t textParseModeHTML::ID;

void textParseModeHTML::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textParseModeHTML");
    s.store_class_end();
  }
}

themeParameters::themeParameters()
  : background_color_()
  , secondary_background_color_()
  , header_background_color_()
  , bottom_bar_background_color_()
  , section_background_color_()
  , section_separator_color_()
  , text_color_()
  , accent_text_color_()
  , section_header_text_color_()
  , subtitle_text_color_()
  , destructive_text_color_()
  , hint_color_()
  , link_color_()
  , button_color_()
  , button_text_color_()
{}

themeParameters::themeParameters(int32 background_color_, int32 secondary_background_color_, int32 header_background_color_, int32 bottom_bar_background_color_, int32 section_background_color_, int32 section_separator_color_, int32 text_color_, int32 accent_text_color_, int32 section_header_text_color_, int32 subtitle_text_color_, int32 destructive_text_color_, int32 hint_color_, int32 link_color_, int32 button_color_, int32 button_text_color_)
  : background_color_(background_color_)
  , secondary_background_color_(secondary_background_color_)
  , header_background_color_(header_background_color_)
  , bottom_bar_background_color_(bottom_bar_background_color_)
  , section_background_color_(section_background_color_)
  , section_separator_color_(section_separator_color_)
  , text_color_(text_color_)
  , accent_text_color_(accent_text_color_)
  , section_header_text_color_(section_header_text_color_)
  , subtitle_text_color_(subtitle_text_color_)
  , destructive_text_color_(destructive_text_color_)
  , hint_color_(hint_color_)
  , link_color_(link_color_)
  , button_color_(button_color_)
  , button_text_color_(button_text_color_)
{}

const std::int32_t themeParameters::ID;

void themeParameters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "themeParameters");
    s.store_field("background_color", background_color_);
    s.store_field("secondary_background_color", secondary_background_color_);
    s.store_field("header_background_color", header_background_color_);
    s.store_field("bottom_bar_background_color", bottom_bar_background_color_);
    s.store_field("section_background_color", section_background_color_);
    s.store_field("section_separator_color", section_separator_color_);
    s.store_field("text_color", text_color_);
    s.store_field("accent_text_color", accent_text_color_);
    s.store_field("section_header_text_color", section_header_text_color_);
    s.store_field("subtitle_text_color", subtitle_text_color_);
    s.store_field("destructive_text_color", destructive_text_color_);
    s.store_field("hint_color", hint_color_);
    s.store_field("link_color", link_color_);
    s.store_field("button_color", button_color_);
    s.store_field("button_text_color", button_text_color_);
    s.store_class_end();
  }
}

topChatCategoryUsers::topChatCategoryUsers() {
}

const std::int32_t topChatCategoryUsers::ID;

void topChatCategoryUsers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "topChatCategoryUsers");
    s.store_class_end();
  }
}

topChatCategoryBots::topChatCategoryBots() {
}

const std::int32_t topChatCategoryBots::ID;

void topChatCategoryBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "topChatCategoryBots");
    s.store_class_end();
  }
}

topChatCategoryGroups::topChatCategoryGroups() {
}

const std::int32_t topChatCategoryGroups::ID;

void topChatCategoryGroups::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "topChatCategoryGroups");
    s.store_class_end();
  }
}

topChatCategoryChannels::topChatCategoryChannels() {
}

const std::int32_t topChatCategoryChannels::ID;

void topChatCategoryChannels::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "topChatCategoryChannels");
    s.store_class_end();
  }
}

topChatCategoryInlineBots::topChatCategoryInlineBots() {
}

const std::int32_t topChatCategoryInlineBots::ID;

void topChatCategoryInlineBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "topChatCategoryInlineBots");
    s.store_class_end();
  }
}

topChatCategoryWebAppBots::topChatCategoryWebAppBots() {
}

const std::int32_t topChatCategoryWebAppBots::ID;

void topChatCategoryWebAppBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "topChatCategoryWebAppBots");
    s.store_class_end();
  }
}

topChatCategoryCalls::topChatCategoryCalls() {
}

const std::int32_t topChatCategoryCalls::ID;

void topChatCategoryCalls::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "topChatCategoryCalls");
    s.store_class_end();
  }
}

topChatCategoryForwardChats::topChatCategoryForwardChats() {
}

const std::int32_t topChatCategoryForwardChats::ID;

void topChatCategoryForwardChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "topChatCategoryForwardChats");
    s.store_class_end();
  }
}

upgradedGiftOriginalDetails::upgradedGiftOriginalDetails()
  : sender_id_()
  , receiver_id_()
  , text_()
  , date_()
{}

upgradedGiftOriginalDetails::upgradedGiftOriginalDetails(object_ptr<MessageSender> &&sender_id_, object_ptr<MessageSender> &&receiver_id_, object_ptr<formattedText> &&text_, int32 date_)
  : sender_id_(std::move(sender_id_))
  , receiver_id_(std::move(receiver_id_))
  , text_(std::move(text_))
  , date_(date_)
{}

const std::int32_t upgradedGiftOriginalDetails::ID;

void upgradedGiftOriginalDetails::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradedGiftOriginalDetails");
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_object_field("receiver_id", static_cast<const BaseObject *>(receiver_id_.get()));
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("date", date_);
    s.store_class_end();
  }
}

userLink::userLink()
  : url_()
  , expires_in_()
{}

userLink::userLink(string const &url_, int32 expires_in_)
  : url_(url_)
  , expires_in_(expires_in_)
{}

const std::int32_t userLink::ID;

void userLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userLink");
    s.store_field("url", url_);
    s.store_field("expires_in", expires_in_);
    s.store_class_end();
  }
}

userPrivacySettingShowStatus::userPrivacySettingShowStatus() {
}

const std::int32_t userPrivacySettingShowStatus::ID;

void userPrivacySettingShowStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingShowStatus");
    s.store_class_end();
  }
}

userPrivacySettingShowProfilePhoto::userPrivacySettingShowProfilePhoto() {
}

const std::int32_t userPrivacySettingShowProfilePhoto::ID;

void userPrivacySettingShowProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingShowProfilePhoto");
    s.store_class_end();
  }
}

userPrivacySettingShowLinkInForwardedMessages::userPrivacySettingShowLinkInForwardedMessages() {
}

const std::int32_t userPrivacySettingShowLinkInForwardedMessages::ID;

void userPrivacySettingShowLinkInForwardedMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingShowLinkInForwardedMessages");
    s.store_class_end();
  }
}

userPrivacySettingShowPhoneNumber::userPrivacySettingShowPhoneNumber() {
}

const std::int32_t userPrivacySettingShowPhoneNumber::ID;

void userPrivacySettingShowPhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingShowPhoneNumber");
    s.store_class_end();
  }
}

userPrivacySettingShowBio::userPrivacySettingShowBio() {
}

const std::int32_t userPrivacySettingShowBio::ID;

void userPrivacySettingShowBio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingShowBio");
    s.store_class_end();
  }
}

userPrivacySettingShowBirthdate::userPrivacySettingShowBirthdate() {
}

const std::int32_t userPrivacySettingShowBirthdate::ID;

void userPrivacySettingShowBirthdate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingShowBirthdate");
    s.store_class_end();
  }
}

userPrivacySettingAllowChatInvites::userPrivacySettingAllowChatInvites() {
}

const std::int32_t userPrivacySettingAllowChatInvites::ID;

void userPrivacySettingAllowChatInvites::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingAllowChatInvites");
    s.store_class_end();
  }
}

userPrivacySettingAllowCalls::userPrivacySettingAllowCalls() {
}

const std::int32_t userPrivacySettingAllowCalls::ID;

void userPrivacySettingAllowCalls::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingAllowCalls");
    s.store_class_end();
  }
}

userPrivacySettingAllowPeerToPeerCalls::userPrivacySettingAllowPeerToPeerCalls() {
}

const std::int32_t userPrivacySettingAllowPeerToPeerCalls::ID;

void userPrivacySettingAllowPeerToPeerCalls::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingAllowPeerToPeerCalls");
    s.store_class_end();
  }
}

userPrivacySettingAllowFindingByPhoneNumber::userPrivacySettingAllowFindingByPhoneNumber() {
}

const std::int32_t userPrivacySettingAllowFindingByPhoneNumber::ID;

void userPrivacySettingAllowFindingByPhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingAllowFindingByPhoneNumber");
    s.store_class_end();
  }
}

userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages::userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages() {
}

const std::int32_t userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages::ID;

void userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingAllowPrivateVoiceAndVideoNoteMessages");
    s.store_class_end();
  }
}

userPrivacySettingAutosaveGifts::userPrivacySettingAutosaveGifts() {
}

const std::int32_t userPrivacySettingAutosaveGifts::ID;

void userPrivacySettingAutosaveGifts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingAutosaveGifts");
    s.store_class_end();
  }
}

userPrivacySettingAllowUnpaidMessages::userPrivacySettingAllowUnpaidMessages() {
}

const std::int32_t userPrivacySettingAllowUnpaidMessages::ID;

void userPrivacySettingAllowUnpaidMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingAllowUnpaidMessages");
    s.store_class_end();
  }
}

userStatusEmpty::userStatusEmpty() {
}

const std::int32_t userStatusEmpty::ID;

void userStatusEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userStatusEmpty");
    s.store_class_end();
  }
}

userStatusOnline::userStatusOnline()
  : expires_()
{}

userStatusOnline::userStatusOnline(int32 expires_)
  : expires_(expires_)
{}

const std::int32_t userStatusOnline::ID;

void userStatusOnline::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userStatusOnline");
    s.store_field("expires", expires_);
    s.store_class_end();
  }
}

userStatusOffline::userStatusOffline()
  : was_online_()
{}

userStatusOffline::userStatusOffline(int32 was_online_)
  : was_online_(was_online_)
{}

const std::int32_t userStatusOffline::ID;

void userStatusOffline::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userStatusOffline");
    s.store_field("was_online", was_online_);
    s.store_class_end();
  }
}

userStatusRecently::userStatusRecently()
  : by_my_privacy_settings_()
{}

userStatusRecently::userStatusRecently(bool by_my_privacy_settings_)
  : by_my_privacy_settings_(by_my_privacy_settings_)
{}

const std::int32_t userStatusRecently::ID;

void userStatusRecently::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userStatusRecently");
    s.store_field("by_my_privacy_settings", by_my_privacy_settings_);
    s.store_class_end();
  }
}

userStatusLastWeek::userStatusLastWeek()
  : by_my_privacy_settings_()
{}

userStatusLastWeek::userStatusLastWeek(bool by_my_privacy_settings_)
  : by_my_privacy_settings_(by_my_privacy_settings_)
{}

const std::int32_t userStatusLastWeek::ID;

void userStatusLastWeek::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userStatusLastWeek");
    s.store_field("by_my_privacy_settings", by_my_privacy_settings_);
    s.store_class_end();
  }
}

userStatusLastMonth::userStatusLastMonth()
  : by_my_privacy_settings_()
{}

userStatusLastMonth::userStatusLastMonth(bool by_my_privacy_settings_)
  : by_my_privacy_settings_(by_my_privacy_settings_)
{}

const std::int32_t userStatusLastMonth::ID;

void userStatusLastMonth::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userStatusLastMonth");
    s.store_field("by_my_privacy_settings", by_my_privacy_settings_);
    s.store_class_end();
  }
}

vectorPathCommandLine::vectorPathCommandLine()
  : end_point_()
{}

vectorPathCommandLine::vectorPathCommandLine(object_ptr<point> &&end_point_)
  : end_point_(std::move(end_point_))
{}

const std::int32_t vectorPathCommandLine::ID;

void vectorPathCommandLine::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "vectorPathCommandLine");
    s.store_object_field("end_point", static_cast<const BaseObject *>(end_point_.get()));
    s.store_class_end();
  }
}

vectorPathCommandCubicBezierCurve::vectorPathCommandCubicBezierCurve()
  : start_control_point_()
  , end_control_point_()
  , end_point_()
{}

vectorPathCommandCubicBezierCurve::vectorPathCommandCubicBezierCurve(object_ptr<point> &&start_control_point_, object_ptr<point> &&end_control_point_, object_ptr<point> &&end_point_)
  : start_control_point_(std::move(start_control_point_))
  , end_control_point_(std::move(end_control_point_))
  , end_point_(std::move(end_point_))
{}

const std::int32_t vectorPathCommandCubicBezierCurve::ID;

void vectorPathCommandCubicBezierCurve::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "vectorPathCommandCubicBezierCurve");
    s.store_object_field("start_control_point", static_cast<const BaseObject *>(start_control_point_.get()));
    s.store_object_field("end_control_point", static_cast<const BaseObject *>(end_control_point_.get()));
    s.store_object_field("end_point", static_cast<const BaseObject *>(end_point_.get()));
    s.store_class_end();
  }
}

addChatFolderByInviteLink::addChatFolderByInviteLink()
  : invite_link_()
  , chat_ids_()
{}

addChatFolderByInviteLink::addChatFolderByInviteLink(string const &invite_link_, array<int53> &&chat_ids_)
  : invite_link_(invite_link_)
  , chat_ids_(std::move(chat_ids_))
{}

const std::int32_t addChatFolderByInviteLink::ID;

void addChatFolderByInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addChatFolderByInviteLink");
    s.store_field("invite_link", invite_link_);
    { s.store_vector_begin("chat_ids", chat_ids_.size()); for (const auto &_value : chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

addContact::addContact()
  : user_id_()
  , contact_()
  , share_phone_number_()
{}

addContact::addContact(int53 user_id_, object_ptr<importedContact> &&contact_, bool share_phone_number_)
  : user_id_(user_id_)
  , contact_(std::move(contact_))
  , share_phone_number_(share_phone_number_)
{}

const std::int32_t addContact::ID;

void addContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addContact");
    s.store_field("user_id", user_id_);
    s.store_object_field("contact", static_cast<const BaseObject *>(contact_.get()));
    s.store_field("share_phone_number", share_phone_number_);
    s.store_class_end();
  }
}

addMessageReaction::addMessageReaction()
  : chat_id_()
  , message_id_()
  , reaction_type_()
  , is_big_()
  , update_recent_reactions_()
{}

addMessageReaction::addMessageReaction(int53 chat_id_, int53 message_id_, object_ptr<ReactionType> &&reaction_type_, bool is_big_, bool update_recent_reactions_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , reaction_type_(std::move(reaction_type_))
  , is_big_(is_big_)
  , update_recent_reactions_(update_recent_reactions_)
{}

const std::int32_t addMessageReaction::ID;

void addMessageReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addMessageReaction");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("reaction_type", static_cast<const BaseObject *>(reaction_type_.get()));
    s.store_field("is_big", is_big_);
    s.store_field("update_recent_reactions", update_recent_reactions_);
    s.store_class_end();
  }
}

addStickerToSet::addStickerToSet()
  : user_id_()
  , name_()
  , sticker_()
{}

addStickerToSet::addStickerToSet(int53 user_id_, string const &name_, object_ptr<inputSticker> &&sticker_)
  : user_id_(user_id_)
  , name_(name_)
  , sticker_(std::move(sticker_))
{}

const std::int32_t addStickerToSet::ID;

void addStickerToSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addStickerToSet");
    s.store_field("user_id", user_id_);
    s.store_field("name", name_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

answerWebAppQuery::answerWebAppQuery()
  : web_app_query_id_()
  , result_()
{}

answerWebAppQuery::answerWebAppQuery(string const &web_app_query_id_, object_ptr<InputInlineQueryResult> &&result_)
  : web_app_query_id_(web_app_query_id_)
  , result_(std::move(result_))
{}

const std::int32_t answerWebAppQuery::ID;

void answerWebAppQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "answerWebAppQuery");
    s.store_field("web_app_query_id", web_app_query_id_);
    s.store_object_field("result", static_cast<const BaseObject *>(result_.get()));
    s.store_class_end();
  }
}

banChatMember::banChatMember()
  : chat_id_()
  , member_id_()
  , banned_until_date_()
  , revoke_messages_()
{}

banChatMember::banChatMember(int53 chat_id_, object_ptr<MessageSender> &&member_id_, int32 banned_until_date_, bool revoke_messages_)
  : chat_id_(chat_id_)
  , member_id_(std::move(member_id_))
  , banned_until_date_(banned_until_date_)
  , revoke_messages_(revoke_messages_)
{}

const std::int32_t banChatMember::ID;

void banChatMember::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "banChatMember");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("member_id", static_cast<const BaseObject *>(member_id_.get()));
    s.store_field("banned_until_date", banned_until_date_);
    s.store_field("revoke_messages", revoke_messages_);
    s.store_class_end();
  }
}

checkAuthenticationEmailCode::checkAuthenticationEmailCode()
  : code_()
{}

checkAuthenticationEmailCode::checkAuthenticationEmailCode(object_ptr<EmailAddressAuthentication> &&code_)
  : code_(std::move(code_))
{}

const std::int32_t checkAuthenticationEmailCode::ID;

void checkAuthenticationEmailCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkAuthenticationEmailCode");
    s.store_object_field("code", static_cast<const BaseObject *>(code_.get()));
    s.store_class_end();
  }
}

checkChatInviteLink::checkChatInviteLink()
  : invite_link_()
{}

checkChatInviteLink::checkChatInviteLink(string const &invite_link_)
  : invite_link_(invite_link_)
{}

const std::int32_t checkChatInviteLink::ID;

void checkChatInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkChatInviteLink");
    s.store_field("invite_link", invite_link_);
    s.store_class_end();
  }
}

clearAllDraftMessages::clearAllDraftMessages()
  : exclude_secret_chats_()
{}

clearAllDraftMessages::clearAllDraftMessages(bool exclude_secret_chats_)
  : exclude_secret_chats_(exclude_secret_chats_)
{}

const std::int32_t clearAllDraftMessages::ID;

void clearAllDraftMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "clearAllDraftMessages");
    s.store_field("exclude_secret_chats", exclude_secret_chats_);
    s.store_class_end();
  }
}

createVideoChat::createVideoChat()
  : chat_id_()
  , title_()
  , start_date_()
  , is_rtmp_stream_()
{}

createVideoChat::createVideoChat(int53 chat_id_, string const &title_, int32 start_date_, bool is_rtmp_stream_)
  : chat_id_(chat_id_)
  , title_(title_)
  , start_date_(start_date_)
  , is_rtmp_stream_(is_rtmp_stream_)
{}

const std::int32_t createVideoChat::ID;

void createVideoChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createVideoChat");
    s.store_field("chat_id", chat_id_);
    s.store_field("title", title_);
    s.store_field("start_date", start_date_);
    s.store_field("is_rtmp_stream", is_rtmp_stream_);
    s.store_class_end();
  }
}

deleteChatBackground::deleteChatBackground()
  : chat_id_()
  , restore_previous_()
{}

deleteChatBackground::deleteChatBackground(int53 chat_id_, bool restore_previous_)
  : chat_id_(chat_id_)
  , restore_previous_(restore_previous_)
{}

const std::int32_t deleteChatBackground::ID;

void deleteChatBackground::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteChatBackground");
    s.store_field("chat_id", chat_id_);
    s.store_field("restore_previous", restore_previous_);
    s.store_class_end();
  }
}

deleteDirectMessagesChatTopicHistory::deleteDirectMessagesChatTopicHistory()
  : chat_id_()
  , topic_id_()
{}

deleteDirectMessagesChatTopicHistory::deleteDirectMessagesChatTopicHistory(int53 chat_id_, int53 topic_id_)
  : chat_id_(chat_id_)
  , topic_id_(topic_id_)
{}

const std::int32_t deleteDirectMessagesChatTopicHistory::ID;

void deleteDirectMessagesChatTopicHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteDirectMessagesChatTopicHistory");
    s.store_field("chat_id", chat_id_);
    s.store_field("topic_id", topic_id_);
    s.store_class_end();
  }
}

deleteStoryAlbum::deleteStoryAlbum()
  : chat_id_()
  , story_album_id_()
{}

deleteStoryAlbum::deleteStoryAlbum(int53 chat_id_, int32 story_album_id_)
  : chat_id_(chat_id_)
  , story_album_id_(story_album_id_)
{}

const std::int32_t deleteStoryAlbum::ID;

void deleteStoryAlbum::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteStoryAlbum");
    s.store_field("chat_id", chat_id_);
    s.store_field("story_album_id", story_album_id_);
    s.store_class_end();
  }
}

disconnectAffiliateProgram::disconnectAffiliateProgram()
  : affiliate_()
  , url_()
{}

disconnectAffiliateProgram::disconnectAffiliateProgram(object_ptr<AffiliateType> &&affiliate_, string const &url_)
  : affiliate_(std::move(affiliate_))
  , url_(url_)
{}

const std::int32_t disconnectAffiliateProgram::ID;

void disconnectAffiliateProgram::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "disconnectAffiliateProgram");
    s.store_object_field("affiliate", static_cast<const BaseObject *>(affiliate_.get()));
    s.store_field("url", url_);
    s.store_class_end();
  }
}

disconnectWebsite::disconnectWebsite()
  : website_id_()
{}

disconnectWebsite::disconnectWebsite(int64 website_id_)
  : website_id_(website_id_)
{}

const std::int32_t disconnectWebsite::ID;

void disconnectWebsite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "disconnectWebsite");
    s.store_field("website_id", website_id_);
    s.store_class_end();
  }
}

editBusinessStory::editBusinessStory()
  : story_poster_chat_id_()
  , story_id_()
  , content_()
  , areas_()
  , caption_()
  , privacy_settings_()
{}

editBusinessStory::editBusinessStory(int53 story_poster_chat_id_, int32 story_id_, object_ptr<InputStoryContent> &&content_, object_ptr<inputStoryAreas> &&areas_, object_ptr<formattedText> &&caption_, object_ptr<StoryPrivacySettings> &&privacy_settings_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , story_id_(story_id_)
  , content_(std::move(content_))
  , areas_(std::move(areas_))
  , caption_(std::move(caption_))
  , privacy_settings_(std::move(privacy_settings_))
{}

const std::int32_t editBusinessStory::ID;

void editBusinessStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editBusinessStory");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("story_id", story_id_);
    s.store_object_field("content", static_cast<const BaseObject *>(content_.get()));
    s.store_object_field("areas", static_cast<const BaseObject *>(areas_.get()));
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_object_field("privacy_settings", static_cast<const BaseObject *>(privacy_settings_.get()));
    s.store_class_end();
  }
}

endGroupCallRecording::endGroupCallRecording()
  : group_call_id_()
{}

endGroupCallRecording::endGroupCallRecording(int32 group_call_id_)
  : group_call_id_(group_call_id_)
{}

const std::int32_t endGroupCallRecording::ID;

void endGroupCallRecording::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "endGroupCallRecording");
    s.store_field("group_call_id", group_call_id_);
    s.store_class_end();
  }
}

getAccountTtl::getAccountTtl() {
}

const std::int32_t getAccountTtl::ID;

void getAccountTtl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getAccountTtl");
    s.store_class_end();
  }
}

getBackgroundUrl::getBackgroundUrl()
  : name_()
  , type_()
{}

getBackgroundUrl::getBackgroundUrl(string const &name_, object_ptr<BackgroundType> &&type_)
  : name_(name_)
  , type_(std::move(type_))
{}

const std::int32_t getBackgroundUrl::ID;

void getBackgroundUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getBackgroundUrl");
    s.store_field("name", name_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

getBusinessAccountStarAmount::getBusinessAccountStarAmount()
  : business_connection_id_()
{}

getBusinessAccountStarAmount::getBusinessAccountStarAmount(string const &business_connection_id_)
  : business_connection_id_(business_connection_id_)
{}

const std::int32_t getBusinessAccountStarAmount::ID;

void getBusinessAccountStarAmount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getBusinessAccountStarAmount");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_class_end();
  }
}

getBusinessChatLinks::getBusinessChatLinks() {
}

const std::int32_t getBusinessChatLinks::ID;

void getBusinessChatLinks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getBusinessChatLinks");
    s.store_class_end();
  }
}

getCallbackQueryAnswer::getCallbackQueryAnswer()
  : chat_id_()
  , message_id_()
  , payload_()
{}

getCallbackQueryAnswer::getCallbackQueryAnswer(int53 chat_id_, int53 message_id_, object_ptr<CallbackQueryPayload> &&payload_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , payload_(std::move(payload_))
{}

const std::int32_t getCallbackQueryAnswer::ID;

void getCallbackQueryAnswer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getCallbackQueryAnswer");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("payload", static_cast<const BaseObject *>(payload_.get()));
    s.store_class_end();
  }
}

getChatArchivedStories::getChatArchivedStories()
  : chat_id_()
  , from_story_id_()
  , limit_()
{}

getChatArchivedStories::getChatArchivedStories(int53 chat_id_, int32 from_story_id_, int32 limit_)
  : chat_id_(chat_id_)
  , from_story_id_(from_story_id_)
  , limit_(limit_)
{}

const std::int32_t getChatArchivedStories::ID;

void getChatArchivedStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatArchivedStories");
    s.store_field("chat_id", chat_id_);
    s.store_field("from_story_id", from_story_id_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getChatFolderDefaultIconName::getChatFolderDefaultIconName()
  : folder_()
{}

getChatFolderDefaultIconName::getChatFolderDefaultIconName(object_ptr<chatFolder> &&folder_)
  : folder_(std::move(folder_))
{}

const std::int32_t getChatFolderDefaultIconName::ID;

void getChatFolderDefaultIconName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatFolderDefaultIconName");
    s.store_object_field("folder", static_cast<const BaseObject *>(folder_.get()));
    s.store_class_end();
  }
}

getCloseFriends::getCloseFriends() {
}

const std::int32_t getCloseFriends::ID;

void getCloseFriends::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getCloseFriends");
    s.store_class_end();
  }
}

getCountryCode::getCountryCode() {
}

const std::int32_t getCountryCode::ID;

void getCountryCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getCountryCode");
    s.store_class_end();
  }
}

getDefaultEmojiStatuses::getDefaultEmojiStatuses() {
}

const std::int32_t getDefaultEmojiStatuses::ID;

void getDefaultEmojiStatuses::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getDefaultEmojiStatuses");
    s.store_class_end();
  }
}

getExternalLink::getExternalLink()
  : link_()
  , allow_write_access_()
{}

getExternalLink::getExternalLink(string const &link_, bool allow_write_access_)
  : link_(link_)
  , allow_write_access_(allow_write_access_)
{}

const std::int32_t getExternalLink::ID;

void getExternalLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getExternalLink");
    s.store_field("link", link_);
    s.store_field("allow_write_access", allow_write_access_);
    s.store_class_end();
  }
}

getGroupCallParticipants::getGroupCallParticipants()
  : input_group_call_()
  , limit_()
{}

getGroupCallParticipants::getGroupCallParticipants(object_ptr<InputGroupCall> &&input_group_call_, int32 limit_)
  : input_group_call_(std::move(input_group_call_))
  , limit_(limit_)
{}

const std::int32_t getGroupCallParticipants::ID;

void getGroupCallParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getGroupCallParticipants");
    s.store_object_field("input_group_call", static_cast<const BaseObject *>(input_group_call_.get()));
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getInstalledBackgrounds::getInstalledBackgrounds()
  : for_dark_theme_()
{}

getInstalledBackgrounds::getInstalledBackgrounds(bool for_dark_theme_)
  : for_dark_theme_(for_dark_theme_)
{}

const std::int32_t getInstalledBackgrounds::ID;

void getInstalledBackgrounds::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getInstalledBackgrounds");
    s.store_field("for_dark_theme", for_dark_theme_);
    s.store_class_end();
  }
}

getInternalLink::getInternalLink()
  : type_()
  , is_http_()
{}

getInternalLink::getInternalLink(object_ptr<InternalLinkType> &&type_, bool is_http_)
  : type_(std::move(type_))
  , is_http_(is_http_)
{}

const std::int32_t getInternalLink::ID;

void getInternalLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getInternalLink");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("is_http", is_http_);
    s.store_class_end();
  }
}

getLogTags::getLogTags() {
}

const std::int32_t getLogTags::ID;

void getLogTags::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getLogTags");
    s.store_class_end();
  }
}

getMenuButton::getMenuButton()
  : user_id_()
{}

getMenuButton::getMenuButton(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t getMenuButton::ID;

void getMenuButton::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMenuButton");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

getMessageThread::getMessageThread()
  : chat_id_()
  , message_id_()
{}

getMessageThread::getMessageThread(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t getMessageThread::ID;

void getMessageThread::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMessageThread");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

getMessageThreadHistory::getMessageThreadHistory()
  : chat_id_()
  , message_id_()
  , from_message_id_()
  , offset_()
  , limit_()
{}

getMessageThreadHistory::getMessageThreadHistory(int53 chat_id_, int53 message_id_, int53 from_message_id_, int32 offset_, int32 limit_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , from_message_id_(from_message_id_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getMessageThreadHistory::ID;

void getMessageThreadHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMessageThreadHistory");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("from_message_id", from_message_id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getNetworkStatistics::getNetworkStatistics()
  : only_current_()
{}

getNetworkStatistics::getNetworkStatistics(bool only_current_)
  : only_current_(only_current_)
{}

const std::int32_t getNetworkStatistics::ID;

void getNetworkStatistics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getNetworkStatistics");
    s.store_field("only_current", only_current_);
    s.store_class_end();
  }
}

getPassportAuthorizationForm::getPassportAuthorizationForm()
  : bot_user_id_()
  , scope_()
  , public_key_()
  , nonce_()
{}

getPassportAuthorizationForm::getPassportAuthorizationForm(int53 bot_user_id_, string const &scope_, string const &public_key_, string const &nonce_)
  : bot_user_id_(bot_user_id_)
  , scope_(scope_)
  , public_key_(public_key_)
  , nonce_(nonce_)
{}

const std::int32_t getPassportAuthorizationForm::ID;

void getPassportAuthorizationForm::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPassportAuthorizationForm");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("scope", scope_);
    s.store_field("public_key", public_key_);
    s.store_field("nonce", nonce_);
    s.store_class_end();
  }
}

getPhoneNumberInfo::getPhoneNumberInfo()
  : phone_number_prefix_()
{}

getPhoneNumberInfo::getPhoneNumberInfo(string const &phone_number_prefix_)
  : phone_number_prefix_(phone_number_prefix_)
{}

const std::int32_t getPhoneNumberInfo::ID;

void getPhoneNumberInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPhoneNumberInfo");
    s.store_field("phone_number_prefix", phone_number_prefix_);
    s.store_class_end();
  }
}

getPremiumStickerExamples::getPremiumStickerExamples() {
}

const std::int32_t getPremiumStickerExamples::ID;

void getPremiumStickerExamples::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPremiumStickerExamples");
    s.store_class_end();
  }
}

getPremiumStickers::getPremiumStickers()
  : limit_()
{}

getPremiumStickers::getPremiumStickers(int32 limit_)
  : limit_(limit_)
{}

const std::int32_t getPremiumStickers::ID;

void getPremiumStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPremiumStickers");
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getPublicPostSearchLimits::getPublicPostSearchLimits()
  : query_()
{}

getPublicPostSearchLimits::getPublicPostSearchLimits(string const &query_)
  : query_(query_)
{}

const std::int32_t getPublicPostSearchLimits::ID;

void getPublicPostSearchLimits::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPublicPostSearchLimits");
    s.store_field("query", query_);
    s.store_class_end();
  }
}

getReceivedGift::getReceivedGift()
  : received_gift_id_()
{}

getReceivedGift::getReceivedGift(string const &received_gift_id_)
  : received_gift_id_(received_gift_id_)
{}

const std::int32_t getReceivedGift::ID;

void getReceivedGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getReceivedGift");
    s.store_field("received_gift_id", received_gift_id_);
    s.store_class_end();
  }
}

getRecentEmojiStatuses::getRecentEmojiStatuses() {
}

const std::int32_t getRecentEmojiStatuses::ID;

void getRecentEmojiStatuses::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getRecentEmojiStatuses");
    s.store_class_end();
  }
}

getRecoveryEmailAddress::getRecoveryEmailAddress()
  : password_()
{}

getRecoveryEmailAddress::getRecoveryEmailAddress(string const &password_)
  : password_(password_)
{}

const std::int32_t getRecoveryEmailAddress::ID;

void getRecoveryEmailAddress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getRecoveryEmailAddress");
    s.store_field("password", password_);
    s.store_class_end();
  }
}

getStoryNotificationSettingsExceptions::getStoryNotificationSettingsExceptions() {
}

const std::int32_t getStoryNotificationSettingsExceptions::ID;

void getStoryNotificationSettingsExceptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStoryNotificationSettingsExceptions");
    s.store_class_end();
  }
}

getSuggestedStickerSetName::getSuggestedStickerSetName()
  : title_()
{}

getSuggestedStickerSetName::getSuggestedStickerSetName(string const &title_)
  : title_(title_)
{}

const std::int32_t getSuggestedStickerSetName::ID;

void getSuggestedStickerSetName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getSuggestedStickerSetName");
    s.store_field("title", title_);
    s.store_class_end();
  }
}

getThemedChatEmojiStatuses::getThemedChatEmojiStatuses() {
}

const std::int32_t getThemedChatEmojiStatuses::ID;

void getThemedChatEmojiStatuses::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getThemedChatEmojiStatuses");
    s.store_class_end();
  }
}

getTrendingStickerSets::getTrendingStickerSets()
  : sticker_type_()
  , offset_()
  , limit_()
{}

getTrendingStickerSets::getTrendingStickerSets(object_ptr<StickerType> &&sticker_type_, int32 offset_, int32 limit_)
  : sticker_type_(std::move(sticker_type_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getTrendingStickerSets::ID;

void getTrendingStickerSets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getTrendingStickerSets");
    s.store_object_field("sticker_type", static_cast<const BaseObject *>(sticker_type_.get()));
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getUserProfilePhotos::getUserProfilePhotos()
  : user_id_()
  , offset_()
  , limit_()
{}

getUserProfilePhotos::getUserProfilePhotos(int53 user_id_, int32 offset_, int32 limit_)
  : user_id_(user_id_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getUserProfilePhotos::ID;

void getUserProfilePhotos::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getUserProfilePhotos");
    s.store_field("user_id", user_id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

loadGroupCallParticipants::loadGroupCallParticipants()
  : group_call_id_()
  , limit_()
{}

loadGroupCallParticipants::loadGroupCallParticipants(int32 group_call_id_, int32 limit_)
  : group_call_id_(group_call_id_)
  , limit_(limit_)
{}

const std::int32_t loadGroupCallParticipants::ID;

void loadGroupCallParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "loadGroupCallParticipants");
    s.store_field("group_call_id", group_call_id_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

markChecklistTasksAsDone::markChecklistTasksAsDone()
  : chat_id_()
  , message_id_()
  , marked_as_done_task_ids_()
  , marked_as_not_done_task_ids_()
{}

markChecklistTasksAsDone::markChecklistTasksAsDone(int53 chat_id_, int53 message_id_, array<int32> &&marked_as_done_task_ids_, array<int32> &&marked_as_not_done_task_ids_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , marked_as_done_task_ids_(std::move(marked_as_done_task_ids_))
  , marked_as_not_done_task_ids_(std::move(marked_as_not_done_task_ids_))
{}

const std::int32_t markChecklistTasksAsDone::ID;

void markChecklistTasksAsDone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "markChecklistTasksAsDone");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    { s.store_vector_begin("marked_as_done_task_ids", marked_as_done_task_ids_.size()); for (const auto &_value : marked_as_done_task_ids_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("marked_as_not_done_task_ids", marked_as_not_done_task_ids_.size()); for (const auto &_value : marked_as_not_done_task_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

openWebApp::openWebApp()
  : chat_id_()
  , bot_user_id_()
  , url_()
  , topic_id_()
  , reply_to_()
  , parameters_()
{}

openWebApp::openWebApp(int53 chat_id_, int53 bot_user_id_, string const &url_, object_ptr<MessageTopic> &&topic_id_, object_ptr<InputMessageReplyTo> &&reply_to_, object_ptr<webAppOpenParameters> &&parameters_)
  : chat_id_(chat_id_)
  , bot_user_id_(bot_user_id_)
  , url_(url_)
  , topic_id_(std::move(topic_id_))
  , reply_to_(std::move(reply_to_))
  , parameters_(std::move(parameters_))
{}

const std::int32_t openWebApp::ID;

void openWebApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "openWebApp");
    s.store_field("chat_id", chat_id_);
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("url", url_);
    s.store_object_field("topic_id", static_cast<const BaseObject *>(topic_id_.get()));
    s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get()));
    s.store_object_field("parameters", static_cast<const BaseObject *>(parameters_.get()));
    s.store_class_end();
  }
}

parseTextEntities::parseTextEntities()
  : text_()
  , parse_mode_()
{}

parseTextEntities::parseTextEntities(string const &text_, object_ptr<TextParseMode> &&parse_mode_)
  : text_(text_)
  , parse_mode_(std::move(parse_mode_))
{}

const std::int32_t parseTextEntities::ID;

void parseTextEntities::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "parseTextEntities");
    s.store_field("text", text_);
    s.store_object_field("parse_mode", static_cast<const BaseObject *>(parse_mode_.get()));
    s.store_class_end();
  }
}

readAllForumTopicReactions::readAllForumTopicReactions()
  : chat_id_()
  , forum_topic_id_()
{}

readAllForumTopicReactions::readAllForumTopicReactions(int53 chat_id_, int32 forum_topic_id_)
  : chat_id_(chat_id_)
  , forum_topic_id_(forum_topic_id_)
{}

const std::int32_t readAllForumTopicReactions::ID;

void readAllForumTopicReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "readAllForumTopicReactions");
    s.store_field("chat_id", chat_id_);
    s.store_field("forum_topic_id", forum_topic_id_);
    s.store_class_end();
  }
}

removeContacts::removeContacts()
  : user_ids_()
{}

removeContacts::removeContacts(array<int53> &&user_ids_)
  : user_ids_(std::move(user_ids_))
{}

const std::int32_t removeContacts::ID;

void removeContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeContacts");
    { s.store_vector_begin("user_ids", user_ids_.size()); for (const auto &_value : user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

removeFileFromDownloads::removeFileFromDownloads()
  : file_id_()
  , delete_from_cache_()
{}

removeFileFromDownloads::removeFileFromDownloads(int32 file_id_, bool delete_from_cache_)
  : file_id_(file_id_)
  , delete_from_cache_(delete_from_cache_)
{}

const std::int32_t removeFileFromDownloads::ID;

void removeFileFromDownloads::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeFileFromDownloads");
    s.store_field("file_id", file_id_);
    s.store_field("delete_from_cache", delete_from_cache_);
    s.store_class_end();
  }
}

removeGiftCollectionGifts::removeGiftCollectionGifts()
  : owner_id_()
  , collection_id_()
  , received_gift_ids_()
{}

removeGiftCollectionGifts::removeGiftCollectionGifts(object_ptr<MessageSender> &&owner_id_, int32 collection_id_, array<string> &&received_gift_ids_)
  : owner_id_(std::move(owner_id_))
  , collection_id_(collection_id_)
  , received_gift_ids_(std::move(received_gift_ids_))
{}

const std::int32_t removeGiftCollectionGifts::ID;

void removeGiftCollectionGifts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeGiftCollectionGifts");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_field("collection_id", collection_id_);
    { s.store_vector_begin("received_gift_ids", received_gift_ids_.size()); for (const auto &_value : received_gift_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

removeStickerFromSet::removeStickerFromSet()
  : sticker_()
{}

removeStickerFromSet::removeStickerFromSet(object_ptr<InputFile> &&sticker_)
  : sticker_(std::move(sticker_))
{}

const std::int32_t removeStickerFromSet::ID;

void removeStickerFromSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeStickerFromSet");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

reportMessageReactions::reportMessageReactions()
  : chat_id_()
  , message_id_()
  , sender_id_()
{}

reportMessageReactions::reportMessageReactions(int53 chat_id_, int53 message_id_, object_ptr<MessageSender> &&sender_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , sender_id_(std::move(sender_id_))
{}

const std::int32_t reportMessageReactions::ID;

void reportMessageReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportMessageReactions");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_class_end();
  }
}

searchChatAffiliateProgram::searchChatAffiliateProgram()
  : username_()
  , referrer_()
{}

searchChatAffiliateProgram::searchChatAffiliateProgram(string const &username_, string const &referrer_)
  : username_(username_)
  , referrer_(referrer_)
{}

const std::int32_t searchChatAffiliateProgram::ID;

void searchChatAffiliateProgram::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchChatAffiliateProgram");
    s.store_field("username", username_);
    s.store_field("referrer", referrer_);
    s.store_class_end();
  }
}

searchSavedMessages::searchSavedMessages()
  : saved_messages_topic_id_()
  , tag_()
  , query_()
  , from_message_id_()
  , offset_()
  , limit_()
{}

searchSavedMessages::searchSavedMessages(int53 saved_messages_topic_id_, object_ptr<ReactionType> &&tag_, string const &query_, int53 from_message_id_, int32 offset_, int32 limit_)
  : saved_messages_topic_id_(saved_messages_topic_id_)
  , tag_(std::move(tag_))
  , query_(query_)
  , from_message_id_(from_message_id_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t searchSavedMessages::ID;

void searchSavedMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchSavedMessages");
    s.store_field("saved_messages_topic_id", saved_messages_topic_id_);
    s.store_object_field("tag", static_cast<const BaseObject *>(tag_.get()));
    s.store_field("query", query_);
    s.store_field("from_message_id", from_message_id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

searchStringsByPrefix::searchStringsByPrefix()
  : strings_()
  , query_()
  , limit_()
  , return_none_for_empty_query_()
{}

searchStringsByPrefix::searchStringsByPrefix(array<string> &&strings_, string const &query_, int32 limit_, bool return_none_for_empty_query_)
  : strings_(std::move(strings_))
  , query_(query_)
  , limit_(limit_)
  , return_none_for_empty_query_(return_none_for_empty_query_)
{}

const std::int32_t searchStringsByPrefix::ID;

void searchStringsByPrefix::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchStringsByPrefix");
    { s.store_vector_begin("strings", strings_.size()); for (const auto &_value : strings_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("query", query_);
    s.store_field("limit", limit_);
    s.store_field("return_none_for_empty_query", return_none_for_empty_query_);
    s.store_class_end();
  }
}

sendMessage::sendMessage()
  : chat_id_()
  , topic_id_()
  , reply_to_()
  , options_()
  , reply_markup_()
  , input_message_content_()
{}

sendMessage::sendMessage(int53 chat_id_, object_ptr<MessageTopic> &&topic_id_, object_ptr<InputMessageReplyTo> &&reply_to_, object_ptr<messageSendOptions> &&options_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_)
  : chat_id_(chat_id_)
  , topic_id_(std::move(topic_id_))
  , reply_to_(std::move(reply_to_))
  , options_(std::move(options_))
  , reply_markup_(std::move(reply_markup_))
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t sendMessage::ID;

void sendMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessage");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("topic_id", static_cast<const BaseObject *>(topic_id_.get()));
    s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get()));
    s.store_object_field("options", static_cast<const BaseObject *>(options_.get()));
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

sendResoldGift::sendResoldGift()
  : gift_name_()
  , owner_id_()
  , price_()
{}

sendResoldGift::sendResoldGift(string const &gift_name_, object_ptr<MessageSender> &&owner_id_, object_ptr<GiftResalePrice> &&price_)
  : gift_name_(gift_name_)
  , owner_id_(std::move(owner_id_))
  , price_(std::move(price_))
{}

const std::int32_t sendResoldGift::ID;

void sendResoldGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendResoldGift");
    s.store_field("gift_name", gift_name_);
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_object_field("price", static_cast<const BaseObject *>(price_.get()));
    s.store_class_end();
  }
}

sendWebAppCustomRequest::sendWebAppCustomRequest()
  : bot_user_id_()
  , method_()
  , parameters_()
{}

sendWebAppCustomRequest::sendWebAppCustomRequest(int53 bot_user_id_, string const &method_, string const &parameters_)
  : bot_user_id_(bot_user_id_)
  , method_(method_)
  , parameters_(parameters_)
{}

const std::int32_t sendWebAppCustomRequest::ID;

void sendWebAppCustomRequest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendWebAppCustomRequest");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("method", method_);
    s.store_field("parameters", parameters_);
    s.store_class_end();
  }
}

setArchiveChatListSettings::setArchiveChatListSettings()
  : settings_()
{}

setArchiveChatListSettings::setArchiveChatListSettings(object_ptr<archiveChatListSettings> &&settings_)
  : settings_(std::move(settings_))
{}

const std::int32_t setArchiveChatListSettings::ID;

void setArchiveChatListSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setArchiveChatListSettings");
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

setBusinessStartPage::setBusinessStartPage()
  : start_page_()
{}

setBusinessStartPage::setBusinessStartPage(object_ptr<inputBusinessStartPage> &&start_page_)
  : start_page_(std::move(start_page_))
{}

const std::int32_t setBusinessStartPage::ID;

void setBusinessStartPage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setBusinessStartPage");
    s.store_object_field("start_page", static_cast<const BaseObject *>(start_page_.get()));
    s.store_class_end();
  }
}

setChatAffiliateProgram::setChatAffiliateProgram()
  : chat_id_()
  , parameters_()
{}

setChatAffiliateProgram::setChatAffiliateProgram(int53 chat_id_, object_ptr<affiliateProgramParameters> &&parameters_)
  : chat_id_(chat_id_)
  , parameters_(std::move(parameters_))
{}

const std::int32_t setChatAffiliateProgram::ID;

void setChatAffiliateProgram::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatAffiliateProgram");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("parameters", static_cast<const BaseObject *>(parameters_.get()));
    s.store_class_end();
  }
}

setCloseFriends::setCloseFriends()
  : user_ids_()
{}

setCloseFriends::setCloseFriends(array<int53> &&user_ids_)
  : user_ids_(std::move(user_ids_))
{}

const std::int32_t setCloseFriends::ID;

void setCloseFriends::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setCloseFriends");
    { s.store_vector_begin("user_ids", user_ids_.size()); for (const auto &_value : user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

setDefaultBackground::setDefaultBackground()
  : background_()
  , type_()
  , for_dark_theme_()
{}

setDefaultBackground::setDefaultBackground(object_ptr<InputBackground> &&background_, object_ptr<BackgroundType> &&type_, bool for_dark_theme_)
  : background_(std::move(background_))
  , type_(std::move(type_))
  , for_dark_theme_(for_dark_theme_)
{}

const std::int32_t setDefaultBackground::ID;

void setDefaultBackground::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setDefaultBackground");
    s.store_object_field("background", static_cast<const BaseObject *>(background_.get()));
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("for_dark_theme", for_dark_theme_);
    s.store_class_end();
  }
}

setGiftResalePrice::setGiftResalePrice()
  : received_gift_id_()
  , price_()
{}

setGiftResalePrice::setGiftResalePrice(string const &received_gift_id_, object_ptr<GiftResalePrice> &&price_)
  : received_gift_id_(received_gift_id_)
  , price_(std::move(price_))
{}

const std::int32_t setGiftResalePrice::ID;

void setGiftResalePrice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setGiftResalePrice");
    s.store_field("received_gift_id", received_gift_id_);
    s.store_object_field("price", static_cast<const BaseObject *>(price_.get()));
    s.store_class_end();
  }
}

setInactiveSessionTtl::setInactiveSessionTtl()
  : inactive_session_ttl_days_()
{}

setInactiveSessionTtl::setInactiveSessionTtl(int32 inactive_session_ttl_days_)
  : inactive_session_ttl_days_(inactive_session_ttl_days_)
{}

const std::int32_t setInactiveSessionTtl::ID;

void setInactiveSessionTtl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setInactiveSessionTtl");
    s.store_field("inactive_session_ttl_days", inactive_session_ttl_days_);
    s.store_class_end();
  }
}

setInlineGameScore::setInlineGameScore()
  : inline_message_id_()
  , edit_message_()
  , user_id_()
  , score_()
  , force_()
{}

setInlineGameScore::setInlineGameScore(string const &inline_message_id_, bool edit_message_, int53 user_id_, int32 score_, bool force_)
  : inline_message_id_(inline_message_id_)
  , edit_message_(edit_message_)
  , user_id_(user_id_)
  , score_(score_)
  , force_(force_)
{}

const std::int32_t setInlineGameScore::ID;

void setInlineGameScore::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setInlineGameScore");
    s.store_field("inline_message_id", inline_message_id_);
    s.store_field("edit_message", edit_message_);
    s.store_field("user_id", user_id_);
    s.store_field("score", score_);
    s.store_field("force", force_);
    s.store_class_end();
  }
}

setLogVerbosityLevel::setLogVerbosityLevel()
  : new_verbosity_level_()
{}

setLogVerbosityLevel::setLogVerbosityLevel(int32 new_verbosity_level_)
  : new_verbosity_level_(new_verbosity_level_)
{}

const std::int32_t setLogVerbosityLevel::ID;

void setLogVerbosityLevel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setLogVerbosityLevel");
    s.store_field("new_verbosity_level", new_verbosity_level_);
    s.store_class_end();
  }
}

setLoginEmailAddress::setLoginEmailAddress()
  : new_login_email_address_()
{}

setLoginEmailAddress::setLoginEmailAddress(string const &new_login_email_address_)
  : new_login_email_address_(new_login_email_address_)
{}

const std::int32_t setLoginEmailAddress::ID;

void setLoginEmailAddress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setLoginEmailAddress");
    s.store_field("new_login_email_address", new_login_email_address_);
    s.store_class_end();
  }
}

setMessageFactCheck::setMessageFactCheck()
  : chat_id_()
  , message_id_()
  , text_()
{}

setMessageFactCheck::setMessageFactCheck(int53 chat_id_, int53 message_id_, object_ptr<formattedText> &&text_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , text_(std::move(text_))
{}

const std::int32_t setMessageFactCheck::ID;

void setMessageFactCheck::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setMessageFactCheck");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

setName::setName()
  : first_name_()
  , last_name_()
{}

setName::setName(string const &first_name_, string const &last_name_)
  : first_name_(first_name_)
  , last_name_(last_name_)
{}

const std::int32_t setName::ID;

void setName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setName");
    s.store_field("first_name", first_name_);
    s.store_field("last_name", last_name_);
    s.store_class_end();
  }
}

setReactionNotificationSettings::setReactionNotificationSettings()
  : notification_settings_()
{}

setReactionNotificationSettings::setReactionNotificationSettings(object_ptr<reactionNotificationSettings> &&notification_settings_)
  : notification_settings_(std::move(notification_settings_))
{}

const std::int32_t setReactionNotificationSettings::ID;

void setReactionNotificationSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setReactionNotificationSettings");
    s.store_object_field("notification_settings", static_cast<const BaseObject *>(notification_settings_.get()));
    s.store_class_end();
  }
}

setSupergroupUnrestrictBoostCount::setSupergroupUnrestrictBoostCount()
  : supergroup_id_()
  , unrestrict_boost_count_()
{}

setSupergroupUnrestrictBoostCount::setSupergroupUnrestrictBoostCount(int53 supergroup_id_, int32 unrestrict_boost_count_)
  : supergroup_id_(supergroup_id_)
  , unrestrict_boost_count_(unrestrict_boost_count_)
{}

const std::int32_t setSupergroupUnrestrictBoostCount::ID;

void setSupergroupUnrestrictBoostCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setSupergroupUnrestrictBoostCount");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_field("unrestrict_boost_count", unrestrict_boost_count_);
    s.store_class_end();
  }
}

setUserEmojiStatus::setUserEmojiStatus()
  : user_id_()
  , emoji_status_()
{}

setUserEmojiStatus::setUserEmojiStatus(int53 user_id_, object_ptr<emojiStatus> &&emoji_status_)
  : user_id_(user_id_)
  , emoji_status_(std::move(emoji_status_))
{}

const std::int32_t setUserEmojiStatus::ID;

void setUserEmojiStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setUserEmojiStatus");
    s.store_field("user_id", user_id_);
    s.store_object_field("emoji_status", static_cast<const BaseObject *>(emoji_status_.get()));
    s.store_class_end();
  }
}

toggleChatHasProtectedContent::toggleChatHasProtectedContent()
  : chat_id_()
  , has_protected_content_()
{}

toggleChatHasProtectedContent::toggleChatHasProtectedContent(int53 chat_id_, bool has_protected_content_)
  : chat_id_(chat_id_)
  , has_protected_content_(has_protected_content_)
{}

const std::int32_t toggleChatHasProtectedContent::ID;

void toggleChatHasProtectedContent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleChatHasProtectedContent");
    s.store_field("chat_id", chat_id_);
    s.store_field("has_protected_content", has_protected_content_);
    s.store_class_end();
  }
}

toggleGroupCallIsMyVideoEnabled::toggleGroupCallIsMyVideoEnabled()
  : group_call_id_()
  , is_my_video_enabled_()
{}

toggleGroupCallIsMyVideoEnabled::toggleGroupCallIsMyVideoEnabled(int32 group_call_id_, bool is_my_video_enabled_)
  : group_call_id_(group_call_id_)
  , is_my_video_enabled_(is_my_video_enabled_)
{}

const std::int32_t toggleGroupCallIsMyVideoEnabled::ID;

void toggleGroupCallIsMyVideoEnabled::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleGroupCallIsMyVideoEnabled");
    s.store_field("group_call_id", group_call_id_);
    s.store_field("is_my_video_enabled", is_my_video_enabled_);
    s.store_class_end();
  }
}

toggleGroupCallParticipantIsMuted::toggleGroupCallParticipantIsMuted()
  : group_call_id_()
  , participant_id_()
  , is_muted_()
{}

toggleGroupCallParticipantIsMuted::toggleGroupCallParticipantIsMuted(int32 group_call_id_, object_ptr<MessageSender> &&participant_id_, bool is_muted_)
  : group_call_id_(group_call_id_)
  , participant_id_(std::move(participant_id_))
  , is_muted_(is_muted_)
{}

const std::int32_t toggleGroupCallParticipantIsMuted::ID;

void toggleGroupCallParticipantIsMuted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleGroupCallParticipantIsMuted");
    s.store_field("group_call_id", group_call_id_);
    s.store_object_field("participant_id", static_cast<const BaseObject *>(participant_id_.get()));
    s.store_field("is_muted", is_muted_);
    s.store_class_end();
  }
}

toggleGroupCallScreenSharingIsPaused::toggleGroupCallScreenSharingIsPaused()
  : group_call_id_()
  , is_paused_()
{}

toggleGroupCallScreenSharingIsPaused::toggleGroupCallScreenSharingIsPaused(int32 group_call_id_, bool is_paused_)
  : group_call_id_(group_call_id_)
  , is_paused_(is_paused_)
{}

const std::int32_t toggleGroupCallScreenSharingIsPaused::ID;

void toggleGroupCallScreenSharingIsPaused::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleGroupCallScreenSharingIsPaused");
    s.store_field("group_call_id", group_call_id_);
    s.store_field("is_paused", is_paused_);
    s.store_class_end();
  }
}

toggleSupergroupJoinToSendMessages::toggleSupergroupJoinToSendMessages()
  : supergroup_id_()
  , join_to_send_messages_()
{}

toggleSupergroupJoinToSendMessages::toggleSupergroupJoinToSendMessages(int53 supergroup_id_, bool join_to_send_messages_)
  : supergroup_id_(supergroup_id_)
  , join_to_send_messages_(join_to_send_messages_)
{}

const std::int32_t toggleSupergroupJoinToSendMessages::ID;

void toggleSupergroupJoinToSendMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleSupergroupJoinToSendMessages");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_field("join_to_send_messages", join_to_send_messages_);
    s.store_class_end();
  }
}

unpinChatMessage::unpinChatMessage()
  : chat_id_()
  , message_id_()
{}

unpinChatMessage::unpinChatMessage(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t unpinChatMessage::ID;

void unpinChatMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "unpinChatMessage");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

viewMessages::viewMessages()
  : chat_id_()
  , message_ids_()
  , source_()
  , force_read_()
{}

viewMessages::viewMessages(int53 chat_id_, array<int53> &&message_ids_, object_ptr<MessageSource> &&source_, bool force_read_)
  : chat_id_(chat_id_)
  , message_ids_(std::move(message_ids_))
  , source_(std::move(source_))
  , force_read_(force_read_)
{}

const std::int32_t viewMessages::ID;

void viewMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "viewMessages");
    s.store_field("chat_id", chat_id_);
    { s.store_vector_begin("message_ids", message_ids_.size()); for (const auto &_value : message_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_object_field("source", static_cast<const BaseObject *>(source_.get()));
    s.store_field("force_read", force_read_);
    s.store_class_end();
  }
}
}  // namespace td_api
}  // namespace td
