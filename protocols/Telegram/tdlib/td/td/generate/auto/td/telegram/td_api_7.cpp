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


audio::audio()
  : duration_()
  , title_()
  , performer_()
  , file_name_()
  , mime_type_()
  , album_cover_minithumbnail_()
  , album_cover_thumbnail_()
  , external_album_covers_()
  , audio_()
{}

audio::audio(int32 duration_, string const &title_, string const &performer_, string const &file_name_, string const &mime_type_, object_ptr<minithumbnail> &&album_cover_minithumbnail_, object_ptr<thumbnail> &&album_cover_thumbnail_, array<object_ptr<thumbnail>> &&external_album_covers_, object_ptr<file> &&audio_)
  : duration_(duration_)
  , title_(title_)
  , performer_(performer_)
  , file_name_(file_name_)
  , mime_type_(mime_type_)
  , album_cover_minithumbnail_(std::move(album_cover_minithumbnail_))
  , album_cover_thumbnail_(std::move(album_cover_thumbnail_))
  , external_album_covers_(std::move(external_album_covers_))
  , audio_(std::move(audio_))
{}

const std::int32_t audio::ID;

void audio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "audio");
    s.store_field("duration", duration_);
    s.store_field("title", title_);
    s.store_field("performer", performer_);
    s.store_field("file_name", file_name_);
    s.store_field("mime_type", mime_type_);
    s.store_object_field("album_cover_minithumbnail", static_cast<const BaseObject *>(album_cover_minithumbnail_.get()));
    s.store_object_field("album_cover_thumbnail", static_cast<const BaseObject *>(album_cover_thumbnail_.get()));
    { s.store_vector_begin("external_album_covers", external_album_covers_.size()); for (const auto &_value : external_album_covers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("audio", static_cast<const BaseObject *>(audio_.get()));
    s.store_class_end();
  }
}

autoDownloadSettingsPresets::autoDownloadSettingsPresets()
  : low_()
  , medium_()
  , high_()
{}

autoDownloadSettingsPresets::autoDownloadSettingsPresets(object_ptr<autoDownloadSettings> &&low_, object_ptr<autoDownloadSettings> &&medium_, object_ptr<autoDownloadSettings> &&high_)
  : low_(std::move(low_))
  , medium_(std::move(medium_))
  , high_(std::move(high_))
{}

const std::int32_t autoDownloadSettingsPresets::ID;

void autoDownloadSettingsPresets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "autoDownloadSettingsPresets");
    s.store_object_field("low", static_cast<const BaseObject *>(low_.get()));
    s.store_object_field("medium", static_cast<const BaseObject *>(medium_.get()));
    s.store_object_field("high", static_cast<const BaseObject *>(high_.get()));
    s.store_class_end();
  }
}

botMediaPreviews::botMediaPreviews()
  : previews_()
{}

botMediaPreviews::botMediaPreviews(array<object_ptr<botMediaPreview>> &&previews_)
  : previews_(std::move(previews_))
{}

const std::int32_t botMediaPreviews::ID;

void botMediaPreviews::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botMediaPreviews");
    { s.store_vector_begin("previews", previews_.size()); for (const auto &_value : previews_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

botVerificationParameters::botVerificationParameters()
  : icon_custom_emoji_id_()
  , organization_name_()
  , default_custom_description_()
  , can_set_custom_description_()
{}

botVerificationParameters::botVerificationParameters(int64 icon_custom_emoji_id_, string const &organization_name_, object_ptr<formattedText> &&default_custom_description_, bool can_set_custom_description_)
  : icon_custom_emoji_id_(icon_custom_emoji_id_)
  , organization_name_(organization_name_)
  , default_custom_description_(std::move(default_custom_description_))
  , can_set_custom_description_(can_set_custom_description_)
{}

const std::int32_t botVerificationParameters::ID;

void botVerificationParameters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botVerificationParameters");
    s.store_field("icon_custom_emoji_id", icon_custom_emoji_id_);
    s.store_field("organization_name", organization_name_);
    s.store_object_field("default_custom_description", static_cast<const BaseObject *>(default_custom_description_.get()));
    s.store_field("can_set_custom_description", can_set_custom_description_);
    s.store_class_end();
  }
}

businessConnection::businessConnection()
  : id_()
  , user_id_()
  , user_chat_id_()
  , date_()
  , rights_()
  , is_enabled_()
{}

businessConnection::businessConnection(string const &id_, int53 user_id_, int53 user_chat_id_, int32 date_, object_ptr<businessBotRights> &&rights_, bool is_enabled_)
  : id_(id_)
  , user_id_(user_id_)
  , user_chat_id_(user_chat_id_)
  , date_(date_)
  , rights_(std::move(rights_))
  , is_enabled_(is_enabled_)
{}

const std::int32_t businessConnection::ID;

void businessConnection::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessConnection");
    s.store_field("id", id_);
    s.store_field("user_id", user_id_);
    s.store_field("user_chat_id", user_chat_id_);
    s.store_field("date", date_);
    s.store_object_field("rights", static_cast<const BaseObject *>(rights_.get()));
    s.store_field("is_enabled", is_enabled_);
    s.store_class_end();
  }
}

businessFeatureLocation::businessFeatureLocation() {
}

const std::int32_t businessFeatureLocation::ID;

void businessFeatureLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessFeatureLocation");
    s.store_class_end();
  }
}

businessFeatureOpeningHours::businessFeatureOpeningHours() {
}

const std::int32_t businessFeatureOpeningHours::ID;

void businessFeatureOpeningHours::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessFeatureOpeningHours");
    s.store_class_end();
  }
}

businessFeatureQuickReplies::businessFeatureQuickReplies() {
}

const std::int32_t businessFeatureQuickReplies::ID;

void businessFeatureQuickReplies::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessFeatureQuickReplies");
    s.store_class_end();
  }
}

businessFeatureGreetingMessage::businessFeatureGreetingMessage() {
}

const std::int32_t businessFeatureGreetingMessage::ID;

void businessFeatureGreetingMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessFeatureGreetingMessage");
    s.store_class_end();
  }
}

businessFeatureAwayMessage::businessFeatureAwayMessage() {
}

const std::int32_t businessFeatureAwayMessage::ID;

void businessFeatureAwayMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessFeatureAwayMessage");
    s.store_class_end();
  }
}

businessFeatureAccountLinks::businessFeatureAccountLinks() {
}

const std::int32_t businessFeatureAccountLinks::ID;

void businessFeatureAccountLinks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessFeatureAccountLinks");
    s.store_class_end();
  }
}

businessFeatureStartPage::businessFeatureStartPage() {
}

const std::int32_t businessFeatureStartPage::ID;

void businessFeatureStartPage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessFeatureStartPage");
    s.store_class_end();
  }
}

businessFeatureBots::businessFeatureBots() {
}

const std::int32_t businessFeatureBots::ID;

void businessFeatureBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessFeatureBots");
    s.store_class_end();
  }
}

businessFeatureEmojiStatus::businessFeatureEmojiStatus() {
}

const std::int32_t businessFeatureEmojiStatus::ID;

void businessFeatureEmojiStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessFeatureEmojiStatus");
    s.store_class_end();
  }
}

businessFeatureChatFolderTags::businessFeatureChatFolderTags() {
}

const std::int32_t businessFeatureChatFolderTags::ID;

void businessFeatureChatFolderTags::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessFeatureChatFolderTags");
    s.store_class_end();
  }
}

businessFeatureUpgradedStories::businessFeatureUpgradedStories() {
}

const std::int32_t businessFeatureUpgradedStories::ID;

void businessFeatureUpgradedStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessFeatureUpgradedStories");
    s.store_class_end();
  }
}

businessInfo::businessInfo()
  : location_()
  , opening_hours_()
  , local_opening_hours_()
  , next_open_in_()
  , next_close_in_()
  , greeting_message_settings_()
  , away_message_settings_()
  , start_page_()
{}

businessInfo::businessInfo(object_ptr<businessLocation> &&location_, object_ptr<businessOpeningHours> &&opening_hours_, object_ptr<businessOpeningHours> &&local_opening_hours_, int32 next_open_in_, int32 next_close_in_, object_ptr<businessGreetingMessageSettings> &&greeting_message_settings_, object_ptr<businessAwayMessageSettings> &&away_message_settings_, object_ptr<businessStartPage> &&start_page_)
  : location_(std::move(location_))
  , opening_hours_(std::move(opening_hours_))
  , local_opening_hours_(std::move(local_opening_hours_))
  , next_open_in_(next_open_in_)
  , next_close_in_(next_close_in_)
  , greeting_message_settings_(std::move(greeting_message_settings_))
  , away_message_settings_(std::move(away_message_settings_))
  , start_page_(std::move(start_page_))
{}

const std::int32_t businessInfo::ID;

void businessInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessInfo");
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_object_field("opening_hours", static_cast<const BaseObject *>(opening_hours_.get()));
    s.store_object_field("local_opening_hours", static_cast<const BaseObject *>(local_opening_hours_.get()));
    s.store_field("next_open_in", next_open_in_);
    s.store_field("next_close_in", next_close_in_);
    s.store_object_field("greeting_message_settings", static_cast<const BaseObject *>(greeting_message_settings_.get()));
    s.store_object_field("away_message_settings", static_cast<const BaseObject *>(away_message_settings_.get()));
    s.store_object_field("start_page", static_cast<const BaseObject *>(start_page_.get()));
    s.store_class_end();
  }
}

businessRecipients::businessRecipients()
  : chat_ids_()
  , excluded_chat_ids_()
  , select_existing_chats_()
  , select_new_chats_()
  , select_contacts_()
  , select_non_contacts_()
  , exclude_selected_()
{}

businessRecipients::businessRecipients(array<int53> &&chat_ids_, array<int53> &&excluded_chat_ids_, bool select_existing_chats_, bool select_new_chats_, bool select_contacts_, bool select_non_contacts_, bool exclude_selected_)
  : chat_ids_(std::move(chat_ids_))
  , excluded_chat_ids_(std::move(excluded_chat_ids_))
  , select_existing_chats_(select_existing_chats_)
  , select_new_chats_(select_new_chats_)
  , select_contacts_(select_contacts_)
  , select_non_contacts_(select_non_contacts_)
  , exclude_selected_(exclude_selected_)
{}

const std::int32_t businessRecipients::ID;

void businessRecipients::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessRecipients");
    { s.store_vector_begin("chat_ids", chat_ids_.size()); for (const auto &_value : chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("excluded_chat_ids", excluded_chat_ids_.size()); for (const auto &_value : excluded_chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("select_existing_chats", select_existing_chats_);
    s.store_field("select_new_chats", select_new_chats_);
    s.store_field("select_contacts", select_contacts_);
    s.store_field("select_non_contacts", select_non_contacts_);
    s.store_field("exclude_selected", exclude_selected_);
    s.store_class_end();
  }
}

chatBoostSlots::chatBoostSlots()
  : slots_()
{}

chatBoostSlots::chatBoostSlots(array<object_ptr<chatBoostSlot>> &&slots_)
  : slots_(std::move(slots_))
{}

const std::int32_t chatBoostSlots::ID;

void chatBoostSlots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatBoostSlots");
    { s.store_vector_begin("slots", slots_.size()); for (const auto &_value : slots_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatInviteLinkMembers::chatInviteLinkMembers()
  : total_count_()
  , members_()
{}

chatInviteLinkMembers::chatInviteLinkMembers(int32 total_count_, array<object_ptr<chatInviteLinkMember>> &&members_)
  : total_count_(total_count_)
  , members_(std::move(members_))
{}

const std::int32_t chatInviteLinkMembers::ID;

void chatInviteLinkMembers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatInviteLinkMembers");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("members", members_.size()); for (const auto &_value : members_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatMember::chatMember()
  : member_id_()
  , inviter_user_id_()
  , joined_chat_date_()
  , status_()
{}

chatMember::chatMember(object_ptr<MessageSender> &&member_id_, int53 inviter_user_id_, int32 joined_chat_date_, object_ptr<ChatMemberStatus> &&status_)
  : member_id_(std::move(member_id_))
  , inviter_user_id_(inviter_user_id_)
  , joined_chat_date_(joined_chat_date_)
  , status_(std::move(status_))
{}

const std::int32_t chatMember::ID;

void chatMember::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatMember");
    s.store_object_field("member_id", static_cast<const BaseObject *>(member_id_.get()));
    s.store_field("inviter_user_id", inviter_user_id_);
    s.store_field("joined_chat_date", joined_chat_date_);
    s.store_object_field("status", static_cast<const BaseObject *>(status_.get()));
    s.store_class_end();
  }
}

chatPermissions::chatPermissions()
  : can_send_basic_messages_()
  , can_send_audios_()
  , can_send_documents_()
  , can_send_photos_()
  , can_send_videos_()
  , can_send_video_notes_()
  , can_send_voice_notes_()
  , can_send_polls_()
  , can_send_other_messages_()
  , can_add_link_previews_()
  , can_change_info_()
  , can_invite_users_()
  , can_pin_messages_()
  , can_create_topics_()
{}

chatPermissions::chatPermissions(bool can_send_basic_messages_, bool can_send_audios_, bool can_send_documents_, bool can_send_photos_, bool can_send_videos_, bool can_send_video_notes_, bool can_send_voice_notes_, bool can_send_polls_, bool can_send_other_messages_, bool can_add_link_previews_, bool can_change_info_, bool can_invite_users_, bool can_pin_messages_, bool can_create_topics_)
  : can_send_basic_messages_(can_send_basic_messages_)
  , can_send_audios_(can_send_audios_)
  , can_send_documents_(can_send_documents_)
  , can_send_photos_(can_send_photos_)
  , can_send_videos_(can_send_videos_)
  , can_send_video_notes_(can_send_video_notes_)
  , can_send_voice_notes_(can_send_voice_notes_)
  , can_send_polls_(can_send_polls_)
  , can_send_other_messages_(can_send_other_messages_)
  , can_add_link_previews_(can_add_link_previews_)
  , can_change_info_(can_change_info_)
  , can_invite_users_(can_invite_users_)
  , can_pin_messages_(can_pin_messages_)
  , can_create_topics_(can_create_topics_)
{}

const std::int32_t chatPermissions::ID;

void chatPermissions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatPermissions");
    s.store_field("can_send_basic_messages", can_send_basic_messages_);
    s.store_field("can_send_audios", can_send_audios_);
    s.store_field("can_send_documents", can_send_documents_);
    s.store_field("can_send_photos", can_send_photos_);
    s.store_field("can_send_videos", can_send_videos_);
    s.store_field("can_send_video_notes", can_send_video_notes_);
    s.store_field("can_send_voice_notes", can_send_voice_notes_);
    s.store_field("can_send_polls", can_send_polls_);
    s.store_field("can_send_other_messages", can_send_other_messages_);
    s.store_field("can_add_link_previews", can_add_link_previews_);
    s.store_field("can_change_info", can_change_info_);
    s.store_field("can_invite_users", can_invite_users_);
    s.store_field("can_pin_messages", can_pin_messages_);
    s.store_field("can_create_topics", can_create_topics_);
    s.store_class_end();
  }
}

chatRevenueAmount::chatRevenueAmount()
  : cryptocurrency_()
  , total_amount_()
  , balance_amount_()
  , available_amount_()
  , withdrawal_enabled_()
{}

chatRevenueAmount::chatRevenueAmount(string const &cryptocurrency_, int64 total_amount_, int64 balance_amount_, int64 available_amount_, bool withdrawal_enabled_)
  : cryptocurrency_(cryptocurrency_)
  , total_amount_(total_amount_)
  , balance_amount_(balance_amount_)
  , available_amount_(available_amount_)
  , withdrawal_enabled_(withdrawal_enabled_)
{}

const std::int32_t chatRevenueAmount::ID;

void chatRevenueAmount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatRevenueAmount");
    s.store_field("cryptocurrency", cryptocurrency_);
    s.store_field("total_amount", total_amount_);
    s.store_field("balance_amount", balance_amount_);
    s.store_field("available_amount", available_amount_);
    s.store_field("withdrawal_enabled", withdrawal_enabled_);
    s.store_class_end();
  }
}

chatRevenueTransactionTypeUnsupported::chatRevenueTransactionTypeUnsupported() {
}

const std::int32_t chatRevenueTransactionTypeUnsupported::ID;

void chatRevenueTransactionTypeUnsupported::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatRevenueTransactionTypeUnsupported");
    s.store_class_end();
  }
}

chatRevenueTransactionTypeSponsoredMessageEarnings::chatRevenueTransactionTypeSponsoredMessageEarnings()
  : start_date_()
  , end_date_()
{}

chatRevenueTransactionTypeSponsoredMessageEarnings::chatRevenueTransactionTypeSponsoredMessageEarnings(int32 start_date_, int32 end_date_)
  : start_date_(start_date_)
  , end_date_(end_date_)
{}

const std::int32_t chatRevenueTransactionTypeSponsoredMessageEarnings::ID;

void chatRevenueTransactionTypeSponsoredMessageEarnings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatRevenueTransactionTypeSponsoredMessageEarnings");
    s.store_field("start_date", start_date_);
    s.store_field("end_date", end_date_);
    s.store_class_end();
  }
}

chatRevenueTransactionTypeSuggestedPostEarnings::chatRevenueTransactionTypeSuggestedPostEarnings()
  : user_id_()
{}

chatRevenueTransactionTypeSuggestedPostEarnings::chatRevenueTransactionTypeSuggestedPostEarnings(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t chatRevenueTransactionTypeSuggestedPostEarnings::ID;

void chatRevenueTransactionTypeSuggestedPostEarnings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatRevenueTransactionTypeSuggestedPostEarnings");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

chatRevenueTransactionTypeFragmentWithdrawal::chatRevenueTransactionTypeFragmentWithdrawal()
  : withdrawal_date_()
  , state_()
{}

chatRevenueTransactionTypeFragmentWithdrawal::chatRevenueTransactionTypeFragmentWithdrawal(int32 withdrawal_date_, object_ptr<RevenueWithdrawalState> &&state_)
  : withdrawal_date_(withdrawal_date_)
  , state_(std::move(state_))
{}

const std::int32_t chatRevenueTransactionTypeFragmentWithdrawal::ID;

void chatRevenueTransactionTypeFragmentWithdrawal::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatRevenueTransactionTypeFragmentWithdrawal");
    s.store_field("withdrawal_date", withdrawal_date_);
    s.store_object_field("state", static_cast<const BaseObject *>(state_.get()));
    s.store_class_end();
  }
}

chatRevenueTransactionTypeFragmentRefund::chatRevenueTransactionTypeFragmentRefund()
  : refund_date_()
{}

chatRevenueTransactionTypeFragmentRefund::chatRevenueTransactionTypeFragmentRefund(int32 refund_date_)
  : refund_date_(refund_date_)
{}

