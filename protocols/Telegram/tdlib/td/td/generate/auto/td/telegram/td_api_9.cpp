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


alternativeVideo::alternativeVideo()
  : id_()
  , width_()
  , height_()
  , codec_()
  , hls_file_()
  , video_()
{}

alternativeVideo::alternativeVideo(int64 id_, int32 width_, int32 height_, string const &codec_, object_ptr<file> &&hls_file_, object_ptr<file> &&video_)
  : id_(id_)
  , width_(width_)
  , height_(height_)
  , codec_(codec_)
  , hls_file_(std::move(hls_file_))
  , video_(std::move(video_))
{}

const std::int32_t alternativeVideo::ID;

void alternativeVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "alternativeVideo");
    s.store_field("id", id_);
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_field("codec", codec_);
    s.store_object_field("hls_file", static_cast<const BaseObject *>(hls_file_.get()));
    s.store_object_field("video", static_cast<const BaseObject *>(video_.get()));
    s.store_class_end();
  }
}

background::background()
  : id_()
  , is_default_()
  , is_dark_()
  , name_()
  , document_()
  , type_()
{}

background::background(int64 id_, bool is_default_, bool is_dark_, string const &name_, object_ptr<document> &&document_, object_ptr<BackgroundType> &&type_)
  : id_(id_)
  , is_default_(is_default_)
  , is_dark_(is_dark_)
  , name_(name_)
  , document_(std::move(document_))
  , type_(std::move(type_))
{}

const std::int32_t background::ID;

void background::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "background");
    s.store_field("id", id_);
    s.store_field("is_default", is_default_);
    s.store_field("is_dark", is_dark_);
    s.store_field("name", name_);
    s.store_object_field("document", static_cast<const BaseObject *>(document_.get()));
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

basicGroupFullInfo::basicGroupFullInfo()
  : photo_()
  , description_()
  , creator_user_id_()
  , members_()
  , can_hide_members_()
  , can_toggle_aggressive_anti_spam_()
  , invite_link_()
  , bot_commands_()
{}

basicGroupFullInfo::basicGroupFullInfo(object_ptr<chatPhoto> &&photo_, string const &description_, int53 creator_user_id_, array<object_ptr<chatMember>> &&members_, bool can_hide_members_, bool can_toggle_aggressive_anti_spam_, object_ptr<chatInviteLink> &&invite_link_, array<object_ptr<botCommands>> &&bot_commands_)
  : photo_(std::move(photo_))
  , description_(description_)
  , creator_user_id_(creator_user_id_)
  , members_(std::move(members_))
  , can_hide_members_(can_hide_members_)
  , can_toggle_aggressive_anti_spam_(can_toggle_aggressive_anti_spam_)
  , invite_link_(std::move(invite_link_))
  , bot_commands_(std::move(bot_commands_))
{}

const std::int32_t basicGroupFullInfo::ID;

void basicGroupFullInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "basicGroupFullInfo");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_field("description", description_);
    s.store_field("creator_user_id", creator_user_id_);
    { s.store_vector_begin("members", members_.size()); for (const auto &_value : members_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("can_hide_members", can_hide_members_);
    s.store_field("can_toggle_aggressive_anti_spam", can_toggle_aggressive_anti_spam_);
    s.store_object_field("invite_link", static_cast<const BaseObject *>(invite_link_.get()));
    { s.store_vector_begin("bot_commands", bot_commands_.size()); for (const auto &_value : bot_commands_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

businessBotRights::businessBotRights()
  : can_reply_()
  , can_read_messages_()
  , can_delete_sent_messages_()
  , can_delete_all_messages_()
  , can_edit_name_()
  , can_edit_bio_()
  , can_edit_profile_photo_()
  , can_edit_username_()
  , can_view_gifts_and_stars_()
  , can_sell_gifts_()
  , can_change_gift_settings_()
  , can_transfer_and_upgrade_gifts_()
  , can_transfer_stars_()
  , can_manage_stories_()
{}

businessBotRights::businessBotRights(bool can_reply_, bool can_read_messages_, bool can_delete_sent_messages_, bool can_delete_all_messages_, bool can_edit_name_, bool can_edit_bio_, bool can_edit_profile_photo_, bool can_edit_username_, bool can_view_gifts_and_stars_, bool can_sell_gifts_, bool can_change_gift_settings_, bool can_transfer_and_upgrade_gifts_, bool can_transfer_stars_, bool can_manage_stories_)
  : can_reply_(can_reply_)
  , can_read_messages_(can_read_messages_)
  , can_delete_sent_messages_(can_delete_sent_messages_)
  , can_delete_all_messages_(can_delete_all_messages_)
  , can_edit_name_(can_edit_name_)
  , can_edit_bio_(can_edit_bio_)
  , can_edit_profile_photo_(can_edit_profile_photo_)
  , can_edit_username_(can_edit_username_)
  , can_view_gifts_and_stars_(can_view_gifts_and_stars_)
  , can_sell_gifts_(can_sell_gifts_)
  , can_change_gift_settings_(can_change_gift_settings_)
  , can_transfer_and_upgrade_gifts_(can_transfer_and_upgrade_gifts_)
  , can_transfer_stars_(can_transfer_stars_)
  , can_manage_stories_(can_manage_stories_)
{}

const std::int32_t businessBotRights::ID;

void businessBotRights::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessBotRights");
    s.store_field("can_reply", can_reply_);
    s.store_field("can_read_messages", can_read_messages_);
    s.store_field("can_delete_sent_messages", can_delete_sent_messages_);
    s.store_field("can_delete_all_messages", can_delete_all_messages_);
    s.store_field("can_edit_name", can_edit_name_);
    s.store_field("can_edit_bio", can_edit_bio_);
    s.store_field("can_edit_profile_photo", can_edit_profile_photo_);
    s.store_field("can_edit_username", can_edit_username_);
    s.store_field("can_view_gifts_and_stars", can_view_gifts_and_stars_);
    s.store_field("can_sell_gifts", can_sell_gifts_);
    s.store_field("can_change_gift_settings", can_change_gift_settings_);
    s.store_field("can_transfer_and_upgrade_gifts", can_transfer_and_upgrade_gifts_);
    s.store_field("can_transfer_stars", can_transfer_stars_);
    s.store_field("can_manage_stories", can_manage_stories_);
    s.store_class_end();
  }
}

businessConnectedBot::businessConnectedBot()
  : bot_user_id_()
  , recipients_()
  , rights_()
{}

businessConnectedBot::businessConnectedBot(int53 bot_user_id_, object_ptr<businessRecipients> &&recipients_, object_ptr<businessBotRights> &&rights_)
  : bot_user_id_(bot_user_id_)
  , recipients_(std::move(recipients_))
  , rights_(std::move(rights_))
{}

const std::int32_t businessConnectedBot::ID;

void businessConnectedBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessConnectedBot");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_object_field("recipients", static_cast<const BaseObject *>(recipients_.get()));
    s.store_object_field("rights", static_cast<const BaseObject *>(rights_.get()));
    s.store_class_end();
  }
}

chat::chat()
  : id_()
  , type_()
  , title_()
  , photo_()
  , accent_color_id_()
  , background_custom_emoji_id_()
  , upgraded_gift_colors_()
  , profile_accent_color_id_()
  , profile_background_custom_emoji_id_()
  , permissions_()
  , last_message_()
  , positions_()
  , chat_lists_()
  , message_sender_id_()
  , block_list_()
  , has_protected_content_()
  , is_translatable_()
  , is_marked_as_unread_()
  , view_as_topics_()
  , has_scheduled_messages_()
  , can_be_deleted_only_for_self_()
  , can_be_deleted_for_all_users_()
  , can_be_reported_()
  , default_disable_notification_()
  , unread_count_()
  , last_read_inbox_message_id_()
  , last_read_outbox_message_id_()
  , unread_mention_count_()
  , unread_reaction_count_()
  , notification_settings_()
  , available_reactions_()
  , message_auto_delete_time_()
  , emoji_status_()
  , background_()
  , theme_()
  , action_bar_()
  , business_bot_manage_bar_()
  , video_chat_()
  , pending_join_requests_()
  , reply_markup_message_id_()
  , draft_message_()
  , client_data_()
{}

chat::chat(int53 id_, object_ptr<ChatType> &&type_, string const &title_, object_ptr<chatPhotoInfo> &&photo_, int32 accent_color_id_, int64 background_custom_emoji_id_, object_ptr<upgradedGiftColors> &&upgraded_gift_colors_, int32 profile_accent_color_id_, int64 profile_background_custom_emoji_id_, object_ptr<chatPermissions> &&permissions_, object_ptr<message> &&last_message_, array<object_ptr<chatPosition>> &&positions_, array<object_ptr<ChatList>> &&chat_lists_, object_ptr<MessageSender> &&message_sender_id_, object_ptr<BlockList> &&block_list_, bool has_protected_content_, bool is_translatable_, bool is_marked_as_unread_, bool view_as_topics_, bool has_scheduled_messages_, bool can_be_deleted_only_for_self_, bool can_be_deleted_for_all_users_, bool can_be_reported_, bool default_disable_notification_, int32 unread_count_, int53 last_read_inbox_message_id_, int53 last_read_outbox_message_id_, int32 unread_mention_count_, int32 unread_reaction_count_, object_ptr<chatNotificationSettings> &&notification_settings_, object_ptr<ChatAvailableReactions> &&available_reactions_, int32 message_auto_delete_time_, object_ptr<emojiStatus> &&emoji_status_, object_ptr<chatBackground> &&background_, object_ptr<ChatTheme> &&theme_, object_ptr<ChatActionBar> &&action_bar_, object_ptr<businessBotManageBar> &&business_bot_manage_bar_, object_ptr<videoChat> &&video_chat_, object_ptr<chatJoinRequestsInfo> &&pending_join_requests_, int53 reply_markup_message_id_, object_ptr<draftMessage> &&draft_message_, string const &client_data_)
  : id_(id_)
  , type_(std::move(type_))
  , title_(title_)
  , photo_(std::move(photo_))
  , accent_color_id_(accent_color_id_)
  , background_custom_emoji_id_(background_custom_emoji_id_)
  , upgraded_gift_colors_(std::move(upgraded_gift_colors_))
  , profile_accent_color_id_(profile_accent_color_id_)
  , profile_background_custom_emoji_id_(profile_background_custom_emoji_id_)
  , permissions_(std::move(permissions_))
  , last_message_(std::move(last_message_))
  , positions_(std::move(positions_))
  , chat_lists_(std::move(chat_lists_))
  , message_sender_id_(std::move(message_sender_id_))
  , block_list_(std::move(block_list_))
  , has_protected_content_(has_protected_content_)
  , is_translatable_(is_translatable_)
  , is_marked_as_unread_(is_marked_as_unread_)
  , view_as_topics_(view_as_topics_)
  , has_scheduled_messages_(has_scheduled_messages_)
  , can_be_deleted_only_for_self_(can_be_deleted_only_for_self_)
  , can_be_deleted_for_all_users_(can_be_deleted_for_all_users_)
  , can_be_reported_(can_be_reported_)
  , default_disable_notification_(default_disable_notification_)
  , unread_count_(unread_count_)
  , last_read_inbox_message_id_(last_read_inbox_message_id_)
  , last_read_outbox_message_id_(last_read_outbox_message_id_)
  , unread_mention_count_(unread_mention_count_)
  , unread_reaction_count_(unread_reaction_count_)
  , notification_settings_(std::move(notification_settings_))
  , available_reactions_(std::move(available_reactions_))
  , message_auto_delete_time_(message_auto_delete_time_)
  , emoji_status_(std::move(emoji_status_))
  , background_(std::move(background_))
  , theme_(std::move(theme_))
  , action_bar_(std::move(action_bar_))
  , business_bot_manage_bar_(std::move(business_bot_manage_bar_))
  , video_chat_(std::move(video_chat_))
  , pending_join_requests_(std::move(pending_join_requests_))
  , reply_markup_message_id_(reply_markup_message_id_)
  , draft_message_(std::move(draft_message_))
  , client_data_(client_data_)
{}

const std::int32_t chat::ID;

void chat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chat");
    s.store_field("id", id_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("title", title_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_field("accent_color_id", accent_color_id_);
    s.store_field("background_custom_emoji_id", background_custom_emoji_id_);
    s.store_object_field("upgraded_gift_colors", static_cast<const BaseObject *>(upgraded_gift_colors_.get()));
    s.store_field("profile_accent_color_id", profile_accent_color_id_);
    s.store_field("profile_background_custom_emoji_id", profile_background_custom_emoji_id_);
    s.store_object_field("permissions", static_cast<const BaseObject *>(permissions_.get()));
    s.store_object_field("last_message", static_cast<const BaseObject *>(last_message_.get()));
    { s.store_vector_begin("positions", positions_.size()); for (const auto &_value : positions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chat_lists", chat_lists_.size()); for (const auto &_value : chat_lists_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("message_sender_id", static_cast<const BaseObject *>(message_sender_id_.get()));
    s.store_object_field("block_list", static_cast<const BaseObject *>(block_list_.get()));
    s.store_field("has_protected_content", has_protected_content_);
    s.store_field("is_translatable", is_translatable_);
    s.store_field("is_marked_as_unread", is_marked_as_unread_);
    s.store_field("view_as_topics", view_as_topics_);
    s.store_field("has_scheduled_messages", has_scheduled_messages_);
    s.store_field("can_be_deleted_only_for_self", can_be_deleted_only_for_self_);
    s.store_field("can_be_deleted_for_all_users", can_be_deleted_for_all_users_);
    s.store_field("can_be_reported", can_be_reported_);
    s.store_field("default_disable_notification", default_disable_notification_);
    s.store_field("unread_count", unread_count_);
    s.store_field("last_read_inbox_message_id", last_read_inbox_message_id_);
    s.store_field("last_read_outbox_message_id", last_read_outbox_message_id_);
    s.store_field("unread_mention_count", unread_mention_count_);
    s.store_field("unread_reaction_count", unread_reaction_count_);
    s.store_object_field("notification_settings", static_cast<const BaseObject *>(notification_settings_.get()));
    s.store_object_field("available_reactions", static_cast<const BaseObject *>(available_reactions_.get()));
    s.store_field("message_auto_delete_time", message_auto_delete_time_);
    s.store_object_field("emoji_status", static_cast<const BaseObject *>(emoji_status_.get()));
    s.store_object_field("background", static_cast<const BaseObject *>(background_.get()));
    s.store_object_field("theme", static_cast<const BaseObject *>(theme_.get()));
    s.store_object_field("action_bar", static_cast<const BaseObject *>(action_bar_.get()));
    s.store_object_field("business_bot_manage_bar", static_cast<const BaseObject *>(business_bot_manage_bar_.get()));
    s.store_object_field("video_chat", static_cast<const BaseObject *>(video_chat_.get()));
    s.store_object_field("pending_join_requests", static_cast<const BaseObject *>(pending_join_requests_.get()));
    s.store_field("reply_markup_message_id", reply_markup_message_id_);
    s.store_object_field("draft_message", static_cast<const BaseObject *>(draft_message_.get()));
    s.store_field("client_data", client_data_);
    s.store_class_end();
  }
}

chatBoostSourceGiftCode::chatBoostSourceGiftCode()
  : user_id_()
  , gift_code_()
{}

chatBoostSourceGiftCode::chatBoostSourceGiftCode(int53 user_id_, string const &gift_code_)
  : user_id_(user_id_)
  , gift_code_(gift_code_)
{}

const std::int32_t chatBoostSourceGiftCode::ID;

void chatBoostSourceGiftCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatBoostSourceGiftCode");
    s.store_field("user_id", user_id_);
    s.store_field("gift_code", gift_code_);
    s.store_class_end();
  }
}

chatBoostSourceGiveaway::chatBoostSourceGiveaway()
  : user_id_()
  , gift_code_()
  , star_count_()
  , giveaway_message_id_()
  , is_unclaimed_()
{}

chatBoostSourceGiveaway::chatBoostSourceGiveaway(int53 user_id_, string const &gift_code_, int53 star_count_, int53 giveaway_message_id_, bool is_unclaimed_)
  : user_id_(user_id_)
  , gift_code_(gift_code_)
  , star_count_(star_count_)
  , giveaway_message_id_(giveaway_message_id_)
  , is_unclaimed_(is_unclaimed_)
{}

const std::int32_t chatBoostSourceGiveaway::ID;

void chatBoostSourceGiveaway::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatBoostSourceGiveaway");
    s.store_field("user_id", user_id_);
    s.store_field("gift_code", gift_code_);
    s.store_field("star_count", star_count_);
    s.store_field("giveaway_message_id", giveaway_message_id_);
    s.store_field("is_unclaimed", is_unclaimed_);
    s.store_class_end();
  }
}

chatBoostSourcePremium::chatBoostSourcePremium()
  : user_id_()
{}

chatBoostSourcePremium::chatBoostSourcePremium(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t chatBoostSourcePremium::ID;

void chatBoostSourcePremium::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatBoostSourcePremium");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

chatEventMessageEdited::chatEventMessageEdited()
  : old_message_()
  , new_message_()
{}

chatEventMessageEdited::chatEventMessageEdited(object_ptr<message> &&old_message_, object_ptr<message> &&new_message_)
  : old_message_(std::move(old_message_))
  , new_message_(std::move(new_message_))
{}

const std::int32_t chatEventMessageEdited::ID;

void chatEventMessageEdited::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventMessageEdited");
    s.store_object_field("old_message", static_cast<const BaseObject *>(old_message_.get()));
    s.store_object_field("new_message", static_cast<const BaseObject *>(new_message_.get()));
    s.store_class_end();
  }
}

chatEventMessageDeleted::chatEventMessageDeleted()
  : message_()
  , can_report_anti_spam_false_positive_()
{}

chatEventMessageDeleted::chatEventMessageDeleted(object_ptr<message> &&message_, bool can_report_anti_spam_false_positive_)
  : message_(std::move(message_))
  , can_report_anti_spam_false_positive_(can_report_anti_spam_false_positive_)
{}

const std::int32_t chatEventMessageDeleted::ID;

void chatEventMessageDeleted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventMessageDeleted");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_field("can_report_anti_spam_false_positive", can_report_anti_spam_false_positive_);
    s.store_class_end();
  }
}

chatEventMessagePinned::chatEventMessagePinned()
  : message_()
{}

chatEventMessagePinned::chatEventMessagePinned(object_ptr<message> &&message_)
  : message_(std::move(message_))
{}

const std::int32_t chatEventMessagePinned::ID;

void chatEventMessagePinned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventMessagePinned");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

chatEventMessageUnpinned::chatEventMessageUnpinned()
  : message_()
{}

chatEventMessageUnpinned::chatEventMessageUnpinned(object_ptr<message> &&message_)
  : message_(std::move(message_))
{}

const std::int32_t chatEventMessageUnpinned::ID;

void chatEventMessageUnpinned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventMessageUnpinned");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

chatEventPollStopped::chatEventPollStopped()
  : message_()
{}

chatEventPollStopped::chatEventPollStopped(object_ptr<message> &&message_)
  : message_(std::move(message_))
{}

const std::int32_t chatEventPollStopped::ID;

void chatEventPollStopped::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventPollStopped");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

chatEventMemberJoined::chatEventMemberJoined() {
}

const std::int32_t chatEventMemberJoined::ID;

void chatEventMemberJoined::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventMemberJoined");
    s.store_class_end();
  }
}

chatEventMemberJoinedByInviteLink::chatEventMemberJoinedByInviteLink()
  : invite_link_()
  , via_chat_folder_invite_link_()
{}

chatEventMemberJoinedByInviteLink::chatEventMemberJoinedByInviteLink(object_ptr<chatInviteLink> &&invite_link_, bool via_chat_folder_invite_link_)
  : invite_link_(std::move(invite_link_))
  , via_chat_folder_invite_link_(via_chat_folder_invite_link_)
{}

const std::int32_t chatEventMemberJoinedByInviteLink::ID;

void chatEventMemberJoinedByInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventMemberJoinedByInviteLink");
    s.store_object_field("invite_link", static_cast<const BaseObject *>(invite_link_.get()));
    s.store_field("via_chat_folder_invite_link", via_chat_folder_invite_link_);
    s.store_class_end();
  }
}

chatEventMemberJoinedByRequest::chatEventMemberJoinedByRequest()
  : approver_user_id_()
  , invite_link_()
{}

chatEventMemberJoinedByRequest::chatEventMemberJoinedByRequest(int53 approver_user_id_, object_ptr<chatInviteLink> &&invite_link_)
  : approver_user_id_(approver_user_id_)
  , invite_link_(std::move(invite_link_))
{}

const std::int32_t chatEventMemberJoinedByRequest::ID;

void chatEventMemberJoinedByRequest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventMemberJoinedByRequest");
    s.store_field("approver_user_id", approver_user_id_);
    s.store_object_field("invite_link", static_cast<const BaseObject *>(invite_link_.get()));
    s.store_class_end();
  }
}

chatEventMemberInvited::chatEventMemberInvited()
  : user_id_()
  , status_()
{}

chatEventMemberInvited::chatEventMemberInvited(int53 user_id_, object_ptr<ChatMemberStatus> &&status_)
  : user_id_(user_id_)
  , status_(std::move(status_))
{}

const std::int32_t chatEventMemberInvited::ID;

void chatEventMemberInvited::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventMemberInvited");
    s.store_field("user_id", user_id_);
    s.store_object_field("status", static_cast<const BaseObject *>(status_.get()));
    s.store_class_end();
  }
}

chatEventMemberLeft::chatEventMemberLeft() {
}

const std::int32_t chatEventMemberLeft::ID;

void chatEventMemberLeft::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventMemberLeft");
    s.store_class_end();
  }
}

chatEventMemberPromoted::chatEventMemberPromoted()
  : user_id_()
  , old_status_()
  , new_status_()
{}

chatEventMemberPromoted::chatEventMemberPromoted(int53 user_id_, object_ptr<ChatMemberStatus> &&old_status_, object_ptr<ChatMemberStatus> &&new_status_)
  : user_id_(user_id_)
  , old_status_(std::move(old_status_))
  , new_status_(std::move(new_status_))
{}

const std::int32_t chatEventMemberPromoted::ID;

void chatEventMemberPromoted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventMemberPromoted");
    s.store_field("user_id", user_id_);
    s.store_object_field("old_status", static_cast<const BaseObject *>(old_status_.get()));
    s.store_object_field("new_status", static_cast<const BaseObject *>(new_status_.get()));
    s.store_class_end();
  }
}

chatEventMemberRestricted::chatEventMemberRestricted()
  : member_id_()
  , old_status_()
  , new_status_()
{}

chatEventMemberRestricted::chatEventMemberRestricted(object_ptr<MessageSender> &&member_id_, object_ptr<ChatMemberStatus> &&old_status_, object_ptr<ChatMemberStatus> &&new_status_)
  : member_id_(std::move(member_id_))
  , old_status_(std::move(old_status_))
  , new_status_(std::move(new_status_))
{}

const std::int32_t chatEventMemberRestricted::ID;

void chatEventMemberRestricted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventMemberRestricted");
    s.store_object_field("member_id", static_cast<const BaseObject *>(member_id_.get()));
    s.store_object_field("old_status", static_cast<const BaseObject *>(old_status_.get()));
    s.store_object_field("new_status", static_cast<const BaseObject *>(new_status_.get()));
    s.store_class_end();
  }
}

chatEventMemberSubscriptionExtended::chatEventMemberSubscriptionExtended()
  : user_id_()
  , old_status_()
  , new_status_()
{}

chatEventMemberSubscriptionExtended::chatEventMemberSubscriptionExtended(int53 user_id_, object_ptr<ChatMemberStatus> &&old_status_, object_ptr<ChatMemberStatus> &&new_status_)
  : user_id_(user_id_)
  , old_status_(std::move(old_status_))
  , new_status_(std::move(new_status_))
{}

const std::int32_t chatEventMemberSubscriptionExtended::ID;

void chatEventMemberSubscriptionExtended::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventMemberSubscriptionExtended");
    s.store_field("user_id", user_id_);
    s.store_object_field("old_status", static_cast<const BaseObject *>(old_status_.get()));
    s.store_object_field("new_status", static_cast<const BaseObject *>(new_status_.get()));
    s.store_class_end();
  }
}