const std::int32_t chatRevenueTransactionTypeFragmentRefund::ID;

void chatRevenueTransactionTypeFragmentRefund::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatRevenueTransactionTypeFragmentRefund");
    s.store_field("refund_date", refund_date_);
    s.store_class_end();
  }
}

chatTypePrivate::chatTypePrivate()
  : user_id_()
{}

chatTypePrivate::chatTypePrivate(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t chatTypePrivate::ID;

void chatTypePrivate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatTypePrivate");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

chatTypeBasicGroup::chatTypeBasicGroup()
  : basic_group_id_()
{}

chatTypeBasicGroup::chatTypeBasicGroup(int53 basic_group_id_)
  : basic_group_id_(basic_group_id_)
{}

const std::int32_t chatTypeBasicGroup::ID;

void chatTypeBasicGroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatTypeBasicGroup");
    s.store_field("basic_group_id", basic_group_id_);
    s.store_class_end();
  }
}

chatTypeSupergroup::chatTypeSupergroup()
  : supergroup_id_()
  , is_channel_()
{}

chatTypeSupergroup::chatTypeSupergroup(int53 supergroup_id_, bool is_channel_)
  : supergroup_id_(supergroup_id_)
  , is_channel_(is_channel_)
{}

const std::int32_t chatTypeSupergroup::ID;

void chatTypeSupergroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatTypeSupergroup");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_field("is_channel", is_channel_);
    s.store_class_end();
  }
}

chatTypeSecret::chatTypeSecret()
  : secret_chat_id_()
  , user_id_()
{}

chatTypeSecret::chatTypeSecret(int32 secret_chat_id_, int53 user_id_)
  : secret_chat_id_(secret_chat_id_)
  , user_id_(user_id_)
{}

const std::int32_t chatTypeSecret::ID;

void chatTypeSecret::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatTypeSecret");
    s.store_field("secret_chat_id", secret_chat_id_);
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

connectedWebsite::connectedWebsite()
  : id_()
  , domain_name_()
  , bot_user_id_()
  , browser_()
  , platform_()
  , log_in_date_()
  , last_active_date_()
  , ip_address_()
  , location_()
{}

connectedWebsite::connectedWebsite(int64 id_, string const &domain_name_, int53 bot_user_id_, string const &browser_, string const &platform_, int32 log_in_date_, int32 last_active_date_, string const &ip_address_, string const &location_)
  : id_(id_)
  , domain_name_(domain_name_)
  , bot_user_id_(bot_user_id_)
  , browser_(browser_)
  , platform_(platform_)
  , log_in_date_(log_in_date_)
  , last_active_date_(last_active_date_)
  , ip_address_(ip_address_)
  , location_(location_)
{}

const std::int32_t connectedWebsite::ID;

void connectedWebsite::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "connectedWebsite");
    s.store_field("id", id_);
    s.store_field("domain_name", domain_name_);
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("browser", browser_);
    s.store_field("platform", platform_);
    s.store_field("log_in_date", log_in_date_);
    s.store_field("last_active_date", last_active_date_);
    s.store_field("ip_address", ip_address_);
    s.store_field("location", location_);
    s.store_class_end();
  }
}

data::data()
  : data_()
{}

data::data(bytes const &data_)
  : data_(std::move(data_))
{}

const std::int32_t data::ID;

void data::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "data");
    s.store_bytes_field("data", data_);
    s.store_class_end();
  }
}

datedFile::datedFile()
  : file_()
  , date_()
{}

datedFile::datedFile(object_ptr<file> &&file_, int32 date_)
  : file_(std::move(file_))
  , date_(date_)
{}

const std::int32_t datedFile::ID;

void datedFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "datedFile");
    s.store_object_field("file", static_cast<const BaseObject *>(file_.get()));
    s.store_field("date", date_);
    s.store_class_end();
  }
}

emailAddressAuthenticationCode::emailAddressAuthenticationCode()
  : code_()
{}

emailAddressAuthenticationCode::emailAddressAuthenticationCode(string const &code_)
  : code_(code_)
{}

const std::int32_t emailAddressAuthenticationCode::ID;

void emailAddressAuthenticationCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emailAddressAuthenticationCode");
    s.store_field("code", code_);
    s.store_class_end();
  }
}

emailAddressAuthenticationAppleId::emailAddressAuthenticationAppleId()
  : token_()
{}

emailAddressAuthenticationAppleId::emailAddressAuthenticationAppleId(string const &token_)
  : token_(token_)
{}

const std::int32_t emailAddressAuthenticationAppleId::ID;

void emailAddressAuthenticationAppleId::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emailAddressAuthenticationAppleId");
    s.store_field("token", token_);
    s.store_class_end();
  }
}

emailAddressAuthenticationGoogleId::emailAddressAuthenticationGoogleId()
  : token_()
{}

emailAddressAuthenticationGoogleId::emailAddressAuthenticationGoogleId(string const &token_)
  : token_(token_)
{}

const std::int32_t emailAddressAuthenticationGoogleId::ID;

void emailAddressAuthenticationGoogleId::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emailAddressAuthenticationGoogleId");
    s.store_field("token", token_);
    s.store_class_end();
  }
}

emojiStatusCustomEmojis::emojiStatusCustomEmojis()
  : custom_emoji_ids_()
{}

emojiStatusCustomEmojis::emojiStatusCustomEmojis(array<int64> &&custom_emoji_ids_)
  : custom_emoji_ids_(std::move(custom_emoji_ids_))
{}

const std::int32_t emojiStatusCustomEmojis::ID;

void emojiStatusCustomEmojis::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiStatusCustomEmojis");
    { s.store_vector_begin("custom_emoji_ids", custom_emoji_ids_.size()); for (const auto &_value : custom_emoji_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

firebaseDeviceVerificationParametersSafetyNet::firebaseDeviceVerificationParametersSafetyNet()
  : nonce_()
{}

firebaseDeviceVerificationParametersSafetyNet::firebaseDeviceVerificationParametersSafetyNet(bytes const &nonce_)
  : nonce_(std::move(nonce_))
{}

const std::int32_t firebaseDeviceVerificationParametersSafetyNet::ID;

void firebaseDeviceVerificationParametersSafetyNet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "firebaseDeviceVerificationParametersSafetyNet");
    s.store_bytes_field("nonce", nonce_);
    s.store_class_end();
  }
}

firebaseDeviceVerificationParametersPlayIntegrity::firebaseDeviceVerificationParametersPlayIntegrity()
  : nonce_()
  , cloud_project_number_()
{}

firebaseDeviceVerificationParametersPlayIntegrity::firebaseDeviceVerificationParametersPlayIntegrity(string const &nonce_, int64 cloud_project_number_)
  : nonce_(nonce_)
  , cloud_project_number_(cloud_project_number_)
{}

const std::int32_t firebaseDeviceVerificationParametersPlayIntegrity::ID;

void firebaseDeviceVerificationParametersPlayIntegrity::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "firebaseDeviceVerificationParametersPlayIntegrity");
    s.store_field("nonce", nonce_);
    s.store_field("cloud_project_number", cloud_project_number_);
    s.store_class_end();
  }
}

forumTopic::forumTopic()
  : info_()
  , last_message_()
  , order_()
  , is_pinned_()
  , unread_count_()
  , last_read_inbox_message_id_()
  , last_read_outbox_message_id_()
  , unread_mention_count_()
  , unread_reaction_count_()
  , notification_settings_()
  , draft_message_()
{}

forumTopic::forumTopic(object_ptr<forumTopicInfo> &&info_, object_ptr<message> &&last_message_, int64 order_, bool is_pinned_, int32 unread_count_, int53 last_read_inbox_message_id_, int53 last_read_outbox_message_id_, int32 unread_mention_count_, int32 unread_reaction_count_, object_ptr<chatNotificationSettings> &&notification_settings_, object_ptr<draftMessage> &&draft_message_)
  : info_(std::move(info_))
  , last_message_(std::move(last_message_))
  , order_(order_)
  , is_pinned_(is_pinned_)
  , unread_count_(unread_count_)
  , last_read_inbox_message_id_(last_read_inbox_message_id_)
  , last_read_outbox_message_id_(last_read_outbox_message_id_)
  , unread_mention_count_(unread_mention_count_)
  , unread_reaction_count_(unread_reaction_count_)
  , notification_settings_(std::move(notification_settings_))
  , draft_message_(std::move(draft_message_))
{}

const std::int32_t forumTopic::ID;

void forumTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "forumTopic");
    s.store_object_field("info", static_cast<const BaseObject *>(info_.get()));
    s.store_object_field("last_message", static_cast<const BaseObject *>(last_message_.get()));
    s.store_field("order", order_);
    s.store_field("is_pinned", is_pinned_);
    s.store_field("unread_count", unread_count_);
    s.store_field("last_read_inbox_message_id", last_read_inbox_message_id_);
    s.store_field("last_read_outbox_message_id", last_read_outbox_message_id_);
    s.store_field("unread_mention_count", unread_mention_count_);
    s.store_field("unread_reaction_count", unread_reaction_count_);
    s.store_object_field("notification_settings", static_cast<const BaseObject *>(notification_settings_.get()));
    s.store_object_field("draft_message", static_cast<const BaseObject *>(draft_message_.get()));
    s.store_class_end();
  }
}

foundPositions::foundPositions()
  : total_count_()
  , positions_()
{}

foundPositions::foundPositions(int32 total_count_, array<int32> &&positions_)
  : total_count_(total_count_)
  , positions_(std::move(positions_))
{}

const std::int32_t foundPositions::ID;

void foundPositions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "foundPositions");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("positions", positions_.size()); for (const auto &_value : positions_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

giftResaleParameters::giftResaleParameters()
  : star_count_()
  , toncoin_cent_count_()
  , toncoin_only_()
{}

giftResaleParameters::giftResaleParameters(int53 star_count_, int53 toncoin_cent_count_, bool toncoin_only_)
  : star_count_(star_count_)
  , toncoin_cent_count_(toncoin_cent_count_)
  , toncoin_only_(toncoin_only_)
{}

const std::int32_t giftResaleParameters::ID;

void giftResaleParameters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giftResaleParameters");
    s.store_field("star_count", star_count_);
    s.store_field("toncoin_cent_count", toncoin_cent_count_);
    s.store_field("toncoin_only", toncoin_only_);
    s.store_class_end();
  }
}

groupCallId::groupCallId()
  : id_()
{}

groupCallId::groupCallId(int32 id_)
  : id_(id_)
{}

const std::int32_t groupCallId::ID;

void groupCallId::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCallId");
    s.store_field("id", id_);
    s.store_class_end();
  }
}

groupCallParticipant::groupCallParticipant()
  : participant_id_()
  , audio_source_id_()
  , screen_sharing_audio_source_id_()
  , video_info_()
  , screen_sharing_video_info_()
  , bio_()
  , is_current_user_()
  , is_speaking_()
  , is_hand_raised_()
  , can_be_muted_for_all_users_()
  , can_be_unmuted_for_all_users_()
  , can_be_muted_for_current_user_()
  , can_be_unmuted_for_current_user_()
  , is_muted_for_all_users_()
  , is_muted_for_current_user_()
  , can_unmute_self_()
  , volume_level_()
  , order_()
{}

groupCallParticipant::groupCallParticipant(object_ptr<MessageSender> &&participant_id_, int32 audio_source_id_, int32 screen_sharing_audio_source_id_, object_ptr<groupCallParticipantVideoInfo> &&video_info_, object_ptr<groupCallParticipantVideoInfo> &&screen_sharing_video_info_, string const &bio_, bool is_current_user_, bool is_speaking_, bool is_hand_raised_, bool can_be_muted_for_all_users_, bool can_be_unmuted_for_all_users_, bool can_be_muted_for_current_user_, bool can_be_unmuted_for_current_user_, bool is_muted_for_all_users_, bool is_muted_for_current_user_, bool can_unmute_self_, int32 volume_level_, string const &order_)
  : participant_id_(std::move(participant_id_))
  , audio_source_id_(audio_source_id_)
  , screen_sharing_audio_source_id_(screen_sharing_audio_source_id_)
  , video_info_(std::move(video_info_))
  , screen_sharing_video_info_(std::move(screen_sharing_video_info_))
  , bio_(bio_)
  , is_current_user_(is_current_user_)
  , is_speaking_(is_speaking_)
  , is_hand_raised_(is_hand_raised_)
  , can_be_muted_for_all_users_(can_be_muted_for_all_users_)
  , can_be_unmuted_for_all_users_(can_be_unmuted_for_all_users_)
  , can_be_muted_for_current_user_(can_be_muted_for_current_user_)
  , can_be_unmuted_for_current_user_(can_be_unmuted_for_current_user_)
  , is_muted_for_all_users_(is_muted_for_all_users_)
  , is_muted_for_current_user_(is_muted_for_current_user_)
  , can_unmute_self_(can_unmute_self_)
  , volume_level_(volume_level_)
  , order_(order_)
{}

const std::int32_t groupCallParticipant::ID;

void groupCallParticipant::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCallParticipant");
    s.store_object_field("participant_id", static_cast<const BaseObject *>(participant_id_.get()));
    s.store_field("audio_source_id", audio_source_id_);
    s.store_field("screen_sharing_audio_source_id", screen_sharing_audio_source_id_);
    s.store_object_field("video_info", static_cast<const BaseObject *>(video_info_.get()));
    s.store_object_field("screen_sharing_video_info", static_cast<const BaseObject *>(screen_sharing_video_info_.get()));
    s.store_field("bio", bio_);
    s.store_field("is_current_user", is_current_user_);
    s.store_field("is_speaking", is_speaking_);
    s.store_field("is_hand_raised", is_hand_raised_);
    s.store_field("can_be_muted_for_all_users", can_be_muted_for_all_users_);
    s.store_field("can_be_unmuted_for_all_users", can_be_unmuted_for_all_users_);
    s.store_field("can_be_muted_for_current_user", can_be_muted_for_current_user_);
    s.store_field("can_be_unmuted_for_current_user", can_be_unmuted_for_current_user_);
    s.store_field("is_muted_for_all_users", is_muted_for_all_users_);
    s.store_field("is_muted_for_current_user", is_muted_for_current_user_);
    s.store_field("can_unmute_self", can_unmute_self_);
    s.store_field("volume_level", volume_level_);
    s.store_field("order", order_);
    s.store_class_end();
  }
}

importedContact::importedContact()
  : phone_number_()
  , first_name_()
  , last_name_()
  , note_()
{}

importedContact::importedContact(string const &phone_number_, string const &first_name_, string const &last_name_, object_ptr<formattedText> &&note_)
  : phone_number_(phone_number_)
  , first_name_(first_name_)
  , last_name_(last_name_)
  , note_(std::move(note_))
{}

const std::int32_t importedContact::ID;

void importedContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "importedContact");
    s.store_field("phone_number", phone_number_);
    s.store_field("first_name", first_name_);
    s.store_field("last_name", last_name_);
    s.store_object_field("note", static_cast<const BaseObject *>(note_.get()));
    s.store_class_end();
  }
}

inputChatPhotoPrevious::inputChatPhotoPrevious()
  : chat_photo_id_()
{}

inputChatPhotoPrevious::inputChatPhotoPrevious(int64 chat_photo_id_)
  : chat_photo_id_(chat_photo_id_)
{}

const std::int32_t inputChatPhotoPrevious::ID;

void inputChatPhotoPrevious::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputChatPhotoPrevious");
    s.store_field("chat_photo_id", chat_photo_id_);
    s.store_class_end();
  }
}

inputChatPhotoStatic::inputChatPhotoStatic()
  : photo_()
{}

inputChatPhotoStatic::inputChatPhotoStatic(object_ptr<InputFile> &&photo_)
  : photo_(std::move(photo_))
{}

const std::int32_t inputChatPhotoStatic::ID;

void inputChatPhotoStatic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputChatPhotoStatic");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

inputChatPhotoAnimation::inputChatPhotoAnimation()
  : animation_()
  , main_frame_timestamp_()
{}

inputChatPhotoAnimation::inputChatPhotoAnimation(object_ptr<InputFile> &&animation_, double main_frame_timestamp_)
  : animation_(std::move(animation_))
  , main_frame_timestamp_(main_frame_timestamp_)
{}

const std::int32_t inputChatPhotoAnimation::ID;

void inputChatPhotoAnimation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputChatPhotoAnimation");
    s.store_object_field("animation", static_cast<const BaseObject *>(animation_.get()));
    s.store_field("main_frame_timestamp", main_frame_timestamp_);
    s.store_class_end();
  }
}

inputChatPhotoSticker::inputChatPhotoSticker()
  : sticker_()
{}

inputChatPhotoSticker::inputChatPhotoSticker(object_ptr<chatPhotoSticker> &&sticker_)
  : sticker_(std::move(sticker_))
{}

const std::int32_t inputChatPhotoSticker::ID;

void inputChatPhotoSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputChatPhotoSticker");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

inputCredentialsSaved::inputCredentialsSaved()
  : saved_credentials_id_()
{}

inputCredentialsSaved::inputCredentialsSaved(string const &saved_credentials_id_)
  : saved_credentials_id_(saved_credentials_id_)
{}

const std::int32_t inputCredentialsSaved::ID;

void inputCredentialsSaved::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputCredentialsSaved");
    s.store_field("saved_credentials_id", saved_credentials_id_);
    s.store_class_end();
  }
}

inputCredentialsNew::inputCredentialsNew()
  : data_()
  , allow_save_()
{}

inputCredentialsNew::inputCredentialsNew(string const &data_, bool allow_save_)
  : data_(data_)
  , allow_save_(allow_save_)
{}

const std::int32_t inputCredentialsNew::ID;

void inputCredentialsNew::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputCredentialsNew");
    s.store_field("data", data_);
    s.store_field("allow_save", allow_save_);
    s.store_class_end();
  }
}

inputCredentialsApplePay::inputCredentialsApplePay()
  : data_()
{}

inputCredentialsApplePay::inputCredentialsApplePay(string const &data_)
  : data_(data_)
{}

const std::int32_t inputCredentialsApplePay::ID;

void inputCredentialsApplePay::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputCredentialsApplePay");
    s.store_field("data", data_);
    s.store_class_end();
  }
}

inputCredentialsGooglePay::inputCredentialsGooglePay()
  : data_()
{}

inputCredentialsGooglePay::inputCredentialsGooglePay(string const &data_)
  : data_(data_)
{}

const std::int32_t inputCredentialsGooglePay::ID;

void inputCredentialsGooglePay::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputCredentialsGooglePay");
    s.store_field("data", data_);
    s.store_class_end();
  }
}

inputFileId::inputFileId()
  : id_()
{}

inputFileId::inputFileId(int32 id_)
  : id_(id_)
{}

const std::int32_t inputFileId::ID;