chatEventAvailableReactionsChanged::chatEventAvailableReactionsChanged()
  : old_available_reactions_()
  , new_available_reactions_()
{}

chatEventAvailableReactionsChanged::chatEventAvailableReactionsChanged(object_ptr<ChatAvailableReactions> &&old_available_reactions_, object_ptr<ChatAvailableReactions> &&new_available_reactions_)
  : old_available_reactions_(std::move(old_available_reactions_))
  , new_available_reactions_(std::move(new_available_reactions_))
{}

const std::int32_t chatEventAvailableReactionsChanged::ID;

void chatEventAvailableReactionsChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventAvailableReactionsChanged");
    s.store_object_field("old_available_reactions", static_cast<const BaseObject *>(old_available_reactions_.get()));
    s.store_object_field("new_available_reactions", static_cast<const BaseObject *>(new_available_reactions_.get()));
    s.store_class_end();
  }
}

chatEventBackgroundChanged::chatEventBackgroundChanged()
  : old_background_()
  , new_background_()
{}

chatEventBackgroundChanged::chatEventBackgroundChanged(object_ptr<chatBackground> &&old_background_, object_ptr<chatBackground> &&new_background_)
  : old_background_(std::move(old_background_))
  , new_background_(std::move(new_background_))
{}

const std::int32_t chatEventBackgroundChanged::ID;

void chatEventBackgroundChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventBackgroundChanged");
    s.store_object_field("old_background", static_cast<const BaseObject *>(old_background_.get()));
    s.store_object_field("new_background", static_cast<const BaseObject *>(new_background_.get()));
    s.store_class_end();
  }
}

chatEventDescriptionChanged::chatEventDescriptionChanged()
  : old_description_()
  , new_description_()
{}

chatEventDescriptionChanged::chatEventDescriptionChanged(string const &old_description_, string const &new_description_)
  : old_description_(old_description_)
  , new_description_(new_description_)
{}

const std::int32_t chatEventDescriptionChanged::ID;

void chatEventDescriptionChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventDescriptionChanged");
    s.store_field("old_description", old_description_);
    s.store_field("new_description", new_description_);
    s.store_class_end();
  }
}

chatEventEmojiStatusChanged::chatEventEmojiStatusChanged()
  : old_emoji_status_()
  , new_emoji_status_()
{}

chatEventEmojiStatusChanged::chatEventEmojiStatusChanged(object_ptr<emojiStatus> &&old_emoji_status_, object_ptr<emojiStatus> &&new_emoji_status_)
  : old_emoji_status_(std::move(old_emoji_status_))
  , new_emoji_status_(std::move(new_emoji_status_))
{}

const std::int32_t chatEventEmojiStatusChanged::ID;

void chatEventEmojiStatusChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventEmojiStatusChanged");
    s.store_object_field("old_emoji_status", static_cast<const BaseObject *>(old_emoji_status_.get()));
    s.store_object_field("new_emoji_status", static_cast<const BaseObject *>(new_emoji_status_.get()));
    s.store_class_end();
  }
}

chatEventLinkedChatChanged::chatEventLinkedChatChanged()
  : old_linked_chat_id_()
  , new_linked_chat_id_()
{}

chatEventLinkedChatChanged::chatEventLinkedChatChanged(int53 old_linked_chat_id_, int53 new_linked_chat_id_)
  : old_linked_chat_id_(old_linked_chat_id_)
  , new_linked_chat_id_(new_linked_chat_id_)
{}

const std::int32_t chatEventLinkedChatChanged::ID;

void chatEventLinkedChatChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventLinkedChatChanged");
    s.store_field("old_linked_chat_id", old_linked_chat_id_);
    s.store_field("new_linked_chat_id", new_linked_chat_id_);
    s.store_class_end();
  }
}

chatEventLocationChanged::chatEventLocationChanged()
  : old_location_()
  , new_location_()
{}

chatEventLocationChanged::chatEventLocationChanged(object_ptr<chatLocation> &&old_location_, object_ptr<chatLocation> &&new_location_)
  : old_location_(std::move(old_location_))
  , new_location_(std::move(new_location_))
{}

const std::int32_t chatEventLocationChanged::ID;

void chatEventLocationChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventLocationChanged");
    s.store_object_field("old_location", static_cast<const BaseObject *>(old_location_.get()));
    s.store_object_field("new_location", static_cast<const BaseObject *>(new_location_.get()));
    s.store_class_end();
  }
}

chatEventMessageAutoDeleteTimeChanged::chatEventMessageAutoDeleteTimeChanged()
  : old_message_auto_delete_time_()
  , new_message_auto_delete_time_()
{}

chatEventMessageAutoDeleteTimeChanged::chatEventMessageAutoDeleteTimeChanged(int32 old_message_auto_delete_time_, int32 new_message_auto_delete_time_)
  : old_message_auto_delete_time_(old_message_auto_delete_time_)
  , new_message_auto_delete_time_(new_message_auto_delete_time_)
{}

const std::int32_t chatEventMessageAutoDeleteTimeChanged::ID;

void chatEventMessageAutoDeleteTimeChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventMessageAutoDeleteTimeChanged");
    s.store_field("old_message_auto_delete_time", old_message_auto_delete_time_);
    s.store_field("new_message_auto_delete_time", new_message_auto_delete_time_);
    s.store_class_end();
  }
}

chatEventPermissionsChanged::chatEventPermissionsChanged()
  : old_permissions_()
  , new_permissions_()
{}

chatEventPermissionsChanged::chatEventPermissionsChanged(object_ptr<chatPermissions> &&old_permissions_, object_ptr<chatPermissions> &&new_permissions_)
  : old_permissions_(std::move(old_permissions_))
  , new_permissions_(std::move(new_permissions_))
{}

const std::int32_t chatEventPermissionsChanged::ID;

void chatEventPermissionsChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventPermissionsChanged");
    s.store_object_field("old_permissions", static_cast<const BaseObject *>(old_permissions_.get()));
    s.store_object_field("new_permissions", static_cast<const BaseObject *>(new_permissions_.get()));
    s.store_class_end();
  }
}

chatEventPhotoChanged::chatEventPhotoChanged()
  : old_photo_()
  , new_photo_()
{}

chatEventPhotoChanged::chatEventPhotoChanged(object_ptr<chatPhoto> &&old_photo_, object_ptr<chatPhoto> &&new_photo_)
  : old_photo_(std::move(old_photo_))
  , new_photo_(std::move(new_photo_))
{}

const std::int32_t chatEventPhotoChanged::ID;

void chatEventPhotoChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventPhotoChanged");
    s.store_object_field("old_photo", static_cast<const BaseObject *>(old_photo_.get()));
    s.store_object_field("new_photo", static_cast<const BaseObject *>(new_photo_.get()));
    s.store_class_end();
  }
}

chatEventSlowModeDelayChanged::chatEventSlowModeDelayChanged()
  : old_slow_mode_delay_()
  , new_slow_mode_delay_()
{}

chatEventSlowModeDelayChanged::chatEventSlowModeDelayChanged(int32 old_slow_mode_delay_, int32 new_slow_mode_delay_)
  : old_slow_mode_delay_(old_slow_mode_delay_)
  , new_slow_mode_delay_(new_slow_mode_delay_)
{}

const std::int32_t chatEventSlowModeDelayChanged::ID;

void chatEventSlowModeDelayChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventSlowModeDelayChanged");
    s.store_field("old_slow_mode_delay", old_slow_mode_delay_);
    s.store_field("new_slow_mode_delay", new_slow_mode_delay_);
    s.store_class_end();
  }
}

chatEventStickerSetChanged::chatEventStickerSetChanged()
  : old_sticker_set_id_()
  , new_sticker_set_id_()
{}

chatEventStickerSetChanged::chatEventStickerSetChanged(int64 old_sticker_set_id_, int64 new_sticker_set_id_)
  : old_sticker_set_id_(old_sticker_set_id_)
  , new_sticker_set_id_(new_sticker_set_id_)
{}

const std::int32_t chatEventStickerSetChanged::ID;

void chatEventStickerSetChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventStickerSetChanged");
    s.store_field("old_sticker_set_id", old_sticker_set_id_);
    s.store_field("new_sticker_set_id", new_sticker_set_id_);
    s.store_class_end();
  }
}

chatEventCustomEmojiStickerSetChanged::chatEventCustomEmojiStickerSetChanged()
  : old_sticker_set_id_()
  , new_sticker_set_id_()
{}

chatEventCustomEmojiStickerSetChanged::chatEventCustomEmojiStickerSetChanged(int64 old_sticker_set_id_, int64 new_sticker_set_id_)
  : old_sticker_set_id_(old_sticker_set_id_)
  , new_sticker_set_id_(new_sticker_set_id_)
{}

const std::int32_t chatEventCustomEmojiStickerSetChanged::ID;

void chatEventCustomEmojiStickerSetChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventCustomEmojiStickerSetChanged");
    s.store_field("old_sticker_set_id", old_sticker_set_id_);
    s.store_field("new_sticker_set_id", new_sticker_set_id_);
    s.store_class_end();
  }
}

chatEventTitleChanged::chatEventTitleChanged()
  : old_title_()
  , new_title_()
{}

chatEventTitleChanged::chatEventTitleChanged(string const &old_title_, string const &new_title_)
  : old_title_(old_title_)
  , new_title_(new_title_)
{}

const std::int32_t chatEventTitleChanged::ID;

void chatEventTitleChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventTitleChanged");
    s.store_field("old_title", old_title_);
    s.store_field("new_title", new_title_);
    s.store_class_end();
  }
}

chatEventUsernameChanged::chatEventUsernameChanged()
  : old_username_()
  , new_username_()
{}

chatEventUsernameChanged::chatEventUsernameChanged(string const &old_username_, string const &new_username_)
  : old_username_(old_username_)
  , new_username_(new_username_)
{}

const std::int32_t chatEventUsernameChanged::ID;

void chatEventUsernameChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventUsernameChanged");
    s.store_field("old_username", old_username_);
    s.store_field("new_username", new_username_);
    s.store_class_end();
  }
}

chatEventActiveUsernamesChanged::chatEventActiveUsernamesChanged()
  : old_usernames_()
  , new_usernames_()
{}

chatEventActiveUsernamesChanged::chatEventActiveUsernamesChanged(array<string> &&old_usernames_, array<string> &&new_usernames_)
  : old_usernames_(std::move(old_usernames_))
  , new_usernames_(std::move(new_usernames_))
{}

const std::int32_t chatEventActiveUsernamesChanged::ID;

void chatEventActiveUsernamesChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventActiveUsernamesChanged");
    { s.store_vector_begin("old_usernames", old_usernames_.size()); for (const auto &_value : old_usernames_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("new_usernames", new_usernames_.size()); for (const auto &_value : new_usernames_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatEventAccentColorChanged::chatEventAccentColorChanged()
  : old_accent_color_id_()
  , old_background_custom_emoji_id_()
  , new_accent_color_id_()
  , new_background_custom_emoji_id_()
{}

chatEventAccentColorChanged::chatEventAccentColorChanged(int32 old_accent_color_id_, int64 old_background_custom_emoji_id_, int32 new_accent_color_id_, int64 new_background_custom_emoji_id_)
  : old_accent_color_id_(old_accent_color_id_)
  , old_background_custom_emoji_id_(old_background_custom_emoji_id_)
  , new_accent_color_id_(new_accent_color_id_)
  , new_background_custom_emoji_id_(new_background_custom_emoji_id_)
{}

const std::int32_t chatEventAccentColorChanged::ID;

void chatEventAccentColorChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventAccentColorChanged");
    s.store_field("old_accent_color_id", old_accent_color_id_);
    s.store_field("old_background_custom_emoji_id", old_background_custom_emoji_id_);
    s.store_field("new_accent_color_id", new_accent_color_id_);
    s.store_field("new_background_custom_emoji_id", new_background_custom_emoji_id_);
    s.store_class_end();
  }
}

chatEventProfileAccentColorChanged::chatEventProfileAccentColorChanged()
  : old_profile_accent_color_id_()
  , old_profile_background_custom_emoji_id_()
  , new_profile_accent_color_id_()
  , new_profile_background_custom_emoji_id_()
{}

chatEventProfileAccentColorChanged::chatEventProfileAccentColorChanged(int32 old_profile_accent_color_id_, int64 old_profile_background_custom_emoji_id_, int32 new_profile_accent_color_id_, int64 new_profile_background_custom_emoji_id_)
  : old_profile_accent_color_id_(old_profile_accent_color_id_)
  , old_profile_background_custom_emoji_id_(old_profile_background_custom_emoji_id_)
  , new_profile_accent_color_id_(new_profile_accent_color_id_)
  , new_profile_background_custom_emoji_id_(new_profile_background_custom_emoji_id_)
{}

const std::int32_t chatEventProfileAccentColorChanged::ID;

void chatEventProfileAccentColorChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventProfileAccentColorChanged");
    s.store_field("old_profile_accent_color_id", old_profile_accent_color_id_);
    s.store_field("old_profile_background_custom_emoji_id", old_profile_background_custom_emoji_id_);
    s.store_field("new_profile_accent_color_id", new_profile_accent_color_id_);
    s.store_field("new_profile_background_custom_emoji_id", new_profile_background_custom_emoji_id_);
    s.store_class_end();
  }
}

chatEventHasProtectedContentToggled::chatEventHasProtectedContentToggled()
  : has_protected_content_()
{}

chatEventHasProtectedContentToggled::chatEventHasProtectedContentToggled(bool has_protected_content_)
  : has_protected_content_(has_protected_content_)
{}

const std::int32_t chatEventHasProtectedContentToggled::ID;

void chatEventHasProtectedContentToggled::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventHasProtectedContentToggled");
    s.store_field("has_protected_content", has_protected_content_);
    s.store_class_end();
  }
}

chatEventInvitesToggled::chatEventInvitesToggled()
  : can_invite_users_()
{}

chatEventInvitesToggled::chatEventInvitesToggled(bool can_invite_users_)
  : can_invite_users_(can_invite_users_)
{}

const std::int32_t chatEventInvitesToggled::ID;

void chatEventInvitesToggled::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventInvitesToggled");
    s.store_field("can_invite_users", can_invite_users_);
    s.store_class_end();
  }
}

chatEventIsAllHistoryAvailableToggled::chatEventIsAllHistoryAvailableToggled()
  : is_all_history_available_()
{}

chatEventIsAllHistoryAvailableToggled::chatEventIsAllHistoryAvailableToggled(bool is_all_history_available_)
  : is_all_history_available_(is_all_history_available_)
{}

const std::int32_t chatEventIsAllHistoryAvailableToggled::ID;

void chatEventIsAllHistoryAvailableToggled::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventIsAllHistoryAvailableToggled");
    s.store_field("is_all_history_available", is_all_history_available_);
    s.store_class_end();
  }
}

chatEventHasAggressiveAntiSpamEnabledToggled::chatEventHasAggressiveAntiSpamEnabledToggled()
  : has_aggressive_anti_spam_enabled_()
{}

chatEventHasAggressiveAntiSpamEnabledToggled::chatEventHasAggressiveAntiSpamEnabledToggled(bool has_aggressive_anti_spam_enabled_)
  : has_aggressive_anti_spam_enabled_(has_aggressive_anti_spam_enabled_)
{}

const std::int32_t chatEventHasAggressiveAntiSpamEnabledToggled::ID;

void chatEventHasAggressiveAntiSpamEnabledToggled::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventHasAggressiveAntiSpamEnabledToggled");
    s.store_field("has_aggressive_anti_spam_enabled", has_aggressive_anti_spam_enabled_);
    s.store_class_end();
  }
}

chatEventSignMessagesToggled::chatEventSignMessagesToggled()
  : sign_messages_()
{}

chatEventSignMessagesToggled::chatEventSignMessagesToggled(bool sign_messages_)
  : sign_messages_(sign_messages_)
{}

const std::int32_t chatEventSignMessagesToggled::ID;

void chatEventSignMessagesToggled::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventSignMessagesToggled");
    s.store_field("sign_messages", sign_messages_);
    s.store_class_end();
  }
}

chatEventShowMessageSenderToggled::chatEventShowMessageSenderToggled()
  : show_message_sender_()
{}

chatEventShowMessageSenderToggled::chatEventShowMessageSenderToggled(bool show_message_sender_)
  : show_message_sender_(show_message_sender_)
{}

const std::int32_t chatEventShowMessageSenderToggled::ID;

void chatEventShowMessageSenderToggled::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventShowMessageSenderToggled");
    s.store_field("show_message_sender", show_message_sender_);
    s.store_class_end();
  }
}

chatEventAutomaticTranslationToggled::chatEventAutomaticTranslationToggled()
  : has_automatic_translation_()
{}

chatEventAutomaticTranslationToggled::chatEventAutomaticTranslationToggled(bool has_automatic_translation_)
  : has_automatic_translation_(has_automatic_translation_)
{}

const std::int32_t chatEventAutomaticTranslationToggled::ID;

void chatEventAutomaticTranslationToggled::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventAutomaticTranslationToggled");
    s.store_field("has_automatic_translation", has_automatic_translation_);
    s.store_class_end();
  }
}

chatEventInviteLinkEdited::chatEventInviteLinkEdited()
  : old_invite_link_()
  , new_invite_link_()
{}

chatEventInviteLinkEdited::chatEventInviteLinkEdited(object_ptr<chatInviteLink> &&old_invite_link_, object_ptr<chatInviteLink> &&new_invite_link_)
  : old_invite_link_(std::move(old_invite_link_))
  , new_invite_link_(std::move(new_invite_link_))
{}

const std::int32_t chatEventInviteLinkEdited::ID;

void chatEventInviteLinkEdited::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventInviteLinkEdited");
    s.store_object_field("old_invite_link", static_cast<const BaseObject *>(old_invite_link_.get()));
    s.store_object_field("new_invite_link", static_cast<const BaseObject *>(new_invite_link_.get()));
    s.store_class_end();
  }
}

chatEventInviteLinkRevoked::chatEventInviteLinkRevoked()
  : invite_link_()
{}

chatEventInviteLinkRevoked::chatEventInviteLinkRevoked(object_ptr<chatInviteLink> &&invite_link_)
  : invite_link_(std::move(invite_link_))
{}

const std::int32_t chatEventInviteLinkRevoked::ID;

void chatEventInviteLinkRevoked::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventInviteLinkRevoked");
    s.store_object_field("invite_link", static_cast<const BaseObject *>(invite_link_.get()));
    s.store_class_end();
  }
}

chatEventInviteLinkDeleted::chatEventInviteLinkDeleted()
  : invite_link_()
{}

chatEventInviteLinkDeleted::chatEventInviteLinkDeleted(object_ptr<chatInviteLink> &&invite_link_)
  : invite_link_(std::move(invite_link_))
{}

const std::int32_t chatEventInviteLinkDeleted::ID;

void chatEventInviteLinkDeleted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventInviteLinkDeleted");
    s.store_object_field("invite_link", static_cast<const BaseObject *>(invite_link_.get()));
    s.store_class_end();
  }
}

chatEventVideoChatCreated::chatEventVideoChatCreated()
  : group_call_id_()
{}

chatEventVideoChatCreated::chatEventVideoChatCreated(int32 group_call_id_)
  : group_call_id_(group_call_id_)
{}

const std::int32_t chatEventVideoChatCreated::ID;

void chatEventVideoChatCreated::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventVideoChatCreated");
    s.store_field("group_call_id", group_call_id_);
    s.store_class_end();
  }
}

chatEventVideoChatEnded::chatEventVideoChatEnded()
  : group_call_id_()
{}

chatEventVideoChatEnded::chatEventVideoChatEnded(int32 group_call_id_)
  : group_call_id_(group_call_id_)
{}

const std::int32_t chatEventVideoChatEnded::ID;

void chatEventVideoChatEnded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventVideoChatEnded");
    s.store_field("group_call_id", group_call_id_);
    s.store_class_end();
  }
}

chatEventVideoChatMuteNewParticipantsToggled::chatEventVideoChatMuteNewParticipantsToggled()
  : mute_new_participants_()
{}

chatEventVideoChatMuteNewParticipantsToggled::chatEventVideoChatMuteNewParticipantsToggled(bool mute_new_participants_)
  : mute_new_participants_(mute_new_participants_)
{}

const std::int32_t chatEventVideoChatMuteNewParticipantsToggled::ID;

void chatEventVideoChatMuteNewParticipantsToggled::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventVideoChatMuteNewParticipantsToggled");
    s.store_field("mute_new_participants", mute_new_participants_);
    s.store_class_end();
  }
}

chatEventVideoChatParticipantIsMutedToggled::chatEventVideoChatParticipantIsMutedToggled()
  : participant_id_()
  , is_muted_()
{}

chatEventVideoChatParticipantIsMutedToggled::chatEventVideoChatParticipantIsMutedToggled(object_ptr<MessageSender> &&participant_id_, bool is_muted_)
  : participant_id_(std::move(participant_id_))
  , is_muted_(is_muted_)
{}

const std::int32_t chatEventVideoChatParticipantIsMutedToggled::ID;

void chatEventVideoChatParticipantIsMutedToggled::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventVideoChatParticipantIsMutedToggled");
    s.store_object_field("participant_id", static_cast<const BaseObject *>(participant_id_.get()));
    s.store_field("is_muted", is_muted_);
    s.store_class_end();
  }
}

chatEventVideoChatParticipantVolumeLevelChanged::chatEventVideoChatParticipantVolumeLevelChanged()
  : participant_id_()
  , volume_level_()
{}

chatEventVideoChatParticipantVolumeLevelChanged::chatEventVideoChatParticipantVolumeLevelChanged(object_ptr<MessageSender> &&participant_id_, int32 volume_level_)
  : participant_id_(std::move(participant_id_))
  , volume_level_(volume_level_)
{}

const std::int32_t chatEventVideoChatParticipantVolumeLevelChanged::ID;

void chatEventVideoChatParticipantVolumeLevelChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventVideoChatParticipantVolumeLevelChanged");
    s.store_object_field("participant_id", static_cast<const BaseObject *>(participant_id_.get()));
    s.store_field("volume_level", volume_level_);
    s.store_class_end();
  }
}

chatEventIsForumToggled::chatEventIsForumToggled()
  : is_forum_()
{}

chatEventIsForumToggled::chatEventIsForumToggled(bool is_forum_)
  : is_forum_(is_forum_)
{}

const std::int32_t chatEventIsForumToggled::ID;

void chatEventIsForumToggled::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventIsForumToggled");
    s.store_field("is_forum", is_forum_);
    s.store_class_end();
  }
}

chatEventForumTopicCreated::chatEventForumTopicCreated()
  : topic_info_()
{}

chatEventForumTopicCreated::chatEventForumTopicCreated(object_ptr<forumTopicInfo> &&topic_info_)
  : topic_info_(std::move(topic_info_))
{}

const std::int32_t chatEventForumTopicCreated::ID;

void chatEventForumTopicCreated::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventForumTopicCreated");
    s.store_object_field("topic_info", static_cast<const BaseObject *>(topic_info_.get()));
    s.store_class_end();
  }
}

chatEventForumTopicEdited::chatEventForumTopicEdited()
  : old_topic_info_()
  , new_topic_info_()
{}

chatEventForumTopicEdited::chatEventForumTopicEdited(object_ptr<forumTopicInfo> &&old_topic_info_, object_ptr<forumTopicInfo> &&new_topic_info_)
  : old_topic_info_(std::move(old_topic_info_))
  , new_topic_info_(std::move(new_topic_info_))
{}

const std::int32_t chatEventForumTopicEdited::ID;

void chatEventForumTopicEdited::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventForumTopicEdited");
    s.store_object_field("old_topic_info", static_cast<const BaseObject *>(old_topic_info_.get()));
    s.store_object_field("new_topic_info", static_cast<const BaseObject *>(new_topic_info_.get()));
    s.store_class_end();
  }
}

chatEventForumTopicToggleIsClosed::chatEventForumTopicToggleIsClosed()
  : topic_info_()
{}

chatEventForumTopicToggleIsClosed::chatEventForumTopicToggleIsClosed(object_ptr<forumTopicInfo> &&topic_info_)
  : topic_info_(std::move(topic_info_))
{}

const std::int32_t chatEventForumTopicToggleIsClosed::ID;

void chatEventForumTopicToggleIsClosed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventForumTopicToggleIsClosed");
    s.store_object_field("topic_info", static_cast<const BaseObject *>(topic_info_.get()));
    s.store_class_end();
  }
}

chatEventForumTopicToggleIsHidden::chatEventForumTopicToggleIsHidden()
  : topic_info_()
{}

chatEventForumTopicToggleIsHidden::chatEventForumTopicToggleIsHidden(object_ptr<forumTopicInfo> &&topic_info_)
  : topic_info_(std::move(topic_info_))
{}

const std::int32_t chatEventForumTopicToggleIsHidden::ID;

void chatEventForumTopicToggleIsHidden::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventForumTopicToggleIsHidden");
    s.store_object_field("topic_info", static_cast<const BaseObject *>(topic_info_.get()));
    s.store_class_end();
  }
}

chatEventForumTopicDeleted::chatEventForumTopicDeleted()
  : topic_info_()
{}

chatEventForumTopicDeleted::chatEventForumTopicDeleted(object_ptr<forumTopicInfo> &&topic_info_)
  : topic_info_(std::move(topic_info_))
{}

const std::int32_t chatEventForumTopicDeleted::ID;

void chatEventForumTopicDeleted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventForumTopicDeleted");
    s.store_object_field("topic_info", static_cast<const BaseObject *>(topic_info_.get()));
    s.store_class_end();
  }
}

chatEventForumTopicPinned::chatEventForumTopicPinned()
  : old_topic_info_()
  , new_topic_info_()
{}

chatEventForumTopicPinned::chatEventForumTopicPinned(object_ptr<forumTopicInfo> &&old_topic_info_, object_ptr<forumTopicInfo> &&new_topic_info_)
  : old_topic_info_(std::move(old_topic_info_))
  , new_topic_info_(std::move(new_topic_info_))
{}

const std::int32_t chatEventForumTopicPinned::ID;

void chatEventForumTopicPinned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEventForumTopicPinned");
    s.store_object_field("old_topic_info", static_cast<const BaseObject *>(old_topic_info_.get()));
    s.store_object_field("new_topic_info", static_cast<const BaseObject *>(new_topic_info_.get()));
    s.store_class_end();
  }
}

chatFolder::chatFolder()
  : name_()
  , icon_()
  , color_id_()
  , is_shareable_()
  , pinned_chat_ids_()
  , included_chat_ids_()
  , excluded_chat_ids_()
  , exclude_muted_()
  , exclude_read_()
  , exclude_archived_()
  , include_contacts_()
  , include_non_contacts_()
  , include_bots_()
  , include_groups_()
  , include_channels_()
{}

chatFolder::chatFolder(object_ptr<chatFolderName> &&name_, object_ptr<chatFolderIcon> &&icon_, int32 color_id_, bool is_shareable_, array<int53> &&pinned_chat_ids_, array<int53> &&included_chat_ids_, array<int53> &&excluded_chat_ids_, bool exclude_muted_, bool exclude_read_, bool exclude_archived_, bool include_contacts_, bool include_non_contacts_, bool include_bots_, bool include_groups_, bool include_channels_)
  : name_(std::move(name_))
  , icon_(std::move(icon_))
  , color_id_(color_id_)
  , is_shareable_(is_shareable_)
  , pinned_chat_ids_(std::move(pinned_chat_ids_))
  , included_chat_ids_(std::move(included_chat_ids_))
  , excluded_chat_ids_(std::move(excluded_chat_ids_))
  , exclude_muted_(exclude_muted_)
  , exclude_read_(exclude_read_)
  , exclude_archived_(exclude_archived_)
  , include_contacts_(include_contacts_)
  , include_non_contacts_(include_non_contacts_)
  , include_bots_(include_bots_)
  , include_groups_(include_groups_)
  , include_channels_(include_channels_)
{}

const std::int32_t chatFolder::ID;

void chatFolder::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatFolder");
    s.store_object_field("name", static_cast<const BaseObject *>(name_.get()));
    s.store_object_field("icon", static_cast<const BaseObject *>(icon_.get()));
    s.store_field("color_id", color_id_);
    s.store_field("is_shareable", is_shareable_);
    { s.store_vector_begin("pinned_chat_ids", pinned_chat_ids_.size()); for (const auto &_value : pinned_chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("included_chat_ids", included_chat_ids_.size()); for (const auto &_value : included_chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("excluded_chat_ids", excluded_chat_ids_.size()); for (const auto &_value : excluded_chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("exclude_muted", exclude_muted_);
    s.store_field("exclude_read", exclude_read_);
    s.store_field("exclude_archived", exclude_archived_);
    s.store_field("include_contacts", include_contacts_);
    s.store_field("include_non_contacts", include_non_contacts_);
    s.store_field("include_bots", include_bots_);
    s.store_field("include_groups", include_groups_);
    s.store_field("include_channels", include_channels_);
    s.store_class_end();
  }
}

chatFolderInfo::chatFolderInfo()
  : id_()
  , name_()
  , icon_()
  , color_id_()
  , is_shareable_()
  , has_my_invite_links_()
{}

chatFolderInfo::chatFolderInfo(int32 id_, object_ptr<chatFolderName> &&name_, object_ptr<chatFolderIcon> &&icon_, int32 color_id_, bool is_shareable_, bool has_my_invite_links_)
  : id_(id_)
  , name_(std::move(name_))
  , icon_(std::move(icon_))
  , color_id_(color_id_)
  , is_shareable_(is_shareable_)
  , has_my_invite_links_(has_my_invite_links_)
{}

const std::int32_t chatFolderInfo::ID;

void chatFolderInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatFolderInfo");
    s.store_field("id", id_);
    s.store_object_field("name", static_cast<const BaseObject *>(name_.get()));
    s.store_object_field("icon", static_cast<const BaseObject *>(icon_.get()));
    s.store_field("color_id", color_id_);
    s.store_field("is_shareable", is_shareable_);
    s.store_field("has_my_invite_links", has_my_invite_links_);
    s.store_class_end();
  }
}

chatInviteLinkSubscriptionInfo::chatInviteLinkSubscriptionInfo()
  : pricing_()
  , can_reuse_()
  , form_id_()
{}

chatInviteLinkSubscriptionInfo::chatInviteLinkSubscriptionInfo(object_ptr<starSubscriptionPricing> &&pricing_, bool can_reuse_, int64 form_id_)
  : pricing_(std::move(pricing_))
  , can_reuse_(can_reuse_)
  , form_id_(form_id_)
{}

const std::int32_t chatInviteLinkSubscriptionInfo::ID;

void chatInviteLinkSubscriptionInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatInviteLinkSubscriptionInfo");
    s.store_object_field("pricing", static_cast<const BaseObject *>(pricing_.get()));
    s.store_field("can_reuse", can_reuse_);
    s.store_field("form_id", form_id_);
    s.store_class_end();
  }
}

chatInviteLinks::chatInviteLinks()
  : total_count_()
  , invite_links_()
{}

chatInviteLinks::chatInviteLinks(int32 total_count_, array<object_ptr<chatInviteLink>> &&invite_links_)
  : total_count_(total_count_)
  , invite_links_(std::move(invite_links_))
{}

const std::int32_t chatInviteLinks::ID;

void chatInviteLinks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatInviteLinks");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("invite_links", invite_links_.size()); for (const auto &_value : invite_links_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatMessageSender::chatMessageSender()
  : sender_()
  , needs_premium_()
{}

chatMessageSender::chatMessageSender(object_ptr<MessageSender> &&sender_, bool needs_premium_)
  : sender_(std::move(sender_))
  , needs_premium_(needs_premium_)
{}

const std::int32_t chatMessageSender::ID;

void chatMessageSender::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatMessageSender");
    s.store_object_field("sender", static_cast<const BaseObject *>(sender_.get()));
    s.store_field("needs_premium", needs_premium_);
    s.store_class_end();
  }
}

chatNotificationSettings::chatNotificationSettings()
  : use_default_mute_for_()
  , mute_for_()
  , use_default_sound_()
  , sound_id_()
  , use_default_show_preview_()
  , show_preview_()
  , use_default_mute_stories_()
  , mute_stories_()
  , use_default_story_sound_()
  , story_sound_id_()
  , use_default_show_story_poster_()
  , show_story_poster_()
  , use_default_disable_pinned_message_notifications_()
  , disable_pinned_message_notifications_()
  , use_default_disable_mention_notifications_()
  , disable_mention_notifications_()
{}

chatNotificationSettings::chatNotificationSettings(bool use_default_mute_for_, int32 mute_for_, bool use_default_sound_, int64 sound_id_, bool use_default_show_preview_, bool show_preview_, bool use_default_mute_stories_, bool mute_stories_, bool use_default_story_sound_, int64 story_sound_id_, bool use_default_show_story_poster_, bool show_story_poster_, bool use_default_disable_pinned_message_notifications_, bool disable_pinned_message_notifications_, bool use_default_disable_mention_notifications_, bool disable_mention_notifications_)
  : use_default_mute_for_(use_default_mute_for_)
  , mute_for_(mute_for_)
  , use_default_sound_(use_default_sound_)
  , sound_id_(sound_id_)
  , use_default_show_preview_(use_default_show_preview_)
  , show_preview_(show_preview_)
  , use_default_mute_stories_(use_default_mute_stories_)
  , mute_stories_(mute_stories_)
  , use_default_story_sound_(use_default_story_sound_)
  , story_sound_id_(story_sound_id_)
  , use_default_show_story_poster_(use_default_show_story_poster_)
  , show_story_poster_(show_story_poster_)
  , use_default_disable_pinned_message_notifications_(use_default_disable_pinned_message_notifications_)
  , disable_pinned_message_notifications_(disable_pinned_message_notifications_)
  , use_default_disable_mention_notifications_(use_default_disable_mention_notifications_)
  , disable_mention_notifications_(disable_mention_notifications_)
{}

const std::int32_t chatNotificationSettings::ID;

void chatNotificationSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatNotificationSettings");
    s.store_field("use_default_mute_for", use_default_mute_for_);
    s.store_field("mute_for", mute_for_);
    s.store_field("use_default_sound", use_default_sound_);
    s.store_field("sound_id", sound_id_);
    s.store_field("use_default_show_preview", use_default_show_preview_);
    s.store_field("show_preview", show_preview_);
    s.store_field("use_default_mute_stories", use_default_mute_stories_);
    s.store_field("mute_stories", mute_stories_);
    s.store_field("use_default_story_sound", use_default_story_sound_);
    s.store_field("story_sound_id", story_sound_id_);
    s.store_field("use_default_show_story_poster", use_default_show_story_poster_);
    s.store_field("show_story_poster", show_story_poster_);
    s.store_field("use_default_disable_pinned_message_notifications", use_default_disable_pinned_message_notifications_);
    s.store_field("disable_pinned_message_notifications", disable_pinned_message_notifications_);
    s.store_field("use_default_disable_mention_notifications", use_default_disable_mention_notifications_);
    s.store_field("disable_mention_notifications", disable_mention_notifications_);
    s.store_class_end();
  }
}

chatStatisticsObjectTypeMessage::chatStatisticsObjectTypeMessage()
  : message_id_()
{}

chatStatisticsObjectTypeMessage::chatStatisticsObjectTypeMessage(int53 message_id_)
  : message_id_(message_id_)
{}

const std::int32_t chatStatisticsObjectTypeMessage::ID;

void chatStatisticsObjectTypeMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatStatisticsObjectTypeMessage");
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

chatStatisticsObjectTypeStory::chatStatisticsObjectTypeStory()
  : story_id_()
{}

chatStatisticsObjectTypeStory::chatStatisticsObjectTypeStory(int32 story_id_)
  : story_id_(story_id_)
{}

const std::int32_t chatStatisticsObjectTypeStory::ID;

void chatStatisticsObjectTypeStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatStatisticsObjectTypeStory");
    s.store_field("story_id", story_id_);
    s.store_class_end();
  }
}

closeBirthdayUser::closeBirthdayUser()
  : user_id_()
  , birthdate_()
{}

closeBirthdayUser::closeBirthdayUser(int53 user_id_, object_ptr<birthdate> &&birthdate_)
  : user_id_(user_id_)
  , birthdate_(std::move(birthdate_))
{}

const std::int32_t closeBirthdayUser::ID;

void closeBirthdayUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "closeBirthdayUser");
    s.store_field("user_id", user_id_);
    s.store_object_field("birthdate", static_cast<const BaseObject *>(birthdate_.get()));
    s.store_class_end();
  }
}

draftMessage::draftMessage()
  : reply_to_()
  , date_()
  , input_message_text_()
  , effect_id_()
  , suggested_post_info_()
{}

draftMessage::draftMessage(object_ptr<InputMessageReplyTo> &&reply_to_, int32 date_, object_ptr<InputMessageContent> &&input_message_text_, int64 effect_id_, object_ptr<inputSuggestedPostInfo> &&suggested_post_info_)
  : reply_to_(std::move(reply_to_))
  , date_(date_)
  , input_message_text_(std::move(input_message_text_))
  , effect_id_(effect_id_)
  , suggested_post_info_(std::move(suggested_post_info_))
{}

const std::int32_t draftMessage::ID;

void draftMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "draftMessage");
    s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get()));
    s.store_field("date", date_);
    s.store_object_field("input_message_text", static_cast<const BaseObject *>(input_message_text_.get()));
    s.store_field("effect_id", effect_id_);
    s.store_object_field("suggested_post_info", static_cast<const BaseObject *>(suggested_post_info_.get()));
    s.store_class_end();
  }
}

emojiCategories::emojiCategories()
  : categories_()
{}

emojiCategories::emojiCategories(array<object_ptr<emojiCategory>> &&categories_)
  : categories_(std::move(categories_))
{}

const std::int32_t emojiCategories::ID;

void emojiCategories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiCategories");
    { s.store_vector_begin("categories", categories_.size()); for (const auto &_value : categories_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

emojiStatusTypeCustomEmoji::emojiStatusTypeCustomEmoji()
  : custom_emoji_id_()
{}

emojiStatusTypeCustomEmoji::emojiStatusTypeCustomEmoji(int64 custom_emoji_id_)
  : custom_emoji_id_(custom_emoji_id_)
{}

const std::int32_t emojiStatusTypeCustomEmoji::ID;

void emojiStatusTypeCustomEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiStatusTypeCustomEmoji");
    s.store_field("custom_emoji_id", custom_emoji_id_);
    s.store_class_end();
  }
}

emojiStatusTypeUpgradedGift::emojiStatusTypeUpgradedGift()
  : upgraded_gift_id_()
  , gift_title_()
  , gift_name_()
  , model_custom_emoji_id_()
  , symbol_custom_emoji_id_()
  , backdrop_colors_()
{}

emojiStatusTypeUpgradedGift::emojiStatusTypeUpgradedGift(int64 upgraded_gift_id_, string const &gift_title_, string const &gift_name_, int64 model_custom_emoji_id_, int64 symbol_custom_emoji_id_, object_ptr<upgradedGiftBackdropColors> &&backdrop_colors_)
  : upgraded_gift_id_(upgraded_gift_id_)
  , gift_title_(gift_title_)
  , gift_name_(gift_name_)
  , model_custom_emoji_id_(model_custom_emoji_id_)
  , symbol_custom_emoji_id_(symbol_custom_emoji_id_)
  , backdrop_colors_(std::move(backdrop_colors_))
{}

const std::int32_t emojiStatusTypeUpgradedGift::ID;

void emojiStatusTypeUpgradedGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiStatusTypeUpgradedGift");
    s.store_field("upgraded_gift_id", upgraded_gift_id_);
    s.store_field("gift_title", gift_title_);
    s.store_field("gift_name", gift_name_);
    s.store_field("model_custom_emoji_id", model_custom_emoji_id_);
    s.store_field("symbol_custom_emoji_id", symbol_custom_emoji_id_);
    s.store_object_field("backdrop_colors", static_cast<const BaseObject *>(backdrop_colors_.get()));
    s.store_class_end();
  }
}

error::error()
  : code_()
  , message_()
{}

error::error(int32 code_, string const &message_)
  : code_(code_)
  , message_(message_)
{}

const std::int32_t error::ID;

void error::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "error");
    s.store_field("code", code_);
    s.store_field("message", message_);
    s.store_class_end();
  }
}

game::game()
  : id_()
  , short_name_()
  , title_()
  , text_()
  , description_()
  , photo_()
  , animation_()
{}

game::game(int64 id_, string const &short_name_, string const &title_, object_ptr<formattedText> &&text_, string const &description_, object_ptr<photo> &&photo_, object_ptr<animation> &&animation_)
  : id_(id_)
  , short_name_(short_name_)
  , title_(title_)
  , text_(std::move(text_))
  , description_(description_)
  , photo_(std::move(photo_))
  , animation_(std::move(animation_))
{}

const std::int32_t game::ID;

void game::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "game");
    s.store_field("id", id_);
    s.store_field("short_name", short_name_);
    s.store_field("title", title_);
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("description", description_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_object_field("animation", static_cast<const BaseObject *>(animation_.get()));
    s.store_class_end();
  }
}

giftChatThemes::giftChatThemes()
  : themes_()
  , next_offset_()
{}

giftChatThemes::giftChatThemes(array<object_ptr<giftChatTheme>> &&themes_, string const &next_offset_)
  : themes_(std::move(themes_))
  , next_offset_(next_offset_)
{}

const std::int32_t giftChatThemes::ID;

void giftChatThemes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giftChatThemes");
    { s.store_vector_begin("themes", themes_.size()); for (const auto &_value : themes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

giftCollection::giftCollection()
  : id_()
  , name_()
  , icon_()
  , gift_count_()
{}

giftCollection::giftCollection(int32 id_, string const &name_, object_ptr<sticker> &&icon_, int32 gift_count_)
  : id_(id_)
  , name_(name_)
  , icon_(std::move(icon_))
  , gift_count_(gift_count_)
{}

const std::int32_t giftCollection::ID;

void giftCollection::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giftCollection");
    s.store_field("id", id_);
    s.store_field("name", name_);
    s.store_object_field("icon", static_cast<const BaseObject *>(icon_.get()));
    s.store_field("gift_count", gift_count_);
    s.store_class_end();
  }
}

giftUpgradePreview::giftUpgradePreview()
  : models_()
  , symbols_()
  , backdrops_()
  , prices_()
  , next_prices_()
{}

giftUpgradePreview::giftUpgradePreview(array<object_ptr<upgradedGiftModel>> &&models_, array<object_ptr<upgradedGiftSymbol>> &&symbols_, array<object_ptr<upgradedGiftBackdrop>> &&backdrops_, array<object_ptr<giftUpgradePrice>> &&prices_, array<object_ptr<giftUpgradePrice>> &&next_prices_)
  : models_(std::move(models_))
  , symbols_(std::move(symbols_))
  , backdrops_(std::move(backdrops_))
  , prices_(std::move(prices_))
  , next_prices_(std::move(next_prices_))
{}

const std::int32_t giftUpgradePreview::ID;

void giftUpgradePreview::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giftUpgradePreview");
    { s.store_vector_begin("models", models_.size()); for (const auto &_value : models_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("symbols", symbols_.size()); for (const auto &_value : symbols_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("backdrops", backdrops_.size()); for (const auto &_value : backdrops_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("prices", prices_.size()); for (const auto &_value : prices_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("next_prices", next_prices_.size()); for (const auto &_value : next_prices_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

giveawayParticipantStatusEligible::giveawayParticipantStatusEligible() {
}

const std::int32_t giveawayParticipantStatusEligible::ID;

void giveawayParticipantStatusEligible::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giveawayParticipantStatusEligible");
    s.store_class_end();
  }
}

giveawayParticipantStatusParticipating::giveawayParticipantStatusParticipating() {
}

const std::int32_t giveawayParticipantStatusParticipating::ID;

void giveawayParticipantStatusParticipating::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giveawayParticipantStatusParticipating");
    s.store_class_end();
  }
}

giveawayParticipantStatusAlreadyWasMember::giveawayParticipantStatusAlreadyWasMember()
  : joined_chat_date_()
{}

giveawayParticipantStatusAlreadyWasMember::giveawayParticipantStatusAlreadyWasMember(int32 joined_chat_date_)
  : joined_chat_date_(joined_chat_date_)
{}

const std::int32_t giveawayParticipantStatusAlreadyWasMember::ID;

void giveawayParticipantStatusAlreadyWasMember::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giveawayParticipantStatusAlreadyWasMember");
    s.store_field("joined_chat_date", joined_chat_date_);
    s.store_class_end();
  }
}

giveawayParticipantStatusAdministrator::giveawayParticipantStatusAdministrator()
  : chat_id_()
{}

giveawayParticipantStatusAdministrator::giveawayParticipantStatusAdministrator(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t giveawayParticipantStatusAdministrator::ID;

void giveawayParticipantStatusAdministrator::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giveawayParticipantStatusAdministrator");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

giveawayParticipantStatusDisallowedCountry::giveawayParticipantStatusDisallowedCountry()
  : user_country_code_()
{}

giveawayParticipantStatusDisallowedCountry::giveawayParticipantStatusDisallowedCountry(string const &user_country_code_)
  : user_country_code_(user_country_code_)
{}

const std::int32_t giveawayParticipantStatusDisallowedCountry::ID;

void giveawayParticipantStatusDisallowedCountry::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giveawayParticipantStatusDisallowedCountry");
    s.store_field("user_country_code", user_country_code_);
    s.store_class_end();
  }
}

inlineQueryResultArticle::inlineQueryResultArticle()
  : id_()
  , url_()
  , title_()
  , description_()
  , thumbnail_()
{}

inlineQueryResultArticle::inlineQueryResultArticle(string const &id_, string const &url_, string const &title_, string const &description_, object_ptr<thumbnail> &&thumbnail_)
  : id_(id_)
  , url_(url_)
  , title_(title_)
  , description_(description_)
  , thumbnail_(std::move(thumbnail_))
{}

const std::int32_t inlineQueryResultArticle::ID;

void inlineQueryResultArticle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryResultArticle");
    s.store_field("id", id_);
    s.store_field("url", url_);
    s.store_field("title", title_);
    s.store_field("description", description_);
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_class_end();
  }
}

inlineQueryResultContact::inlineQueryResultContact()
  : id_()
  , contact_()
  , thumbnail_()
{}

inlineQueryResultContact::inlineQueryResultContact(string const &id_, object_ptr<contact> &&contact_, object_ptr<thumbnail> &&thumbnail_)
  : id_(id_)
  , contact_(std::move(contact_))
  , thumbnail_(std::move(thumbnail_))
{}

const std::int32_t inlineQueryResultContact::ID;

void inlineQueryResultContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryResultContact");
    s.store_field("id", id_);
    s.store_object_field("contact", static_cast<const BaseObject *>(contact_.get()));
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_class_end();
  }
}

inlineQueryResultLocation::inlineQueryResultLocation()
  : id_()
  , location_()
  , title_()
  , thumbnail_()
{}

inlineQueryResultLocation::inlineQueryResultLocation(string const &id_, object_ptr<location> &&location_, string const &title_, object_ptr<thumbnail> &&thumbnail_)
  : id_(id_)
  , location_(std::move(location_))
  , title_(title_)
  , thumbnail_(std::move(thumbnail_))
{}

const std::int32_t inlineQueryResultLocation::ID;

void inlineQueryResultLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryResultLocation");
    s.store_field("id", id_);
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_field("title", title_);
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_class_end();
  }
}

inlineQueryResultVenue::inlineQueryResultVenue()
  : id_()
  , venue_()
  , thumbnail_()
{}

inlineQueryResultVenue::inlineQueryResultVenue(string const &id_, object_ptr<venue> &&venue_, object_ptr<thumbnail> &&thumbnail_)
  : id_(id_)
  , venue_(std::move(venue_))
  , thumbnail_(std::move(thumbnail_))
{}

const std::int32_t inlineQueryResultVenue::ID;

void inlineQueryResultVenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryResultVenue");
    s.store_field("id", id_);
    s.store_object_field("venue", static_cast<const BaseObject *>(venue_.get()));
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_class_end();
  }
}

inlineQueryResultGame::inlineQueryResultGame()
  : id_()
  , game_()
{}