void inputFileId::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputFileId");
    s.store_field("id", id_);
    s.store_class_end();
  }
}

inputFileRemote::inputFileRemote()
  : id_()
{}

inputFileRemote::inputFileRemote(string const &id_)
  : id_(id_)
{}

const std::int32_t inputFileRemote::ID;

void inputFileRemote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputFileRemote");
    s.store_field("id", id_);
    s.store_class_end();
  }
}

inputFileLocal::inputFileLocal()
  : path_()
{}

inputFileLocal::inputFileLocal(string const &path_)
  : path_(path_)
{}

const std::int32_t inputFileLocal::ID;

void inputFileLocal::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputFileLocal");
    s.store_field("path", path_);
    s.store_class_end();
  }
}

inputFileGenerated::inputFileGenerated()
  : original_path_()
  , conversion_()
  , expected_size_()
{}

inputFileGenerated::inputFileGenerated(string const &original_path_, string const &conversion_, int53 expected_size_)
  : original_path_(original_path_)
  , conversion_(conversion_)
  , expected_size_(expected_size_)
{}

const std::int32_t inputFileGenerated::ID;

void inputFileGenerated::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputFileGenerated");
    s.store_field("original_path", original_path_);
    s.store_field("conversion", conversion_);
    s.store_field("expected_size", expected_size_);
    s.store_class_end();
  }
}

localizationTargetInfo::localizationTargetInfo()
  : language_packs_()
{}

localizationTargetInfo::localizationTargetInfo(array<object_ptr<languagePackInfo>> &&language_packs_)
  : language_packs_(std::move(language_packs_))
{}

const std::int32_t localizationTargetInfo::ID;

void localizationTargetInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "localizationTargetInfo");
    { s.store_vector_begin("language_packs", language_packs_.size()); for (const auto &_value : language_packs_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messageEffect::messageEffect()
  : id_()
  , static_icon_()
  , emoji_()
  , is_premium_()
  , type_()
{}

messageEffect::messageEffect(int64 id_, object_ptr<sticker> &&static_icon_, string const &emoji_, bool is_premium_, object_ptr<MessageEffectType> &&type_)
  : id_(id_)
  , static_icon_(std::move(static_icon_))
  , emoji_(emoji_)
  , is_premium_(is_premium_)
  , type_(std::move(type_))
{}

const std::int32_t messageEffect::ID;

void messageEffect::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEffect");
    s.store_field("id", id_);
    s.store_object_field("static_icon", static_cast<const BaseObject *>(static_icon_.get()));
    s.store_field("emoji", emoji_);
    s.store_field("is_premium", is_premium_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

messageForwardInfo::messageForwardInfo()
  : origin_()
  , date_()
  , source_()
  , public_service_announcement_type_()
{}

messageForwardInfo::messageForwardInfo(object_ptr<MessageOrigin> &&origin_, int32 date_, object_ptr<forwardSource> &&source_, string const &public_service_announcement_type_)
  : origin_(std::move(origin_))
  , date_(date_)
  , source_(std::move(source_))
  , public_service_announcement_type_(public_service_announcement_type_)
{}

const std::int32_t messageForwardInfo::ID;

void messageForwardInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageForwardInfo");
    s.store_object_field("origin", static_cast<const BaseObject *>(origin_.get()));
    s.store_field("date", date_);
    s.store_object_field("source", static_cast<const BaseObject *>(source_.get()));
    s.store_field("public_service_announcement_type", public_service_announcement_type_);
    s.store_class_end();
  }
}

messageProperties::messageProperties()
  : can_add_offer_()
  , can_add_tasks_()
  , can_be_approved_()
  , can_be_copied_()
  , can_be_copied_to_secret_chat_()
  , can_be_declined_()
  , can_be_deleted_only_for_self_()
  , can_be_deleted_for_all_users_()
  , can_be_edited_()
  , can_be_forwarded_()
  , can_be_paid_()
  , can_be_pinned_()
  , can_be_replied_()
  , can_be_replied_in_another_chat_()
  , can_be_saved_()
  , can_be_shared_in_story_()
  , can_edit_media_()
  , can_edit_scheduling_state_()
  , can_edit_suggested_post_info_()
  , can_get_author_()
  , can_get_embedding_code_()
  , can_get_link_()
  , can_get_media_timestamp_links_()
  , can_get_message_thread_()
  , can_get_read_date_()
  , can_get_statistics_()
  , can_get_video_advertisements_()
  , can_get_viewers_()
  , can_mark_tasks_as_done_()
  , can_recognize_speech_()
  , can_report_chat_()
  , can_report_reactions_()
  , can_report_supergroup_spam_()
  , can_set_fact_check_()
  , need_show_statistics_()
{}

messageProperties::messageProperties(bool can_add_offer_, bool can_add_tasks_, bool can_be_approved_, bool can_be_copied_, bool can_be_copied_to_secret_chat_, bool can_be_declined_, bool can_be_deleted_only_for_self_, bool can_be_deleted_for_all_users_, bool can_be_edited_, bool can_be_forwarded_, bool can_be_paid_, bool can_be_pinned_, bool can_be_replied_, bool can_be_replied_in_another_chat_, bool can_be_saved_, bool can_be_shared_in_story_, bool can_edit_media_, bool can_edit_scheduling_state_, bool can_edit_suggested_post_info_, bool can_get_author_, bool can_get_embedding_code_, bool can_get_link_, bool can_get_media_timestamp_links_, bool can_get_message_thread_, bool can_get_read_date_, bool can_get_statistics_, bool can_get_video_advertisements_, bool can_get_viewers_, bool can_mark_tasks_as_done_, bool can_recognize_speech_, bool can_report_chat_, bool can_report_reactions_, bool can_report_supergroup_spam_, bool can_set_fact_check_, bool need_show_statistics_)
  : can_add_offer_(can_add_offer_)
  , can_add_tasks_(can_add_tasks_)
  , can_be_approved_(can_be_approved_)
  , can_be_copied_(can_be_copied_)
  , can_be_copied_to_secret_chat_(can_be_copied_to_secret_chat_)
  , can_be_declined_(can_be_declined_)
  , can_be_deleted_only_for_self_(can_be_deleted_only_for_self_)
  , can_be_deleted_for_all_users_(can_be_deleted_for_all_users_)
  , can_be_edited_(can_be_edited_)
  , can_be_forwarded_(can_be_forwarded_)
  , can_be_paid_(can_be_paid_)
  , can_be_pinned_(can_be_pinned_)
  , can_be_replied_(can_be_replied_)
  , can_be_replied_in_another_chat_(can_be_replied_in_another_chat_)
  , can_be_saved_(can_be_saved_)
  , can_be_shared_in_story_(can_be_shared_in_story_)
  , can_edit_media_(can_edit_media_)
  , can_edit_scheduling_state_(can_edit_scheduling_state_)
  , can_edit_suggested_post_info_(can_edit_suggested_post_info_)
  , can_get_author_(can_get_author_)
  , can_get_embedding_code_(can_get_embedding_code_)
  , can_get_link_(can_get_link_)
  , can_get_media_timestamp_links_(can_get_media_timestamp_links_)
  , can_get_message_thread_(can_get_message_thread_)
  , can_get_read_date_(can_get_read_date_)
  , can_get_statistics_(can_get_statistics_)
  , can_get_video_advertisements_(can_get_video_advertisements_)
  , can_get_viewers_(can_get_viewers_)
  , can_mark_tasks_as_done_(can_mark_tasks_as_done_)
  , can_recognize_speech_(can_recognize_speech_)
  , can_report_chat_(can_report_chat_)
  , can_report_reactions_(can_report_reactions_)
  , can_report_supergroup_spam_(can_report_supergroup_spam_)
  , can_set_fact_check_(can_set_fact_check_)
  , need_show_statistics_(need_show_statistics_)
{}

const std::int32_t messageProperties::ID;

void messageProperties::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageProperties");
    s.store_field("can_add_offer", can_add_offer_);
    s.store_field("can_add_tasks", can_add_tasks_);
    s.store_field("can_be_approved", can_be_approved_);
    s.store_field("can_be_copied", can_be_copied_);
    s.store_field("can_be_copied_to_secret_chat", can_be_copied_to_secret_chat_);
    s.store_field("can_be_declined", can_be_declined_);
    s.store_field("can_be_deleted_only_for_self", can_be_deleted_only_for_self_);
    s.store_field("can_be_deleted_for_all_users", can_be_deleted_for_all_users_);
    s.store_field("can_be_edited", can_be_edited_);
    s.store_field("can_be_forwarded", can_be_forwarded_);
    s.store_field("can_be_paid", can_be_paid_);
    s.store_field("can_be_pinned", can_be_pinned_);
    s.store_field("can_be_replied", can_be_replied_);
    s.store_field("can_be_replied_in_another_chat", can_be_replied_in_another_chat_);
    s.store_field("can_be_saved", can_be_saved_);
    s.store_field("can_be_shared_in_story", can_be_shared_in_story_);
    s.store_field("can_edit_media", can_edit_media_);
    s.store_field("can_edit_scheduling_state", can_edit_scheduling_state_);
    s.store_field("can_edit_suggested_post_info", can_edit_suggested_post_info_);
    s.store_field("can_get_author", can_get_author_);
    s.store_field("can_get_embedding_code", can_get_embedding_code_);
    s.store_field("can_get_link", can_get_link_);
    s.store_field("can_get_media_timestamp_links", can_get_media_timestamp_links_);
    s.store_field("can_get_message_thread", can_get_message_thread_);
    s.store_field("can_get_read_date", can_get_read_date_);
    s.store_field("can_get_statistics", can_get_statistics_);
    s.store_field("can_get_video_advertisements", can_get_video_advertisements_);
    s.store_field("can_get_viewers", can_get_viewers_);
    s.store_field("can_mark_tasks_as_done", can_mark_tasks_as_done_);
    s.store_field("can_recognize_speech", can_recognize_speech_);
    s.store_field("can_report_chat", can_report_chat_);
    s.store_field("can_report_reactions", can_report_reactions_);
    s.store_field("can_report_supergroup_spam", can_report_supergroup_spam_);
    s.store_field("can_set_fact_check", can_set_fact_check_);
    s.store_field("need_show_statistics", need_show_statistics_);
    s.store_class_end();
  }
}

messageSendingStatePending::messageSendingStatePending()
  : sending_id_()
{}

messageSendingStatePending::messageSendingStatePending(int32 sending_id_)
  : sending_id_(sending_id_)
{}

const std::int32_t messageSendingStatePending::ID;

void messageSendingStatePending::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSendingStatePending");
    s.store_field("sending_id", sending_id_);
    s.store_class_end();
  }
}

messageSendingStateFailed::messageSendingStateFailed()
  : error_()
  , can_retry_()
  , need_another_sender_()
  , need_another_reply_quote_()
  , need_drop_reply_()
  , required_paid_message_star_count_()
  , retry_after_()
{}

messageSendingStateFailed::messageSendingStateFailed(object_ptr<error> &&error_, bool can_retry_, bool need_another_sender_, bool need_another_reply_quote_, bool need_drop_reply_, int53 required_paid_message_star_count_, double retry_after_)
  : error_(std::move(error_))
  , can_retry_(can_retry_)
  , need_another_sender_(need_another_sender_)
  , need_another_reply_quote_(need_another_reply_quote_)
  , need_drop_reply_(need_drop_reply_)
  , required_paid_message_star_count_(required_paid_message_star_count_)
  , retry_after_(retry_after_)
{}

const std::int32_t messageSendingStateFailed::ID;

void messageSendingStateFailed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSendingStateFailed");
    s.store_object_field("error", static_cast<const BaseObject *>(error_.get()));
    s.store_field("can_retry", can_retry_);
    s.store_field("need_another_sender", need_another_sender_);
    s.store_field("need_another_reply_quote", need_another_reply_quote_);
    s.store_field("need_drop_reply", need_drop_reply_);
    s.store_field("required_paid_message_star_count", required_paid_message_star_count_);
    s.store_field("retry_after", retry_after_);
    s.store_class_end();
  }
}

messageStatistics::messageStatistics()
  : message_interaction_graph_()
  , message_reaction_graph_()
{}

messageStatistics::messageStatistics(object_ptr<StatisticalGraph> &&message_interaction_graph_, object_ptr<StatisticalGraph> &&message_reaction_graph_)
  : message_interaction_graph_(std::move(message_interaction_graph_))
  , message_reaction_graph_(std::move(message_reaction_graph_))
{}

const std::int32_t messageStatistics::ID;

void messageStatistics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageStatistics");
    s.store_object_field("message_interaction_graph", static_cast<const BaseObject *>(message_interaction_graph_.get()));
    s.store_object_field("message_reaction_graph", static_cast<const BaseObject *>(message_reaction_graph_.get()));
    s.store_class_end();
  }
}

notificationGroupTypeMessages::notificationGroupTypeMessages() {
}

const std::int32_t notificationGroupTypeMessages::ID;

void notificationGroupTypeMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notificationGroupTypeMessages");
    s.store_class_end();
  }
}

notificationGroupTypeMentions::notificationGroupTypeMentions() {
}

const std::int32_t notificationGroupTypeMentions::ID;

void notificationGroupTypeMentions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notificationGroupTypeMentions");
    s.store_class_end();
  }
}

notificationGroupTypeSecretChat::notificationGroupTypeSecretChat() {
}

const std::int32_t notificationGroupTypeSecretChat::ID;

void notificationGroupTypeSecretChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notificationGroupTypeSecretChat");
    s.store_class_end();
  }
}

notificationGroupTypeCalls::notificationGroupTypeCalls() {
}

const std::int32_t notificationGroupTypeCalls::ID;

void notificationGroupTypeCalls::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notificationGroupTypeCalls");
    s.store_class_end();
  }
}

notificationSettingsScopePrivateChats::notificationSettingsScopePrivateChats() {
}

const std::int32_t notificationSettingsScopePrivateChats::ID;

void notificationSettingsScopePrivateChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notificationSettingsScopePrivateChats");
    s.store_class_end();
  }
}

notificationSettingsScopeGroupChats::notificationSettingsScopeGroupChats() {
}

const std::int32_t notificationSettingsScopeGroupChats::ID;

void notificationSettingsScopeGroupChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notificationSettingsScopeGroupChats");
    s.store_class_end();
  }
}

notificationSettingsScopeChannelChats::notificationSettingsScopeChannelChats() {
}

const std::int32_t notificationSettingsScopeChannelChats::ID;

void notificationSettingsScopeChannelChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notificationSettingsScopeChannelChats");
    s.store_class_end();
  }
}

ok::ok() {
}

const std::int32_t ok::ID;

void ok::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "ok");
    s.store_class_end();
  }
}

orderInfo::orderInfo()
  : name_()
  , phone_number_()
  , email_address_()
  , shipping_address_()
{}

orderInfo::orderInfo(string const &name_, string const &phone_number_, string const &email_address_, object_ptr<address> &&shipping_address_)
  : name_(name_)
  , phone_number_(phone_number_)
  , email_address_(email_address_)
  , shipping_address_(std::move(shipping_address_))
{}

const std::int32_t orderInfo::ID;

void orderInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "orderInfo");
    s.store_field("name", name_);
    s.store_field("phone_number", phone_number_);
    s.store_field("email_address", email_address_);
    s.store_object_field("shipping_address", static_cast<const BaseObject *>(shipping_address_.get()));
    s.store_class_end();
  }
}

pageBlockTableCell::pageBlockTableCell()
  : text_()
  , is_header_()
  , colspan_()
  , rowspan_()
  , align_()
  , valign_()
{}

pageBlockTableCell::pageBlockTableCell(object_ptr<RichText> &&text_, bool is_header_, int32 colspan_, int32 rowspan_, object_ptr<PageBlockHorizontalAlignment> &&align_, object_ptr<PageBlockVerticalAlignment> &&valign_)
  : text_(std::move(text_))
  , is_header_(is_header_)
  , colspan_(colspan_)
  , rowspan_(rowspan_)
  , align_(std::move(align_))
  , valign_(std::move(valign_))
{}

const std::int32_t pageBlockTableCell::ID;

void pageBlockTableCell::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockTableCell");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("is_header", is_header_);
    s.store_field("colspan", colspan_);
    s.store_field("rowspan", rowspan_);
    s.store_object_field("align", static_cast<const BaseObject *>(align_.get()));
    s.store_object_field("valign", static_cast<const BaseObject *>(valign_.get()));
    s.store_class_end();
  }
}

passportElementError::passportElementError()
  : type_()
  , message_()
  , source_()
{}

passportElementError::passportElementError(object_ptr<PassportElementType> &&type_, string const &message_, object_ptr<PassportElementErrorSource> &&source_)
  : type_(std::move(type_))
  , message_(message_)
  , source_(std::move(source_))
{}

const std::int32_t passportElementError::ID;

void passportElementError::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementError");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("message", message_);
    s.store_object_field("source", static_cast<const BaseObject *>(source_.get()));
    s.store_class_end();
  }
}

passwordState::passwordState()
  : has_password_()
  , password_hint_()
  , has_recovery_email_address_()
  , has_passport_data_()
  , recovery_email_address_code_info_()
  , login_email_address_pattern_()
  , pending_reset_date_()
{}

passwordState::passwordState(bool has_password_, string const &password_hint_, bool has_recovery_email_address_, bool has_passport_data_, object_ptr<emailAddressAuthenticationCodeInfo> &&recovery_email_address_code_info_, string const &login_email_address_pattern_, int32 pending_reset_date_)
  : has_password_(has_password_)
  , password_hint_(password_hint_)
  , has_recovery_email_address_(has_recovery_email_address_)
  , has_passport_data_(has_passport_data_)
  , recovery_email_address_code_info_(std::move(recovery_email_address_code_info_))
  , login_email_address_pattern_(login_email_address_pattern_)
  , pending_reset_date_(pending_reset_date_)
{}

const std::int32_t passwordState::ID;

void passwordState::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passwordState");
    s.store_field("has_password", has_password_);
    s.store_field("password_hint", password_hint_);
    s.store_field("has_recovery_email_address", has_recovery_email_address_);
    s.store_field("has_passport_data", has_passport_data_);
    s.store_object_field("recovery_email_address_code_info", static_cast<const BaseObject *>(recovery_email_address_code_info_.get()));
    s.store_field("login_email_address_pattern", login_email_address_pattern_);
    s.store_field("pending_reset_date", pending_reset_date_);
    s.store_class_end();
  }
}

paymentReceiptTypeRegular::paymentReceiptTypeRegular()
  : payment_provider_user_id_()
  , invoice_()
  , order_info_()
  , shipping_option_()
  , credentials_title_()
  , tip_amount_()
{}