inlineQueryResultGame::inlineQueryResultGame(string const &id_, object_ptr<game> &&game_)
  : id_(id_)
  , game_(std::move(game_))
{}

const std::int32_t inlineQueryResultGame::ID;

void inlineQueryResultGame::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryResultGame");
    s.store_field("id", id_);
    s.store_object_field("game", static_cast<const BaseObject *>(game_.get()));
    s.store_class_end();
  }
}

inlineQueryResultAnimation::inlineQueryResultAnimation()
  : id_()
  , animation_()
  , title_()
{}

inlineQueryResultAnimation::inlineQueryResultAnimation(string const &id_, object_ptr<animation> &&animation_, string const &title_)
  : id_(id_)
  , animation_(std::move(animation_))
  , title_(title_)
{}

const std::int32_t inlineQueryResultAnimation::ID;

void inlineQueryResultAnimation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryResultAnimation");
    s.store_field("id", id_);
    s.store_object_field("animation", static_cast<const BaseObject *>(animation_.get()));
    s.store_field("title", title_);
    s.store_class_end();
  }
}

inlineQueryResultAudio::inlineQueryResultAudio()
  : id_()
  , audio_()
{}

inlineQueryResultAudio::inlineQueryResultAudio(string const &id_, object_ptr<audio> &&audio_)
  : id_(id_)
  , audio_(std::move(audio_))
{}

const std::int32_t inlineQueryResultAudio::ID;

void inlineQueryResultAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryResultAudio");
    s.store_field("id", id_);
    s.store_object_field("audio", static_cast<const BaseObject *>(audio_.get()));
    s.store_class_end();
  }
}

inlineQueryResultDocument::inlineQueryResultDocument()
  : id_()
  , document_()
  , title_()
  , description_()
{}

inlineQueryResultDocument::inlineQueryResultDocument(string const &id_, object_ptr<document> &&document_, string const &title_, string const &description_)
  : id_(id_)
  , document_(std::move(document_))
  , title_(title_)
  , description_(description_)
{}

const std::int32_t inlineQueryResultDocument::ID;

void inlineQueryResultDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryResultDocument");
    s.store_field("id", id_);
    s.store_object_field("document", static_cast<const BaseObject *>(document_.get()));
    s.store_field("title", title_);
    s.store_field("description", description_);
    s.store_class_end();
  }
}

inlineQueryResultPhoto::inlineQueryResultPhoto()
  : id_()
  , photo_()
  , title_()
  , description_()
{}

inlineQueryResultPhoto::inlineQueryResultPhoto(string const &id_, object_ptr<photo> &&photo_, string const &title_, string const &description_)
  : id_(id_)
  , photo_(std::move(photo_))
  , title_(title_)
  , description_(description_)
{}

const std::int32_t inlineQueryResultPhoto::ID;

void inlineQueryResultPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryResultPhoto");
    s.store_field("id", id_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_field("title", title_);
    s.store_field("description", description_);
    s.store_class_end();
  }
}

inlineQueryResultSticker::inlineQueryResultSticker()
  : id_()
  , sticker_()
{}

inlineQueryResultSticker::inlineQueryResultSticker(string const &id_, object_ptr<sticker> &&sticker_)
  : id_(id_)
  , sticker_(std::move(sticker_))
{}

const std::int32_t inlineQueryResultSticker::ID;

void inlineQueryResultSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryResultSticker");
    s.store_field("id", id_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

inlineQueryResultVideo::inlineQueryResultVideo()
  : id_()
  , video_()
  , title_()
  , description_()
{}

inlineQueryResultVideo::inlineQueryResultVideo(string const &id_, object_ptr<video> &&video_, string const &title_, string const &description_)
  : id_(id_)
  , video_(std::move(video_))
  , title_(title_)
  , description_(description_)
{}

const std::int32_t inlineQueryResultVideo::ID;

void inlineQueryResultVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryResultVideo");
    s.store_field("id", id_);
    s.store_object_field("video", static_cast<const BaseObject *>(video_.get()));
    s.store_field("title", title_);
    s.store_field("description", description_);
    s.store_class_end();
  }
}

inlineQueryResultVoiceNote::inlineQueryResultVoiceNote()
  : id_()
  , voice_note_()
  , title_()
{}

inlineQueryResultVoiceNote::inlineQueryResultVoiceNote(string const &id_, object_ptr<voiceNote> &&voice_note_, string const &title_)
  : id_(id_)
  , voice_note_(std::move(voice_note_))
  , title_(title_)
{}

const std::int32_t inlineQueryResultVoiceNote::ID;

void inlineQueryResultVoiceNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineQueryResultVoiceNote");
    s.store_field("id", id_);
    s.store_object_field("voice_note", static_cast<const BaseObject *>(voice_note_.get()));
    s.store_field("title", title_);
    s.store_class_end();
  }
}

inputPassportElementPersonalDetails::inputPassportElementPersonalDetails()
  : personal_details_()
{}

inputPassportElementPersonalDetails::inputPassportElementPersonalDetails(object_ptr<personalDetails> &&personal_details_)
  : personal_details_(std::move(personal_details_))
{}

const std::int32_t inputPassportElementPersonalDetails::ID;

void inputPassportElementPersonalDetails::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementPersonalDetails");
    s.store_object_field("personal_details", static_cast<const BaseObject *>(personal_details_.get()));
    s.store_class_end();
  }
}

inputPassportElementPassport::inputPassportElementPassport()
  : passport_()
{}

inputPassportElementPassport::inputPassportElementPassport(object_ptr<inputIdentityDocument> &&passport_)
  : passport_(std::move(passport_))
{}

const std::int32_t inputPassportElementPassport::ID;

void inputPassportElementPassport::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementPassport");
    s.store_object_field("passport", static_cast<const BaseObject *>(passport_.get()));
    s.store_class_end();
  }
}

inputPassportElementDriverLicense::inputPassportElementDriverLicense()
  : driver_license_()
{}

inputPassportElementDriverLicense::inputPassportElementDriverLicense(object_ptr<inputIdentityDocument> &&driver_license_)
  : driver_license_(std::move(driver_license_))
{}

const std::int32_t inputPassportElementDriverLicense::ID;

void inputPassportElementDriverLicense::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementDriverLicense");
    s.store_object_field("driver_license", static_cast<const BaseObject *>(driver_license_.get()));
    s.store_class_end();
  }
}

inputPassportElementIdentityCard::inputPassportElementIdentityCard()
  : identity_card_()
{}

inputPassportElementIdentityCard::inputPassportElementIdentityCard(object_ptr<inputIdentityDocument> &&identity_card_)
  : identity_card_(std::move(identity_card_))
{}

const std::int32_t inputPassportElementIdentityCard::ID;

void inputPassportElementIdentityCard::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementIdentityCard");
    s.store_object_field("identity_card", static_cast<const BaseObject *>(identity_card_.get()));
    s.store_class_end();
  }
}

inputPassportElementInternalPassport::inputPassportElementInternalPassport()
  : internal_passport_()
{}

inputPassportElementInternalPassport::inputPassportElementInternalPassport(object_ptr<inputIdentityDocument> &&internal_passport_)
  : internal_passport_(std::move(internal_passport_))
{}

const std::int32_t inputPassportElementInternalPassport::ID;

void inputPassportElementInternalPassport::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementInternalPassport");
    s.store_object_field("internal_passport", static_cast<const BaseObject *>(internal_passport_.get()));
    s.store_class_end();
  }
}

inputPassportElementAddress::inputPassportElementAddress()
  : address_()
{}

inputPassportElementAddress::inputPassportElementAddress(object_ptr<address> &&address_)
  : address_(std::move(address_))
{}

const std::int32_t inputPassportElementAddress::ID;

void inputPassportElementAddress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementAddress");
    s.store_object_field("address", static_cast<const BaseObject *>(address_.get()));
    s.store_class_end();
  }
}

inputPassportElementUtilityBill::inputPassportElementUtilityBill()
  : utility_bill_()
{}

inputPassportElementUtilityBill::inputPassportElementUtilityBill(object_ptr<inputPersonalDocument> &&utility_bill_)
  : utility_bill_(std::move(utility_bill_))
{}

const std::int32_t inputPassportElementUtilityBill::ID;

void inputPassportElementUtilityBill::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementUtilityBill");
    s.store_object_field("utility_bill", static_cast<const BaseObject *>(utility_bill_.get()));
    s.store_class_end();
  }
}

inputPassportElementBankStatement::inputPassportElementBankStatement()
  : bank_statement_()
{}

inputPassportElementBankStatement::inputPassportElementBankStatement(object_ptr<inputPersonalDocument> &&bank_statement_)
  : bank_statement_(std::move(bank_statement_))
{}

const std::int32_t inputPassportElementBankStatement::ID;

void inputPassportElementBankStatement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementBankStatement");
    s.store_object_field("bank_statement", static_cast<const BaseObject *>(bank_statement_.get()));
    s.store_class_end();
  }
}

inputPassportElementRentalAgreement::inputPassportElementRentalAgreement()
  : rental_agreement_()
{}

inputPassportElementRentalAgreement::inputPassportElementRentalAgreement(object_ptr<inputPersonalDocument> &&rental_agreement_)
  : rental_agreement_(std::move(rental_agreement_))
{}

const std::int32_t inputPassportElementRentalAgreement::ID;

void inputPassportElementRentalAgreement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementRentalAgreement");
    s.store_object_field("rental_agreement", static_cast<const BaseObject *>(rental_agreement_.get()));
    s.store_class_end();
  }
}

inputPassportElementPassportRegistration::inputPassportElementPassportRegistration()
  : passport_registration_()
{}

inputPassportElementPassportRegistration::inputPassportElementPassportRegistration(object_ptr<inputPersonalDocument> &&passport_registration_)
  : passport_registration_(std::move(passport_registration_))
{}

const std::int32_t inputPassportElementPassportRegistration::ID;

void inputPassportElementPassportRegistration::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementPassportRegistration");
    s.store_object_field("passport_registration", static_cast<const BaseObject *>(passport_registration_.get()));
    s.store_class_end();
  }
}

inputPassportElementTemporaryRegistration::inputPassportElementTemporaryRegistration()
  : temporary_registration_()
{}

inputPassportElementTemporaryRegistration::inputPassportElementTemporaryRegistration(object_ptr<inputPersonalDocument> &&temporary_registration_)
  : temporary_registration_(std::move(temporary_registration_))
{}

const std::int32_t inputPassportElementTemporaryRegistration::ID;

void inputPassportElementTemporaryRegistration::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementTemporaryRegistration");
    s.store_object_field("temporary_registration", static_cast<const BaseObject *>(temporary_registration_.get()));
    s.store_class_end();
  }
}

inputPassportElementPhoneNumber::inputPassportElementPhoneNumber()
  : phone_number_()
{}

inputPassportElementPhoneNumber::inputPassportElementPhoneNumber(string const &phone_number_)
  : phone_number_(phone_number_)
{}

const std::int32_t inputPassportElementPhoneNumber::ID;

void inputPassportElementPhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementPhoneNumber");
    s.store_field("phone_number", phone_number_);
    s.store_class_end();
  }
}

inputPassportElementEmailAddress::inputPassportElementEmailAddress()
  : email_address_()
{}

inputPassportElementEmailAddress::inputPassportElementEmailAddress(string const &email_address_)
  : email_address_(email_address_)
{}

const std::int32_t inputPassportElementEmailAddress::ID;

void inputPassportElementEmailAddress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementEmailAddress");
    s.store_field("email_address", email_address_);
    s.store_class_end();
  }
}

inputStoryContentPhoto::inputStoryContentPhoto()
  : photo_()
  , added_sticker_file_ids_()
{}

inputStoryContentPhoto::inputStoryContentPhoto(object_ptr<InputFile> &&photo_, array<int32> &&added_sticker_file_ids_)
  : photo_(std::move(photo_))
  , added_sticker_file_ids_(std::move(added_sticker_file_ids_))
{}

const std::int32_t inputStoryContentPhoto::ID;

void inputStoryContentPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStoryContentPhoto");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    { s.store_vector_begin("added_sticker_file_ids", added_sticker_file_ids_.size()); for (const auto &_value : added_sticker_file_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

inputStoryContentVideo::inputStoryContentVideo()
  : video_()
  , added_sticker_file_ids_()
  , duration_()
  , cover_frame_timestamp_()
  , is_animation_()
{}

inputStoryContentVideo::inputStoryContentVideo(object_ptr<InputFile> &&video_, array<int32> &&added_sticker_file_ids_, double duration_, double cover_frame_timestamp_, bool is_animation_)
  : video_(std::move(video_))
  , added_sticker_file_ids_(std::move(added_sticker_file_ids_))
  , duration_(duration_)
  , cover_frame_timestamp_(cover_frame_timestamp_)
  , is_animation_(is_animation_)
{}

const std::int32_t inputStoryContentVideo::ID;

void inputStoryContentVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStoryContentVideo");
    s.store_object_field("video", static_cast<const BaseObject *>(video_.get()));
    { s.store_vector_begin("added_sticker_file_ids", added_sticker_file_ids_.size()); for (const auto &_value : added_sticker_file_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("duration", duration_);
    s.store_field("cover_frame_timestamp", cover_frame_timestamp_);
    s.store_field("is_animation", is_animation_);
    s.store_class_end();
  }
}

linkPreviewOptions::linkPreviewOptions()
  : is_disabled_()
  , url_()
  , force_small_media_()
  , force_large_media_()
  , show_above_text_()
{}

linkPreviewOptions::linkPreviewOptions(bool is_disabled_, string const &url_, bool force_small_media_, bool force_large_media_, bool show_above_text_)
  : is_disabled_(is_disabled_)
  , url_(url_)
  , force_small_media_(force_small_media_)
  , force_large_media_(force_large_media_)
  , show_above_text_(show_above_text_)
{}

const std::int32_t linkPreviewOptions::ID;

void linkPreviewOptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewOptions");
    s.store_field("is_disabled", is_disabled_);
    s.store_field("url", url_);
    s.store_field("force_small_media", force_small_media_);
    s.store_field("force_large_media", force_large_media_);
    s.store_field("show_above_text", show_above_text_);
    s.store_class_end();
  }
}

mainWebApp::mainWebApp()
  : url_()
  , mode_()
{}

mainWebApp::mainWebApp(string const &url_, object_ptr<WebAppOpenMode> &&mode_)
  : url_(url_)
  , mode_(std::move(mode_))
{}

const std::int32_t mainWebApp::ID;

void mainWebApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "mainWebApp");
    s.store_field("url", url_);
    s.store_object_field("mode", static_cast<const BaseObject *>(mode_.get()));
    s.store_class_end();
  }
}

messageImportInfo::messageImportInfo()
  : sender_name_()
  , date_()
{}

messageImportInfo::messageImportInfo(string const &sender_name_, int32 date_)
  : sender_name_(sender_name_)
  , date_(date_)
{}

const std::int32_t messageImportInfo::ID;

void messageImportInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageImportInfo");
    s.store_field("sender_name", sender_name_);
    s.store_field("date", date_);
    s.store_class_end();
  }
}

messageSourceChatHistory::messageSourceChatHistory() {
}

const std::int32_t messageSourceChatHistory::ID;

void messageSourceChatHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSourceChatHistory");
    s.store_class_end();
  }
}

messageSourceMessageThreadHistory::messageSourceMessageThreadHistory() {
}

const std::int32_t messageSourceMessageThreadHistory::ID;

void messageSourceMessageThreadHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSourceMessageThreadHistory");
    s.store_class_end();
  }
}

messageSourceForumTopicHistory::messageSourceForumTopicHistory() {
}

const std::int32_t messageSourceForumTopicHistory::ID;

void messageSourceForumTopicHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSourceForumTopicHistory");
    s.store_class_end();
  }
}

messageSourceDirectMessagesChatTopicHistory::messageSourceDirectMessagesChatTopicHistory() {
}

const std::int32_t messageSourceDirectMessagesChatTopicHistory::ID;

void messageSourceDirectMessagesChatTopicHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSourceDirectMessagesChatTopicHistory");
    s.store_class_end();
  }
}

messageSourceHistoryPreview::messageSourceHistoryPreview() {
}

const std::int32_t messageSourceHistoryPreview::ID;

void messageSourceHistoryPreview::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSourceHistoryPreview");
    s.store_class_end();
  }
}

messageSourceChatList::messageSourceChatList() {
}

const std::int32_t messageSourceChatList::ID;

void messageSourceChatList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSourceChatList");
    s.store_class_end();
  }
}

messageSourceSearch::messageSourceSearch() {
}

const std::int32_t messageSourceSearch::ID;

void messageSourceSearch::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSourceSearch");
    s.store_class_end();
  }
}

messageSourceChatEventLog::messageSourceChatEventLog() {
}

const std::int32_t messageSourceChatEventLog::ID;

void messageSourceChatEventLog::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSourceChatEventLog");
    s.store_class_end();
  }
}

messageSourceNotification::messageSourceNotification() {
}

const std::int32_t messageSourceNotification::ID;

void messageSourceNotification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSourceNotification");
    s.store_class_end();
  }
}

messageSourceScreenshot::messageSourceScreenshot() {
}

const std::int32_t messageSourceScreenshot::ID;

void messageSourceScreenshot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSourceScreenshot");
    s.store_class_end();
  }
}

messageSourceOther::messageSourceOther() {
}

const std::int32_t messageSourceOther::ID;

void messageSourceOther::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSourceOther");
    s.store_class_end();
  }
}

messageTopicThread::messageTopicThread()
  : message_thread_id_()
{}

messageTopicThread::messageTopicThread(int53 message_thread_id_)
  : message_thread_id_(message_thread_id_)
{}

const std::int32_t messageTopicThread::ID;

void messageTopicThread::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageTopicThread");
    s.store_field("message_thread_id", message_thread_id_);
    s.store_class_end();
  }
}

messageTopicForum::messageTopicForum()
  : forum_topic_id_()
{}

messageTopicForum::messageTopicForum(int32 forum_topic_id_)
  : forum_topic_id_(forum_topic_id_)
{}

const std::int32_t messageTopicForum::ID;

void messageTopicForum::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageTopicForum");
    s.store_field("forum_topic_id", forum_topic_id_);
    s.store_class_end();
  }
}

messageTopicDirectMessages::messageTopicDirectMessages()
  : direct_messages_chat_topic_id_()
{}

messageTopicDirectMessages::messageTopicDirectMessages(int53 direct_messages_chat_topic_id_)
  : direct_messages_chat_topic_id_(direct_messages_chat_topic_id_)
{}

const std::int32_t messageTopicDirectMessages::ID;

void messageTopicDirectMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageTopicDirectMessages");
    s.store_field("direct_messages_chat_topic_id", direct_messages_chat_topic_id_);
    s.store_class_end();
  }
}

messageTopicSavedMessages::messageTopicSavedMessages()
  : saved_messages_topic_id_()
{}

messageTopicSavedMessages::messageTopicSavedMessages(int53 saved_messages_topic_id_)
  : saved_messages_topic_id_(saved_messages_topic_id_)
{}

const std::int32_t messageTopicSavedMessages::ID;

void messageTopicSavedMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageTopicSavedMessages");
    s.store_field("saved_messages_topic_id", saved_messages_topic_id_);
    s.store_class_end();
  }
}

messageViewers::messageViewers()
  : viewers_()
{}

messageViewers::messageViewers(array<object_ptr<messageViewer>> &&viewers_)
  : viewers_(std::move(viewers_))
{}

const std::int32_t messageViewers::ID;

void messageViewers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageViewers");
    { s.store_vector_begin("viewers", viewers_.size()); for (const auto &_value : viewers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messages::messages()
  : total_count_()
  , messages_()
{}

messages::messages(int32 total_count_, array<object_ptr<message>> &&messages_)
  : total_count_(total_count_)
  , messages_(std::move(messages_))
{}

const std::int32_t messages::ID;

void messages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messages");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

notificationGroup::notificationGroup()
  : id_()
  , type_()
  , chat_id_()
  , total_count_()
  , notifications_()
{}

notificationGroup::notificationGroup(int32 id_, object_ptr<NotificationGroupType> &&type_, int53 chat_id_, int32 total_count_, array<object_ptr<notification>> &&notifications_)
  : id_(id_)
  , type_(std::move(type_))
  , chat_id_(chat_id_)
  , total_count_(total_count_)
  , notifications_(std::move(notifications_))
{}

const std::int32_t notificationGroup::ID;

void notificationGroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notificationGroup");
    s.store_field("id", id_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("chat_id", chat_id_);
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("notifications", notifications_.size()); for (const auto &_value : notifications_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

pageBlockCaption::pageBlockCaption()
  : text_()
  , credit_()
{}

pageBlockCaption::pageBlockCaption(object_ptr<RichText> &&text_, object_ptr<RichText> &&credit_)
  : text_(std::move(text_))
  , credit_(std::move(credit_))
{}

const std::int32_t pageBlockCaption::ID;

void pageBlockCaption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockCaption");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_object_field("credit", static_cast<const BaseObject *>(credit_.get()));
    s.store_class_end();
  }
}

paidMediaPreview::paidMediaPreview()
  : width_()
  , height_()
  , duration_()
  , minithumbnail_()
{}

paidMediaPreview::paidMediaPreview(int32 width_, int32 height_, int32 duration_, object_ptr<minithumbnail> &&minithumbnail_)
  : width_(width_)
  , height_(height_)
  , duration_(duration_)
  , minithumbnail_(std::move(minithumbnail_))
{}

const std::int32_t paidMediaPreview::ID;

void paidMediaPreview::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paidMediaPreview");
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_field("duration", duration_);
    s.store_object_field("minithumbnail", static_cast<const BaseObject *>(minithumbnail_.get()));
    s.store_class_end();
  }
}

paidMediaPhoto::paidMediaPhoto()
  : photo_()
{}

paidMediaPhoto::paidMediaPhoto(object_ptr<photo> &&photo_)
  : photo_(std::move(photo_))
{}

const std::int32_t paidMediaPhoto::ID;

void paidMediaPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paidMediaPhoto");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

paidMediaVideo::paidMediaVideo()
  : video_()
  , cover_()
  , start_timestamp_()
{}

paidMediaVideo::paidMediaVideo(object_ptr<video> &&video_, object_ptr<photo> &&cover_, int32 start_timestamp_)
  : video_(std::move(video_))
  , cover_(std::move(cover_))
  , start_timestamp_(start_timestamp_)
{}

const std::int32_t paidMediaVideo::ID;

void paidMediaVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paidMediaVideo");
    s.store_object_field("video", static_cast<const BaseObject *>(video_.get()));
    s.store_object_field("cover", static_cast<const BaseObject *>(cover_.get()));
    s.store_field("start_timestamp", start_timestamp_);
    s.store_class_end();
  }
}

paidMediaUnsupported::paidMediaUnsupported() {
}

const std::int32_t paidMediaUnsupported::ID;

void paidMediaUnsupported::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paidMediaUnsupported");
    s.store_class_end();
  }
}

passportElementTypePersonalDetails::passportElementTypePersonalDetails() {
}

const std::int32_t passportElementTypePersonalDetails::ID;

void passportElementTypePersonalDetails::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementTypePersonalDetails");
    s.store_class_end();
  }
}

passportElementTypePassport::passportElementTypePassport() {
}

const std::int32_t passportElementTypePassport::ID;

void passportElementTypePassport::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementTypePassport");
    s.store_class_end();
  }
}

passportElementTypeDriverLicense::passportElementTypeDriverLicense() {
}

const std::int32_t passportElementTypeDriverLicense::ID;

void passportElementTypeDriverLicense::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementTypeDriverLicense");
    s.store_class_end();
  }
}

passportElementTypeIdentityCard::passportElementTypeIdentityCard() {
}

const std::int32_t passportElementTypeIdentityCard::ID;

void passportElementTypeIdentityCard::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementTypeIdentityCard");
    s.store_class_end();
  }
}

passportElementTypeInternalPassport::passportElementTypeInternalPassport() {
}

const std::int32_t passportElementTypeInternalPassport::ID;

void passportElementTypeInternalPassport::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementTypeInternalPassport");
    s.store_class_end();
  }
}

passportElementTypeAddress::passportElementTypeAddress() {
}

const std::int32_t passportElementTypeAddress::ID;