paymentReceiptTypeRegular::paymentReceiptTypeRegular(int53 payment_provider_user_id_, object_ptr<invoice> &&invoice_, object_ptr<orderInfo> &&order_info_, object_ptr<shippingOption> &&shipping_option_, string const &credentials_title_, int53 tip_amount_)
  : payment_provider_user_id_(payment_provider_user_id_)
  , invoice_(std::move(invoice_))
  , order_info_(std::move(order_info_))
  , shipping_option_(std::move(shipping_option_))
  , credentials_title_(credentials_title_)
  , tip_amount_(tip_amount_)
{}

const std::int32_t paymentReceiptTypeRegular::ID;

void paymentReceiptTypeRegular::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paymentReceiptTypeRegular");
    s.store_field("payment_provider_user_id", payment_provider_user_id_);
    s.store_object_field("invoice", static_cast<const BaseObject *>(invoice_.get()));
    s.store_object_field("order_info", static_cast<const BaseObject *>(order_info_.get()));
    s.store_object_field("shipping_option", static_cast<const BaseObject *>(shipping_option_.get()));
    s.store_field("credentials_title", credentials_title_);
    s.store_field("tip_amount", tip_amount_);
    s.store_class_end();
  }
}

paymentReceiptTypeStars::paymentReceiptTypeStars()
  : star_count_()
  , transaction_id_()
{}

paymentReceiptTypeStars::paymentReceiptTypeStars(int53 star_count_, string const &transaction_id_)
  : star_count_(star_count_)
  , transaction_id_(transaction_id_)
{}

const std::int32_t paymentReceiptTypeStars::ID;

void paymentReceiptTypeStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paymentReceiptTypeStars");
    s.store_field("star_count", star_count_);
    s.store_field("transaction_id", transaction_id_);
    s.store_class_end();
  }
}

point::point()
  : x_()
  , y_()
{}

point::point(double x_, double y_)
  : x_(x_)
  , y_(y_)
{}

const std::int32_t point::ID;

void point::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "point");
    s.store_field("x", x_);
    s.store_field("y", y_);
    s.store_class_end();
  }
}

pollOption::pollOption()
  : text_()
  , voter_count_()
  , vote_percentage_()
  , is_chosen_()
  , is_being_chosen_()
{}

pollOption::pollOption(object_ptr<formattedText> &&text_, int32 voter_count_, int32 vote_percentage_, bool is_chosen_, bool is_being_chosen_)
  : text_(std::move(text_))
  , voter_count_(voter_count_)
  , vote_percentage_(vote_percentage_)
  , is_chosen_(is_chosen_)
  , is_being_chosen_(is_being_chosen_)
{}

const std::int32_t pollOption::ID;

void pollOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pollOption");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("voter_count", voter_count_);
    s.store_field("vote_percentage", vote_percentage_);
    s.store_field("is_chosen", is_chosen_);
    s.store_field("is_being_chosen", is_being_chosen_);
    s.store_class_end();
  }
}

premiumGiftCodeInfo::premiumGiftCodeInfo()
  : creator_id_()
  , creation_date_()
  , is_from_giveaway_()
  , giveaway_message_id_()
  , month_count_()
  , user_id_()
  , use_date_()
{}

premiumGiftCodeInfo::premiumGiftCodeInfo(object_ptr<MessageSender> &&creator_id_, int32 creation_date_, bool is_from_giveaway_, int53 giveaway_message_id_, int32 month_count_, int53 user_id_, int32 use_date_)
  : creator_id_(std::move(creator_id_))
  , creation_date_(creation_date_)
  , is_from_giveaway_(is_from_giveaway_)
  , giveaway_message_id_(giveaway_message_id_)
  , month_count_(month_count_)
  , user_id_(user_id_)
  , use_date_(use_date_)
{}

const std::int32_t premiumGiftCodeInfo::ID;

void premiumGiftCodeInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumGiftCodeInfo");
    s.store_object_field("creator_id", static_cast<const BaseObject *>(creator_id_.get()));
    s.store_field("creation_date", creation_date_);
    s.store_field("is_from_giveaway", is_from_giveaway_);
    s.store_field("giveaway_message_id", giveaway_message_id_);
    s.store_field("month_count", month_count_);
    s.store_field("user_id", user_id_);
    s.store_field("use_date", use_date_);
    s.store_class_end();
  }
}

premiumGiftPaymentOption::premiumGiftPaymentOption()
  : currency_()
  , amount_()
  , star_count_()
  , discount_percentage_()
  , month_count_()
  , store_product_id_()
  , sticker_()
{}

premiumGiftPaymentOption::premiumGiftPaymentOption(string const &currency_, int53 amount_, int53 star_count_, int32 discount_percentage_, int32 month_count_, string const &store_product_id_, object_ptr<sticker> &&sticker_)
  : currency_(currency_)
  , amount_(amount_)
  , star_count_(star_count_)
  , discount_percentage_(discount_percentage_)
  , month_count_(month_count_)
  , store_product_id_(store_product_id_)
  , sticker_(std::move(sticker_))
{}

const std::int32_t premiumGiftPaymentOption::ID;

void premiumGiftPaymentOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumGiftPaymentOption");
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("star_count", star_count_);
    s.store_field("discount_percentage", discount_percentage_);
    s.store_field("month_count", month_count_);
    s.store_field("store_product_id", store_product_id_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

premiumLimit::premiumLimit()
  : type_()
  , default_value_()
  , premium_value_()
{}

premiumLimit::premiumLimit(object_ptr<PremiumLimitType> &&type_, int32 default_value_, int32 premium_value_)
  : type_(std::move(type_))
  , default_value_(default_value_)
  , premium_value_(premium_value_)
{}

const std::int32_t premiumLimit::ID;

void premiumLimit::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumLimit");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("default_value", default_value_);
    s.store_field("premium_value", premium_value_);
    s.store_class_end();
  }
}

premiumSourceLimitExceeded::premiumSourceLimitExceeded()
  : limit_type_()
{}

premiumSourceLimitExceeded::premiumSourceLimitExceeded(object_ptr<PremiumLimitType> &&limit_type_)
  : limit_type_(std::move(limit_type_))
{}

const std::int32_t premiumSourceLimitExceeded::ID;

void premiumSourceLimitExceeded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumSourceLimitExceeded");
    s.store_object_field("limit_type", static_cast<const BaseObject *>(limit_type_.get()));
    s.store_class_end();
  }
}

premiumSourceFeature::premiumSourceFeature()
  : feature_()
{}

premiumSourceFeature::premiumSourceFeature(object_ptr<PremiumFeature> &&feature_)
  : feature_(std::move(feature_))
{}

const std::int32_t premiumSourceFeature::ID;

void premiumSourceFeature::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumSourceFeature");
    s.store_object_field("feature", static_cast<const BaseObject *>(feature_.get()));
    s.store_class_end();
  }
}

premiumSourceBusinessFeature::premiumSourceBusinessFeature()
  : feature_()
{}

premiumSourceBusinessFeature::premiumSourceBusinessFeature(object_ptr<BusinessFeature> &&feature_)
  : feature_(std::move(feature_))
{}

const std::int32_t premiumSourceBusinessFeature::ID;

void premiumSourceBusinessFeature::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumSourceBusinessFeature");
    s.store_object_field("feature", static_cast<const BaseObject *>(feature_.get()));
    s.store_class_end();
  }
}

premiumSourceStoryFeature::premiumSourceStoryFeature()
  : feature_()
{}

premiumSourceStoryFeature::premiumSourceStoryFeature(object_ptr<PremiumStoryFeature> &&feature_)
  : feature_(std::move(feature_))
{}

const std::int32_t premiumSourceStoryFeature::ID;

void premiumSourceStoryFeature::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumSourceStoryFeature");
    s.store_object_field("feature", static_cast<const BaseObject *>(feature_.get()));
    s.store_class_end();
  }
}

premiumSourceLink::premiumSourceLink()
  : referrer_()
{}

premiumSourceLink::premiumSourceLink(string const &referrer_)
  : referrer_(referrer_)
{}

const std::int32_t premiumSourceLink::ID;

void premiumSourceLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumSourceLink");
    s.store_field("referrer", referrer_);
    s.store_class_end();
  }
}

premiumSourceSettings::premiumSourceSettings() {
}

const std::int32_t premiumSourceSettings::ID;

void premiumSourceSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumSourceSettings");
    s.store_class_end();
  }
}

proxies::proxies()
  : proxies_()
{}

proxies::proxies(array<object_ptr<proxy>> &&proxies_)
  : proxies_(std::move(proxies_))
{}

const std::int32_t proxies::ID;

void proxies::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "proxies");
    { s.store_vector_begin("proxies", proxies_.size()); for (const auto &_value : proxies_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

quickReplyMessage::quickReplyMessage()
  : id_()
  , sending_state_()
  , can_be_edited_()
  , reply_to_message_id_()
  , via_bot_user_id_()
  , media_album_id_()
  , content_()
  , reply_markup_()
{}

quickReplyMessage::quickReplyMessage(int53 id_, object_ptr<MessageSendingState> &&sending_state_, bool can_be_edited_, int53 reply_to_message_id_, int53 via_bot_user_id_, int64 media_album_id_, object_ptr<MessageContent> &&content_, object_ptr<ReplyMarkup> &&reply_markup_)
  : id_(id_)
  , sending_state_(std::move(sending_state_))
  , can_be_edited_(can_be_edited_)
  , reply_to_message_id_(reply_to_message_id_)
  , via_bot_user_id_(via_bot_user_id_)
  , media_album_id_(media_album_id_)
  , content_(std::move(content_))
  , reply_markup_(std::move(reply_markup_))
{}

const std::int32_t quickReplyMessage::ID;

void quickReplyMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "quickReplyMessage");
    s.store_field("id", id_);
    s.store_object_field("sending_state", static_cast<const BaseObject *>(sending_state_.get()));
    s.store_field("can_be_edited", can_be_edited_);
    s.store_field("reply_to_message_id", reply_to_message_id_);
    s.store_field("via_bot_user_id", via_bot_user_id_);
    s.store_field("media_album_id", media_album_id_);
    s.store_object_field("content", static_cast<const BaseObject *>(content_.get()));
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_class_end();
  }
}

recommendedChatFolders::recommendedChatFolders()
  : chat_folders_()
{}

recommendedChatFolders::recommendedChatFolders(array<object_ptr<recommendedChatFolder>> &&chat_folders_)
  : chat_folders_(std::move(chat_folders_))
{}

const std::int32_t recommendedChatFolders::ID;

void recommendedChatFolders::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "recommendedChatFolders");
    { s.store_vector_begin("chat_folders", chat_folders_.size()); for (const auto &_value : chat_folders_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

replyMarkupRemoveKeyboard::replyMarkupRemoveKeyboard()
  : is_personal_()
{}

replyMarkupRemoveKeyboard::replyMarkupRemoveKeyboard(bool is_personal_)
  : is_personal_(is_personal_)
{}

const std::int32_t replyMarkupRemoveKeyboard::ID;

void replyMarkupRemoveKeyboard::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "replyMarkupRemoveKeyboard");
    s.store_field("is_personal", is_personal_);
    s.store_class_end();
  }
}

replyMarkupForceReply::replyMarkupForceReply()
  : is_personal_()
  , input_field_placeholder_()
{}

replyMarkupForceReply::replyMarkupForceReply(bool is_personal_, string const &input_field_placeholder_)
  : is_personal_(is_personal_)
  , input_field_placeholder_(input_field_placeholder_)
{}

const std::int32_t replyMarkupForceReply::ID;

void replyMarkupForceReply::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "replyMarkupForceReply");
    s.store_field("is_personal", is_personal_);
    s.store_field("input_field_placeholder", input_field_placeholder_);
    s.store_class_end();
  }
}

replyMarkupShowKeyboard::replyMarkupShowKeyboard()
  : rows_()
  , is_persistent_()
  , resize_keyboard_()
  , one_time_()
  , is_personal_()
  , input_field_placeholder_()
{}

replyMarkupShowKeyboard::replyMarkupShowKeyboard(array<array<object_ptr<keyboardButton>>> &&rows_, bool is_persistent_, bool resize_keyboard_, bool one_time_, bool is_personal_, string const &input_field_placeholder_)
  : rows_(std::move(rows_))
  , is_persistent_(is_persistent_)
  , resize_keyboard_(resize_keyboard_)
  , one_time_(one_time_)
  , is_personal_(is_personal_)
  , input_field_placeholder_(input_field_placeholder_)
{}

const std::int32_t replyMarkupShowKeyboard::ID;

void replyMarkupShowKeyboard::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "replyMarkupShowKeyboard");
    { s.store_vector_begin("rows", rows_.size()); for (const auto &_value : rows_) { { s.store_vector_begin("", _value.size()); for (const auto &_value2 : _value) { s.store_object_field("", static_cast<const BaseObject *>(_value2.get())); } s.store_class_end(); } } s.store_class_end(); }
    s.store_field("is_persistent", is_persistent_);
    s.store_field("resize_keyboard", resize_keyboard_);
    s.store_field("one_time", one_time_);
    s.store_field("is_personal", is_personal_);
    s.store_field("input_field_placeholder", input_field_placeholder_);
    s.store_class_end();
  }
}

replyMarkupInlineKeyboard::replyMarkupInlineKeyboard()
  : rows_()
{}

replyMarkupInlineKeyboard::replyMarkupInlineKeyboard(array<array<object_ptr<inlineKeyboardButton>>> &&rows_)
  : rows_(std::move(rows_))
{}

const std::int32_t replyMarkupInlineKeyboard::ID;

void replyMarkupInlineKeyboard::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "replyMarkupInlineKeyboard");
    { s.store_vector_begin("rows", rows_.size()); for (const auto &_value : rows_) { { s.store_vector_begin("", _value.size()); for (const auto &_value2 : _value) { s.store_object_field("", static_cast<const BaseObject *>(_value2.get())); } s.store_class_end(); } } s.store_class_end(); }
    s.store_class_end();
  }
}

rtmpUrl::rtmpUrl()
  : url_()
  , stream_key_()
{}

rtmpUrl::rtmpUrl(string const &url_, string const &stream_key_)
  : url_(url_)
  , stream_key_(stream_key_)
{}

const std::int32_t rtmpUrl::ID;

void rtmpUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "rtmpUrl");
    s.store_field("url", url_);
    s.store_field("stream_key", stream_key_);
    s.store_class_end();
  }
}

sentWebAppMessage::sentWebAppMessage()
  : inline_message_id_()
{}

sentWebAppMessage::sentWebAppMessage(string const &inline_message_id_)
  : inline_message_id_(inline_message_id_)
{}

const std::int32_t sentWebAppMessage::ID;

void sentWebAppMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sentWebAppMessage");
    s.store_field("inline_message_id", inline_message_id_);
    s.store_class_end();
  }
}

sharedUser::sharedUser()
  : user_id_()
  , first_name_()
  , last_name_()
  , username_()
  , photo_()
{}

sharedUser::sharedUser(int53 user_id_, string const &first_name_, string const &last_name_, string const &username_, object_ptr<photo> &&photo_)
  : user_id_(user_id_)
  , first_name_(first_name_)
  , last_name_(last_name_)
  , username_(username_)
  , photo_(std::move(photo_))
{}

const std::int32_t sharedUser::ID;

void sharedUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sharedUser");
    s.store_field("user_id", user_id_);
    s.store_field("first_name", first_name_);
    s.store_field("last_name", last_name_);
    s.store_field("username", username_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

starRevenueStatus::starRevenueStatus()
  : total_amount_()
  , current_amount_()
  , available_amount_()
  , withdrawal_enabled_()
  , next_withdrawal_in_()
{}

starRevenueStatus::starRevenueStatus(object_ptr<starAmount> &&total_amount_, object_ptr<starAmount> &&current_amount_, object_ptr<starAmount> &&available_amount_, bool withdrawal_enabled_, int32 next_withdrawal_in_)
  : total_amount_(std::move(total_amount_))
  , current_amount_(std::move(current_amount_))
  , available_amount_(std::move(available_amount_))
  , withdrawal_enabled_(withdrawal_enabled_)
  , next_withdrawal_in_(next_withdrawal_in_)
{}

const std::int32_t starRevenueStatus::ID;

void starRevenueStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starRevenueStatus");
    s.store_object_field("total_amount", static_cast<const BaseObject *>(total_amount_.get()));
    s.store_object_field("current_amount", static_cast<const BaseObject *>(current_amount_.get()));
    s.store_object_field("available_amount", static_cast<const BaseObject *>(available_amount_.get()));
    s.store_field("withdrawal_enabled", withdrawal_enabled_);
    s.store_field("next_withdrawal_in", next_withdrawal_in_);
    s.store_class_end();
  }
}

starTransactionTypePremiumBotDeposit::starTransactionTypePremiumBotDeposit() {
}

const std::int32_t starTransactionTypePremiumBotDeposit::ID;

void starTransactionTypePremiumBotDeposit::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypePremiumBotDeposit");
    s.store_class_end();
  }
}

starTransactionTypeAppStoreDeposit::starTransactionTypeAppStoreDeposit() {
}

const std::int32_t starTransactionTypeAppStoreDeposit::ID;

void starTransactionTypeAppStoreDeposit::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeAppStoreDeposit");
    s.store_class_end();
  }
}

starTransactionTypeGooglePlayDeposit::starTransactionTypeGooglePlayDeposit() {
}

const std::int32_t starTransactionTypeGooglePlayDeposit::ID;

void starTransactionTypeGooglePlayDeposit::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeGooglePlayDeposit");
    s.store_class_end();
  }
}

starTransactionTypeFragmentDeposit::starTransactionTypeFragmentDeposit() {
}

const std::int32_t starTransactionTypeFragmentDeposit::ID;

void starTransactionTypeFragmentDeposit::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeFragmentDeposit");
    s.store_class_end();
  }
}

starTransactionTypeUserDeposit::starTransactionTypeUserDeposit()
  : user_id_()
  , sticker_()
{}

starTransactionTypeUserDeposit::starTransactionTypeUserDeposit(int53 user_id_, object_ptr<sticker> &&sticker_)
  : user_id_(user_id_)
  , sticker_(std::move(sticker_))
{}

const std::int32_t starTransactionTypeUserDeposit::ID;

void starTransactionTypeUserDeposit::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeUserDeposit");
    s.store_field("user_id", user_id_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

starTransactionTypeGiveawayDeposit::starTransactionTypeGiveawayDeposit()
  : chat_id_()
  , giveaway_message_id_()
{}

starTransactionTypeGiveawayDeposit::starTransactionTypeGiveawayDeposit(int53 chat_id_, int53 giveaway_message_id_)
  : chat_id_(chat_id_)
  , giveaway_message_id_(giveaway_message_id_)
{}

const std::int32_t starTransactionTypeGiveawayDeposit::ID;

void starTransactionTypeGiveawayDeposit::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeGiveawayDeposit");
    s.store_field("chat_id", chat_id_);
    s.store_field("giveaway_message_id", giveaway_message_id_);
    s.store_class_end();
  }
}