void passportElementTypeAddress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementTypeAddress");
    s.store_class_end();
  }
}

passportElementTypeUtilityBill::passportElementTypeUtilityBill() {
}

const std::int32_t passportElementTypeUtilityBill::ID;

void passportElementTypeUtilityBill::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementTypeUtilityBill");
    s.store_class_end();
  }
}

passportElementTypeBankStatement::passportElementTypeBankStatement() {
}

const std::int32_t passportElementTypeBankStatement::ID;

void passportElementTypeBankStatement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementTypeBankStatement");
    s.store_class_end();
  }
}

passportElementTypeRentalAgreement::passportElementTypeRentalAgreement() {
}

const std::int32_t passportElementTypeRentalAgreement::ID;

void passportElementTypeRentalAgreement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementTypeRentalAgreement");
    s.store_class_end();
  }
}

passportElementTypePassportRegistration::passportElementTypePassportRegistration() {
}

const std::int32_t passportElementTypePassportRegistration::ID;

void passportElementTypePassportRegistration::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementTypePassportRegistration");
    s.store_class_end();
  }
}

passportElementTypeTemporaryRegistration::passportElementTypeTemporaryRegistration() {
}

const std::int32_t passportElementTypeTemporaryRegistration::ID;

void passportElementTypeTemporaryRegistration::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementTypeTemporaryRegistration");
    s.store_class_end();
  }
}

passportElementTypePhoneNumber::passportElementTypePhoneNumber() {
}

const std::int32_t passportElementTypePhoneNumber::ID;

void passportElementTypePhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementTypePhoneNumber");
    s.store_class_end();
  }
}

passportElementTypeEmailAddress::passportElementTypeEmailAddress() {
}

const std::int32_t passportElementTypeEmailAddress::ID;

void passportElementTypeEmailAddress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementTypeEmailAddress");
    s.store_class_end();
  }
}

paymentReceipt::paymentReceipt()
  : product_info_()
  , date_()
  , seller_bot_user_id_()
  , type_()
{}

paymentReceipt::paymentReceipt(object_ptr<productInfo> &&product_info_, int32 date_, int53 seller_bot_user_id_, object_ptr<PaymentReceiptType> &&type_)
  : product_info_(std::move(product_info_))
  , date_(date_)
  , seller_bot_user_id_(seller_bot_user_id_)
  , type_(std::move(type_))
{}

const std::int32_t paymentReceipt::ID;

void paymentReceipt::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paymentReceipt");
    s.store_object_field("product_info", static_cast<const BaseObject *>(product_info_.get()));
    s.store_field("date", date_);
    s.store_field("seller_bot_user_id", seller_bot_user_id_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

profileAccentColor::profileAccentColor()
  : id_()
  , light_theme_colors_()
  , dark_theme_colors_()
  , min_supergroup_chat_boost_level_()
  , min_channel_chat_boost_level_()
{}

profileAccentColor::profileAccentColor(int32 id_, object_ptr<profileAccentColors> &&light_theme_colors_, object_ptr<profileAccentColors> &&dark_theme_colors_, int32 min_supergroup_chat_boost_level_, int32 min_channel_chat_boost_level_)
  : id_(id_)
  , light_theme_colors_(std::move(light_theme_colors_))
  , dark_theme_colors_(std::move(dark_theme_colors_))
  , min_supergroup_chat_boost_level_(min_supergroup_chat_boost_level_)
  , min_channel_chat_boost_level_(min_channel_chat_boost_level_)
{}

const std::int32_t profileAccentColor::ID;

void profileAccentColor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "profileAccentColor");
    s.store_field("id", id_);
    s.store_object_field("light_theme_colors", static_cast<const BaseObject *>(light_theme_colors_.get()));
    s.store_object_field("dark_theme_colors", static_cast<const BaseObject *>(dark_theme_colors_.get()));
    s.store_field("min_supergroup_chat_boost_level", min_supergroup_chat_boost_level_);
    s.store_field("min_channel_chat_boost_level", min_channel_chat_boost_level_);
    s.store_class_end();
  }
}

publicForwardMessage::publicForwardMessage()
  : message_()
{}

publicForwardMessage::publicForwardMessage(object_ptr<message> &&message_)
  : message_(std::move(message_))
{}

const std::int32_t publicForwardMessage::ID;

void publicForwardMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "publicForwardMessage");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

publicForwardStory::publicForwardStory()
  : story_()
{}

publicForwardStory::publicForwardStory(object_ptr<story> &&story_)
  : story_(std::move(story_))
{}

const std::int32_t publicForwardStory::ID;

void publicForwardStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "publicForwardStory");
    s.store_object_field("story", static_cast<const BaseObject *>(story_.get()));
    s.store_class_end();
  }
}

pushMessageContentHidden::pushMessageContentHidden()
  : is_pinned_()
{}

pushMessageContentHidden::pushMessageContentHidden(bool is_pinned_)
  : is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentHidden::ID;

void pushMessageContentHidden::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentHidden");
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentAnimation::pushMessageContentAnimation()
  : animation_()
  , caption_()
  , is_pinned_()
{}

pushMessageContentAnimation::pushMessageContentAnimation(object_ptr<animation> &&animation_, string const &caption_, bool is_pinned_)
  : animation_(std::move(animation_))
  , caption_(caption_)
  , is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentAnimation::ID;

void pushMessageContentAnimation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentAnimation");
    s.store_object_field("animation", static_cast<const BaseObject *>(animation_.get()));
    s.store_field("caption", caption_);
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentAudio::pushMessageContentAudio()
  : audio_()
  , is_pinned_()
{}

pushMessageContentAudio::pushMessageContentAudio(object_ptr<audio> &&audio_, bool is_pinned_)
  : audio_(std::move(audio_))
  , is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentAudio::ID;

void pushMessageContentAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentAudio");
    s.store_object_field("audio", static_cast<const BaseObject *>(audio_.get()));
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentContact::pushMessageContentContact()
  : name_()
  , is_pinned_()
{}

pushMessageContentContact::pushMessageContentContact(string const &name_, bool is_pinned_)
  : name_(name_)
  , is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentContact::ID;

void pushMessageContentContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentContact");
    s.store_field("name", name_);
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentContactRegistered::pushMessageContentContactRegistered()
  : as_premium_account_()
{}

pushMessageContentContactRegistered::pushMessageContentContactRegistered(bool as_premium_account_)
  : as_premium_account_(as_premium_account_)
{}

const std::int32_t pushMessageContentContactRegistered::ID;

void pushMessageContentContactRegistered::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentContactRegistered");
    s.store_field("as_premium_account", as_premium_account_);
    s.store_class_end();
  }
}

pushMessageContentDocument::pushMessageContentDocument()
  : document_()
  , is_pinned_()
{}

pushMessageContentDocument::pushMessageContentDocument(object_ptr<document> &&document_, bool is_pinned_)
  : document_(std::move(document_))
  , is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentDocument::ID;

void pushMessageContentDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentDocument");
    s.store_object_field("document", static_cast<const BaseObject *>(document_.get()));
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentGame::pushMessageContentGame()
  : title_()
  , is_pinned_()
{}

pushMessageContentGame::pushMessageContentGame(string const &title_, bool is_pinned_)
  : title_(title_)
  , is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentGame::ID;

void pushMessageContentGame::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentGame");
    s.store_field("title", title_);
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentGameScore::pushMessageContentGameScore()
  : title_()
  , score_()
  , is_pinned_()
{}

pushMessageContentGameScore::pushMessageContentGameScore(string const &title_, int32 score_, bool is_pinned_)
  : title_(title_)
  , score_(score_)
  , is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentGameScore::ID;

void pushMessageContentGameScore::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentGameScore");
    s.store_field("title", title_);
    s.store_field("score", score_);
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentInvoice::pushMessageContentInvoice()
  : price_()
  , is_pinned_()
{}

pushMessageContentInvoice::pushMessageContentInvoice(string const &price_, bool is_pinned_)
  : price_(price_)
  , is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentInvoice::ID;

void pushMessageContentInvoice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentInvoice");
    s.store_field("price", price_);
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentLocation::pushMessageContentLocation()
  : is_live_()
  , is_pinned_()
{}

pushMessageContentLocation::pushMessageContentLocation(bool is_live_, bool is_pinned_)
  : is_live_(is_live_)
  , is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentLocation::ID;

void pushMessageContentLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentLocation");
    s.store_field("is_live", is_live_);
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentPaidMedia::pushMessageContentPaidMedia()
  : star_count_()
  , is_pinned_()
{}

pushMessageContentPaidMedia::pushMessageContentPaidMedia(int53 star_count_, bool is_pinned_)
  : star_count_(star_count_)
  , is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentPaidMedia::ID;

void pushMessageContentPaidMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentPaidMedia");
    s.store_field("star_count", star_count_);
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentPhoto::pushMessageContentPhoto()
  : photo_()
  , caption_()
  , is_secret_()
  , is_pinned_()
{}

pushMessageContentPhoto::pushMessageContentPhoto(object_ptr<photo> &&photo_, string const &caption_, bool is_secret_, bool is_pinned_)
  : photo_(std::move(photo_))
  , caption_(caption_)
  , is_secret_(is_secret_)
  , is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentPhoto::ID;

void pushMessageContentPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentPhoto");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_field("caption", caption_);
    s.store_field("is_secret", is_secret_);
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentPoll::pushMessageContentPoll()
  : question_()
  , is_regular_()
  , is_pinned_()
{}

pushMessageContentPoll::pushMessageContentPoll(string const &question_, bool is_regular_, bool is_pinned_)
  : question_(question_)
  , is_regular_(is_regular_)
  , is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentPoll::ID;

void pushMessageContentPoll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentPoll");
    s.store_field("question", question_);
    s.store_field("is_regular", is_regular_);
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentPremiumGiftCode::pushMessageContentPremiumGiftCode()
  : month_count_()
{}

pushMessageContentPremiumGiftCode::pushMessageContentPremiumGiftCode(int32 month_count_)
  : month_count_(month_count_)
{}

const std::int32_t pushMessageContentPremiumGiftCode::ID;

void pushMessageContentPremiumGiftCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentPremiumGiftCode");
    s.store_field("month_count", month_count_);
    s.store_class_end();
  }
}

pushMessageContentGiveaway::pushMessageContentGiveaway()
  : winner_count_()
  , prize_()
  , is_pinned_()
{}

pushMessageContentGiveaway::pushMessageContentGiveaway(int32 winner_count_, object_ptr<GiveawayPrize> &&prize_, bool is_pinned_)
  : winner_count_(winner_count_)
  , prize_(std::move(prize_))
  , is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentGiveaway::ID;

void pushMessageContentGiveaway::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentGiveaway");
    s.store_field("winner_count", winner_count_);
    s.store_object_field("prize", static_cast<const BaseObject *>(prize_.get()));
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentGift::pushMessageContentGift()
  : star_count_()
  , is_prepaid_upgrade_()
{}

pushMessageContentGift::pushMessageContentGift(int53 star_count_, bool is_prepaid_upgrade_)
  : star_count_(star_count_)
  , is_prepaid_upgrade_(is_prepaid_upgrade_)
{}

const std::int32_t pushMessageContentGift::ID;

void pushMessageContentGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentGift");
    s.store_field("star_count", star_count_);
    s.store_field("is_prepaid_upgrade", is_prepaid_upgrade_);
    s.store_class_end();
  }
}

pushMessageContentUpgradedGift::pushMessageContentUpgradedGift()
  : is_upgrade_()
  , is_prepaid_upgrade_()
{}

pushMessageContentUpgradedGift::pushMessageContentUpgradedGift(bool is_upgrade_, bool is_prepaid_upgrade_)
  : is_upgrade_(is_upgrade_)
  , is_prepaid_upgrade_(is_prepaid_upgrade_)
{}

const std::int32_t pushMessageContentUpgradedGift::ID;

void pushMessageContentUpgradedGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentUpgradedGift");
    s.store_field("is_upgrade", is_upgrade_);
    s.store_field("is_prepaid_upgrade", is_prepaid_upgrade_);
    s.store_class_end();
  }
}

pushMessageContentScreenshotTaken::pushMessageContentScreenshotTaken() {
}

const std::int32_t pushMessageContentScreenshotTaken::ID;

void pushMessageContentScreenshotTaken::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentScreenshotTaken");
    s.store_class_end();
  }
}

pushMessageContentSticker::pushMessageContentSticker()
  : sticker_()
  , emoji_()
  , is_pinned_()
{}

pushMessageContentSticker::pushMessageContentSticker(object_ptr<sticker> &&sticker_, string const &emoji_, bool is_pinned_)
  : sticker_(std::move(sticker_))
  , emoji_(emoji_)
  , is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentSticker::ID;

void pushMessageContentSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentSticker");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_field("emoji", emoji_);
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentStory::pushMessageContentStory()
  : is_mention_()
  , is_pinned_()
{}

pushMessageContentStory::pushMessageContentStory(bool is_mention_, bool is_pinned_)
  : is_mention_(is_mention_)
  , is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentStory::ID;

void pushMessageContentStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentStory");
    s.store_field("is_mention", is_mention_);
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentText::pushMessageContentText()
  : text_()
  , is_pinned_()
{}

pushMessageContentText::pushMessageContentText(string const &text_, bool is_pinned_)
  : text_(text_)
  , is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentText::ID;

void pushMessageContentText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentText");
    s.store_field("text", text_);
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentChecklist::pushMessageContentChecklist()
  : title_()
  , is_pinned_()
{}

pushMessageContentChecklist::pushMessageContentChecklist(string const &title_, bool is_pinned_)
  : title_(title_)
  , is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentChecklist::ID;

void pushMessageContentChecklist::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentChecklist");
    s.store_field("title", title_);
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentVideo::pushMessageContentVideo()
  : video_()
  , caption_()
  , is_secret_()
  , is_pinned_()
{}

pushMessageContentVideo::pushMessageContentVideo(object_ptr<video> &&video_, string const &caption_, bool is_secret_, bool is_pinned_)
  : video_(std::move(video_))
  , caption_(caption_)
  , is_secret_(is_secret_)
  , is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentVideo::ID;

void pushMessageContentVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentVideo");
    s.store_object_field("video", static_cast<const BaseObject *>(video_.get()));
    s.store_field("caption", caption_);
    s.store_field("is_secret", is_secret_);
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentVideoNote::pushMessageContentVideoNote()
  : video_note_()
  , is_pinned_()
{}

pushMessageContentVideoNote::pushMessageContentVideoNote(object_ptr<videoNote> &&video_note_, bool is_pinned_)
  : video_note_(std::move(video_note_))
  , is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentVideoNote::ID;

void pushMessageContentVideoNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentVideoNote");
    s.store_object_field("video_note", static_cast<const BaseObject *>(video_note_.get()));
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentVoiceNote::pushMessageContentVoiceNote()
  : voice_note_()
  , is_pinned_()
{}

pushMessageContentVoiceNote::pushMessageContentVoiceNote(object_ptr<voiceNote> &&voice_note_, bool is_pinned_)
  : voice_note_(std::move(voice_note_))
  , is_pinned_(is_pinned_)
{}

const std::int32_t pushMessageContentVoiceNote::ID;

void pushMessageContentVoiceNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentVoiceNote");
    s.store_object_field("voice_note", static_cast<const BaseObject *>(voice_note_.get()));
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

pushMessageContentBasicGroupChatCreate::pushMessageContentBasicGroupChatCreate() {
}

const std::int32_t pushMessageContentBasicGroupChatCreate::ID;

void pushMessageContentBasicGroupChatCreate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentBasicGroupChatCreate");
    s.store_class_end();
  }
}

pushMessageContentVideoChatStarted::pushMessageContentVideoChatStarted() {
}

const std::int32_t pushMessageContentVideoChatStarted::ID;

void pushMessageContentVideoChatStarted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentVideoChatStarted");
    s.store_class_end();
  }
}

pushMessageContentVideoChatEnded::pushMessageContentVideoChatEnded() {
}

const std::int32_t pushMessageContentVideoChatEnded::ID;

void pushMessageContentVideoChatEnded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentVideoChatEnded");
    s.store_class_end();
  }
}

pushMessageContentInviteVideoChatParticipants::pushMessageContentInviteVideoChatParticipants()
  : is_current_user_()
{}

pushMessageContentInviteVideoChatParticipants::pushMessageContentInviteVideoChatParticipants(bool is_current_user_)
  : is_current_user_(is_current_user_)
{}

const std::int32_t pushMessageContentInviteVideoChatParticipants::ID;

void pushMessageContentInviteVideoChatParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentInviteVideoChatParticipants");
    s.store_field("is_current_user", is_current_user_);
    s.store_class_end();
  }
}

pushMessageContentChatAddMembers::pushMessageContentChatAddMembers()
  : member_name_()
  , is_current_user_()
  , is_returned_()
{}

pushMessageContentChatAddMembers::pushMessageContentChatAddMembers(string const &member_name_, bool is_current_user_, bool is_returned_)
  : member_name_(member_name_)
  , is_current_user_(is_current_user_)
  , is_returned_(is_returned_)
{}

const std::int32_t pushMessageContentChatAddMembers::ID;

void pushMessageContentChatAddMembers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentChatAddMembers");
    s.store_field("member_name", member_name_);
    s.store_field("is_current_user", is_current_user_);
    s.store_field("is_returned", is_returned_);
    s.store_class_end();
  }
}

pushMessageContentChatChangePhoto::pushMessageContentChatChangePhoto() {
}

const std::int32_t pushMessageContentChatChangePhoto::ID;

void pushMessageContentChatChangePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentChatChangePhoto");
    s.store_class_end();
  }
}

pushMessageContentChatChangeTitle::pushMessageContentChatChangeTitle()
  : title_()
{}

pushMessageContentChatChangeTitle::pushMessageContentChatChangeTitle(string const &title_)
  : title_(title_)
{}

const std::int32_t pushMessageContentChatChangeTitle::ID;

void pushMessageContentChatChangeTitle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentChatChangeTitle");
    s.store_field("title", title_);
    s.store_class_end();
  }
}

pushMessageContentChatSetBackground::pushMessageContentChatSetBackground()
  : is_same_()
{}

pushMessageContentChatSetBackground::pushMessageContentChatSetBackground(bool is_same_)
  : is_same_(is_same_)
{}

const std::int32_t pushMessageContentChatSetBackground::ID;

void pushMessageContentChatSetBackground::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentChatSetBackground");
    s.store_field("is_same", is_same_);
    s.store_class_end();
  }
}

pushMessageContentChatSetTheme::pushMessageContentChatSetTheme()
  : name_()
{}

pushMessageContentChatSetTheme::pushMessageContentChatSetTheme(string const &name_)
  : name_(name_)
{}

const std::int32_t pushMessageContentChatSetTheme::ID;

void pushMessageContentChatSetTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentChatSetTheme");
    s.store_field("name", name_);
    s.store_class_end();
  }
}

pushMessageContentChatDeleteMember::pushMessageContentChatDeleteMember()
  : member_name_()
  , is_current_user_()
  , is_left_()
{}

pushMessageContentChatDeleteMember::pushMessageContentChatDeleteMember(string const &member_name_, bool is_current_user_, bool is_left_)
  : member_name_(member_name_)
  , is_current_user_(is_current_user_)
  , is_left_(is_left_)
{}

const std::int32_t pushMessageContentChatDeleteMember::ID;

void pushMessageContentChatDeleteMember::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentChatDeleteMember");
    s.store_field("member_name", member_name_);
    s.store_field("is_current_user", is_current_user_);
    s.store_field("is_left", is_left_);
    s.store_class_end();
  }
}

pushMessageContentChatJoinByLink::pushMessageContentChatJoinByLink() {
}

const std::int32_t pushMessageContentChatJoinByLink::ID;

void pushMessageContentChatJoinByLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentChatJoinByLink");
    s.store_class_end();
  }
}

pushMessageContentChatJoinByRequest::pushMessageContentChatJoinByRequest() {
}

const std::int32_t pushMessageContentChatJoinByRequest::ID;

void pushMessageContentChatJoinByRequest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentChatJoinByRequest");
    s.store_class_end();
  }
}

pushMessageContentRecurringPayment::pushMessageContentRecurringPayment()
  : amount_()
{}

pushMessageContentRecurringPayment::pushMessageContentRecurringPayment(string const &amount_)
  : amount_(amount_)
{}

const std::int32_t pushMessageContentRecurringPayment::ID;

void pushMessageContentRecurringPayment::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentRecurringPayment");
    s.store_field("amount", amount_);
    s.store_class_end();
  }
}

pushMessageContentSuggestProfilePhoto::pushMessageContentSuggestProfilePhoto() {
}

const std::int32_t pushMessageContentSuggestProfilePhoto::ID;

void pushMessageContentSuggestProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentSuggestProfilePhoto");
    s.store_class_end();
  }
}

pushMessageContentSuggestBirthdate::pushMessageContentSuggestBirthdate() {
}

const std::int32_t pushMessageContentSuggestBirthdate::ID;

void pushMessageContentSuggestBirthdate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentSuggestBirthdate");
    s.store_class_end();
  }
}

pushMessageContentProximityAlertTriggered::pushMessageContentProximityAlertTriggered()
  : distance_()
{}

pushMessageContentProximityAlertTriggered::pushMessageContentProximityAlertTriggered(int32 distance_)
  : distance_(distance_)
{}

const std::int32_t pushMessageContentProximityAlertTriggered::ID;

void pushMessageContentProximityAlertTriggered::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentProximityAlertTriggered");
    s.store_field("distance", distance_);
    s.store_class_end();
  }
}

pushMessageContentChecklistTasksAdded::pushMessageContentChecklistTasksAdded()
  : task_count_()
{}

pushMessageContentChecklistTasksAdded::pushMessageContentChecklistTasksAdded(int32 task_count_)
  : task_count_(task_count_)
{}

const std::int32_t pushMessageContentChecklistTasksAdded::ID;

void pushMessageContentChecklistTasksAdded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentChecklistTasksAdded");
    s.store_field("task_count", task_count_);
    s.store_class_end();
  }
}

pushMessageContentChecklistTasksDone::pushMessageContentChecklistTasksDone()
  : task_count_()
{}

pushMessageContentChecklistTasksDone::pushMessageContentChecklistTasksDone(int32 task_count_)
  : task_count_(task_count_)
{}

const std::int32_t pushMessageContentChecklistTasksDone::ID;

void pushMessageContentChecklistTasksDone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentChecklistTasksDone");
    s.store_field("task_count", task_count_);
    s.store_class_end();
  }
}

pushMessageContentMessageForwards::pushMessageContentMessageForwards()
  : total_count_()
{}

pushMessageContentMessageForwards::pushMessageContentMessageForwards(int32 total_count_)
  : total_count_(total_count_)
{}

const std::int32_t pushMessageContentMessageForwards::ID;

void pushMessageContentMessageForwards::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentMessageForwards");
    s.store_field("total_count", total_count_);
    s.store_class_end();
  }
}

pushMessageContentMediaAlbum::pushMessageContentMediaAlbum()
  : total_count_()
  , has_photos_()
  , has_videos_()
  , has_audios_()
  , has_documents_()
{}

pushMessageContentMediaAlbum::pushMessageContentMediaAlbum(int32 total_count_, bool has_photos_, bool has_videos_, bool has_audios_, bool has_documents_)
  : total_count_(total_count_)
  , has_photos_(has_photos_)
  , has_videos_(has_videos_)
  , has_audios_(has_audios_)
  , has_documents_(has_documents_)
{}

const std::int32_t pushMessageContentMediaAlbum::ID;

void pushMessageContentMediaAlbum::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushMessageContentMediaAlbum");
    s.store_field("total_count", total_count_);
    s.store_field("has_photos", has_photos_);
    s.store_field("has_videos", has_videos_);
    s.store_field("has_audios", has_audios_);
    s.store_field("has_documents", has_documents_);
    s.store_class_end();
  }
}

remoteFile::remoteFile()
  : id_()
  , unique_id_()
  , is_uploading_active_()
  , is_uploading_completed_()
  , uploaded_size_()
{}

remoteFile::remoteFile(string const &id_, string const &unique_id_, bool is_uploading_active_, bool is_uploading_completed_, int53 uploaded_size_)
  : id_(id_)
  , unique_id_(unique_id_)
  , is_uploading_active_(is_uploading_active_)
  , is_uploading_completed_(is_uploading_completed_)
  , uploaded_size_(uploaded_size_)
{}

const std::int32_t remoteFile::ID;

void remoteFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "remoteFile");
    s.store_field("id", id_);
    s.store_field("unique_id", unique_id_);
    s.store_field("is_uploading_active", is_uploading_active_);
    s.store_field("is_uploading_completed", is_uploading_completed_);
    s.store_field("uploaded_size", uploaded_size_);
    s.store_class_end();
  }
}

reportStoryResultOk::reportStoryResultOk() {
}

const std::int32_t reportStoryResultOk::ID;

void reportStoryResultOk::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportStoryResultOk");
    s.store_class_end();
  }
}

reportStoryResultOptionRequired::reportStoryResultOptionRequired()
  : title_()
  , options_()
{}

reportStoryResultOptionRequired::reportStoryResultOptionRequired(string const &title_, array<object_ptr<reportOption>> &&options_)
  : title_(title_)
  , options_(std::move(options_))
{}

const std::int32_t reportStoryResultOptionRequired::ID;

void reportStoryResultOptionRequired::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportStoryResultOptionRequired");
    s.store_field("title", title_);
    { s.store_vector_begin("options", options_.size()); for (const auto &_value : options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

reportStoryResultTextRequired::reportStoryResultTextRequired()
  : option_id_()
  , is_optional_()
{}

reportStoryResultTextRequired::reportStoryResultTextRequired(bytes const &option_id_, bool is_optional_)
  : option_id_(std::move(option_id_))
  , is_optional_(is_optional_)
{}

const std::int32_t reportStoryResultTextRequired::ID;

void reportStoryResultTextRequired::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportStoryResultTextRequired");
    s.store_bytes_field("option_id", option_id_);
    s.store_field("is_optional", is_optional_);
    s.store_class_end();
  }
}

savedMessagesTopic::savedMessagesTopic()
  : id_()
  , type_()
  , is_pinned_()
  , order_()
  , last_message_()
  , draft_message_()
{}

savedMessagesTopic::savedMessagesTopic(int53 id_, object_ptr<SavedMessagesTopicType> &&type_, bool is_pinned_, int64 order_, object_ptr<message> &&last_message_, object_ptr<draftMessage> &&draft_message_)
  : id_(id_)
  , type_(std::move(type_))
  , is_pinned_(is_pinned_)
  , order_(order_)
  , last_message_(std::move(last_message_))
  , draft_message_(std::move(draft_message_))
{}

const std::int32_t savedMessagesTopic::ID;

void savedMessagesTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "savedMessagesTopic");
    s.store_field("id", id_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("is_pinned", is_pinned_);
    s.store_field("order", order_);
    s.store_object_field("last_message", static_cast<const BaseObject *>(last_message_.get()));
    s.store_object_field("draft_message", static_cast<const BaseObject *>(draft_message_.get()));
    s.store_class_end();
  }
}

savedMessagesTopicTypeMyNotes::savedMessagesTopicTypeMyNotes() {
}

const std::int32_t savedMessagesTopicTypeMyNotes::ID;

void savedMessagesTopicTypeMyNotes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "savedMessagesTopicTypeMyNotes");
    s.store_class_end();
  }
}

savedMessagesTopicTypeAuthorHidden::savedMessagesTopicTypeAuthorHidden() {
}

const std::int32_t savedMessagesTopicTypeAuthorHidden::ID;

void savedMessagesTopicTypeAuthorHidden::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "savedMessagesTopicTypeAuthorHidden");
    s.store_class_end();
  }
}

savedMessagesTopicTypeSavedFromChat::savedMessagesTopicTypeSavedFromChat()
  : chat_id_()
{}

savedMessagesTopicTypeSavedFromChat::savedMessagesTopicTypeSavedFromChat(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t savedMessagesTopicTypeSavedFromChat::ID;

void savedMessagesTopicTypeSavedFromChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "savedMessagesTopicTypeSavedFromChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

secretChat::secretChat()
  : id_()
  , user_id_()
  , state_()
  , is_outbound_()
  , key_hash_()
  , layer_()
{}

secretChat::secretChat(int32 id_, int53 user_id_, object_ptr<SecretChatState> &&state_, bool is_outbound_, bytes const &key_hash_, int32 layer_)
  : id_(id_)
  , user_id_(user_id_)
  , state_(std::move(state_))
  , is_outbound_(is_outbound_)
  , key_hash_(std::move(key_hash_))
  , layer_(layer_)
{}

const std::int32_t secretChat::ID;

void secretChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "secretChat");
    s.store_field("id", id_);
    s.store_field("user_id", user_id_);
    s.store_object_field("state", static_cast<const BaseObject *>(state_.get()));
    s.store_field("is_outbound", is_outbound_);
    s.store_bytes_field("key_hash", key_hash_);
    s.store_field("layer", layer_);
    s.store_class_end();
  }
}

starAmount::starAmount()
  : star_count_()
  , nanostar_count_()
{}

starAmount::starAmount(int53 star_count_, int32 nanostar_count_)
  : star_count_(star_count_)
  , nanostar_count_(nanostar_count_)
{}

const std::int32_t starAmount::ID;

void starAmount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starAmount");
    s.store_field("star_count", star_count_);
    s.store_field("nanostar_count", nanostar_count_);
    s.store_class_end();
  }
}

starSubscriptions::starSubscriptions()
  : star_amount_()
  , subscriptions_()
  , required_star_count_()
  , next_offset_()
{}

starSubscriptions::starSubscriptions(object_ptr<starAmount> &&star_amount_, array<object_ptr<starSubscription>> &&subscriptions_, int53 required_star_count_, string const &next_offset_)
  : star_amount_(std::move(star_amount_))
  , subscriptions_(std::move(subscriptions_))
  , required_star_count_(required_star_count_)
  , next_offset_(next_offset_)
{}

const std::int32_t starSubscriptions::ID;

void starSubscriptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starSubscriptions");
    s.store_object_field("star_amount", static_cast<const BaseObject *>(star_amount_.get()));
    { s.store_vector_begin("subscriptions", subscriptions_.size()); for (const auto &_value : subscriptions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("required_star_count", required_star_count_);
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

stickerSets::stickerSets()
  : total_count_()
  , sets_()
{}

stickerSets::stickerSets(int32 total_count_, array<object_ptr<stickerSetInfo>> &&sets_)
  : total_count_(total_count_)
  , sets_(std::move(sets_))
{}

const std::int32_t stickerSets::ID;

void stickerSets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickerSets");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("sets", sets_.size()); for (const auto &_value : sets_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

storyAreaTypeLocation::storyAreaTypeLocation()
  : location_()
  , address_()
{}

storyAreaTypeLocation::storyAreaTypeLocation(object_ptr<location> &&location_, object_ptr<locationAddress> &&address_)
  : location_(std::move(location_))
  , address_(std::move(address_))
{}

const std::int32_t storyAreaTypeLocation::ID;

void storyAreaTypeLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyAreaTypeLocation");
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_object_field("address", static_cast<const BaseObject *>(address_.get()));
    s.store_class_end();
  }
}

storyAreaTypeVenue::storyAreaTypeVenue()
  : venue_()
{}

storyAreaTypeVenue::storyAreaTypeVenue(object_ptr<venue> &&venue_)
  : venue_(std::move(venue_))
{}

const std::int32_t storyAreaTypeVenue::ID;

void storyAreaTypeVenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyAreaTypeVenue");
    s.store_object_field("venue", static_cast<const BaseObject *>(venue_.get()));
    s.store_class_end();
  }
}

storyAreaTypeSuggestedReaction::storyAreaTypeSuggestedReaction()
  : reaction_type_()
  , total_count_()
  , is_dark_()
  , is_flipped_()
{}

storyAreaTypeSuggestedReaction::storyAreaTypeSuggestedReaction(object_ptr<ReactionType> &&reaction_type_, int32 total_count_, bool is_dark_, bool is_flipped_)
  : reaction_type_(std::move(reaction_type_))
  , total_count_(total_count_)
  , is_dark_(is_dark_)
  , is_flipped_(is_flipped_)
{}

const std::int32_t storyAreaTypeSuggestedReaction::ID;

void storyAreaTypeSuggestedReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyAreaTypeSuggestedReaction");
    s.store_object_field("reaction_type", static_cast<const BaseObject *>(reaction_type_.get()));
    s.store_field("total_count", total_count_);
    s.store_field("is_dark", is_dark_);
    s.store_field("is_flipped", is_flipped_);
    s.store_class_end();
  }
}

storyAreaTypeMessage::storyAreaTypeMessage()
  : chat_id_()
  , message_id_()
{}

storyAreaTypeMessage::storyAreaTypeMessage(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t storyAreaTypeMessage::ID;

void storyAreaTypeMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyAreaTypeMessage");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

storyAreaTypeLink::storyAreaTypeLink()
  : url_()
{}

storyAreaTypeLink::storyAreaTypeLink(string const &url_)
  : url_(url_)
{}

const std::int32_t storyAreaTypeLink::ID;

void storyAreaTypeLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyAreaTypeLink");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

storyAreaTypeWeather::storyAreaTypeWeather()
  : temperature_()
  , emoji_()
  , background_color_()
{}

storyAreaTypeWeather::storyAreaTypeWeather(double temperature_, string const &emoji_, int32 background_color_)
  : temperature_(temperature_)
  , emoji_(emoji_)
  , background_color_(background_color_)
{}

const std::int32_t storyAreaTypeWeather::ID;

void storyAreaTypeWeather::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyAreaTypeWeather");
    s.store_field("temperature", temperature_);
    s.store_field("emoji", emoji_);
    s.store_field("background_color", background_color_);
    s.store_class_end();
  }
}

storyAreaTypeUpgradedGift::storyAreaTypeUpgradedGift()
  : gift_name_()
{}

storyAreaTypeUpgradedGift::storyAreaTypeUpgradedGift(string const &gift_name_)
  : gift_name_(gift_name_)
{}

const std::int32_t storyAreaTypeUpgradedGift::ID;

void storyAreaTypeUpgradedGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyAreaTypeUpgradedGift");
    s.store_field("gift_name", gift_name_);
    s.store_class_end();
  }
}

storyRepostInfo::storyRepostInfo()
  : origin_()
  , is_content_modified_()
{}

storyRepostInfo::storyRepostInfo(object_ptr<StoryOrigin> &&origin_, bool is_content_modified_)
  : origin_(std::move(origin_))
  , is_content_modified_(is_content_modified_)
{}

const std::int32_t storyRepostInfo::ID;

void storyRepostInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyRepostInfo");
    s.store_object_field("origin", static_cast<const BaseObject *>(origin_.get()));
    s.store_field("is_content_modified", is_content_modified_);
    s.store_class_end();
  }
}

suggestedPostStatePending::suggestedPostStatePending() {
}

const std::int32_t suggestedPostStatePending::ID;

void suggestedPostStatePending::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedPostStatePending");
    s.store_class_end();
  }
}

suggestedPostStateApproved::suggestedPostStateApproved() {
}

const std::int32_t suggestedPostStateApproved::ID;

void suggestedPostStateApproved::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedPostStateApproved");
    s.store_class_end();
  }
}

suggestedPostStateDeclined::suggestedPostStateDeclined() {
}

const std::int32_t suggestedPostStateDeclined::ID;

void suggestedPostStateDeclined::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedPostStateDeclined");
    s.store_class_end();
  }
}

testVectorString::testVectorString()
  : value_()
{}

testVectorString::testVectorString(array<string> &&value_)
  : value_(std::move(value_))
{}

const std::int32_t testVectorString::ID;

void testVectorString::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testVectorString");
    { s.store_vector_begin("value", value_.size()); for (const auto &_value : value_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

textEntity::textEntity()
  : offset_()
  , length_()
  , type_()
{}

textEntity::textEntity(int32 offset_, int32 length_, object_ptr<TextEntityType> &&type_)
  : offset_(offset_)
  , length_(length_)
  , type_(std::move(type_))
{}

const std::int32_t textEntity::ID;

void textEntity::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textEntity");
    s.store_field("offset", offset_);
    s.store_field("length", length_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

timeZones::timeZones()
  : time_zones_()
{}

timeZones::timeZones(array<object_ptr<timeZone>> &&time_zones_)
  : time_zones_(std::move(time_zones_))
{}

const std::int32_t timeZones::ID;

void timeZones::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "timeZones");
    { s.store_vector_begin("time_zones", time_zones_.size()); for (const auto &_value : time_zones_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

tonTransactions::tonTransactions()
  : ton_amount_()
  , transactions_()
  , next_offset_()
{}

tonTransactions::tonTransactions(int53 ton_amount_, array<object_ptr<tonTransaction>> &&transactions_, string const &next_offset_)
  : ton_amount_(ton_amount_)
  , transactions_(std::move(transactions_))
  , next_offset_(next_offset_)
{}

const std::int32_t tonTransactions::ID;

void tonTransactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "tonTransactions");
    s.store_field("ton_amount", ton_amount_);
    { s.store_vector_begin("transactions", transactions_.size()); for (const auto &_value : transactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

unconfirmedSession::unconfirmedSession()
  : id_()
  , log_in_date_()
  , device_model_()
  , location_()
{}

unconfirmedSession::unconfirmedSession(int64 id_, int32 log_in_date_, string const &device_model_, string const &location_)
  : id_(id_)
  , log_in_date_(log_in_date_)
  , device_model_(device_model_)
  , location_(location_)
{}

const std::int32_t unconfirmedSession::ID;

void unconfirmedSession::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "unconfirmedSession");
    s.store_field("id", id_);
    s.store_field("log_in_date", log_in_date_);
    s.store_field("device_model", device_model_);
    s.store_field("location", location_);
    s.store_class_end();
  }
}

upgradedGift::upgradedGift()
  : id_()
  , regular_gift_id_()
  , publisher_chat_id_()
  , title_()
  , name_()
  , number_()
  , total_upgraded_count_()
  , max_upgraded_count_()
  , is_premium_()
  , is_theme_available_()
  , used_theme_chat_id_()
  , host_id_()
  , owner_id_()
  , owner_address_()
  , owner_name_()
  , gift_address_()
  , model_()
  , symbol_()
  , backdrop_()
  , original_details_()
  , colors_()
  , resale_parameters_()
  , value_currency_()
  , value_amount_()
{}

upgradedGift::upgradedGift(int64 id_, int64 regular_gift_id_, int53 publisher_chat_id_, string const &title_, string const &name_, int32 number_, int32 total_upgraded_count_, int32 max_upgraded_count_, bool is_premium_, bool is_theme_available_, int53 used_theme_chat_id_, object_ptr<MessageSender> &&host_id_, object_ptr<MessageSender> &&owner_id_, string const &owner_address_, string const &owner_name_, string const &gift_address_, object_ptr<upgradedGiftModel> &&model_, object_ptr<upgradedGiftSymbol> &&symbol_, object_ptr<upgradedGiftBackdrop> &&backdrop_, object_ptr<upgradedGiftOriginalDetails> &&original_details_, object_ptr<upgradedGiftColors> &&colors_, object_ptr<giftResaleParameters> &&resale_parameters_, string const &value_currency_, int53 value_amount_)
  : id_(id_)
  , regular_gift_id_(regular_gift_id_)
  , publisher_chat_id_(publisher_chat_id_)
  , title_(title_)
  , name_(name_)
  , number_(number_)
  , total_upgraded_count_(total_upgraded_count_)
  , max_upgraded_count_(max_upgraded_count_)
  , is_premium_(is_premium_)
  , is_theme_available_(is_theme_available_)
  , used_theme_chat_id_(used_theme_chat_id_)
  , host_id_(std::move(host_id_))
  , owner_id_(std::move(owner_id_))
  , owner_address_(owner_address_)
  , owner_name_(owner_name_)
  , gift_address_(gift_address_)
  , model_(std::move(model_))
  , symbol_(std::move(symbol_))
  , backdrop_(std::move(backdrop_))
  , original_details_(std::move(original_details_))
  , colors_(std::move(colors_))
  , resale_parameters_(std::move(resale_parameters_))
  , value_currency_(value_currency_)
  , value_amount_(value_amount_)
{}

const std::int32_t upgradedGift::ID;

void upgradedGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradedGift");
    s.store_field("id", id_);
    s.store_field("regular_gift_id", regular_gift_id_);
    s.store_field("publisher_chat_id", publisher_chat_id_);
    s.store_field("title", title_);
    s.store_field("name", name_);
    s.store_field("number", number_);
    s.store_field("total_upgraded_count", total_upgraded_count_);
    s.store_field("max_upgraded_count", max_upgraded_count_);
    s.store_field("is_premium", is_premium_);
    s.store_field("is_theme_available", is_theme_available_);
    s.store_field("used_theme_chat_id", used_theme_chat_id_);
    s.store_object_field("host_id", static_cast<const BaseObject *>(host_id_.get()));
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_field("owner_address", owner_address_);
    s.store_field("owner_name", owner_name_);
    s.store_field("gift_address", gift_address_);
    s.store_object_field("model", static_cast<const BaseObject *>(model_.get()));
    s.store_object_field("symbol", static_cast<const BaseObject *>(symbol_.get()));
    s.store_object_field("backdrop", static_cast<const BaseObject *>(backdrop_.get()));
    s.store_object_field("original_details", static_cast<const BaseObject *>(original_details_.get()));
    s.store_object_field("colors", static_cast<const BaseObject *>(colors_.get()));
    s.store_object_field("resale_parameters", static_cast<const BaseObject *>(resale_parameters_.get()));
    s.store_field("value_currency", value_currency_);
    s.store_field("value_amount", value_amount_);
    s.store_class_end();
  }
}

userFullInfo::userFullInfo()
  : personal_photo_()
  , photo_()
  , public_photo_()
  , block_list_()
  , can_be_called_()
  , supports_video_calls_()
  , has_private_calls_()
  , has_private_forwards_()
  , has_restricted_voice_and_video_note_messages_()
  , has_posted_to_profile_stories_()
  , has_sponsored_messages_enabled_()
  , need_phone_number_privacy_exception_()
  , set_chat_background_()
  , bio_()
  , birthdate_()
  , personal_chat_id_()
  , gift_count_()
  , group_in_common_count_()
  , incoming_paid_message_star_count_()
  , outgoing_paid_message_star_count_()
  , gift_settings_()
  , bot_verification_()
  , main_profile_tab_()
  , first_profile_audio_()
  , rating_()
  , pending_rating_()
  , pending_rating_date_()
  , note_()
  , business_info_()
  , bot_info_()
{}

userFullInfo::userFullInfo(object_ptr<chatPhoto> &&personal_photo_, object_ptr<chatPhoto> &&photo_, object_ptr<chatPhoto> &&public_photo_, object_ptr<BlockList> &&block_list_, bool can_be_called_, bool supports_video_calls_, bool has_private_calls_, bool has_private_forwards_, bool has_restricted_voice_and_video_note_messages_, bool has_posted_to_profile_stories_, bool has_sponsored_messages_enabled_, bool need_phone_number_privacy_exception_, bool set_chat_background_, object_ptr<formattedText> &&bio_, object_ptr<birthdate> &&birthdate_, int53 personal_chat_id_, int32 gift_count_, int32 group_in_common_count_, int53 incoming_paid_message_star_count_, int53 outgoing_paid_message_star_count_, object_ptr<giftSettings> &&gift_settings_, object_ptr<botVerification> &&bot_verification_, object_ptr<ProfileTab> &&main_profile_tab_, object_ptr<audio> &&first_profile_audio_, object_ptr<userRating> &&rating_, object_ptr<userRating> &&pending_rating_, int32 pending_rating_date_, object_ptr<formattedText> &&note_, object_ptr<businessInfo> &&business_info_, object_ptr<botInfo> &&bot_info_)
  : personal_photo_(std::move(personal_photo_))
  , photo_(std::move(photo_))
  , public_photo_(std::move(public_photo_))
  , block_list_(std::move(block_list_))
  , can_be_called_(can_be_called_)
  , supports_video_calls_(supports_video_calls_)
  , has_private_calls_(has_private_calls_)
  , has_private_forwards_(has_private_forwards_)
  , has_restricted_voice_and_video_note_messages_(has_restricted_voice_and_video_note_messages_)
  , has_posted_to_profile_stories_(has_posted_to_profile_stories_)
  , has_sponsored_messages_enabled_(has_sponsored_messages_enabled_)
  , need_phone_number_privacy_exception_(need_phone_number_privacy_exception_)
  , set_chat_background_(set_chat_background_)
  , bio_(std::move(bio_))
  , birthdate_(std::move(birthdate_))
  , personal_chat_id_(personal_chat_id_)
  , gift_count_(gift_count_)
  , group_in_common_count_(group_in_common_count_)
  , incoming_paid_message_star_count_(incoming_paid_message_star_count_)
  , outgoing_paid_message_star_count_(outgoing_paid_message_star_count_)
  , gift_settings_(std::move(gift_settings_))
  , bot_verification_(std::move(bot_verification_))
  , main_profile_tab_(std::move(main_profile_tab_))
  , first_profile_audio_(std::move(first_profile_audio_))
  , rating_(std::move(rating_))
  , pending_rating_(std::move(pending_rating_))
  , pending_rating_date_(pending_rating_date_)
  , note_(std::move(note_))
  , business_info_(std::move(business_info_))
  , bot_info_(std::move(bot_info_))
{}

const std::int32_t userFullInfo::ID;

void userFullInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userFullInfo");
    s.store_object_field("personal_photo", static_cast<const BaseObject *>(personal_photo_.get()));
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_object_field("public_photo", static_cast<const BaseObject *>(public_photo_.get()));
    s.store_object_field("block_list", static_cast<const BaseObject *>(block_list_.get()));
    s.store_field("can_be_called", can_be_called_);
    s.store_field("supports_video_calls", supports_video_calls_);
    s.store_field("has_private_calls", has_private_calls_);
    s.store_field("has_private_forwards", has_private_forwards_);
    s.store_field("has_restricted_voice_and_video_note_messages", has_restricted_voice_and_video_note_messages_);
    s.store_field("has_posted_to_profile_stories", has_posted_to_profile_stories_);
    s.store_field("has_sponsored_messages_enabled", has_sponsored_messages_enabled_);
    s.store_field("need_phone_number_privacy_exception", need_phone_number_privacy_exception_);
    s.store_field("set_chat_background", set_chat_background_);
    s.store_object_field("bio", static_cast<const BaseObject *>(bio_.get()));
    s.store_object_field("birthdate", static_cast<const BaseObject *>(birthdate_.get()));
    s.store_field("personal_chat_id", personal_chat_id_);
    s.store_field("gift_count", gift_count_);
    s.store_field("group_in_common_count", group_in_common_count_);
    s.store_field("incoming_paid_message_star_count", incoming_paid_message_star_count_);
    s.store_field("outgoing_paid_message_star_count", outgoing_paid_message_star_count_);
    s.store_object_field("gift_settings", static_cast<const BaseObject *>(gift_settings_.get()));
    s.store_object_field("bot_verification", static_cast<const BaseObject *>(bot_verification_.get()));
    s.store_object_field("main_profile_tab", static_cast<const BaseObject *>(main_profile_tab_.get()));
    s.store_object_field("first_profile_audio", static_cast<const BaseObject *>(first_profile_audio_.get()));
    s.store_object_field("rating", static_cast<const BaseObject *>(rating_.get()));
    s.store_object_field("pending_rating", static_cast<const BaseObject *>(pending_rating_.get()));
    s.store_field("pending_rating_date", pending_rating_date_);
    s.store_object_field("note", static_cast<const BaseObject *>(note_.get()));
    s.store_object_field("business_info", static_cast<const BaseObject *>(business_info_.get()));
    s.store_object_field("bot_info", static_cast<const BaseObject *>(bot_info_.get()));
    s.store_class_end();
  }
}

userPrivacySettingRules::userPrivacySettingRules()
  : rules_()
{}

userPrivacySettingRules::userPrivacySettingRules(array<object_ptr<UserPrivacySettingRule>> &&rules_)
  : rules_(std::move(rules_))
{}

const std::int32_t userPrivacySettingRules::ID;