starTransactionTypeFragmentWithdrawal::starTransactionTypeFragmentWithdrawal()
  : withdrawal_state_()
{}

starTransactionTypeFragmentWithdrawal::starTransactionTypeFragmentWithdrawal(object_ptr<RevenueWithdrawalState> &&withdrawal_state_)
  : withdrawal_state_(std::move(withdrawal_state_))
{}

const std::int32_t starTransactionTypeFragmentWithdrawal::ID;

void starTransactionTypeFragmentWithdrawal::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeFragmentWithdrawal");
    s.store_object_field("withdrawal_state", static_cast<const BaseObject *>(withdrawal_state_.get()));
    s.store_class_end();
  }
}

starTransactionTypeTelegramAdsWithdrawal::starTransactionTypeTelegramAdsWithdrawal() {
}

const std::int32_t starTransactionTypeTelegramAdsWithdrawal::ID;

void starTransactionTypeTelegramAdsWithdrawal::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeTelegramAdsWithdrawal");
    s.store_class_end();
  }
}

starTransactionTypeTelegramApiUsage::starTransactionTypeTelegramApiUsage()
  : request_count_()
{}

starTransactionTypeTelegramApiUsage::starTransactionTypeTelegramApiUsage(int32 request_count_)
  : request_count_(request_count_)
{}

const std::int32_t starTransactionTypeTelegramApiUsage::ID;

void starTransactionTypeTelegramApiUsage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeTelegramApiUsage");
    s.store_field("request_count", request_count_);
    s.store_class_end();
  }
}

starTransactionTypeBotPaidMediaPurchase::starTransactionTypeBotPaidMediaPurchase()
  : user_id_()
  , media_()
{}

starTransactionTypeBotPaidMediaPurchase::starTransactionTypeBotPaidMediaPurchase(int53 user_id_, array<object_ptr<PaidMedia>> &&media_)
  : user_id_(user_id_)
  , media_(std::move(media_))
{}

const std::int32_t starTransactionTypeBotPaidMediaPurchase::ID;

void starTransactionTypeBotPaidMediaPurchase::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeBotPaidMediaPurchase");
    s.store_field("user_id", user_id_);
    { s.store_vector_begin("media", media_.size()); for (const auto &_value : media_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

starTransactionTypeBotPaidMediaSale::starTransactionTypeBotPaidMediaSale()
  : user_id_()
  , media_()
  , payload_()
  , affiliate_()
{}

starTransactionTypeBotPaidMediaSale::starTransactionTypeBotPaidMediaSale(int53 user_id_, array<object_ptr<PaidMedia>> &&media_, string const &payload_, object_ptr<affiliateInfo> &&affiliate_)
  : user_id_(user_id_)
  , media_(std::move(media_))
  , payload_(payload_)
  , affiliate_(std::move(affiliate_))
{}

const std::int32_t starTransactionTypeBotPaidMediaSale::ID;

void starTransactionTypeBotPaidMediaSale::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeBotPaidMediaSale");
    s.store_field("user_id", user_id_);
    { s.store_vector_begin("media", media_.size()); for (const auto &_value : media_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("payload", payload_);
    s.store_object_field("affiliate", static_cast<const BaseObject *>(affiliate_.get()));
    s.store_class_end();
  }
}

starTransactionTypeChannelPaidMediaPurchase::starTransactionTypeChannelPaidMediaPurchase()
  : chat_id_()
  , message_id_()
  , media_()
{}

starTransactionTypeChannelPaidMediaPurchase::starTransactionTypeChannelPaidMediaPurchase(int53 chat_id_, int53 message_id_, array<object_ptr<PaidMedia>> &&media_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , media_(std::move(media_))
{}

const std::int32_t starTransactionTypeChannelPaidMediaPurchase::ID;

void starTransactionTypeChannelPaidMediaPurchase::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeChannelPaidMediaPurchase");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    { s.store_vector_begin("media", media_.size()); for (const auto &_value : media_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

starTransactionTypeChannelPaidMediaSale::starTransactionTypeChannelPaidMediaSale()
  : user_id_()
  , message_id_()
  , media_()
{}

starTransactionTypeChannelPaidMediaSale::starTransactionTypeChannelPaidMediaSale(int53 user_id_, int53 message_id_, array<object_ptr<PaidMedia>> &&media_)
  : user_id_(user_id_)
  , message_id_(message_id_)
  , media_(std::move(media_))
{}

const std::int32_t starTransactionTypeChannelPaidMediaSale::ID;

void starTransactionTypeChannelPaidMediaSale::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeChannelPaidMediaSale");
    s.store_field("user_id", user_id_);
    s.store_field("message_id", message_id_);
    { s.store_vector_begin("media", media_.size()); for (const auto &_value : media_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

starTransactionTypeBotInvoicePurchase::starTransactionTypeBotInvoicePurchase()
  : user_id_()
  , product_info_()
{}

starTransactionTypeBotInvoicePurchase::starTransactionTypeBotInvoicePurchase(int53 user_id_, object_ptr<productInfo> &&product_info_)
  : user_id_(user_id_)
  , product_info_(std::move(product_info_))
{}

const std::int32_t starTransactionTypeBotInvoicePurchase::ID;

void starTransactionTypeBotInvoicePurchase::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeBotInvoicePurchase");
    s.store_field("user_id", user_id_);
    s.store_object_field("product_info", static_cast<const BaseObject *>(product_info_.get()));
    s.store_class_end();
  }
}

starTransactionTypeBotInvoiceSale::starTransactionTypeBotInvoiceSale()
  : user_id_()
  , product_info_()
  , invoice_payload_()
  , affiliate_()
{}

starTransactionTypeBotInvoiceSale::starTransactionTypeBotInvoiceSale(int53 user_id_, object_ptr<productInfo> &&product_info_, bytes const &invoice_payload_, object_ptr<affiliateInfo> &&affiliate_)
  : user_id_(user_id_)
  , product_info_(std::move(product_info_))
  , invoice_payload_(std::move(invoice_payload_))
  , affiliate_(std::move(affiliate_))
{}

const std::int32_t starTransactionTypeBotInvoiceSale::ID;

void starTransactionTypeBotInvoiceSale::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeBotInvoiceSale");
    s.store_field("user_id", user_id_);
    s.store_object_field("product_info", static_cast<const BaseObject *>(product_info_.get()));
    s.store_bytes_field("invoice_payload", invoice_payload_);
    s.store_object_field("affiliate", static_cast<const BaseObject *>(affiliate_.get()));
    s.store_class_end();
  }
}

starTransactionTypeBotSubscriptionPurchase::starTransactionTypeBotSubscriptionPurchase()
  : user_id_()
  , subscription_period_()
  , product_info_()
{}

starTransactionTypeBotSubscriptionPurchase::starTransactionTypeBotSubscriptionPurchase(int53 user_id_, int32 subscription_period_, object_ptr<productInfo> &&product_info_)
  : user_id_(user_id_)
  , subscription_period_(subscription_period_)
  , product_info_(std::move(product_info_))
{}

const std::int32_t starTransactionTypeBotSubscriptionPurchase::ID;

void starTransactionTypeBotSubscriptionPurchase::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeBotSubscriptionPurchase");
    s.store_field("user_id", user_id_);
    s.store_field("subscription_period", subscription_period_);
    s.store_object_field("product_info", static_cast<const BaseObject *>(product_info_.get()));
    s.store_class_end();
  }
}

starTransactionTypeBotSubscriptionSale::starTransactionTypeBotSubscriptionSale()
  : user_id_()
  , subscription_period_()
  , product_info_()
  , invoice_payload_()
  , affiliate_()
{}

starTransactionTypeBotSubscriptionSale::starTransactionTypeBotSubscriptionSale(int53 user_id_, int32 subscription_period_, object_ptr<productInfo> &&product_info_, bytes const &invoice_payload_, object_ptr<affiliateInfo> &&affiliate_)
  : user_id_(user_id_)
  , subscription_period_(subscription_period_)
  , product_info_(std::move(product_info_))
  , invoice_payload_(std::move(invoice_payload_))
  , affiliate_(std::move(affiliate_))
{}

const std::int32_t starTransactionTypeBotSubscriptionSale::ID;

void starTransactionTypeBotSubscriptionSale::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeBotSubscriptionSale");
    s.store_field("user_id", user_id_);
    s.store_field("subscription_period", subscription_period_);
    s.store_object_field("product_info", static_cast<const BaseObject *>(product_info_.get()));
    s.store_bytes_field("invoice_payload", invoice_payload_);
    s.store_object_field("affiliate", static_cast<const BaseObject *>(affiliate_.get()));
    s.store_class_end();
  }
}

starTransactionTypeChannelSubscriptionPurchase::starTransactionTypeChannelSubscriptionPurchase()
  : chat_id_()
  , subscription_period_()
{}

starTransactionTypeChannelSubscriptionPurchase::starTransactionTypeChannelSubscriptionPurchase(int53 chat_id_, int32 subscription_period_)
  : chat_id_(chat_id_)
  , subscription_period_(subscription_period_)
{}

const std::int32_t starTransactionTypeChannelSubscriptionPurchase::ID;

void starTransactionTypeChannelSubscriptionPurchase::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeChannelSubscriptionPurchase");
    s.store_field("chat_id", chat_id_);
    s.store_field("subscription_period", subscription_period_);
    s.store_class_end();
  }
}

starTransactionTypeChannelSubscriptionSale::starTransactionTypeChannelSubscriptionSale()
  : user_id_()
  , subscription_period_()
{}

starTransactionTypeChannelSubscriptionSale::starTransactionTypeChannelSubscriptionSale(int53 user_id_, int32 subscription_period_)
  : user_id_(user_id_)
  , subscription_period_(subscription_period_)
{}

const std::int32_t starTransactionTypeChannelSubscriptionSale::ID;

void starTransactionTypeChannelSubscriptionSale::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeChannelSubscriptionSale");
    s.store_field("user_id", user_id_);
    s.store_field("subscription_period", subscription_period_);
    s.store_class_end();
  }
}

starTransactionTypeGiftPurchase::starTransactionTypeGiftPurchase()
  : owner_id_()
  , gift_()
{}

starTransactionTypeGiftPurchase::starTransactionTypeGiftPurchase(object_ptr<MessageSender> &&owner_id_, object_ptr<gift> &&gift_)
  : owner_id_(std::move(owner_id_))
  , gift_(std::move(gift_))
{}

const std::int32_t starTransactionTypeGiftPurchase::ID;

void starTransactionTypeGiftPurchase::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeGiftPurchase");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_class_end();
  }
}

starTransactionTypeGiftTransfer::starTransactionTypeGiftTransfer()
  : owner_id_()
  , gift_()
{}

starTransactionTypeGiftTransfer::starTransactionTypeGiftTransfer(object_ptr<MessageSender> &&owner_id_, object_ptr<upgradedGift> &&gift_)
  : owner_id_(std::move(owner_id_))
  , gift_(std::move(gift_))
{}

const std::int32_t starTransactionTypeGiftTransfer::ID;

void starTransactionTypeGiftTransfer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeGiftTransfer");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_class_end();
  }
}

starTransactionTypeGiftOriginalDetailsDrop::starTransactionTypeGiftOriginalDetailsDrop()
  : owner_id_()
  , gift_()
{}

starTransactionTypeGiftOriginalDetailsDrop::starTransactionTypeGiftOriginalDetailsDrop(object_ptr<MessageSender> &&owner_id_, object_ptr<upgradedGift> &&gift_)
  : owner_id_(std::move(owner_id_))
  , gift_(std::move(gift_))
{}

const std::int32_t starTransactionTypeGiftOriginalDetailsDrop::ID;

void starTransactionTypeGiftOriginalDetailsDrop::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeGiftOriginalDetailsDrop");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_class_end();
  }
}

starTransactionTypeGiftSale::starTransactionTypeGiftSale()
  : user_id_()
  , gift_()
{}

starTransactionTypeGiftSale::starTransactionTypeGiftSale(int53 user_id_, object_ptr<gift> &&gift_)
  : user_id_(user_id_)
  , gift_(std::move(gift_))
{}

const std::int32_t starTransactionTypeGiftSale::ID;

void starTransactionTypeGiftSale::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeGiftSale");
    s.store_field("user_id", user_id_);
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_class_end();
  }
}

starTransactionTypeGiftUpgrade::starTransactionTypeGiftUpgrade()
  : user_id_()
  , gift_()
{}

starTransactionTypeGiftUpgrade::starTransactionTypeGiftUpgrade(int53 user_id_, object_ptr<upgradedGift> &&gift_)
  : user_id_(user_id_)
  , gift_(std::move(gift_))
{}

const std::int32_t starTransactionTypeGiftUpgrade::ID;

void starTransactionTypeGiftUpgrade::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeGiftUpgrade");
    s.store_field("user_id", user_id_);
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_class_end();
  }
}

starTransactionTypeGiftUpgradePurchase::starTransactionTypeGiftUpgradePurchase()
  : owner_id_()
  , gift_()
{}

starTransactionTypeGiftUpgradePurchase::starTransactionTypeGiftUpgradePurchase(object_ptr<MessageSender> &&owner_id_, object_ptr<gift> &&gift_)
  : owner_id_(std::move(owner_id_))
  , gift_(std::move(gift_))
{}

const std::int32_t starTransactionTypeGiftUpgradePurchase::ID;

void starTransactionTypeGiftUpgradePurchase::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeGiftUpgradePurchase");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_class_end();
  }
}

starTransactionTypeUpgradedGiftPurchase::starTransactionTypeUpgradedGiftPurchase()
  : user_id_()
  , gift_()
{}

starTransactionTypeUpgradedGiftPurchase::starTransactionTypeUpgradedGiftPurchase(int53 user_id_, object_ptr<upgradedGift> &&gift_)
  : user_id_(user_id_)
  , gift_(std::move(gift_))
{}

const std::int32_t starTransactionTypeUpgradedGiftPurchase::ID;

void starTransactionTypeUpgradedGiftPurchase::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeUpgradedGiftPurchase");
    s.store_field("user_id", user_id_);
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_class_end();
  }
}

starTransactionTypeUpgradedGiftSale::starTransactionTypeUpgradedGiftSale()
  : user_id_()
  , gift_()
  , commission_per_mille_()
  , commission_star_amount_()
{}

starTransactionTypeUpgradedGiftSale::starTransactionTypeUpgradedGiftSale(int53 user_id_, object_ptr<upgradedGift> &&gift_, int32 commission_per_mille_, object_ptr<starAmount> &&commission_star_amount_)
  : user_id_(user_id_)
  , gift_(std::move(gift_))
  , commission_per_mille_(commission_per_mille_)
  , commission_star_amount_(std::move(commission_star_amount_))
{}

const std::int32_t starTransactionTypeUpgradedGiftSale::ID;

void starTransactionTypeUpgradedGiftSale::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeUpgradedGiftSale");
    s.store_field("user_id", user_id_);
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_field("commission_per_mille", commission_per_mille_);
    s.store_object_field("commission_star_amount", static_cast<const BaseObject *>(commission_star_amount_.get()));
    s.store_class_end();
  }
}

starTransactionTypeChannelPaidReactionSend::starTransactionTypeChannelPaidReactionSend()
  : chat_id_()
  , message_id_()
{}

starTransactionTypeChannelPaidReactionSend::starTransactionTypeChannelPaidReactionSend(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t starTransactionTypeChannelPaidReactionSend::ID;

void starTransactionTypeChannelPaidReactionSend::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeChannelPaidReactionSend");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

starTransactionTypeChannelPaidReactionReceive::starTransactionTypeChannelPaidReactionReceive()
  : user_id_()
  , message_id_()
{}

starTransactionTypeChannelPaidReactionReceive::starTransactionTypeChannelPaidReactionReceive(int53 user_id_, int53 message_id_)
  : user_id_(user_id_)
  , message_id_(message_id_)
{}

const std::int32_t starTransactionTypeChannelPaidReactionReceive::ID;

void starTransactionTypeChannelPaidReactionReceive::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeChannelPaidReactionReceive");
    s.store_field("user_id", user_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

starTransactionTypeAffiliateProgramCommission::starTransactionTypeAffiliateProgramCommission()
  : chat_id_()
  , commission_per_mille_()
{}

starTransactionTypeAffiliateProgramCommission::starTransactionTypeAffiliateProgramCommission(int53 chat_id_, int32 commission_per_mille_)
  : chat_id_(chat_id_)
  , commission_per_mille_(commission_per_mille_)
{}

const std::int32_t starTransactionTypeAffiliateProgramCommission::ID;

void starTransactionTypeAffiliateProgramCommission::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeAffiliateProgramCommission");
    s.store_field("chat_id", chat_id_);
    s.store_field("commission_per_mille", commission_per_mille_);
    s.store_class_end();
  }
}

starTransactionTypePaidMessageSend::starTransactionTypePaidMessageSend()
  : chat_id_()
  , message_count_()
{}

starTransactionTypePaidMessageSend::starTransactionTypePaidMessageSend(int53 chat_id_, int32 message_count_)
  : chat_id_(chat_id_)
  , message_count_(message_count_)
{}

const std::int32_t starTransactionTypePaidMessageSend::ID;

void starTransactionTypePaidMessageSend::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypePaidMessageSend");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_count", message_count_);
    s.store_class_end();
  }
}

starTransactionTypePaidMessageReceive::starTransactionTypePaidMessageReceive()
  : sender_id_()
  , message_count_()
  , commission_per_mille_()
  , commission_star_amount_()
{}

starTransactionTypePaidMessageReceive::starTransactionTypePaidMessageReceive(object_ptr<MessageSender> &&sender_id_, int32 message_count_, int32 commission_per_mille_, object_ptr<starAmount> &&commission_star_amount_)
  : sender_id_(std::move(sender_id_))
  , message_count_(message_count_)
  , commission_per_mille_(commission_per_mille_)
  , commission_star_amount_(std::move(commission_star_amount_))
{}

const std::int32_t starTransactionTypePaidMessageReceive::ID;

void starTransactionTypePaidMessageReceive::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypePaidMessageReceive");
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_field("message_count", message_count_);
    s.store_field("commission_per_mille", commission_per_mille_);
    s.store_object_field("commission_star_amount", static_cast<const BaseObject *>(commission_star_amount_.get()));
    s.store_class_end();
  }
}

starTransactionTypeSuggestedPostPaymentSend::starTransactionTypeSuggestedPostPaymentSend()
  : chat_id_()
{}