void userPrivacySettingRules::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userPrivacySettingRules");
    { s.store_vector_begin("rules", rules_.size()); for (const auto &_value : rules_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

addChatMembers::addChatMembers()
  : chat_id_()
  , user_ids_()
{}

addChatMembers::addChatMembers(int53 chat_id_, array<int53> &&user_ids_)
  : chat_id_(chat_id_)
  , user_ids_(std::move(user_ids_))
{}

const std::int32_t addChatMembers::ID;

void addChatMembers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addChatMembers");
    s.store_field("chat_id", chat_id_);
    { s.store_vector_begin("user_ids", user_ids_.size()); for (const auto &_value : user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

addQuickReplyShortcutMessageAlbum::addQuickReplyShortcutMessageAlbum()
  : shortcut_name_()
  , reply_to_message_id_()
  , input_message_contents_()
{}

addQuickReplyShortcutMessageAlbum::addQuickReplyShortcutMessageAlbum(string const &shortcut_name_, int53 reply_to_message_id_, array<object_ptr<InputMessageContent>> &&input_message_contents_)
  : shortcut_name_(shortcut_name_)
  , reply_to_message_id_(reply_to_message_id_)
  , input_message_contents_(std::move(input_message_contents_))
{}

const std::int32_t addQuickReplyShortcutMessageAlbum::ID;

void addQuickReplyShortcutMessageAlbum::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addQuickReplyShortcutMessageAlbum");
    s.store_field("shortcut_name", shortcut_name_);
    s.store_field("reply_to_message_id", reply_to_message_id_);
    { s.store_vector_begin("input_message_contents", input_message_contents_.size()); for (const auto &_value : input_message_contents_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

approveSuggestedPost::approveSuggestedPost()
  : chat_id_()
  , message_id_()
  , send_date_()
{}

approveSuggestedPost::approveSuggestedPost(int53 chat_id_, int53 message_id_, int32 send_date_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , send_date_(send_date_)
{}

const std::int32_t approveSuggestedPost::ID;

void approveSuggestedPost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "approveSuggestedPost");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("send_date", send_date_);
    s.store_class_end();
  }
}

assignStoreTransaction::assignStoreTransaction()
  : transaction_()
  , purpose_()
{}

assignStoreTransaction::assignStoreTransaction(object_ptr<StoreTransaction> &&transaction_, object_ptr<StorePaymentPurpose> &&purpose_)
  : transaction_(std::move(transaction_))
  , purpose_(std::move(purpose_))
{}

const std::int32_t assignStoreTransaction::ID;

void assignStoreTransaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "assignStoreTransaction");
    s.store_object_field("transaction", static_cast<const BaseObject *>(transaction_.get()));
    s.store_object_field("purpose", static_cast<const BaseObject *>(purpose_.get()));
    s.store_class_end();
  }
}

banGroupCallParticipants::banGroupCallParticipants()
  : group_call_id_()
  , user_ids_()
{}

banGroupCallParticipants::banGroupCallParticipants(int32 group_call_id_, array<int64> &&user_ids_)
  : group_call_id_(group_call_id_)
  , user_ids_(std::move(user_ids_))
{}

const std::int32_t banGroupCallParticipants::ID;

void banGroupCallParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "banGroupCallParticipants");
    s.store_field("group_call_id", group_call_id_);
    { s.store_vector_begin("user_ids", user_ids_.size()); for (const auto &_value : user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

canPurchaseFromStore::canPurchaseFromStore()
  : purpose_()
{}

canPurchaseFromStore::canPurchaseFromStore(object_ptr<StorePaymentPurpose> &&purpose_)
  : purpose_(std::move(purpose_))
{}

const std::int32_t canPurchaseFromStore::ID;

void canPurchaseFromStore::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canPurchaseFromStore");
    s.store_object_field("purpose", static_cast<const BaseObject *>(purpose_.get()));
    s.store_class_end();
  }
}

canSendGift::canSendGift()
  : gift_id_()
{}

canSendGift::canSendGift(int64 gift_id_)
  : gift_id_(gift_id_)
{}

const std::int32_t canSendGift::ID;

void canSendGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canSendGift");
    s.store_field("gift_id", gift_id_);
    s.store_class_end();
  }
}

checkAuthenticationPremiumPurchase::checkAuthenticationPremiumPurchase()
  : currency_()
  , amount_()
{}

checkAuthenticationPremiumPurchase::checkAuthenticationPremiumPurchase(string const &currency_, int53 amount_)
  : currency_(currency_)
  , amount_(amount_)
{}

const std::int32_t checkAuthenticationPremiumPurchase::ID;

void checkAuthenticationPremiumPurchase::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkAuthenticationPremiumPurchase");
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_class_end();
  }
}

checkEmailAddressVerificationCode::checkEmailAddressVerificationCode()
  : code_()
{}

checkEmailAddressVerificationCode::checkEmailAddressVerificationCode(string const &code_)
  : code_(code_)
{}

const std::int32_t checkEmailAddressVerificationCode::ID;

void checkEmailAddressVerificationCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkEmailAddressVerificationCode");
    s.store_field("code", code_);
    s.store_class_end();
  }
}

checkPremiumGiftCode::checkPremiumGiftCode()
  : code_()
{}

checkPremiumGiftCode::checkPremiumGiftCode(string const &code_)
  : code_(code_)
{}

const std::int32_t checkPremiumGiftCode::ID;

void checkPremiumGiftCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkPremiumGiftCode");
    s.store_field("code", code_);
    s.store_class_end();
  }
}

checkQuickReplyShortcutName::checkQuickReplyShortcutName()
  : name_()
{}

checkQuickReplyShortcutName::checkQuickReplyShortcutName(string const &name_)
  : name_(name_)
{}

const std::int32_t checkQuickReplyShortcutName::ID;

void checkQuickReplyShortcutName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkQuickReplyShortcutName");
    s.store_field("name", name_);
    s.store_class_end();
  }
}

closeChat::closeChat()
  : chat_id_()
{}

closeChat::closeChat(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t closeChat::ID;

void closeChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "closeChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

createCall::createCall()
  : user_id_()
  , protocol_()
  , is_video_()
{}

createCall::createCall(int53 user_id_, object_ptr<callProtocol> &&protocol_, bool is_video_)
  : user_id_(user_id_)
  , protocol_(std::move(protocol_))
  , is_video_(is_video_)
{}

const std::int32_t createCall::ID;

void createCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createCall");
    s.store_field("user_id", user_id_);
    s.store_object_field("protocol", static_cast<const BaseObject *>(protocol_.get()));
    s.store_field("is_video", is_video_);
    s.store_class_end();
  }
}

deleteDefaultBackground::deleteDefaultBackground()
  : for_dark_theme_()
{}

deleteDefaultBackground::deleteDefaultBackground(bool for_dark_theme_)
  : for_dark_theme_(for_dark_theme_)
{}

const std::int32_t deleteDefaultBackground::ID;

void deleteDefaultBackground::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteDefaultBackground");
    s.store_field("for_dark_theme", for_dark_theme_);
    s.store_class_end();
  }
}

deleteQuickReplyShortcutMessages::deleteQuickReplyShortcutMessages()
  : shortcut_id_()
  , message_ids_()
{}

deleteQuickReplyShortcutMessages::deleteQuickReplyShortcutMessages(int32 shortcut_id_, array<int53> &&message_ids_)
  : shortcut_id_(shortcut_id_)
  , message_ids_(std::move(message_ids_))
{}

const std::int32_t deleteQuickReplyShortcutMessages::ID;

void deleteQuickReplyShortcutMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteQuickReplyShortcutMessages");
    s.store_field("shortcut_id", shortcut_id_);
    { s.store_vector_begin("message_ids", message_ids_.size()); for (const auto &_value : message_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

deleteSavedCredentials::deleteSavedCredentials() {
}

const std::int32_t deleteSavedCredentials::ID;

void deleteSavedCredentials::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteSavedCredentials");
    s.store_class_end();
  }
}

downloadFile::downloadFile()
  : file_id_()
  , priority_()
  , offset_()
  , limit_()
  , synchronous_()
{}

downloadFile::downloadFile(int32 file_id_, int32 priority_, int53 offset_, int53 limit_, bool synchronous_)
  : file_id_(file_id_)
  , priority_(priority_)
  , offset_(offset_)
  , limit_(limit_)
  , synchronous_(synchronous_)
{}

const std::int32_t downloadFile::ID;

void downloadFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "downloadFile");
    s.store_field("file_id", file_id_);
    s.store_field("priority", priority_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_field("synchronous", synchronous_);
    s.store_class_end();
  }
}

getBotName::getBotName()
  : bot_user_id_()
  , language_code_()
{}

getBotName::getBotName(int53 bot_user_id_, string const &language_code_)
  : bot_user_id_(bot_user_id_)
  , language_code_(language_code_)
{}

const std::int32_t getBotName::ID;

void getBotName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getBotName");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("language_code", language_code_);
    s.store_class_end();
  }
}

getBotSimilarBotCount::getBotSimilarBotCount()
  : bot_user_id_()
  , return_local_()
{}

getBotSimilarBotCount::getBotSimilarBotCount(int53 bot_user_id_, bool return_local_)
  : bot_user_id_(bot_user_id_)
  , return_local_(return_local_)
{}

const std::int32_t getBotSimilarBotCount::ID;

void getBotSimilarBotCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getBotSimilarBotCount");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("return_local", return_local_);
    s.store_class_end();
  }
}

getChat::getChat()
  : chat_id_()
{}

getChat::getChat(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t getChat::ID;

void getChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

getChatAdministrators::getChatAdministrators()
  : chat_id_()
{}

getChatAdministrators::getChatAdministrators(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t getChatAdministrators::ID;

void getChatAdministrators::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatAdministrators");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

getChatFolder::getChatFolder()
  : chat_folder_id_()
{}

getChatFolder::getChatFolder(int32 chat_folder_id_)
  : chat_folder_id_(chat_folder_id_)
{}

const std::int32_t getChatFolder::ID;

void getChatFolder::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatFolder");
    s.store_field("chat_folder_id", chat_folder_id_);
    s.store_class_end();
  }
}

getChatHistory::getChatHistory()
  : chat_id_()
  , from_message_id_()
  , offset_()
  , limit_()
  , only_local_()
{}

getChatHistory::getChatHistory(int53 chat_id_, int53 from_message_id_, int32 offset_, int32 limit_, bool only_local_)
  : chat_id_(chat_id_)
  , from_message_id_(from_message_id_)
  , offset_(offset_)
  , limit_(limit_)
  , only_local_(only_local_)
{}

const std::int32_t getChatHistory::ID;

void getChatHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatHistory");
    s.store_field("chat_id", chat_id_);
    s.store_field("from_message_id", from_message_id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_field("only_local", only_local_);
    s.store_class_end();
  }
}

getChatPostedToChatPageStories::getChatPostedToChatPageStories()
  : chat_id_()
  , from_story_id_()
  , limit_()
{}

getChatPostedToChatPageStories::getChatPostedToChatPageStories(int53 chat_id_, int32 from_story_id_, int32 limit_)
  : chat_id_(chat_id_)
  , from_story_id_(from_story_id_)
  , limit_(limit_)
{}

const std::int32_t getChatPostedToChatPageStories::ID;

void getChatPostedToChatPageStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatPostedToChatPageStories");
    s.store_field("chat_id", chat_id_);
    s.store_field("from_story_id", from_story_id_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getChatSimilarChatCount::getChatSimilarChatCount()
  : chat_id_()
  , return_local_()
{}

getChatSimilarChatCount::getChatSimilarChatCount(int53 chat_id_, bool return_local_)
  : chat_id_(chat_id_)
  , return_local_(return_local_)
{}

const std::int32_t getChatSimilarChatCount::ID;

void getChatSimilarChatCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatSimilarChatCount");
    s.store_field("chat_id", chat_id_);
    s.store_field("return_local", return_local_);
    s.store_class_end();
  }
}

getDefaultBackgroundCustomEmojiStickers::getDefaultBackgroundCustomEmojiStickers() {
}

const std::int32_t getDefaultBackgroundCustomEmojiStickers::ID;

void getDefaultBackgroundCustomEmojiStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getDefaultBackgroundCustomEmojiStickers");
    s.store_class_end();
  }
}

getDirectMessagesChatTopic::getDirectMessagesChatTopic()
  : chat_id_()
  , topic_id_()
{}

getDirectMessagesChatTopic::getDirectMessagesChatTopic(int53 chat_id_, int53 topic_id_)
  : chat_id_(chat_id_)
  , topic_id_(topic_id_)
{}

const std::int32_t getDirectMessagesChatTopic::ID;

void getDirectMessagesChatTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getDirectMessagesChatTopic");
    s.store_field("chat_id", chat_id_);
    s.store_field("topic_id", topic_id_);
    s.store_class_end();
  }
}

getDirectMessagesChatTopicMessageByDate::getDirectMessagesChatTopicMessageByDate()
  : chat_id_()
  , topic_id_()
  , date_()
{}

getDirectMessagesChatTopicMessageByDate::getDirectMessagesChatTopicMessageByDate(int53 chat_id_, int53 topic_id_, int32 date_)
  : chat_id_(chat_id_)
  , topic_id_(topic_id_)
  , date_(date_)
{}

const std::int32_t getDirectMessagesChatTopicMessageByDate::ID;

void getDirectMessagesChatTopicMessageByDate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getDirectMessagesChatTopicMessageByDate");
    s.store_field("chat_id", chat_id_);
    s.store_field("topic_id", topic_id_);
    s.store_field("date", date_);
    s.store_class_end();
  }
}

getEmojiCategories::getEmojiCategories()
  : type_()
{}

getEmojiCategories::getEmojiCategories(object_ptr<EmojiCategoryType> &&type_)
  : type_(std::move(type_))
{}

const std::int32_t getEmojiCategories::ID;

void getEmojiCategories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getEmojiCategories");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

getFile::getFile()
  : file_id_()
{}

getFile::getFile(int32 file_id_)
  : file_id_(file_id_)
{}

const std::int32_t getFile::ID;

void getFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getFile");
    s.store_field("file_id", file_id_);
    s.store_class_end();
  }
}

getFileDownloadedPrefixSize::getFileDownloadedPrefixSize()
  : file_id_()
  , offset_()
{}

getFileDownloadedPrefixSize::getFileDownloadedPrefixSize(int32 file_id_, int53 offset_)
  : file_id_(file_id_)
  , offset_(offset_)
{}

const std::int32_t getFileDownloadedPrefixSize::ID;

void getFileDownloadedPrefixSize::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getFileDownloadedPrefixSize");
    s.store_field("file_id", file_id_);
    s.store_field("offset", offset_);
    s.store_class_end();
  }
}

getForumTopic::getForumTopic()
  : chat_id_()
  , forum_topic_id_()
{}

getForumTopic::getForumTopic(int53 chat_id_, int32 forum_topic_id_)
  : chat_id_(chat_id_)
  , forum_topic_id_(forum_topic_id_)
{}

const std::int32_t getForumTopic::ID;

void getForumTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getForumTopic");
    s.store_field("chat_id", chat_id_);
    s.store_field("forum_topic_id", forum_topic_id_);
    s.store_class_end();
  }
}

getForumTopicHistory::getForumTopicHistory()
  : chat_id_()
  , forum_topic_id_()
  , from_message_id_()
  , offset_()
  , limit_()
{}

getForumTopicHistory::getForumTopicHistory(int53 chat_id_, int32 forum_topic_id_, int53 from_message_id_, int32 offset_, int32 limit_)
  : chat_id_(chat_id_)
  , forum_topic_id_(forum_topic_id_)
  , from_message_id_(from_message_id_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getForumTopicHistory::ID;

void getForumTopicHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getForumTopicHistory");
    s.store_field("chat_id", chat_id_);
    s.store_field("forum_topic_id", forum_topic_id_);
    s.store_field("from_message_id", from_message_id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getGiftUpgradePreview::getGiftUpgradePreview()
  : gift_id_()
{}

getGiftUpgradePreview::getGiftUpgradePreview(int64 gift_id_)
  : gift_id_(gift_id_)
{}

const std::int32_t getGiftUpgradePreview::ID;

void getGiftUpgradePreview::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getGiftUpgradePreview");
    s.store_field("gift_id", gift_id_);
    s.store_class_end();
  }
}

getInlineGameHighScores::getInlineGameHighScores()
  : inline_message_id_()
  , user_id_()
{}

getInlineGameHighScores::getInlineGameHighScores(string const &inline_message_id_, int53 user_id_)
  : inline_message_id_(inline_message_id_)
  , user_id_(user_id_)
{}

const std::int32_t getInlineGameHighScores::ID;

void getInlineGameHighScores::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getInlineGameHighScores");
    s.store_field("inline_message_id", inline_message_id_);
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

getLanguagePackString::getLanguagePackString()
  : language_pack_database_path_()
  , localization_target_()
  , language_pack_id_()
  , key_()
{}

getLanguagePackString::getLanguagePackString(string const &language_pack_database_path_, string const &localization_target_, string const &language_pack_id_, string const &key_)
  : language_pack_database_path_(language_pack_database_path_)
  , localization_target_(localization_target_)
  , language_pack_id_(language_pack_id_)
  , key_(key_)
{}

const std::int32_t getLanguagePackString::ID;

void getLanguagePackString::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getLanguagePackString");
    s.store_field("language_pack_database_path", language_pack_database_path_);
    s.store_field("localization_target", localization_target_);
    s.store_field("language_pack_id", language_pack_id_);
    s.store_field("key", key_);
    s.store_class_end();
  }
}

getLocalizationTargetInfo::getLocalizationTargetInfo()
  : only_local_()
{}

getLocalizationTargetInfo::getLocalizationTargetInfo(bool only_local_)
  : only_local_(only_local_)
{}

const std::int32_t getLocalizationTargetInfo::ID;

void getLocalizationTargetInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getLocalizationTargetInfo");
    s.store_field("only_local", only_local_);
    s.store_class_end();
  }
}

getLogStream::getLogStream() {
}

const std::int32_t getLogStream::ID;

void getLogStream::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getLogStream");
    s.store_class_end();
  }
}

getLoginUrl::getLoginUrl()
  : chat_id_()
  , message_id_()
  , button_id_()
  , allow_write_access_()
{}

getLoginUrl::getLoginUrl(int53 chat_id_, int53 message_id_, int53 button_id_, bool allow_write_access_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , button_id_(button_id_)
  , allow_write_access_(allow_write_access_)
{}

const std::int32_t getLoginUrl::ID;

void getLoginUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getLoginUrl");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("button_id", button_id_);
    s.store_field("allow_write_access", allow_write_access_);
    s.store_class_end();
  }
}

getMarkdownText::getMarkdownText()
  : text_()
{}

getMarkdownText::getMarkdownText(object_ptr<formattedText> &&text_)
  : text_(std::move(text_))
{}

const std::int32_t getMarkdownText::ID;

void getMarkdownText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMarkdownText");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

getMe::getMe() {
}

const std::int32_t getMe::ID;

void getMe::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMe");
    s.store_class_end();
  }
}

getMessageViewers::getMessageViewers()
  : chat_id_()
  , message_id_()
{}

getMessageViewers::getMessageViewers(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t getMessageViewers::ID;

void getMessageViewers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMessageViewers");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

getMessages::getMessages()
  : chat_id_()
  , message_ids_()
{}

getMessages::getMessages(int53 chat_id_, array<int53> &&message_ids_)
  : chat_id_(chat_id_)
  , message_ids_(std::move(message_ids_))
{}

const std::int32_t getMessages::ID;

void getMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMessages");
    s.store_field("chat_id", chat_id_);
    { s.store_vector_begin("message_ids", message_ids_.size()); for (const auto &_value : message_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

getPaymentReceipt::getPaymentReceipt()
  : chat_id_()
  , message_id_()
{}

getPaymentReceipt::getPaymentReceipt(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t getPaymentReceipt::ID;

void getPaymentReceipt::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPaymentReceipt");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

getPremiumFeatures::getPremiumFeatures()
  : source_()
{}

getPremiumFeatures::getPremiumFeatures(object_ptr<PremiumSource> &&source_)
  : source_(std::move(source_))
{}

const std::int32_t getPremiumFeatures::ID;

void getPremiumFeatures::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPremiumFeatures");
    s.store_object_field("source", static_cast<const BaseObject *>(source_.get()));
    s.store_class_end();
  }
}

getPushReceiverId::getPushReceiverId()
  : payload_()
{}

getPushReceiverId::getPushReceiverId(string const &payload_)
  : payload_(payload_)
{}

const std::int32_t getPushReceiverId::ID;

void getPushReceiverId::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPushReceiverId");
    s.store_field("payload", payload_);
    s.store_class_end();
  }
}

getRemoteFile::getRemoteFile()
  : remote_file_id_()
  , file_type_()
{}

getRemoteFile::getRemoteFile(string const &remote_file_id_, object_ptr<FileType> &&file_type_)
  : remote_file_id_(remote_file_id_)
  , file_type_(std::move(file_type_))
{}

const std::int32_t getRemoteFile::ID;

void getRemoteFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getRemoteFile");
    s.store_field("remote_file_id", remote_file_id_);
    s.store_object_field("file_type", static_cast<const BaseObject *>(file_type_.get()));
    s.store_class_end();
  }
}

getStarGiftPaymentOptions::getStarGiftPaymentOptions()
  : user_id_()
{}

getStarGiftPaymentOptions::getStarGiftPaymentOptions(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t getStarGiftPaymentOptions::ID;

void getStarGiftPaymentOptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStarGiftPaymentOptions");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

getUpgradedGiftEmojiStatuses::getUpgradedGiftEmojiStatuses() {
}

const std::int32_t getUpgradedGiftEmojiStatuses::ID;

void getUpgradedGiftEmojiStatuses::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getUpgradedGiftEmojiStatuses");
    s.store_class_end();
  }
}

getUpgradedGiftWithdrawalUrl::getUpgradedGiftWithdrawalUrl()
  : received_gift_id_()
  , password_()
{}

getUpgradedGiftWithdrawalUrl::getUpgradedGiftWithdrawalUrl(string const &received_gift_id_, string const &password_)
  : received_gift_id_(received_gift_id_)
  , password_(password_)
{}

const std::int32_t getUpgradedGiftWithdrawalUrl::ID;

void getUpgradedGiftWithdrawalUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getUpgradedGiftWithdrawalUrl");
    s.store_field("received_gift_id", received_gift_id_);
    s.store_field("password", password_);
    s.store_class_end();
  }
}

getUserSupportInfo::getUserSupportInfo()
  : user_id_()
{}

getUserSupportInfo::getUserSupportInfo(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t getUserSupportInfo::ID;

void getUserSupportInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getUserSupportInfo");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

getWebAppPlaceholder::getWebAppPlaceholder()
  : bot_user_id_()
{}

getWebAppPlaceholder::getWebAppPlaceholder(int53 bot_user_id_)
  : bot_user_id_(bot_user_id_)
{}

const std::int32_t getWebAppPlaceholder::ID;

void getWebAppPlaceholder::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getWebAppPlaceholder");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_class_end();
  }
}

hideContactCloseBirthdays::hideContactCloseBirthdays() {
}

const std::int32_t hideContactCloseBirthdays::ID;

void hideContactCloseBirthdays::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "hideContactCloseBirthdays");
    s.store_class_end();
  }
}

joinChatByInviteLink::joinChatByInviteLink()
  : invite_link_()
{}

joinChatByInviteLink::joinChatByInviteLink(string const &invite_link_)
  : invite_link_(invite_link_)
{}

const std::int32_t joinChatByInviteLink::ID;

void joinChatByInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "joinChatByInviteLink");
    s.store_field("invite_link", invite_link_);
    s.store_class_end();
  }
}

processChatJoinRequest::processChatJoinRequest()
  : chat_id_()
  , user_id_()
  , approve_()
{}

processChatJoinRequest::processChatJoinRequest(int53 chat_id_, int53 user_id_, bool approve_)
  : chat_id_(chat_id_)
  , user_id_(user_id_)
  , approve_(approve_)
{}

const std::int32_t processChatJoinRequest::ID;

void processChatJoinRequest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "processChatJoinRequest");
    s.store_field("chat_id", chat_id_);
    s.store_field("user_id", user_id_);
    s.store_field("approve", approve_);
    s.store_class_end();
  }
}

recognizeSpeech::recognizeSpeech()
  : chat_id_()
  , message_id_()
{}

recognizeSpeech::recognizeSpeech(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t recognizeSpeech::ID;

void recognizeSpeech::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "recognizeSpeech");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

removeRecentSticker::removeRecentSticker()
  : is_attached_()
  , sticker_()
{}

removeRecentSticker::removeRecentSticker(bool is_attached_, object_ptr<InputFile> &&sticker_)
  : is_attached_(is_attached_)
  , sticker_(std::move(sticker_))
{}

const std::int32_t removeRecentSticker::ID;

void removeRecentSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeRecentSticker");
    s.store_field("is_attached", is_attached_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

reorderActiveUsernames::reorderActiveUsernames()
  : usernames_()
{}

reorderActiveUsernames::reorderActiveUsernames(array<string> &&usernames_)
  : usernames_(std::move(usernames_))
{}

const std::int32_t reorderActiveUsernames::ID;

void reorderActiveUsernames::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reorderActiveUsernames");
    { s.store_vector_begin("usernames", usernames_.size()); for (const auto &_value : usernames_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

reorderStoryAlbumStories::reorderStoryAlbumStories()
  : chat_id_()
  , story_album_id_()
  , story_ids_()
{}

reorderStoryAlbumStories::reorderStoryAlbumStories(int53 chat_id_, int32 story_album_id_, array<int32> &&story_ids_)
  : chat_id_(chat_id_)
  , story_album_id_(story_album_id_)
  , story_ids_(std::move(story_ids_))
{}

const std::int32_t reorderStoryAlbumStories::ID;

void reorderStoryAlbumStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reorderStoryAlbumStories");
    s.store_field("chat_id", chat_id_);
    s.store_field("story_album_id", story_album_id_);
    { s.store_vector_begin("story_ids", story_ids_.size()); for (const auto &_value : story_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

resetInstalledBackgrounds::resetInstalledBackgrounds() {
}

const std::int32_t resetInstalledBackgrounds::ID;

void resetInstalledBackgrounds::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "resetInstalledBackgrounds");
    s.store_class_end();
  }
}

savePreparedInlineMessage::savePreparedInlineMessage()
  : user_id_()
  , result_()
  , chat_types_()
{}

savePreparedInlineMessage::savePreparedInlineMessage(int53 user_id_, object_ptr<InputInlineQueryResult> &&result_, object_ptr<targetChatTypes> &&chat_types_)
  : user_id_(user_id_)
  , result_(std::move(result_))
  , chat_types_(std::move(chat_types_))
{}

const std::int32_t savePreparedInlineMessage::ID;

void savePreparedInlineMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "savePreparedInlineMessage");
    s.store_field("user_id", user_id_);
    s.store_object_field("result", static_cast<const BaseObject *>(result_.get()));
    s.store_object_field("chat_types", static_cast<const BaseObject *>(chat_types_.get()));
    s.store_class_end();
  }
}

searchPublicMessagesByTag::searchPublicMessagesByTag()
  : tag_()
  , offset_()
  , limit_()
{}

searchPublicMessagesByTag::searchPublicMessagesByTag(string const &tag_, string const &offset_, int32 limit_)
  : tag_(tag_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t searchPublicMessagesByTag::ID;

void searchPublicMessagesByTag::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchPublicMessagesByTag");
    s.store_field("tag", tag_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

searchStickerSets::searchStickerSets()
  : sticker_type_()
  , query_()
{}

searchStickerSets::searchStickerSets(object_ptr<StickerType> &&sticker_type_, string const &query_)
  : sticker_type_(std::move(sticker_type_))
  , query_(query_)
{}

const std::int32_t searchStickerSets::ID;

void searchStickerSets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchStickerSets");
    s.store_object_field("sticker_type", static_cast<const BaseObject *>(sticker_type_.get()));
    s.store_field("query", query_);
    s.store_class_end();
  }
}

sendAuthenticationFirebaseSms::sendAuthenticationFirebaseSms()
  : token_()
{}

sendAuthenticationFirebaseSms::sendAuthenticationFirebaseSms(string const &token_)
  : token_(token_)
{}

const std::int32_t sendAuthenticationFirebaseSms::ID;

void sendAuthenticationFirebaseSms::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendAuthenticationFirebaseSms");
    s.store_field("token", token_);
    s.store_class_end();
  }
}

sendBusinessMessageAlbum::sendBusinessMessageAlbum()
  : business_connection_id_()
  , chat_id_()
  , reply_to_()
  , disable_notification_()
  , protect_content_()
  , effect_id_()
  , input_message_contents_()
{}

sendBusinessMessageAlbum::sendBusinessMessageAlbum(string const &business_connection_id_, int53 chat_id_, object_ptr<InputMessageReplyTo> &&reply_to_, bool disable_notification_, bool protect_content_, int64 effect_id_, array<object_ptr<InputMessageContent>> &&input_message_contents_)
  : business_connection_id_(business_connection_id_)
  , chat_id_(chat_id_)
  , reply_to_(std::move(reply_to_))
  , disable_notification_(disable_notification_)
  , protect_content_(protect_content_)
  , effect_id_(effect_id_)
  , input_message_contents_(std::move(input_message_contents_))
{}

const std::int32_t sendBusinessMessageAlbum::ID;

void sendBusinessMessageAlbum::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendBusinessMessageAlbum");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_field("chat_id", chat_id_);
    s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get()));
    s.store_field("disable_notification", disable_notification_);
    s.store_field("protect_content", protect_content_);
    s.store_field("effect_id", effect_id_);
    { s.store_vector_begin("input_message_contents", input_message_contents_.size()); for (const auto &_value : input_message_contents_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

sendCallLog::sendCallLog()
  : call_id_()
  , log_file_()
{}

sendCallLog::sendCallLog(int32 call_id_, object_ptr<InputFile> &&log_file_)
  : call_id_(call_id_)
  , log_file_(std::move(log_file_))
{}

const std::int32_t sendCallLog::ID;

void sendCallLog::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendCallLog");
    s.store_field("call_id", call_id_);
    s.store_object_field("log_file", static_cast<const BaseObject *>(log_file_.get()));
    s.store_class_end();
  }
}

sendChatAction::sendChatAction()
  : chat_id_()
  , topic_id_()
  , business_connection_id_()
  , action_()
{}

sendChatAction::sendChatAction(int53 chat_id_, object_ptr<MessageTopic> &&topic_id_, string const &business_connection_id_, object_ptr<ChatAction> &&action_)
  : chat_id_(chat_id_)
  , topic_id_(std::move(topic_id_))
  , business_connection_id_(business_connection_id_)
  , action_(std::move(action_))
{}

const std::int32_t sendChatAction::ID;

void sendChatAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendChatAction");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("topic_id", static_cast<const BaseObject *>(topic_id_.get()));
    s.store_field("business_connection_id", business_connection_id_);
    s.store_object_field("action", static_cast<const BaseObject *>(action_.get()));
    s.store_class_end();
  }
}

sendEmailAddressVerificationCode::sendEmailAddressVerificationCode()
  : email_address_()
{}

sendEmailAddressVerificationCode::sendEmailAddressVerificationCode(string const &email_address_)
  : email_address_(email_address_)
{}

const std::int32_t sendEmailAddressVerificationCode::ID;

void sendEmailAddressVerificationCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendEmailAddressVerificationCode");
    s.store_field("email_address", email_address_);
    s.store_class_end();
  }
}

sendGift::sendGift()
  : gift_id_()
  , owner_id_()
  , text_()
  , is_private_()
  , pay_for_upgrade_()
{}

sendGift::sendGift(int64 gift_id_, object_ptr<MessageSender> &&owner_id_, object_ptr<formattedText> &&text_, bool is_private_, bool pay_for_upgrade_)
  : gift_id_(gift_id_)
  , owner_id_(std::move(owner_id_))
  , text_(std::move(text_))
  , is_private_(is_private_)
  , pay_for_upgrade_(pay_for_upgrade_)
{}

const std::int32_t sendGift::ID;

void sendGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendGift");
    s.store_field("gift_id", gift_id_);
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("is_private", is_private_);
    s.store_field("pay_for_upgrade", pay_for_upgrade_);
    s.store_class_end();
  }
}

sendGroupCallMessage::sendGroupCallMessage()
  : group_call_id_()
  , text_()
{}

sendGroupCallMessage::sendGroupCallMessage(int32 group_call_id_, object_ptr<formattedText> &&text_)
  : group_call_id_(group_call_id_)
  , text_(std::move(text_))
{}

const std::int32_t sendGroupCallMessage::ID;

void sendGroupCallMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendGroupCallMessage");
    s.store_field("group_call_id", group_call_id_);
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

setBotProfilePhoto::setBotProfilePhoto()
  : bot_user_id_()
  , photo_()
{}

setBotProfilePhoto::setBotProfilePhoto(int53 bot_user_id_, object_ptr<InputChatPhoto> &&photo_)
  : bot_user_id_(bot_user_id_)
  , photo_(std::move(photo_))
{}

const std::int32_t setBotProfilePhoto::ID;

void setBotProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setBotProfilePhoto");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

setChatEmojiStatus::setChatEmojiStatus()
  : chat_id_()
  , emoji_status_()
{}

setChatEmojiStatus::setChatEmojiStatus(int53 chat_id_, object_ptr<emojiStatus> &&emoji_status_)
  : chat_id_(chat_id_)
  , emoji_status_(std::move(emoji_status_))
{}

const std::int32_t setChatEmojiStatus::ID;

void setChatEmojiStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatEmojiStatus");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("emoji_status", static_cast<const BaseObject *>(emoji_status_.get()));
    s.store_class_end();
  }
}

setChatPaidMessageStarCount::setChatPaidMessageStarCount()
  : chat_id_()
  , paid_message_star_count_()
{}

setChatPaidMessageStarCount::setChatPaidMessageStarCount(int53 chat_id_, int53 paid_message_star_count_)
  : chat_id_(chat_id_)
  , paid_message_star_count_(paid_message_star_count_)
{}

const std::int32_t setChatPaidMessageStarCount::ID;

void setChatPaidMessageStarCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatPaidMessageStarCount");
    s.store_field("chat_id", chat_id_);
    s.store_field("paid_message_star_count", paid_message_star_count_);
    s.store_class_end();
  }
}

setChatProfileAccentColor::setChatProfileAccentColor()
  : chat_id_()
  , profile_accent_color_id_()
  , profile_background_custom_emoji_id_()
{}

setChatProfileAccentColor::setChatProfileAccentColor(int53 chat_id_, int32 profile_accent_color_id_, int64 profile_background_custom_emoji_id_)
  : chat_id_(chat_id_)
  , profile_accent_color_id_(profile_accent_color_id_)
  , profile_background_custom_emoji_id_(profile_background_custom_emoji_id_)
{}

const std::int32_t setChatProfileAccentColor::ID;

void setChatProfileAccentColor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatProfileAccentColor");
    s.store_field("chat_id", chat_id_);
    s.store_field("profile_accent_color_id", profile_accent_color_id_);
    s.store_field("profile_background_custom_emoji_id", profile_background_custom_emoji_id_);
    s.store_class_end();
  }
}

setDefaultReactionType::setDefaultReactionType()
  : reaction_type_()
{}

setDefaultReactionType::setDefaultReactionType(object_ptr<ReactionType> &&reaction_type_)
  : reaction_type_(std::move(reaction_type_))
{}

const std::int32_t setDefaultReactionType::ID;

void setDefaultReactionType::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setDefaultReactionType");
    s.store_object_field("reaction_type", static_cast<const BaseObject *>(reaction_type_.get()));
    s.store_class_end();
  }
}

setForumTopicNotificationSettings::setForumTopicNotificationSettings()
  : chat_id_()
  , forum_topic_id_()
  , notification_settings_()
{}

setForumTopicNotificationSettings::setForumTopicNotificationSettings(int53 chat_id_, int32 forum_topic_id_, object_ptr<chatNotificationSettings> &&notification_settings_)
  : chat_id_(chat_id_)
  , forum_topic_id_(forum_topic_id_)
  , notification_settings_(std::move(notification_settings_))
{}

const std::int32_t setForumTopicNotificationSettings::ID;

void setForumTopicNotificationSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setForumTopicNotificationSettings");
    s.store_field("chat_id", chat_id_);
    s.store_field("forum_topic_id", forum_topic_id_);
    s.store_object_field("notification_settings", static_cast<const BaseObject *>(notification_settings_.get()));
    s.store_class_end();
  }
}

setGameScore::setGameScore()
  : chat_id_()
  , message_id_()
  , edit_message_()
  , user_id_()
  , score_()
  , force_()
{}

setGameScore::setGameScore(int53 chat_id_, int53 message_id_, bool edit_message_, int53 user_id_, int32 score_, bool force_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , edit_message_(edit_message_)
  , user_id_(user_id_)
  , score_(score_)
  , force_(force_)
{}

const std::int32_t setGameScore::ID;

void setGameScore::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setGameScore");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("edit_message", edit_message_);
    s.store_field("user_id", user_id_);
    s.store_field("score", score_);
    s.store_field("force", force_);
    s.store_class_end();
  }
}

setLogStream::setLogStream()
  : log_stream_()
{}

setLogStream::setLogStream(object_ptr<LogStream> &&log_stream_)
  : log_stream_(std::move(log_stream_))
{}

const std::int32_t setLogStream::ID;

void setLogStream::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setLogStream");
    s.store_object_field("log_stream", static_cast<const BaseObject *>(log_stream_.get()));
    s.store_class_end();
  }
}

setPassportElement::setPassportElement()
  : element_()
  , password_()
{}

setPassportElement::setPassportElement(object_ptr<InputPassportElement> &&element_, string const &password_)
  : element_(std::move(element_))
  , password_(password_)
{}

const std::int32_t setPassportElement::ID;

void setPassportElement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setPassportElement");
    s.store_object_field("element", static_cast<const BaseObject *>(element_.get()));
    s.store_field("password", password_);
    s.store_class_end();
  }
}

setProfileAudioPosition::setProfileAudioPosition()
  : file_id_()
  , after_file_id_()
{}

setProfileAudioPosition::setProfileAudioPosition(int32 file_id_, int32 after_file_id_)
  : file_id_(file_id_)
  , after_file_id_(after_file_id_)
{}

const std::int32_t setProfileAudioPosition::ID;

void setProfileAudioPosition::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setProfileAudioPosition");
    s.store_field("file_id", file_id_);
    s.store_field("after_file_id", after_file_id_);
    s.store_class_end();
  }
}

setStickerMaskPosition::setStickerMaskPosition()
  : sticker_()
  , mask_position_()
{}

setStickerMaskPosition::setStickerMaskPosition(object_ptr<InputFile> &&sticker_, object_ptr<maskPosition> &&mask_position_)
  : sticker_(std::move(sticker_))
  , mask_position_(std::move(mask_position_))
{}

const std::int32_t setStickerMaskPosition::ID;

void setStickerMaskPosition::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setStickerMaskPosition");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_object_field("mask_position", static_cast<const BaseObject *>(mask_position_.get()));
    s.store_class_end();
  }
}

setUserSupportInfo::setUserSupportInfo()
  : user_id_()
  , message_()
{}

setUserSupportInfo::setUserSupportInfo(int53 user_id_, object_ptr<formattedText> &&message_)
  : user_id_(user_id_)
  , message_(std::move(message_))
{}

const std::int32_t setUserSupportInfo::ID;

void setUserSupportInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setUserSupportInfo");
    s.store_field("user_id", user_id_);
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

shareUsersWithBot::shareUsersWithBot()
  : chat_id_()
  , message_id_()
  , button_id_()
  , shared_user_ids_()
  , only_check_()
{}

shareUsersWithBot::shareUsersWithBot(int53 chat_id_, int53 message_id_, int32 button_id_, array<int53> &&shared_user_ids_, bool only_check_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , button_id_(button_id_)
  , shared_user_ids_(std::move(shared_user_ids_))
  , only_check_(only_check_)
{}

const std::int32_t shareUsersWithBot::ID;

void shareUsersWithBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "shareUsersWithBot");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("button_id", button_id_);
    { s.store_vector_begin("shared_user_ids", shared_user_ids_.size()); for (const auto &_value : shared_user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("only_check", only_check_);
    s.store_class_end();
  }
}

suggestUserBirthdate::suggestUserBirthdate()
  : user_id_()
  , birthdate_()
{}

suggestUserBirthdate::suggestUserBirthdate(int53 user_id_, object_ptr<birthdate> &&birthdate_)
  : user_id_(user_id_)
  , birthdate_(std::move(birthdate_))
{}

const std::int32_t suggestUserBirthdate::ID;

void suggestUserBirthdate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestUserBirthdate");
    s.store_field("user_id", user_id_);
    s.store_object_field("birthdate", static_cast<const BaseObject *>(birthdate_.get()));
    s.store_class_end();
  }
}

suggestUserProfilePhoto::suggestUserProfilePhoto()
  : user_id_()
  , photo_()
{}

suggestUserProfilePhoto::suggestUserProfilePhoto(int53 user_id_, object_ptr<InputChatPhoto> &&photo_)
  : user_id_(user_id_)
  , photo_(std::move(photo_))
{}

const std::int32_t suggestUserProfilePhoto::ID;

void suggestUserProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestUserProfilePhoto");
    s.store_field("user_id", user_id_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

testCallVectorString::testCallVectorString()
  : x_()
{}

testCallVectorString::testCallVectorString(array<string> &&x_)
  : x_(std::move(x_))
{}

const std::int32_t testCallVectorString::ID;

void testCallVectorString::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testCallVectorString");
    { s.store_vector_begin("x", x_.size()); for (const auto &_value : x_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

testNetwork::testNetwork() {
}

const std::int32_t testNetwork::ID;

void testNetwork::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testNetwork");
    s.store_class_end();
  }
}

testReturnError::testReturnError()
  : error_()
{}

testReturnError::testReturnError(object_ptr<error> &&error_)
  : error_(std::move(error_))
{}

const std::int32_t testReturnError::ID;

void testReturnError::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testReturnError");
    s.store_object_field("error", static_cast<const BaseObject *>(error_.get()));
    s.store_class_end();
  }
}

toggleBusinessConnectedBotChatIsPaused::toggleBusinessConnectedBotChatIsPaused()
  : chat_id_()
  , is_paused_()
{}

toggleBusinessConnectedBotChatIsPaused::toggleBusinessConnectedBotChatIsPaused(int53 chat_id_, bool is_paused_)
  : chat_id_(chat_id_)
  , is_paused_(is_paused_)
{}

const std::int32_t toggleBusinessConnectedBotChatIsPaused::ID;

void toggleBusinessConnectedBotChatIsPaused::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleBusinessConnectedBotChatIsPaused");
    s.store_field("chat_id", chat_id_);
    s.store_field("is_paused", is_paused_);
    s.store_class_end();
  }
}

toggleDownloadIsPaused::toggleDownloadIsPaused()
  : file_id_()
  , is_paused_()
{}

toggleDownloadIsPaused::toggleDownloadIsPaused(int32 file_id_, bool is_paused_)
  : file_id_(file_id_)
  , is_paused_(is_paused_)
{}

const std::int32_t toggleDownloadIsPaused::ID;

void toggleDownloadIsPaused::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleDownloadIsPaused");
    s.store_field("file_id", file_id_);
    s.store_field("is_paused", is_paused_);
    s.store_class_end();
  }
}

toggleForumTopicIsPinned::toggleForumTopicIsPinned()
  : chat_id_()
  , forum_topic_id_()
  , is_pinned_()
{}

toggleForumTopicIsPinned::toggleForumTopicIsPinned(int53 chat_id_, int32 forum_topic_id_, bool is_pinned_)
  : chat_id_(chat_id_)
  , forum_topic_id_(forum_topic_id_)
  , is_pinned_(is_pinned_)
{}

const std::int32_t toggleForumTopicIsPinned::ID;

void toggleForumTopicIsPinned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleForumTopicIsPinned");
    s.store_field("chat_id", chat_id_);
    s.store_field("forum_topic_id", forum_topic_id_);
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

toggleGeneralForumTopicIsHidden::toggleGeneralForumTopicIsHidden()
  : chat_id_()
  , is_hidden_()
{}

toggleGeneralForumTopicIsHidden::toggleGeneralForumTopicIsHidden(int53 chat_id_, bool is_hidden_)
  : chat_id_(chat_id_)
  , is_hidden_(is_hidden_)
{}

const std::int32_t toggleGeneralForumTopicIsHidden::ID;

void toggleGeneralForumTopicIsHidden::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleGeneralForumTopicIsHidden");
    s.store_field("chat_id", chat_id_);
    s.store_field("is_hidden", is_hidden_);
    s.store_class_end();
  }
}

toggleGroupCallIsMyVideoPaused::toggleGroupCallIsMyVideoPaused()
  : group_call_id_()
  , is_my_video_paused_()
{}

toggleGroupCallIsMyVideoPaused::toggleGroupCallIsMyVideoPaused(int32 group_call_id_, bool is_my_video_paused_)
  : group_call_id_(group_call_id_)
  , is_my_video_paused_(is_my_video_paused_)
{}

const std::int32_t toggleGroupCallIsMyVideoPaused::ID;

void toggleGroupCallIsMyVideoPaused::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleGroupCallIsMyVideoPaused");
    s.store_field("group_call_id", group_call_id_);
    s.store_field("is_my_video_paused", is_my_video_paused_);
    s.store_class_end();
  }
}

toggleSupergroupIsAllHistoryAvailable::toggleSupergroupIsAllHistoryAvailable()
  : supergroup_id_()
  , is_all_history_available_()
{}

toggleSupergroupIsAllHistoryAvailable::toggleSupergroupIsAllHistoryAvailable(int53 supergroup_id_, bool is_all_history_available_)
  : supergroup_id_(supergroup_id_)
  , is_all_history_available_(is_all_history_available_)
{}

const std::int32_t toggleSupergroupIsAllHistoryAvailable::ID;

void toggleSupergroupIsAllHistoryAvailable::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleSupergroupIsAllHistoryAvailable");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_field("is_all_history_available", is_all_history_available_);
    s.store_class_end();
  }
}

transferChatOwnership::transferChatOwnership()
  : chat_id_()
  , user_id_()
  , password_()
{}

transferChatOwnership::transferChatOwnership(int53 chat_id_, int53 user_id_, string const &password_)
  : chat_id_(chat_id_)
  , user_id_(user_id_)
  , password_(password_)
{}

const std::int32_t transferChatOwnership::ID;

void transferChatOwnership::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "transferChatOwnership");
    s.store_field("chat_id", chat_id_);
    s.store_field("user_id", user_id_);
    s.store_field("password", password_);
    s.store_class_end();
  }
}

translateMessageText::translateMessageText()
  : chat_id_()
  , message_id_()
  , to_language_code_()
{}

translateMessageText::translateMessageText(int53 chat_id_, int53 message_id_, string const &to_language_code_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , to_language_code_(to_language_code_)
{}

const std::int32_t translateMessageText::ID;

void translateMessageText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "translateMessageText");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("to_language_code", to_language_code_);
    s.store_class_end();
  }
}

validateOrderInfo::validateOrderInfo()
  : input_invoice_()
  , order_info_()
  , allow_save_()
{}

validateOrderInfo::validateOrderInfo(object_ptr<InputInvoice> &&input_invoice_, object_ptr<orderInfo> &&order_info_, bool allow_save_)
  : input_invoice_(std::move(input_invoice_))
  , order_info_(std::move(order_info_))
  , allow_save_(allow_save_)
{}

const std::int32_t validateOrderInfo::ID;

void validateOrderInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "validateOrderInfo");
    s.store_object_field("input_invoice", static_cast<const BaseObject *>(input_invoice_.get()));
    s.store_object_field("order_info", static_cast<const BaseObject *>(order_info_.get()));
    s.store_field("allow_save", allow_save_);
    s.store_class_end();
  }
}

viewSponsoredChat::viewSponsoredChat()
  : sponsored_chat_unique_id_()
{}

viewSponsoredChat::viewSponsoredChat(int53 sponsored_chat_unique_id_)
  : sponsored_chat_unique_id_(sponsored_chat_unique_id_)
{}

const std::int32_t viewSponsoredChat::ID;

void viewSponsoredChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "viewSponsoredChat");
    s.store_field("sponsored_chat_unique_id", sponsored_chat_unique_id_);
    s.store_class_end();
  }
}
}  // namespace td_api
}  // namespace td