starTransactionTypeSuggestedPostPaymentSend::starTransactionTypeSuggestedPostPaymentSend(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t starTransactionTypeSuggestedPostPaymentSend::ID;

void starTransactionTypeSuggestedPostPaymentSend::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeSuggestedPostPaymentSend");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

starTransactionTypeSuggestedPostPaymentReceive::starTransactionTypeSuggestedPostPaymentReceive()
  : user_id_()
{}

starTransactionTypeSuggestedPostPaymentReceive::starTransactionTypeSuggestedPostPaymentReceive(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t starTransactionTypeSuggestedPostPaymentReceive::ID;

void starTransactionTypeSuggestedPostPaymentReceive::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeSuggestedPostPaymentReceive");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

starTransactionTypePremiumPurchase::starTransactionTypePremiumPurchase()
  : user_id_()
  , month_count_()
  , sticker_()
{}

starTransactionTypePremiumPurchase::starTransactionTypePremiumPurchase(int53 user_id_, int32 month_count_, object_ptr<sticker> &&sticker_)
  : user_id_(user_id_)
  , month_count_(month_count_)
  , sticker_(std::move(sticker_))
{}

const std::int32_t starTransactionTypePremiumPurchase::ID;

void starTransactionTypePremiumPurchase::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypePremiumPurchase");
    s.store_field("user_id", user_id_);
    s.store_field("month_count", month_count_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

starTransactionTypeBusinessBotTransferSend::starTransactionTypeBusinessBotTransferSend()
  : user_id_()
{}

starTransactionTypeBusinessBotTransferSend::starTransactionTypeBusinessBotTransferSend(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t starTransactionTypeBusinessBotTransferSend::ID;

void starTransactionTypeBusinessBotTransferSend::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeBusinessBotTransferSend");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

starTransactionTypeBusinessBotTransferReceive::starTransactionTypeBusinessBotTransferReceive()
  : user_id_()
{}

starTransactionTypeBusinessBotTransferReceive::starTransactionTypeBusinessBotTransferReceive(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t starTransactionTypeBusinessBotTransferReceive::ID;

void starTransactionTypeBusinessBotTransferReceive::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeBusinessBotTransferReceive");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

starTransactionTypePublicPostSearch::starTransactionTypePublicPostSearch() {
}

const std::int32_t starTransactionTypePublicPostSearch::ID;

void starTransactionTypePublicPostSearch::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypePublicPostSearch");
    s.store_class_end();
  }
}

starTransactionTypeUnsupported::starTransactionTypeUnsupported() {
}

const std::int32_t starTransactionTypeUnsupported::ID;

void starTransactionTypeUnsupported::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starTransactionTypeUnsupported");
    s.store_class_end();
  }
}

statisticalValue::statisticalValue()
  : value_()
  , previous_value_()
  , growth_rate_percentage_()
{}

statisticalValue::statisticalValue(double value_, double previous_value_, double growth_rate_percentage_)
  : value_(value_)
  , previous_value_(previous_value_)
  , growth_rate_percentage_(growth_rate_percentage_)
{}

const std::int32_t statisticalValue::ID;

void statisticalValue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "statisticalValue");
    s.store_field("value", value_);
    s.store_field("previous_value", previous_value_);
    s.store_field("growth_rate_percentage", growth_rate_percentage_);
    s.store_class_end();
  }
}

stickerFullTypeRegular::stickerFullTypeRegular()
  : premium_animation_()
{}

stickerFullTypeRegular::stickerFullTypeRegular(object_ptr<file> &&premium_animation_)
  : premium_animation_(std::move(premium_animation_))
{}

const std::int32_t stickerFullTypeRegular::ID;

void stickerFullTypeRegular::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickerFullTypeRegular");
    s.store_object_field("premium_animation", static_cast<const BaseObject *>(premium_animation_.get()));
    s.store_class_end();
  }
}

stickerFullTypeMask::stickerFullTypeMask()
  : mask_position_()
{}

stickerFullTypeMask::stickerFullTypeMask(object_ptr<maskPosition> &&mask_position_)
  : mask_position_(std::move(mask_position_))
{}

const std::int32_t stickerFullTypeMask::ID;

void stickerFullTypeMask::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickerFullTypeMask");
    s.store_object_field("mask_position", static_cast<const BaseObject *>(mask_position_.get()));
    s.store_class_end();
  }
}

stickerFullTypeCustomEmoji::stickerFullTypeCustomEmoji()
  : custom_emoji_id_()
  , needs_repainting_()
{}

stickerFullTypeCustomEmoji::stickerFullTypeCustomEmoji(int64 custom_emoji_id_, bool needs_repainting_)
  : custom_emoji_id_(custom_emoji_id_)
  , needs_repainting_(needs_repainting_)
{}

const std::int32_t stickerFullTypeCustomEmoji::ID;

void stickerFullTypeCustomEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickerFullTypeCustomEmoji");
    s.store_field("custom_emoji_id", custom_emoji_id_);
    s.store_field("needs_repainting", needs_repainting_);
    s.store_class_end();
  }
}

stickers::stickers()
  : stickers_()
{}

stickers::stickers(array<object_ptr<sticker>> &&stickers_)
  : stickers_(std::move(stickers_))
{}

const std::int32_t stickers::ID;

void stickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickers");
    { s.store_vector_begin("stickers", stickers_.size()); for (const auto &_value : stickers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

storyArea::storyArea()
  : position_()
  , type_()
{}

storyArea::storyArea(object_ptr<storyAreaPosition> &&position_, object_ptr<StoryAreaType> &&type_)
  : position_(std::move(position_))
  , type_(std::move(type_))
{}

const std::int32_t storyArea::ID;

void storyArea::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyArea");
    s.store_object_field("position", static_cast<const BaseObject *>(position_.get()));
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

storyContentPhoto::storyContentPhoto()
  : photo_()
{}

storyContentPhoto::storyContentPhoto(object_ptr<photo> &&photo_)
  : photo_(std::move(photo_))
{}

const std::int32_t storyContentPhoto::ID;

void storyContentPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyContentPhoto");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

storyContentVideo::storyContentVideo()
  : video_()
  , alternative_video_()
{}

storyContentVideo::storyContentVideo(object_ptr<storyVideo> &&video_, object_ptr<storyVideo> &&alternative_video_)
  : video_(std::move(video_))
  , alternative_video_(std::move(alternative_video_))
{}

const std::int32_t storyContentVideo::ID;

void storyContentVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyContentVideo");
    s.store_object_field("video", static_cast<const BaseObject *>(video_.get()));
    s.store_object_field("alternative_video", static_cast<const BaseObject *>(alternative_video_.get()));
    s.store_class_end();
  }
}

storyContentUnsupported::storyContentUnsupported() {
}

const std::int32_t storyContentUnsupported::ID;

void storyContentUnsupported::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyContentUnsupported");
    s.store_class_end();
  }
}

targetChatTypes::targetChatTypes()
  : allow_user_chats_()
  , allow_bot_chats_()
  , allow_group_chats_()
  , allow_channel_chats_()
{}

targetChatTypes::targetChatTypes(bool allow_user_chats_, bool allow_bot_chats_, bool allow_group_chats_, bool allow_channel_chats_)
  : allow_user_chats_(allow_user_chats_)
  , allow_bot_chats_(allow_bot_chats_)
  , allow_group_chats_(allow_group_chats_)
  , allow_channel_chats_(allow_channel_chats_)
{}

const std::int32_t targetChatTypes::ID;

void targetChatTypes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "targetChatTypes");
    s.store_field("allow_user_chats", allow_user_chats_);
    s.store_field("allow_bot_chats", allow_bot_chats_);
    s.store_field("allow_group_chats", allow_group_chats_);
    s.store_field("allow_channel_chats", allow_channel_chats_);
    s.store_class_end();
  }
}

textEntityTypeMention::textEntityTypeMention() {
}

const std::int32_t textEntityTypeMention::ID;

void textEntityTypeMention::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypeMention");
    s.store_class_end();
  }
}

textEntityTypeHashtag::textEntityTypeHashtag() {
}

const std::int32_t textEntityTypeHashtag::ID;

void textEntityTypeHashtag::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypeHashtag");
    s.store_class_end();
  }
}

textEntityTypeCashtag::textEntityTypeCashtag() {
}

const std::int32_t textEntityTypeCashtag::ID;

void textEntityTypeCashtag::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypeCashtag");
    s.store_class_end();
  }
}

textEntityTypeBotCommand::textEntityTypeBotCommand() {
}

const std::int32_t textEntityTypeBotCommand::ID;

void textEntityTypeBotCommand::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypeBotCommand");
    s.store_class_end();
  }
}

textEntityTypeUrl::textEntityTypeUrl() {
}

const std::int32_t textEntityTypeUrl::ID;

void textEntityTypeUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypeUrl");
    s.store_class_end();
  }
}

textEntityTypeEmailAddress::textEntityTypeEmailAddress() {
}

const std::int32_t textEntityTypeEmailAddress::ID;

void textEntityTypeEmailAddress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypeEmailAddress");
    s.store_class_end();
  }
}

textEntityTypePhoneNumber::textEntityTypePhoneNumber() {
}

const std::int32_t textEntityTypePhoneNumber::ID;

void textEntityTypePhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypePhoneNumber");
    s.store_class_end();
  }
}

textEntityTypeBankCardNumber::textEntityTypeBankCardNumber() {
}

const std::int32_t textEntityTypeBankCardNumber::ID;

void textEntityTypeBankCardNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypeBankCardNumber");
    s.store_class_end();
  }
}

textEntityTypeBold::textEntityTypeBold() {
}

const std::int32_t textEntityTypeBold::ID;

void textEntityTypeBold::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypeBold");
    s.store_class_end();
  }
}

textEntityTypeItalic::textEntityTypeItalic() {
}

const std::int32_t textEntityTypeItalic::ID;

void textEntityTypeItalic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypeItalic");
    s.store_class_end();
  }
}

textEntityTypeUnderline::textEntityTypeUnderline() {
}

const std::int32_t textEntityTypeUnderline::ID;

void textEntityTypeUnderline::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypeUnderline");
    s.store_class_end();
  }
}

textEntityTypeStrikethrough::textEntityTypeStrikethrough() {
}

const std::int32_t textEntityTypeStrikethrough::ID;

void textEntityTypeStrikethrough::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypeStrikethrough");
    s.store_class_end();
  }
}

textEntityTypeSpoiler::textEntityTypeSpoiler() {
}

const std::int32_t textEntityTypeSpoiler::ID;

void textEntityTypeSpoiler::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypeSpoiler");
    s.store_class_end();
  }
}

textEntityTypeCode::textEntityTypeCode() {
}

const std::int32_t textEntityTypeCode::ID;

void textEntityTypeCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypeCode");
    s.store_class_end();
  }
}

textEntityTypePre::textEntityTypePre() {
}

const std::int32_t textEntityTypePre::ID;

void textEntityTypePre::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypePre");
    s.store_class_end();
  }
}

textEntityTypePreCode::textEntityTypePreCode()
  : language_()
{}

textEntityTypePreCode::textEntityTypePreCode(string const &language_)
  : language_(language_)
{}

const std::int32_t textEntityTypePreCode::ID;

void textEntityTypePreCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypePreCode");
    s.store_field("language", language_);
    s.store_class_end();
  }
}

textEntityTypeBlockQuote::textEntityTypeBlockQuote() {
}

const std::int32_t textEntityTypeBlockQuote::ID;

void textEntityTypeBlockQuote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypeBlockQuote");
    s.store_class_end();
  }
}

textEntityTypeExpandableBlockQuote::textEntityTypeExpandableBlockQuote() {
}

const std::int32_t textEntityTypeExpandableBlockQuote::ID;

void textEntityTypeExpandableBlockQuote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypeExpandableBlockQuote");
    s.store_class_end();
  }
}

textEntityTypeTextUrl::textEntityTypeTextUrl()
  : url_()
{}

textEntityTypeTextUrl::textEntityTypeTextUrl(string const &url_)
  : url_(url_)
{}

const std::int32_t textEntityTypeTextUrl::ID;

void textEntityTypeTextUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypeTextUrl");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

textEntityTypeMentionName::textEntityTypeMentionName()
  : user_id_()
{}

textEntityTypeMentionName::textEntityTypeMentionName(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t textEntityTypeMentionName::ID;

void textEntityTypeMentionName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypeMentionName");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

textEntityTypeCustomEmoji::textEntityTypeCustomEmoji()
  : custom_emoji_id_()
{}

textEntityTypeCustomEmoji::textEntityTypeCustomEmoji(int64 custom_emoji_id_)
  : custom_emoji_id_(custom_emoji_id_)
{}

const std::int32_t textEntityTypeCustomEmoji::ID;

void textEntityTypeCustomEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypeCustomEmoji");
    s.store_field("custom_emoji_id", custom_emoji_id_);
    s.store_class_end();
  }
}

textEntityTypeMediaTimestamp::textEntityTypeMediaTimestamp()
  : media_timestamp_()
{}

textEntityTypeMediaTimestamp::textEntityTypeMediaTimestamp(int32 media_timestamp_)
  : media_timestamp_(media_timestamp_)
{}

const std::int32_t textEntityTypeMediaTimestamp::ID;

void textEntityTypeMediaTimestamp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntityTypeMediaTimestamp");
    s.store_field("media_timestamp", media_timestamp_);
    s.store_class_end();
  }
}

validatedOrderInfo::validatedOrderInfo()
  : order_info_id_()
  , shipping_options_()
{}

validatedOrderInfo::validatedOrderInfo(string const &order_info_id_, array<object_ptr<shippingOption>> &&shipping_options_)
  : order_info_id_(order_info_id_)
  , shipping_options_(std::move(shipping_options_))
{}

const std::int32_t validatedOrderInfo::ID;

void validatedOrderInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "validatedOrderInfo");
    s.store_field("order_info_id", order_info_id_);
    { s.store_vector_begin("shipping_options", shipping_options_.size()); for (const auto &_value : shipping_options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

videoMessageAdvertisements::videoMessageAdvertisements()
  : advertisements_()
  , start_delay_()
  , between_delay_()
{}

videoMessageAdvertisements::videoMessageAdvertisements(array<object_ptr<videoMessageAdvertisement>> &&advertisements_, int32 start_delay_, int32 between_delay_)
  : advertisements_(std::move(advertisements_))
  , start_delay_(start_delay_)
  , between_delay_(between_delay_)
{}

const std::int32_t videoMessageAdvertisements::ID;

void videoMessageAdvertisements::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "videoMessageAdvertisements");
    { s.store_vector_begin("advertisements", advertisements_.size()); for (const auto &_value : advertisements_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("start_delay", start_delay_);
    s.store_field("between_delay", between_delay_);
    s.store_class_end();
  }
}

videoStoryboard::videoStoryboard()
  : storyboard_file_()
  , width_()
  , height_()
  , map_file_()
{}

videoStoryboard::videoStoryboard(object_ptr<file> &&storyboard_file_, int32 width_, int32 height_, object_ptr<file> &&map_file_)
  : storyboard_file_(std::move(storyboard_file_))
  , width_(width_)
  , height_(height_)
  , map_file_(std::move(map_file_))
{}

const std::int32_t videoStoryboard::ID;

void videoStoryboard::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "videoStoryboard");
    s.store_object_field("storyboard_file", static_cast<const BaseObject *>(storyboard_file_.get()));
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_object_field("map_file", static_cast<const BaseObject *>(map_file_.get()));
    s.store_class_end();
  }
}

addChatToList::addChatToList()
  : chat_id_()
  , chat_list_()
{}

addChatToList::addChatToList(int53 chat_id_, object_ptr<ChatList> &&chat_list_)
  : chat_id_(chat_id_)
  , chat_list_(std::move(chat_list_))
{}

const std::int32_t addChatToList::ID;

void addChatToList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addChatToList");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("chat_list", static_cast<const BaseObject *>(chat_list_.get()));
    s.store_class_end();
  }
}

addFavoriteSticker::addFavoriteSticker()
  : sticker_()
{}

addFavoriteSticker::addFavoriteSticker(object_ptr<InputFile> &&sticker_)
  : sticker_(std::move(sticker_))
{}

const std::int32_t addFavoriteSticker::ID;

void addFavoriteSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addFavoriteSticker");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

addSavedAnimation::addSavedAnimation()
  : animation_()
{}

addSavedAnimation::addSavedAnimation(object_ptr<InputFile> &&animation_)
  : animation_(std::move(animation_))
{}

const std::int32_t addSavedAnimation::ID;

void addSavedAnimation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addSavedAnimation");
    s.store_object_field("animation", static_cast<const BaseObject *>(animation_.get()));
    s.store_class_end();
  }
}

addSavedNotificationSound::addSavedNotificationSound()
  : sound_()
{}

addSavedNotificationSound::addSavedNotificationSound(object_ptr<InputFile> &&sound_)
  : sound_(std::move(sound_))
{}

const std::int32_t addSavedNotificationSound::ID;

void addSavedNotificationSound::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addSavedNotificationSound");
    s.store_object_field("sound", static_cast<const BaseObject *>(sound_.get()));
    s.store_class_end();
  }
}

buyGiftUpgrade::buyGiftUpgrade()
  : owner_id_()
  , prepaid_upgrade_hash_()
  , star_count_()
{}

buyGiftUpgrade::buyGiftUpgrade(object_ptr<MessageSender> &&owner_id_, string const &prepaid_upgrade_hash_, int53 star_count_)
  : owner_id_(std::move(owner_id_))
  , prepaid_upgrade_hash_(prepaid_upgrade_hash_)
  , star_count_(star_count_)
{}

const std::int32_t buyGiftUpgrade::ID;

void buyGiftUpgrade::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "buyGiftUpgrade");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_field("prepaid_upgrade_hash", prepaid_upgrade_hash_);
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

cancelRecoveryEmailAddressVerification::cancelRecoveryEmailAddressVerification() {
}

const std::int32_t cancelRecoveryEmailAddressVerification::ID;

void cancelRecoveryEmailAddressVerification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "cancelRecoveryEmailAddressVerification");
    s.store_class_end();
  }
}

checkAuthenticationBotToken::checkAuthenticationBotToken()
  : token_()
{}

checkAuthenticationBotToken::checkAuthenticationBotToken(string const &token_)
  : token_(token_)
{}

const std::int32_t checkAuthenticationBotToken::ID;

void checkAuthenticationBotToken::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkAuthenticationBotToken");
    s.store_field("token", token_);
    s.store_class_end();
  }
}

checkRecoveryEmailAddressCode::checkRecoveryEmailAddressCode()
  : code_()
{}

checkRecoveryEmailAddressCode::checkRecoveryEmailAddressCode(string const &code_)
  : code_(code_)
{}

const std::int32_t checkRecoveryEmailAddressCode::ID;

void checkRecoveryEmailAddressCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkRecoveryEmailAddressCode");
    s.store_field("code", code_);
    s.store_class_end();
  }
}

clearRecentEmojiStatuses::clearRecentEmojiStatuses() {
}

const std::int32_t clearRecentEmojiStatuses::ID;

void clearRecentEmojiStatuses::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "clearRecentEmojiStatuses");
    s.store_class_end();
  }
}

clearSearchedForTags::clearSearchedForTags()
  : clear_cashtags_()
{}

clearSearchedForTags::clearSearchedForTags(bool clear_cashtags_)
  : clear_cashtags_(clear_cashtags_)
{}

const std::int32_t clearSearchedForTags::ID;

void clearSearchedForTags::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "clearSearchedForTags");
    s.store_field("clear_cashtags", clear_cashtags_);
    s.store_class_end();
  }
}

clickAnimatedEmojiMessage::clickAnimatedEmojiMessage()
  : chat_id_()
  , message_id_()
{}

clickAnimatedEmojiMessage::clickAnimatedEmojiMessage(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t clickAnimatedEmojiMessage::ID;

void clickAnimatedEmojiMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "clickAnimatedEmojiMessage");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

clickChatSponsoredMessage::clickChatSponsoredMessage()
  : chat_id_()
  , message_id_()
  , is_media_click_()
  , from_fullscreen_()
{}

clickChatSponsoredMessage::clickChatSponsoredMessage(int53 chat_id_, int53 message_id_, bool is_media_click_, bool from_fullscreen_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , is_media_click_(is_media_click_)
  , from_fullscreen_(from_fullscreen_)
{}

const std::int32_t clickChatSponsoredMessage::ID;

void clickChatSponsoredMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "clickChatSponsoredMessage");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("is_media_click", is_media_click_);
    s.store_field("from_fullscreen", from_fullscreen_);
    s.store_class_end();
  }
}

createNewSupergroupChat::createNewSupergroupChat()
  : title_()
  , is_forum_()
  , is_channel_()
  , description_()
  , location_()
  , message_auto_delete_time_()
  , for_import_()
{}

createNewSupergroupChat::createNewSupergroupChat(string const &title_, bool is_forum_, bool is_channel_, string const &description_, object_ptr<chatLocation> &&location_, int32 message_auto_delete_time_, bool for_import_)
  : title_(title_)
  , is_forum_(is_forum_)
  , is_channel_(is_channel_)
  , description_(description_)
  , location_(std::move(location_))
  , message_auto_delete_time_(message_auto_delete_time_)
  , for_import_(for_import_)
{}

const std::int32_t createNewSupergroupChat::ID;

void createNewSupergroupChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createNewSupergroupChat");
    s.store_field("title", title_);
    s.store_field("is_forum", is_forum_);
    s.store_field("is_channel", is_channel_);
    s.store_field("description", description_);
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_field("message_auto_delete_time", message_auto_delete_time_);
    s.store_field("for_import", for_import_);
    s.store_class_end();
  }
}

createSecretChat::createSecretChat()
  : secret_chat_id_()
{}

createSecretChat::createSecretChat(int32 secret_chat_id_)
  : secret_chat_id_(secret_chat_id_)
{}

const std::int32_t createSecretChat::ID;

void createSecretChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createSecretChat");
    s.store_field("secret_chat_id", secret_chat_id_);
    s.store_class_end();
  }
}

createSupergroupChat::createSupergroupChat()
  : supergroup_id_()
  , force_()
{}

createSupergroupChat::createSupergroupChat(int53 supergroup_id_, bool force_)
  : supergroup_id_(supergroup_id_)
  , force_(force_)
{}

const std::int32_t createSupergroupChat::ID;

void createSupergroupChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createSupergroupChat");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_field("force", force_);
    s.store_class_end();
  }
}

decryptGroupCallData::decryptGroupCallData()
  : group_call_id_()
  , participant_id_()
  , data_channel_()
  , data_()
{}

decryptGroupCallData::decryptGroupCallData(int32 group_call_id_, object_ptr<MessageSender> &&participant_id_, object_ptr<GroupCallDataChannel> &&data_channel_, bytes const &data_)
  : group_call_id_(group_call_id_)
  , participant_id_(std::move(participant_id_))
  , data_channel_(std::move(data_channel_))
  , data_(std::move(data_))
{}

const std::int32_t decryptGroupCallData::ID;

void decryptGroupCallData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "decryptGroupCallData");
    s.store_field("group_call_id", group_call_id_);
    s.store_object_field("participant_id", static_cast<const BaseObject *>(participant_id_.get()));
    s.store_object_field("data_channel", static_cast<const BaseObject *>(data_channel_.get()));
    s.store_bytes_field("data", data_);
    s.store_class_end();
  }
}

deleteAccount::deleteAccount()
  : reason_()
  , password_()
{}

deleteAccount::deleteAccount(string const &reason_, string const &password_)
  : reason_(reason_)
  , password_(password_)
{}

const std::int32_t deleteAccount::ID;

void deleteAccount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteAccount");
    s.store_field("reason", reason_);
    s.store_field("password", password_);
    s.store_class_end();
  }
}

deleteAllRevokedChatInviteLinks::deleteAllRevokedChatInviteLinks()
  : chat_id_()
  , creator_user_id_()
{}

deleteAllRevokedChatInviteLinks::deleteAllRevokedChatInviteLinks(int53 chat_id_, int53 creator_user_id_)
  : chat_id_(chat_id_)
  , creator_user_id_(creator_user_id_)
{}

const std::int32_t deleteAllRevokedChatInviteLinks::ID;

void deleteAllRevokedChatInviteLinks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteAllRevokedChatInviteLinks");
    s.store_field("chat_id", chat_id_);
    s.store_field("creator_user_id", creator_user_id_);
    s.store_class_end();
  }
}

deleteLanguagePack::deleteLanguagePack()
  : language_pack_id_()
{}

deleteLanguagePack::deleteLanguagePack(string const &language_pack_id_)
  : language_pack_id_(language_pack_id_)
{}

const std::int32_t deleteLanguagePack::ID;

void deleteLanguagePack::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteLanguagePack");
    s.store_field("language_pack_id", language_pack_id_);
    s.store_class_end();
  }
}

deleteStickerSet::deleteStickerSet()
  : name_()
{}

deleteStickerSet::deleteStickerSet(string const &name_)
  : name_(name_)
{}

const std::int32_t deleteStickerSet::ID;

void deleteStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteStickerSet");
    s.store_field("name", name_);
    s.store_class_end();
  }
}

disableAllSupergroupUsernames::disableAllSupergroupUsernames()
  : supergroup_id_()
{}

disableAllSupergroupUsernames::disableAllSupergroupUsernames(int53 supergroup_id_)
  : supergroup_id_(supergroup_id_)
{}

const std::int32_t disableAllSupergroupUsernames::ID;

void disableAllSupergroupUsernames::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "disableAllSupergroupUsernames");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_class_end();
  }
}

dropGiftOriginalDetails::dropGiftOriginalDetails()
  : received_gift_id_()
  , star_count_()
{}

dropGiftOriginalDetails::dropGiftOriginalDetails(string const &received_gift_id_, int53 star_count_)
  : received_gift_id_(received_gift_id_)
  , star_count_(star_count_)
{}

const std::int32_t dropGiftOriginalDetails::ID;

void dropGiftOriginalDetails::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "dropGiftOriginalDetails");
    s.store_field("received_gift_id", received_gift_id_);
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

editBusinessMessageText::editBusinessMessageText()
  : business_connection_id_()
  , chat_id_()
  , message_id_()
  , reply_markup_()
  , input_message_content_()
{}

editBusinessMessageText::editBusinessMessageText(string const &business_connection_id_, int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_)
  : business_connection_id_(business_connection_id_)
  , chat_id_(chat_id_)
  , message_id_(message_id_)
  , reply_markup_(std::move(reply_markup_))
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t editBusinessMessageText::ID;

void editBusinessMessageText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editBusinessMessageText");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

editChatFolder::editChatFolder()
  : chat_folder_id_()
  , folder_()
{}

editChatFolder::editChatFolder(int32 chat_folder_id_, object_ptr<chatFolder> &&folder_)
  : chat_folder_id_(chat_folder_id_)
  , folder_(std::move(folder_))
{}

const std::int32_t editChatFolder::ID;

void editChatFolder::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editChatFolder");
    s.store_field("chat_folder_id", chat_folder_id_);
    s.store_object_field("folder", static_cast<const BaseObject *>(folder_.get()));
    s.store_class_end();
  }
}

editCustomLanguagePackInfo::editCustomLanguagePackInfo()
  : info_()
{}

editCustomLanguagePackInfo::editCustomLanguagePackInfo(object_ptr<languagePackInfo> &&info_)
  : info_(std::move(info_))
{}

const std::int32_t editCustomLanguagePackInfo::ID;

void editCustomLanguagePackInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editCustomLanguagePackInfo");
    s.store_object_field("info", static_cast<const BaseObject *>(info_.get()));
    s.store_class_end();
  }
}

editForumTopic::editForumTopic()
  : chat_id_()
  , forum_topic_id_()
  , name_()
  , edit_icon_custom_emoji_()
  , icon_custom_emoji_id_()
{}

editForumTopic::editForumTopic(int53 chat_id_, int32 forum_topic_id_, string const &name_, bool edit_icon_custom_emoji_, int64 icon_custom_emoji_id_)
  : chat_id_(chat_id_)
  , forum_topic_id_(forum_topic_id_)
  , name_(name_)
  , edit_icon_custom_emoji_(edit_icon_custom_emoji_)
  , icon_custom_emoji_id_(icon_custom_emoji_id_)
{}

const std::int32_t editForumTopic::ID;

void editForumTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editForumTopic");
    s.store_field("chat_id", chat_id_);
    s.store_field("forum_topic_id", forum_topic_id_);
    s.store_field("name", name_);
    s.store_field("edit_icon_custom_emoji", edit_icon_custom_emoji_);
    s.store_field("icon_custom_emoji_id", icon_custom_emoji_id_);
    s.store_class_end();
  }
}

editInlineMessageCaption::editInlineMessageCaption()
  : inline_message_id_()
  , reply_markup_()
  , caption_()
  , show_caption_above_media_()
{}

editInlineMessageCaption::editInlineMessageCaption(string const &inline_message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<formattedText> &&caption_, bool show_caption_above_media_)
  : inline_message_id_(inline_message_id_)
  , reply_markup_(std::move(reply_markup_))
  , caption_(std::move(caption_))
  , show_caption_above_media_(show_caption_above_media_)
{}

const std::int32_t editInlineMessageCaption::ID;

void editInlineMessageCaption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editInlineMessageCaption");
    s.store_field("inline_message_id", inline_message_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_field("show_caption_above_media", show_caption_above_media_);
    s.store_class_end();
  }
}

endGroupCall::endGroupCall()
  : group_call_id_()
{}

endGroupCall::endGroupCall(int32 group_call_id_)
  : group_call_id_(group_call_id_)
{}

const std::int32_t endGroupCall::ID;

void endGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "endGroupCall");
    s.store_field("group_call_id", group_call_id_);
    s.store_class_end();
  }
}

endGroupCallScreenSharing::endGroupCallScreenSharing()
  : group_call_id_()
{}

endGroupCallScreenSharing::endGroupCallScreenSharing(int32 group_call_id_)
  : group_call_id_(group_call_id_)
{}

const std::int32_t endGroupCallScreenSharing::ID;

void endGroupCallScreenSharing::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "endGroupCallScreenSharing");
    s.store_field("group_call_id", group_call_id_);
    s.store_class_end();
  }
}

getApplicationDownloadLink::getApplicationDownloadLink() {
}

const std::int32_t getApplicationDownloadLink::ID;

void getApplicationDownloadLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getApplicationDownloadLink");
    s.store_class_end();
  }
}

getAuthorizationState::getAuthorizationState() {
}

const std::int32_t getAuthorizationState::ID;

void getAuthorizationState::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getAuthorizationState");
    s.store_class_end();
  }
}

getBankCardInfo::getBankCardInfo()
  : bank_card_number_()
{}

getBankCardInfo::getBankCardInfo(string const &bank_card_number_)
  : bank_card_number_(bank_card_number_)
{}

const std::int32_t getBankCardInfo::ID;

void getBankCardInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getBankCardInfo");
    s.store_field("bank_card_number", bank_card_number_);
    s.store_class_end();
  }
}

getBusinessChatLinkInfo::getBusinessChatLinkInfo()
  : link_name_()
{}

getBusinessChatLinkInfo::getBusinessChatLinkInfo(string const &link_name_)
  : link_name_(link_name_)
{}

const std::int32_t getBusinessChatLinkInfo::ID;

void getBusinessChatLinkInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getBusinessChatLinkInfo");
    s.store_field("link_name", link_name_);
    s.store_class_end();
  }
}

getChatBoosts::getChatBoosts()
  : chat_id_()
  , only_gift_codes_()
  , offset_()
  , limit_()
{}

getChatBoosts::getChatBoosts(int53 chat_id_, bool only_gift_codes_, string const &offset_, int32 limit_)
  : chat_id_(chat_id_)
  , only_gift_codes_(only_gift_codes_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getChatBoosts::ID;

void getChatBoosts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatBoosts");
    s.store_field("chat_id", chat_id_);
    s.store_field("only_gift_codes", only_gift_codes_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getChatMessageByDate::getChatMessageByDate()
  : chat_id_()
  , date_()
{}

getChatMessageByDate::getChatMessageByDate(int53 chat_id_, int32 date_)
  : chat_id_(chat_id_)
  , date_(date_)
{}

const std::int32_t getChatMessageByDate::ID;

void getChatMessageByDate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatMessageByDate");
    s.store_field("chat_id", chat_id_);
    s.store_field("date", date_);
    s.store_class_end();
  }
}

getChatMessagePosition::getChatMessagePosition()
  : chat_id_()
  , topic_id_()
  , filter_()
  , message_id_()
{}

getChatMessagePosition::getChatMessagePosition(int53 chat_id_, object_ptr<MessageTopic> &&topic_id_, object_ptr<SearchMessagesFilter> &&filter_, int53 message_id_)
  : chat_id_(chat_id_)
  , topic_id_(std::move(topic_id_))
  , filter_(std::move(filter_))
  , message_id_(message_id_)
{}

const std::int32_t getChatMessagePosition::ID;

void getChatMessagePosition::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatMessagePosition");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("topic_id", static_cast<const BaseObject *>(topic_id_.get()));
    s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get()));
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

getChatRevenueWithdrawalUrl::getChatRevenueWithdrawalUrl()
  : chat_id_()
  , password_()
{}

getChatRevenueWithdrawalUrl::getChatRevenueWithdrawalUrl(int53 chat_id_, string const &password_)
  : chat_id_(chat_id_)
  , password_(password_)
{}

const std::int32_t getChatRevenueWithdrawalUrl::ID;

void getChatRevenueWithdrawalUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatRevenueWithdrawalUrl");
    s.store_field("chat_id", chat_id_);
    s.store_field("password", password_);
    s.store_class_end();
  }
}

getConnectedAffiliateProgram::getConnectedAffiliateProgram()
  : affiliate_()
  , bot_user_id_()
{}

getConnectedAffiliateProgram::getConnectedAffiliateProgram(object_ptr<AffiliateType> &&affiliate_, int53 bot_user_id_)
  : affiliate_(std::move(affiliate_))
  , bot_user_id_(bot_user_id_)
{}

const std::int32_t getConnectedAffiliateProgram::ID;

void getConnectedAffiliateProgram::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getConnectedAffiliateProgram");
    s.store_object_field("affiliate", static_cast<const BaseObject *>(affiliate_.get()));
    s.store_field("bot_user_id", bot_user_id_);
    s.store_class_end();
  }
}

getDirectMessagesChatTopicRevenue::getDirectMessagesChatTopicRevenue()
  : chat_id_()
  , topic_id_()
{}

getDirectMessagesChatTopicRevenue::getDirectMessagesChatTopicRevenue(int53 chat_id_, int53 topic_id_)
  : chat_id_(chat_id_)
  , topic_id_(topic_id_)
{}

const std::int32_t getDirectMessagesChatTopicRevenue::ID;

void getDirectMessagesChatTopicRevenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getDirectMessagesChatTopicRevenue");
    s.store_field("chat_id", chat_id_);
    s.store_field("topic_id", topic_id_);
    s.store_class_end();
  }
}

getFileMimeType::getFileMimeType()
  : file_name_()
{}

getFileMimeType::getFileMimeType(string const &file_name_)
  : file_name_(file_name_)
{}

const std::int32_t getFileMimeType::ID;

void getFileMimeType::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getFileMimeType");
    s.store_field("file_name", file_name_);
    s.store_class_end();
  }
}

getLoginUrlInfo::getLoginUrlInfo()
  : chat_id_()
  , message_id_()
  , button_id_()
{}

getLoginUrlInfo::getLoginUrlInfo(int53 chat_id_, int53 message_id_, int53 button_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , button_id_(button_id_)
{}

const std::int32_t getLoginUrlInfo::ID;

void getLoginUrlInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getLoginUrlInfo");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("button_id", button_id_);
    s.store_class_end();
  }
}

getMapThumbnailFile::getMapThumbnailFile()
  : location_()
  , zoom_()
  , width_()
  , height_()
  , scale_()
  , chat_id_()
{}

getMapThumbnailFile::getMapThumbnailFile(object_ptr<location> &&location_, int32 zoom_, int32 width_, int32 height_, int32 scale_, int53 chat_id_)
  : location_(std::move(location_))
  , zoom_(zoom_)
  , width_(width_)
  , height_(height_)
  , scale_(scale_)
  , chat_id_(chat_id_)
{}

const std::int32_t getMapThumbnailFile::ID;

void getMapThumbnailFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMapThumbnailFile");
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_field("zoom", zoom_);
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_field("scale", scale_);
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

getPhoneNumberInfoSync::getPhoneNumberInfoSync()
  : language_code_()
  , phone_number_prefix_()
{}

getPhoneNumberInfoSync::getPhoneNumberInfoSync(string const &language_code_, string const &phone_number_prefix_)
  : language_code_(language_code_)
  , phone_number_prefix_(phone_number_prefix_)
{}

const std::int32_t getPhoneNumberInfoSync::ID;

void getPhoneNumberInfoSync::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPhoneNumberInfoSync");
    s.store_field("language_code", language_code_);
    s.store_field("phone_number_prefix", phone_number_prefix_);
    s.store_class_end();
  }
}

getPremiumGiveawayPaymentOptions::getPremiumGiveawayPaymentOptions()
  : boosted_chat_id_()
{}

getPremiumGiveawayPaymentOptions::getPremiumGiveawayPaymentOptions(int53 boosted_chat_id_)
  : boosted_chat_id_(boosted_chat_id_)
{}

const std::int32_t getPremiumGiveawayPaymentOptions::ID;

void getPremiumGiveawayPaymentOptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPremiumGiveawayPaymentOptions");
    s.store_field("boosted_chat_id", boosted_chat_id_);
    s.store_class_end();
  }
}

getRecommendedChats::getRecommendedChats() {
}

const std::int32_t getRecommendedChats::ID;

void getRecommendedChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getRecommendedChats");
    s.store_class_end();
  }
}

getRepliedMessage::getRepliedMessage()
  : chat_id_()
  , message_id_()
{}

getRepliedMessage::getRepliedMessage(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t getRepliedMessage::ID;

void getRepliedMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getRepliedMessage");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

getScopeNotificationSettings::getScopeNotificationSettings()
  : scope_()
{}

getScopeNotificationSettings::getScopeNotificationSettings(object_ptr<NotificationSettingsScope> &&scope_)
  : scope_(std::move(scope_))
{}

const std::int32_t getScopeNotificationSettings::ID;

void getScopeNotificationSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getScopeNotificationSettings");
    s.store_object_field("scope", static_cast<const BaseObject *>(scope_.get()));
    s.store_class_end();
  }
}

getStickerEmojis::getStickerEmojis()
  : sticker_()
{}

getStickerEmojis::getStickerEmojis(object_ptr<InputFile> &&sticker_)
  : sticker_(std::move(sticker_))
{}

const std::int32_t getStickerEmojis::ID;

void getStickerEmojis::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStickerEmojis");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

getStickers::getStickers()
  : sticker_type_()
  , query_()
  , limit_()
  , chat_id_()
{}

getStickers::getStickers(object_ptr<StickerType> &&sticker_type_, string const &query_, int32 limit_, int53 chat_id_)
  : sticker_type_(std::move(sticker_type_))
  , query_(query_)
  , limit_(limit_)
  , chat_id_(chat_id_)
{}

const std::int32_t getStickers::ID;

void getStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStickers");
    s.store_object_field("sticker_type", static_cast<const BaseObject *>(sticker_type_.get()));
    s.store_field("query", query_);
    s.store_field("limit", limit_);
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

getStorageStatistics::getStorageStatistics()
  : chat_limit_()
{}

getStorageStatistics::getStorageStatistics(int32 chat_limit_)
  : chat_limit_(chat_limit_)
{}

const std::int32_t getStorageStatistics::ID;

void getStorageStatistics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStorageStatistics");
    s.store_field("chat_limit", chat_limit_);
    s.store_class_end();
  }
}

getSuitableDiscussionChats::getSuitableDiscussionChats() {
}

const std::int32_t getSuitableDiscussionChats::ID;

void getSuitableDiscussionChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getSuitableDiscussionChats");
    s.store_class_end();
  }
}

getSupportName::getSupportName() {
}

const std::int32_t getSupportName::ID;

void getSupportName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getSupportName");
    s.store_class_end();
  }
}

getSupportUser::getSupportUser() {
}

const std::int32_t getSupportUser::ID;

void getSupportUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getSupportUser");
    s.store_class_end();
  }
}

getTonRevenueStatistics::getTonRevenueStatistics()
  : is_dark_()
{}

getTonRevenueStatistics::getTonRevenueStatistics(bool is_dark_)
  : is_dark_(is_dark_)
{}

const std::int32_t getTonRevenueStatistics::ID;

void getTonRevenueStatistics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getTonRevenueStatistics");
    s.store_field("is_dark", is_dark_);
    s.store_class_end();
  }
}

getTonTransactions::getTonTransactions()
  : direction_()
  , offset_()
  , limit_()
{}

getTonTransactions::getTonTransactions(object_ptr<TransactionDirection> &&direction_, string const &offset_, int32 limit_)
  : direction_(std::move(direction_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getTonTransactions::ID;

void getTonTransactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getTonTransactions");
    s.store_object_field("direction", static_cast<const BaseObject *>(direction_.get()));
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getTopChats::getTopChats()
  : category_()
  , limit_()
{}

getTopChats::getTopChats(object_ptr<TopChatCategory> &&category_, int32 limit_)
  : category_(std::move(category_))
  , limit_(limit_)
{}

const std::int32_t getTopChats::ID;

void getTopChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getTopChats");
    s.store_object_field("category", static_cast<const BaseObject *>(category_.get()));
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getVideoChatAvailableParticipants::getVideoChatAvailableParticipants()
  : chat_id_()
{}

getVideoChatAvailableParticipants::getVideoChatAvailableParticipants(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t getVideoChatAvailableParticipants::ID;

void getVideoChatAvailableParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getVideoChatAvailableParticipants");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

getVideoChatStreamSegment::getVideoChatStreamSegment()
  : group_call_id_()
  , time_offset_()
  , scale_()
  , channel_id_()
  , video_quality_()
{}

getVideoChatStreamSegment::getVideoChatStreamSegment(int32 group_call_id_, int53 time_offset_, int32 scale_, int32 channel_id_, object_ptr<GroupCallVideoQuality> &&video_quality_)
  : group_call_id_(group_call_id_)
  , time_offset_(time_offset_)
  , scale_(scale_)
  , channel_id_(channel_id_)
  , video_quality_(std::move(video_quality_))
{}

const std::int32_t getVideoChatStreamSegment::ID;

void getVideoChatStreamSegment::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getVideoChatStreamSegment");
    s.store_field("group_call_id", group_call_id_);
    s.store_field("time_offset", time_offset_);
    s.store_field("scale", scale_);
    s.store_field("channel_id", channel_id_);
    s.store_object_field("video_quality", static_cast<const BaseObject *>(video_quality_.get()));
    s.store_class_end();
  }
}

importContacts::importContacts()
  : contacts_()
{}

importContacts::importContacts(array<object_ptr<importedContact>> &&contacts_)
  : contacts_(std::move(contacts_))
{}

const std::int32_t importContacts::ID;

void importContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "importContacts");
    { s.store_vector_begin("contacts", contacts_.size()); for (const auto &_value : contacts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

leaveGroupCall::leaveGroupCall()
  : group_call_id_()
{}

leaveGroupCall::leaveGroupCall(int32 group_call_id_)
  : group_call_id_(group_call_id_)
{}

const std::int32_t leaveGroupCall::ID;

void leaveGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "leaveGroupCall");
    s.store_field("group_call_id", group_call_id_);
    s.store_class_end();
  }
}

logOut::logOut() {
}

const std::int32_t logOut::ID;

void logOut::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "logOut");
    s.store_class_end();
  }
}

openChat::openChat()
  : chat_id_()
{}

openChat::openChat(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t openChat::ID;

void openChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "openChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

removeInstalledBackground::removeInstalledBackground()
  : background_id_()
{}

removeInstalledBackground::removeInstalledBackground(int64 background_id_)
  : background_id_(background_id_)
{}

const std::int32_t removeInstalledBackground::ID;

void removeInstalledBackground::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeInstalledBackground");
    s.store_field("background_id", background_id_);
    s.store_class_end();
  }
}

removeNotificationGroup::removeNotificationGroup()
  : notification_group_id_()
  , max_notification_id_()
{}

removeNotificationGroup::removeNotificationGroup(int32 notification_group_id_, int32 max_notification_id_)
  : notification_group_id_(notification_group_id_)
  , max_notification_id_(max_notification_id_)
{}

const std::int32_t removeNotificationGroup::ID;

void removeNotificationGroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeNotificationGroup");
    s.store_field("notification_group_id", notification_group_id_);
    s.store_field("max_notification_id", max_notification_id_);
    s.store_class_end();
  }
}

removePendingPaidMessageReactions::removePendingPaidMessageReactions()
  : chat_id_()
  , message_id_()
{}

removePendingPaidMessageReactions::removePendingPaidMessageReactions(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t removePendingPaidMessageReactions::ID;

void removePendingPaidMessageReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removePendingPaidMessageReactions");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

removeSearchedForTag::removeSearchedForTag()
  : tag_()
{}

removeSearchedForTag::removeSearchedForTag(string const &tag_)
  : tag_(tag_)
{}

const std::int32_t removeSearchedForTag::ID;

void removeSearchedForTag::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeSearchedForTag");
    s.store_field("tag", tag_);
    s.store_class_end();
  }
}

reportPhoneNumberCodeMissing::reportPhoneNumberCodeMissing()
  : mobile_network_code_()
{}

reportPhoneNumberCodeMissing::reportPhoneNumberCodeMissing(string const &mobile_network_code_)
  : mobile_network_code_(mobile_network_code_)
{}

const std::int32_t reportPhoneNumberCodeMissing::ID;

void reportPhoneNumberCodeMissing::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportPhoneNumberCodeMissing");
    s.store_field("mobile_network_code", mobile_network_code_);
    s.store_class_end();
  }
}

resetNetworkStatistics::resetNetworkStatistics() {
}

const std::int32_t resetNetworkStatistics::ID;

void resetNetworkStatistics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "resetNetworkStatistics");
    s.store_class_end();
  }
}

searchBackground::searchBackground()
  : name_()
{}

searchBackground::searchBackground(string const &name_)
  : name_(name_)
{}

const std::int32_t searchBackground::ID;

void searchBackground::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchBackground");
    s.store_field("name", name_);
    s.store_class_end();
  }
}

searchGiftsForResale::searchGiftsForResale()
  : gift_id_()
  , order_()
  , attributes_()
  , offset_()
  , limit_()
{}

searchGiftsForResale::searchGiftsForResale(int64 gift_id_, object_ptr<GiftForResaleOrder> &&order_, array<object_ptr<UpgradedGiftAttributeId>> &&attributes_, string const &offset_, int32 limit_)
  : gift_id_(gift_id_)
  , order_(std::move(order_))
  , attributes_(std::move(attributes_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t searchGiftsForResale::ID;

void searchGiftsForResale::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchGiftsForResale");
    s.store_field("gift_id", gift_id_);
    s.store_object_field("order", static_cast<const BaseObject *>(order_.get()));
    { s.store_vector_begin("attributes", attributes_.size()); for (const auto &_value : attributes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

sellGift::sellGift()
  : business_connection_id_()
  , received_gift_id_()
{}

sellGift::sellGift(string const &business_connection_id_, string const &received_gift_id_)
  : business_connection_id_(business_connection_id_)
  , received_gift_id_(received_gift_id_)
{}

const std::int32_t sellGift::ID;

void sellGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sellGift");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_field("received_gift_id", received_gift_id_);
    s.store_class_end();
  }
}

sendBotStartMessage::sendBotStartMessage()
  : bot_user_id_()
  , chat_id_()
  , parameter_()
{}

sendBotStartMessage::sendBotStartMessage(int53 bot_user_id_, int53 chat_id_, string const &parameter_)
  : bot_user_id_(bot_user_id_)
  , chat_id_(chat_id_)
  , parameter_(parameter_)
{}

const std::int32_t sendBotStartMessage::ID;

void sendBotStartMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendBotStartMessage");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("chat_id", chat_id_);
    s.store_field("parameter", parameter_);
    s.store_class_end();
  }
}

setBusinessAccountProfilePhoto::setBusinessAccountProfilePhoto()
  : business_connection_id_()
  , photo_()
  , is_public_()
{}

setBusinessAccountProfilePhoto::setBusinessAccountProfilePhoto(string const &business_connection_id_, object_ptr<InputChatPhoto> &&photo_, bool is_public_)
  : business_connection_id_(business_connection_id_)
  , photo_(std::move(photo_))
  , is_public_(is_public_)
{}

const std::int32_t setBusinessAccountProfilePhoto::ID;

void setBusinessAccountProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setBusinessAccountProfilePhoto");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_field("is_public", is_public_);
    s.store_class_end();
  }
}

setCustomEmojiStickerSetThumbnail::setCustomEmojiStickerSetThumbnail()
  : name_()
  , custom_emoji_id_()
{}

setCustomEmojiStickerSetThumbnail::setCustomEmojiStickerSetThumbnail(string const &name_, int64 custom_emoji_id_)
  : name_(name_)
  , custom_emoji_id_(custom_emoji_id_)
{}

const std::int32_t setCustomEmojiStickerSetThumbnail::ID;

void setCustomEmojiStickerSetThumbnail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setCustomEmojiStickerSetThumbnail");
    s.store_field("name", name_);
    s.store_field("custom_emoji_id", custom_emoji_id_);
    s.store_class_end();
  }
}

setGroupCallParticipantVolumeLevel::setGroupCallParticipantVolumeLevel()
  : group_call_id_()
  , participant_id_()
  , volume_level_()
{}

setGroupCallParticipantVolumeLevel::setGroupCallParticipantVolumeLevel(int32 group_call_id_, object_ptr<MessageSender> &&participant_id_, int32 volume_level_)
  : group_call_id_(group_call_id_)
  , participant_id_(std::move(participant_id_))
  , volume_level_(volume_level_)
{}

const std::int32_t setGroupCallParticipantVolumeLevel::ID;

void setGroupCallParticipantVolumeLevel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setGroupCallParticipantVolumeLevel");
    s.store_field("group_call_id", group_call_id_);
    s.store_object_field("participant_id", static_cast<const BaseObject *>(participant_id_.get()));
    s.store_field("volume_level", volume_level_);
    s.store_class_end();
  }
}

setPersonalChat::setPersonalChat()
  : chat_id_()
{}

setPersonalChat::setPersonalChat(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t setPersonalChat::ID;

void setPersonalChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setPersonalChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

setScopeNotificationSettings::setScopeNotificationSettings()
  : scope_()
  , notification_settings_()
{}

setScopeNotificationSettings::setScopeNotificationSettings(object_ptr<NotificationSettingsScope> &&scope_, object_ptr<scopeNotificationSettings> &&notification_settings_)
  : scope_(std::move(scope_))
  , notification_settings_(std::move(notification_settings_))
{}

const std::int32_t setScopeNotificationSettings::ID;

void setScopeNotificationSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setScopeNotificationSettings");
    s.store_object_field("scope", static_cast<const BaseObject *>(scope_.get()));
    s.store_object_field("notification_settings", static_cast<const BaseObject *>(notification_settings_.get()));
    s.store_class_end();
  }
}

setStickerEmojis::setStickerEmojis()
  : sticker_()
  , emojis_()
{}

setStickerEmojis::setStickerEmojis(object_ptr<InputFile> &&sticker_, string const &emojis_)
  : sticker_(std::move(sticker_))
  , emojis_(emojis_)
{}

const std::int32_t setStickerEmojis::ID;

void setStickerEmojis::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setStickerEmojis");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_field("emojis", emojis_);
    s.store_class_end();
  }
}

setStoryPrivacySettings::setStoryPrivacySettings()
  : story_id_()
  , privacy_settings_()
{}

setStoryPrivacySettings::setStoryPrivacySettings(int32 story_id_, object_ptr<StoryPrivacySettings> &&privacy_settings_)
  : story_id_(story_id_)
  , privacy_settings_(std::move(privacy_settings_))
{}

const std::int32_t setStoryPrivacySettings::ID;

void setStoryPrivacySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setStoryPrivacySettings");
    s.store_field("story_id", story_id_);
    s.store_object_field("privacy_settings", static_cast<const BaseObject *>(privacy_settings_.get()));
    s.store_class_end();
  }
}

setStoryReaction::setStoryReaction()
  : story_poster_chat_id_()
  , story_id_()
  , reaction_type_()
  , update_recent_reactions_()
{}

setStoryReaction::setStoryReaction(int53 story_poster_chat_id_, int32 story_id_, object_ptr<ReactionType> &&reaction_type_, bool update_recent_reactions_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , story_id_(story_id_)
  , reaction_type_(std::move(reaction_type_))
  , update_recent_reactions_(update_recent_reactions_)
{}

const std::int32_t setStoryReaction::ID;

void setStoryReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setStoryReaction");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("story_id", story_id_);
    s.store_object_field("reaction_type", static_cast<const BaseObject *>(reaction_type_.get()));
    s.store_field("update_recent_reactions", update_recent_reactions_);
    s.store_class_end();
  }
}

setUserPersonalProfilePhoto::setUserPersonalProfilePhoto()
  : user_id_()
  , photo_()
{}

setUserPersonalProfilePhoto::setUserPersonalProfilePhoto(int53 user_id_, object_ptr<InputChatPhoto> &&photo_)
  : user_id_(user_id_)
  , photo_(std::move(photo_))
{}

const std::int32_t setUserPersonalProfilePhoto::ID;

void setUserPersonalProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setUserPersonalProfilePhoto");
    s.store_field("user_id", user_id_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

toggleUsernameIsActive::toggleUsernameIsActive()
  : username_()
  , is_active_()
{}

toggleUsernameIsActive::toggleUsernameIsActive(string const &username_, bool is_active_)
  : username_(username_)
  , is_active_(is_active_)
{}

const std::int32_t toggleUsernameIsActive::ID;

void toggleUsernameIsActive::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleUsernameIsActive");
    s.store_field("username", username_);
    s.store_field("is_active", is_active_);
    s.store_class_end();
  }
}

toggleVideoChatMuteNewParticipants::toggleVideoChatMuteNewParticipants()
  : group_call_id_()
  , mute_new_participants_()
{}

toggleVideoChatMuteNewParticipants::toggleVideoChatMuteNewParticipants(int32 group_call_id_, bool mute_new_participants_)
  : group_call_id_(group_call_id_)
  , mute_new_participants_(mute_new_participants_)
{}

const std::int32_t toggleVideoChatMuteNewParticipants::ID;

void toggleVideoChatMuteNewParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleVideoChatMuteNewParticipants");
    s.store_field("group_call_id", group_call_id_);
    s.store_field("mute_new_participants", mute_new_participants_);
    s.store_class_end();
  }
}

uploadStickerFile::uploadStickerFile()
  : user_id_()
  , sticker_format_()
  , sticker_()
{}

uploadStickerFile::uploadStickerFile(int53 user_id_, object_ptr<StickerFormat> &&sticker_format_, object_ptr<InputFile> &&sticker_)
  : user_id_(user_id_)
  , sticker_format_(std::move(sticker_format_))
  , sticker_(std::move(sticker_))
{}

const std::int32_t uploadStickerFile::ID;

void uploadStickerFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "uploadStickerFile");
    s.store_field("user_id", user_id_);
    s.store_object_field("sticker_format", static_cast<const BaseObject *>(sticker_format_.get()));
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

viewTrendingStickerSets::viewTrendingStickerSets()
  : sticker_set_ids_()
{}

viewTrendingStickerSets::viewTrendingStickerSets(array<int64> &&sticker_set_ids_)
  : sticker_set_ids_(std::move(sticker_set_ids_))
{}

const std::int32_t viewTrendingStickerSets::ID;

void viewTrendingStickerSets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "viewTrendingStickerSets");
    { s.store_vector_begin("sticker_set_ids", sticker_set_ids_.size()); for (const auto &_value : sticker_set_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}
}  // namespace td_api
}  // namespace td
