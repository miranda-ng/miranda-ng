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


acceptedGiftTypes::acceptedGiftTypes()
  : unlimited_gifts_()
  , limited_gifts_()
  , upgraded_gifts_()
  , premium_subscription_()
{}

acceptedGiftTypes::acceptedGiftTypes(bool unlimited_gifts_, bool limited_gifts_, bool upgraded_gifts_, bool premium_subscription_)
  : unlimited_gifts_(unlimited_gifts_)
  , limited_gifts_(limited_gifts_)
  , upgraded_gifts_(upgraded_gifts_)
  , premium_subscription_(premium_subscription_)
{}

const std::int32_t acceptedGiftTypes::ID;

void acceptedGiftTypes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "acceptedGiftTypes");
    s.store_field("unlimited_gifts", unlimited_gifts_);
    s.store_field("limited_gifts", limited_gifts_);
    s.store_field("upgraded_gifts", upgraded_gifts_);
    s.store_field("premium_subscription", premium_subscription_);
    s.store_class_end();
  }
}

address::address()
  : country_code_()
  , state_()
  , city_()
  , street_line1_()
  , street_line2_()
  , postal_code_()
{}

address::address(string const &country_code_, string const &state_, string const &city_, string const &street_line1_, string const &street_line2_, string const &postal_code_)
  : country_code_(country_code_)
  , state_(state_)
  , city_(city_)
  , street_line1_(street_line1_)
  , street_line2_(street_line2_)
  , postal_code_(postal_code_)
{}

const std::int32_t address::ID;

void address::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "address");
    s.store_field("country_code", country_code_);
    s.store_field("state", state_);
    s.store_field("city", city_);
    s.store_field("street_line1", street_line1_);
    s.store_field("street_line2", street_line2_);
    s.store_field("postal_code", postal_code_);
    s.store_class_end();
  }
}

attachmentMenuBotColor::attachmentMenuBotColor()
  : light_color_()
  , dark_color_()
{}

attachmentMenuBotColor::attachmentMenuBotColor(int32 light_color_, int32 dark_color_)
  : light_color_(light_color_)
  , dark_color_(dark_color_)
{}

const std::int32_t attachmentMenuBotColor::ID;

void attachmentMenuBotColor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "attachmentMenuBotColor");
    s.store_field("light_color", light_color_);
    s.store_field("dark_color", dark_color_);
    s.store_class_end();
  }
}

autosaveSettingsException::autosaveSettingsException()
  : chat_id_()
  , settings_()
{}

autosaveSettingsException::autosaveSettingsException(int53 chat_id_, object_ptr<scopeAutosaveSettings> &&settings_)
  : chat_id_(chat_id_)
  , settings_(std::move(settings_))
{}

const std::int32_t autosaveSettingsException::ID;

void autosaveSettingsException::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "autosaveSettingsException");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

availableGifts::availableGifts()
  : gifts_()
{}

availableGifts::availableGifts(array<object_ptr<availableGift>> &&gifts_)
  : gifts_(std::move(gifts_))
{}

const std::int32_t availableGifts::ID;

void availableGifts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "availableGifts");
    { s.store_vector_begin("gifts", gifts_.size()); for (const auto &_value : gifts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

bankCardInfo::bankCardInfo()
  : title_()
  , actions_()
{}

bankCardInfo::bankCardInfo(string const &title_, array<object_ptr<bankCardActionOpenUrl>> &&actions_)
  : title_(title_)
  , actions_(std::move(actions_))
{}

const std::int32_t bankCardInfo::ID;

void bankCardInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "bankCardInfo");
    s.store_field("title", title_);
    { s.store_vector_begin("actions", actions_.size()); for (const auto &_value : actions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

businessAwayMessageScheduleAlways::businessAwayMessageScheduleAlways() {
}

const std::int32_t businessAwayMessageScheduleAlways::ID;

void businessAwayMessageScheduleAlways::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessAwayMessageScheduleAlways");
    s.store_class_end();
  }
}

businessAwayMessageScheduleOutsideOfOpeningHours::businessAwayMessageScheduleOutsideOfOpeningHours() {
}

const std::int32_t businessAwayMessageScheduleOutsideOfOpeningHours::ID;

void businessAwayMessageScheduleOutsideOfOpeningHours::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessAwayMessageScheduleOutsideOfOpeningHours");
    s.store_class_end();
  }
}

businessAwayMessageScheduleCustom::businessAwayMessageScheduleCustom()
  : start_date_()
  , end_date_()
{}

businessAwayMessageScheduleCustom::businessAwayMessageScheduleCustom(int32 start_date_, int32 end_date_)
  : start_date_(start_date_)
  , end_date_(end_date_)
{}

const std::int32_t businessAwayMessageScheduleCustom::ID;

void businessAwayMessageScheduleCustom::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessAwayMessageScheduleCustom");
    s.store_field("start_date", start_date_);
    s.store_field("end_date", end_date_);
    s.store_class_end();
  }
}

chatActionTyping::chatActionTyping() {
}

const std::int32_t chatActionTyping::ID;

void chatActionTyping::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionTyping");
    s.store_class_end();
  }
}

chatActionRecordingVideo::chatActionRecordingVideo() {
}

const std::int32_t chatActionRecordingVideo::ID;

void chatActionRecordingVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionRecordingVideo");
    s.store_class_end();
  }
}

chatActionUploadingVideo::chatActionUploadingVideo()
  : progress_()
{}

chatActionUploadingVideo::chatActionUploadingVideo(int32 progress_)
  : progress_(progress_)
{}

const std::int32_t chatActionUploadingVideo::ID;

void chatActionUploadingVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionUploadingVideo");
    s.store_field("progress", progress_);
    s.store_class_end();
  }
}

chatActionRecordingVoiceNote::chatActionRecordingVoiceNote() {
}

const std::int32_t chatActionRecordingVoiceNote::ID;

void chatActionRecordingVoiceNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionRecordingVoiceNote");
    s.store_class_end();
  }
}

chatActionUploadingVoiceNote::chatActionUploadingVoiceNote()
  : progress_()
{}

chatActionUploadingVoiceNote::chatActionUploadingVoiceNote(int32 progress_)
  : progress_(progress_)
{}

const std::int32_t chatActionUploadingVoiceNote::ID;

void chatActionUploadingVoiceNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionUploadingVoiceNote");
    s.store_field("progress", progress_);
    s.store_class_end();
  }
}

chatActionUploadingPhoto::chatActionUploadingPhoto()
  : progress_()
{}

chatActionUploadingPhoto::chatActionUploadingPhoto(int32 progress_)
  : progress_(progress_)
{}

const std::int32_t chatActionUploadingPhoto::ID;

void chatActionUploadingPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionUploadingPhoto");
    s.store_field("progress", progress_);
    s.store_class_end();
  }
}

chatActionUploadingDocument::chatActionUploadingDocument()
  : progress_()
{}

chatActionUploadingDocument::chatActionUploadingDocument(int32 progress_)
  : progress_(progress_)
{}

const std::int32_t chatActionUploadingDocument::ID;

void chatActionUploadingDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionUploadingDocument");
    s.store_field("progress", progress_);
    s.store_class_end();
  }
}

chatActionChoosingSticker::chatActionChoosingSticker() {
}

const std::int32_t chatActionChoosingSticker::ID;

void chatActionChoosingSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionChoosingSticker");
    s.store_class_end();
  }
}

chatActionChoosingLocation::chatActionChoosingLocation() {
}

const std::int32_t chatActionChoosingLocation::ID;

void chatActionChoosingLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionChoosingLocation");
    s.store_class_end();
  }
}

chatActionChoosingContact::chatActionChoosingContact() {
}

const std::int32_t chatActionChoosingContact::ID;

void chatActionChoosingContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionChoosingContact");
    s.store_class_end();
  }
}

chatActionStartPlayingGame::chatActionStartPlayingGame() {
}

const std::int32_t chatActionStartPlayingGame::ID;

void chatActionStartPlayingGame::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionStartPlayingGame");
    s.store_class_end();
  }
}

chatActionRecordingVideoNote::chatActionRecordingVideoNote() {
}

const std::int32_t chatActionRecordingVideoNote::ID;

void chatActionRecordingVideoNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionRecordingVideoNote");
    s.store_class_end();
  }
}

chatActionUploadingVideoNote::chatActionUploadingVideoNote()
  : progress_()
{}

chatActionUploadingVideoNote::chatActionUploadingVideoNote(int32 progress_)
  : progress_(progress_)
{}

const std::int32_t chatActionUploadingVideoNote::ID;

void chatActionUploadingVideoNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionUploadingVideoNote");
    s.store_field("progress", progress_);
    s.store_class_end();
  }
}

chatActionWatchingAnimations::chatActionWatchingAnimations()
  : emoji_()
{}

chatActionWatchingAnimations::chatActionWatchingAnimations(string const &emoji_)
  : emoji_(emoji_)
{}

const std::int32_t chatActionWatchingAnimations::ID;

void chatActionWatchingAnimations::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionWatchingAnimations");
    s.store_field("emoji", emoji_);
    s.store_class_end();
  }
}

chatActionCancel::chatActionCancel() {
}

const std::int32_t chatActionCancel::ID;

void chatActionCancel::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatActionCancel");
    s.store_class_end();
  }
}

chatAdministratorRights::chatAdministratorRights()
  : can_manage_chat_()
  , can_change_info_()
  , can_post_messages_()
  , can_edit_messages_()
  , can_delete_messages_()
  , can_invite_users_()
  , can_restrict_members_()
  , can_pin_messages_()
  , can_manage_topics_()
  , can_promote_members_()
  , can_manage_video_chats_()
  , can_post_stories_()
  , can_edit_stories_()
  , can_delete_stories_()
  , can_manage_direct_messages_()
  , is_anonymous_()
{}

chatAdministratorRights::chatAdministratorRights(bool can_manage_chat_, bool can_change_info_, bool can_post_messages_, bool can_edit_messages_, bool can_delete_messages_, bool can_invite_users_, bool can_restrict_members_, bool can_pin_messages_, bool can_manage_topics_, bool can_promote_members_, bool can_manage_video_chats_, bool can_post_stories_, bool can_edit_stories_, bool can_delete_stories_, bool can_manage_direct_messages_, bool is_anonymous_)
  : can_manage_chat_(can_manage_chat_)
  , can_change_info_(can_change_info_)
  , can_post_messages_(can_post_messages_)
  , can_edit_messages_(can_edit_messages_)
  , can_delete_messages_(can_delete_messages_)
  , can_invite_users_(can_invite_users_)
  , can_restrict_members_(can_restrict_members_)
  , can_pin_messages_(can_pin_messages_)
  , can_manage_topics_(can_manage_topics_)
  , can_promote_members_(can_promote_members_)
  , can_manage_video_chats_(can_manage_video_chats_)
  , can_post_stories_(can_post_stories_)
  , can_edit_stories_(can_edit_stories_)
  , can_delete_stories_(can_delete_stories_)
  , can_manage_direct_messages_(can_manage_direct_messages_)
  , is_anonymous_(is_anonymous_)
{}

const std::int32_t chatAdministratorRights::ID;

void chatAdministratorRights::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatAdministratorRights");
    s.store_field("can_manage_chat", can_manage_chat_);
    s.store_field("can_change_info", can_change_info_);
    s.store_field("can_post_messages", can_post_messages_);
    s.store_field("can_edit_messages", can_edit_messages_);
    s.store_field("can_delete_messages", can_delete_messages_);
    s.store_field("can_invite_users", can_invite_users_);
    s.store_field("can_restrict_members", can_restrict_members_);
    s.store_field("can_pin_messages", can_pin_messages_);
    s.store_field("can_manage_topics", can_manage_topics_);
    s.store_field("can_promote_members", can_promote_members_);
    s.store_field("can_manage_video_chats", can_manage_video_chats_);
    s.store_field("can_post_stories", can_post_stories_);
    s.store_field("can_edit_stories", can_edit_stories_);
    s.store_field("can_delete_stories", can_delete_stories_);
    s.store_field("can_manage_direct_messages", can_manage_direct_messages_);
    s.store_field("is_anonymous", is_anonymous_);
    s.store_class_end();
  }
}

chatBackground::chatBackground()
  : background_()
  , dark_theme_dimming_()
{}

chatBackground::chatBackground(object_ptr<background> &&background_, int32 dark_theme_dimming_)
  : background_(std::move(background_))
  , dark_theme_dimming_(dark_theme_dimming_)
{}

const std::int32_t chatBackground::ID;

void chatBackground::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatBackground");
    s.store_object_field("background", static_cast<const BaseObject *>(background_.get()));
    s.store_field("dark_theme_dimming", dark_theme_dimming_);
    s.store_class_end();
  }
}

chatJoinRequestsInfo::chatJoinRequestsInfo()
  : total_count_()
  , user_ids_()
{}

chatJoinRequestsInfo::chatJoinRequestsInfo(int32 total_count_, array<int53> &&user_ids_)
  : total_count_(total_count_)
  , user_ids_(std::move(user_ids_))
{}

const std::int32_t chatJoinRequestsInfo::ID;

void chatJoinRequestsInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatJoinRequestsInfo");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("user_ids", user_ids_.size()); for (const auto &_value : user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatMemberStatusCreator::chatMemberStatusCreator()
  : custom_title_()
  , is_anonymous_()
  , is_member_()
{}

chatMemberStatusCreator::chatMemberStatusCreator(string const &custom_title_, bool is_anonymous_, bool is_member_)
  : custom_title_(custom_title_)
  , is_anonymous_(is_anonymous_)
  , is_member_(is_member_)
{}

const std::int32_t chatMemberStatusCreator::ID;

void chatMemberStatusCreator::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatMemberStatusCreator");
    s.store_field("custom_title", custom_title_);
    s.store_field("is_anonymous", is_anonymous_);
    s.store_field("is_member", is_member_);
    s.store_class_end();
  }
}

chatMemberStatusAdministrator::chatMemberStatusAdministrator()
  : custom_title_()
  , can_be_edited_()
  , rights_()
{}

chatMemberStatusAdministrator::chatMemberStatusAdministrator(string const &custom_title_, bool can_be_edited_, object_ptr<chatAdministratorRights> &&rights_)
  : custom_title_(custom_title_)
  , can_be_edited_(can_be_edited_)
  , rights_(std::move(rights_))
{}

const std::int32_t chatMemberStatusAdministrator::ID;

void chatMemberStatusAdministrator::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatMemberStatusAdministrator");
    s.store_field("custom_title", custom_title_);
    s.store_field("can_be_edited", can_be_edited_);
    s.store_object_field("rights", static_cast<const BaseObject *>(rights_.get()));
    s.store_class_end();
  }
}

chatMemberStatusMember::chatMemberStatusMember()
  : member_until_date_()
{}

chatMemberStatusMember::chatMemberStatusMember(int32 member_until_date_)
  : member_until_date_(member_until_date_)
{}

const std::int32_t chatMemberStatusMember::ID;

void chatMemberStatusMember::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatMemberStatusMember");
    s.store_field("member_until_date", member_until_date_);
    s.store_class_end();
  }
}

chatMemberStatusRestricted::chatMemberStatusRestricted()
  : is_member_()
  , restricted_until_date_()
  , permissions_()
{}

chatMemberStatusRestricted::chatMemberStatusRestricted(bool is_member_, int32 restricted_until_date_, object_ptr<chatPermissions> &&permissions_)
  : is_member_(is_member_)
  , restricted_until_date_(restricted_until_date_)
  , permissions_(std::move(permissions_))
{}

const std::int32_t chatMemberStatusRestricted::ID;

void chatMemberStatusRestricted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatMemberStatusRestricted");
    s.store_field("is_member", is_member_);
    s.store_field("restricted_until_date", restricted_until_date_);
    s.store_object_field("permissions", static_cast<const BaseObject *>(permissions_.get()));
    s.store_class_end();
  }
}

chatMemberStatusLeft::chatMemberStatusLeft() {
}

const std::int32_t chatMemberStatusLeft::ID;

void chatMemberStatusLeft::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatMemberStatusLeft");
    s.store_class_end();
  }
}

chatMemberStatusBanned::chatMemberStatusBanned()
  : banned_until_date_()
{}

chatMemberStatusBanned::chatMemberStatusBanned(int32 banned_until_date_)
  : banned_until_date_(banned_until_date_)
{}

const std::int32_t chatMemberStatusBanned::ID;

void chatMemberStatusBanned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatMemberStatusBanned");
    s.store_field("banned_until_date", banned_until_date_);
    s.store_class_end();
  }
}

chatPhoto::chatPhoto()
  : id_()
  , added_date_()
  , minithumbnail_()
  , sizes_()
  , animation_()
  , small_animation_()
  , sticker_()
{}

chatPhoto::chatPhoto(int64 id_, int32 added_date_, object_ptr<minithumbnail> &&minithumbnail_, array<object_ptr<photoSize>> &&sizes_, object_ptr<animatedChatPhoto> &&animation_, object_ptr<animatedChatPhoto> &&small_animation_, object_ptr<chatPhotoSticker> &&sticker_)
  : id_(id_)
  , added_date_(added_date_)
  , minithumbnail_(std::move(minithumbnail_))
  , sizes_(std::move(sizes_))
  , animation_(std::move(animation_))
  , small_animation_(std::move(small_animation_))
  , sticker_(std::move(sticker_))
{}

const std::int32_t chatPhoto::ID;

void chatPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatPhoto");
    s.store_field("id", id_);
    s.store_field("added_date", added_date_);
    s.store_object_field("minithumbnail", static_cast<const BaseObject *>(minithumbnail_.get()));
    { s.store_vector_begin("sizes", sizes_.size()); for (const auto &_value : sizes_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("animation", static_cast<const BaseObject *>(animation_.get()));
    s.store_object_field("small_animation", static_cast<const BaseObject *>(small_animation_.get()));
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

databaseStatistics::databaseStatistics()
  : statistics_()
{}

databaseStatistics::databaseStatistics(string const &statistics_)
  : statistics_(statistics_)
{}

const std::int32_t databaseStatistics::ID;

void databaseStatistics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "databaseStatistics");
    s.store_field("statistics", statistics_);
    s.store_class_end();
  }
}

fileDownload::fileDownload()
  : file_id_()
  , message_()
  , add_date_()
  , complete_date_()
  , is_paused_()
{}

fileDownload::fileDownload(int32 file_id_, object_ptr<message> &&message_, int32 add_date_, int32 complete_date_, bool is_paused_)
  : file_id_(file_id_)
  , message_(std::move(message_))
  , add_date_(add_date_)
  , complete_date_(complete_date_)
  , is_paused_(is_paused_)
{}

const std::int32_t fileDownload::ID;

void fileDownload::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileDownload");
    s.store_field("file_id", file_id_);
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_field("add_date", add_date_);
    s.store_field("complete_date", complete_date_);
    s.store_field("is_paused", is_paused_);
    s.store_class_end();
  }
}

fileDownloadedPrefixSize::fileDownloadedPrefixSize()
  : size_()
{}

fileDownloadedPrefixSize::fileDownloadedPrefixSize(int53 size_)
  : size_(size_)
{}

const std::int32_t fileDownloadedPrefixSize::ID;

void fileDownloadedPrefixSize::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileDownloadedPrefixSize");
    s.store_field("size", size_);
    s.store_class_end();
  }
}

fileTypeNone::fileTypeNone() {
}

const std::int32_t fileTypeNone::ID;

void fileTypeNone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeNone");
    s.store_class_end();
  }
}

fileTypeAnimation::fileTypeAnimation() {
}

const std::int32_t fileTypeAnimation::ID;

void fileTypeAnimation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeAnimation");
    s.store_class_end();
  }
}

fileTypeAudio::fileTypeAudio() {
}

const std::int32_t fileTypeAudio::ID;

void fileTypeAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeAudio");
    s.store_class_end();
  }
}

fileTypeDocument::fileTypeDocument() {
}

const std::int32_t fileTypeDocument::ID;

void fileTypeDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeDocument");
    s.store_class_end();
  }
}

fileTypeNotificationSound::fileTypeNotificationSound() {
}

const std::int32_t fileTypeNotificationSound::ID;

void fileTypeNotificationSound::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeNotificationSound");
    s.store_class_end();
  }
}

fileTypePhoto::fileTypePhoto() {
}

const std::int32_t fileTypePhoto::ID;

void fileTypePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypePhoto");
    s.store_class_end();
  }
}

fileTypePhotoStory::fileTypePhotoStory() {
}

const std::int32_t fileTypePhotoStory::ID;

void fileTypePhotoStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypePhotoStory");
    s.store_class_end();
  }
}

fileTypeProfilePhoto::fileTypeProfilePhoto() {
}

const std::int32_t fileTypeProfilePhoto::ID;

void fileTypeProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeProfilePhoto");
    s.store_class_end();
  }
}

fileTypeSecret::fileTypeSecret() {
}

const std::int32_t fileTypeSecret::ID;

void fileTypeSecret::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeSecret");
    s.store_class_end();
  }
}

fileTypeSecretThumbnail::fileTypeSecretThumbnail() {
}

const std::int32_t fileTypeSecretThumbnail::ID;

void fileTypeSecretThumbnail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeSecretThumbnail");
    s.store_class_end();
  }
}

fileTypeSecure::fileTypeSecure() {
}

const std::int32_t fileTypeSecure::ID;

void fileTypeSecure::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeSecure");
    s.store_class_end();
  }
}

fileTypeSelfDestructingPhoto::fileTypeSelfDestructingPhoto() {
}

const std::int32_t fileTypeSelfDestructingPhoto::ID;

void fileTypeSelfDestructingPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeSelfDestructingPhoto");
    s.store_class_end();
  }
}

fileTypeSelfDestructingVideo::fileTypeSelfDestructingVideo() {
}

const std::int32_t fileTypeSelfDestructingVideo::ID;

void fileTypeSelfDestructingVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeSelfDestructingVideo");
    s.store_class_end();
  }
}

fileTypeSelfDestructingVideoNote::fileTypeSelfDestructingVideoNote() {
}

const std::int32_t fileTypeSelfDestructingVideoNote::ID;

void fileTypeSelfDestructingVideoNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeSelfDestructingVideoNote");
    s.store_class_end();
  }
}

fileTypeSelfDestructingVoiceNote::fileTypeSelfDestructingVoiceNote() {
}

const std::int32_t fileTypeSelfDestructingVoiceNote::ID;

void fileTypeSelfDestructingVoiceNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeSelfDestructingVoiceNote");
    s.store_class_end();
  }
}

fileTypeSticker::fileTypeSticker() {
}

const std::int32_t fileTypeSticker::ID;

void fileTypeSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeSticker");
    s.store_class_end();
  }
}

fileTypeThumbnail::fileTypeThumbnail() {
}

const std::int32_t fileTypeThumbnail::ID;

void fileTypeThumbnail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeThumbnail");
    s.store_class_end();
  }
}

fileTypeUnknown::fileTypeUnknown() {
}

const std::int32_t fileTypeUnknown::ID;

void fileTypeUnknown::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeUnknown");
    s.store_class_end();
  }
}

fileTypeVideo::fileTypeVideo() {
}

const std::int32_t fileTypeVideo::ID;

void fileTypeVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeVideo");
    s.store_class_end();
  }
}

fileTypeVideoNote::fileTypeVideoNote() {
}

const std::int32_t fileTypeVideoNote::ID;

void fileTypeVideoNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeVideoNote");
    s.store_class_end();
  }
}

fileTypeVideoStory::fileTypeVideoStory() {
}

const std::int32_t fileTypeVideoStory::ID;

void fileTypeVideoStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeVideoStory");
    s.store_class_end();
  }
}

fileTypeVoiceNote::fileTypeVoiceNote() {
}

const std::int32_t fileTypeVoiceNote::ID;

void fileTypeVoiceNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeVoiceNote");
    s.store_class_end();
  }
}

fileTypeWallpaper::fileTypeWallpaper() {
}

const std::int32_t fileTypeWallpaper::ID;

void fileTypeWallpaper::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "fileTypeWallpaper");
    s.store_class_end();
  }
}

forwardSource::forwardSource()
  : chat_id_()
  , message_id_()
  , sender_id_()
  , sender_name_()
  , date_()
  , is_outgoing_()
{}

forwardSource::forwardSource(int53 chat_id_, int53 message_id_, object_ptr<MessageSender> &&sender_id_, string const &sender_name_, int32 date_, bool is_outgoing_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , sender_id_(std::move(sender_id_))
  , sender_name_(sender_name_)
  , date_(date_)
  , is_outgoing_(is_outgoing_)
{}

const std::int32_t forwardSource::ID;

void forwardSource::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "forwardSource");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_field("sender_name", sender_name_);
    s.store_field("date", date_);
    s.store_field("is_outgoing", is_outgoing_);
    s.store_class_end();
  }
}

foundMessages::foundMessages()
  : total_count_()
  , messages_()
  , next_offset_()
{}

foundMessages::foundMessages(int32 total_count_, array<object_ptr<message>> &&messages_, string const &next_offset_)
  : total_count_(total_count_)
  , messages_(std::move(messages_))
  , next_offset_(next_offset_)
{}

const std::int32_t foundMessages::ID;

void foundMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "foundMessages");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

groupCallDataChannelMain::groupCallDataChannelMain() {
}

const std::int32_t groupCallDataChannelMain::ID;

void groupCallDataChannelMain::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCallDataChannelMain");
    s.store_class_end();
  }
}

groupCallDataChannelScreenSharing::groupCallDataChannelScreenSharing() {
}

const std::int32_t groupCallDataChannelScreenSharing::ID;

void groupCallDataChannelScreenSharing::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCallDataChannelScreenSharing");
    s.store_class_end();
  }
}

inlineKeyboardButton::inlineKeyboardButton()
  : text_()
  , type_()
{}

inlineKeyboardButton::inlineKeyboardButton(string const &text_, object_ptr<InlineKeyboardButtonType> &&type_)
  : text_(text_)
  , type_(std::move(type_))
{}

const std::int32_t inlineKeyboardButton::ID;

void inlineKeyboardButton::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineKeyboardButton");
    s.store_field("text", text_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

inlineKeyboardButtonTypeUrl::inlineKeyboardButtonTypeUrl()
  : url_()
{}

inlineKeyboardButtonTypeUrl::inlineKeyboardButtonTypeUrl(string const &url_)
  : url_(url_)
{}

const std::int32_t inlineKeyboardButtonTypeUrl::ID;

void inlineKeyboardButtonTypeUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineKeyboardButtonTypeUrl");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

inlineKeyboardButtonTypeLoginUrl::inlineKeyboardButtonTypeLoginUrl()
  : url_()
  , id_()
  , forward_text_()
{}

inlineKeyboardButtonTypeLoginUrl::inlineKeyboardButtonTypeLoginUrl(string const &url_, int53 id_, string const &forward_text_)
  : url_(url_)
  , id_(id_)
  , forward_text_(forward_text_)
{}

const std::int32_t inlineKeyboardButtonTypeLoginUrl::ID;

void inlineKeyboardButtonTypeLoginUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineKeyboardButtonTypeLoginUrl");
    s.store_field("url", url_);
    s.store_field("id", id_);
    s.store_field("forward_text", forward_text_);
    s.store_class_end();
  }
}

inlineKeyboardButtonTypeWebApp::inlineKeyboardButtonTypeWebApp()
  : url_()
{}

inlineKeyboardButtonTypeWebApp::inlineKeyboardButtonTypeWebApp(string const &url_)
  : url_(url_)
{}

const std::int32_t inlineKeyboardButtonTypeWebApp::ID;

void inlineKeyboardButtonTypeWebApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineKeyboardButtonTypeWebApp");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

inlineKeyboardButtonTypeCallback::inlineKeyboardButtonTypeCallback()
  : data_()
{}

inlineKeyboardButtonTypeCallback::inlineKeyboardButtonTypeCallback(bytes const &data_)
  : data_(std::move(data_))
{}

const std::int32_t inlineKeyboardButtonTypeCallback::ID;

void inlineKeyboardButtonTypeCallback::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineKeyboardButtonTypeCallback");
    s.store_bytes_field("data", data_);
    s.store_class_end();
  }
}

inlineKeyboardButtonTypeCallbackWithPassword::inlineKeyboardButtonTypeCallbackWithPassword()
  : data_()
{}

inlineKeyboardButtonTypeCallbackWithPassword::inlineKeyboardButtonTypeCallbackWithPassword(bytes const &data_)
  : data_(std::move(data_))
{}

const std::int32_t inlineKeyboardButtonTypeCallbackWithPassword::ID;

void inlineKeyboardButtonTypeCallbackWithPassword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineKeyboardButtonTypeCallbackWithPassword");
    s.store_bytes_field("data", data_);
    s.store_class_end();
  }
}

inlineKeyboardButtonTypeCallbackGame::inlineKeyboardButtonTypeCallbackGame() {
}

const std::int32_t inlineKeyboardButtonTypeCallbackGame::ID;

void inlineKeyboardButtonTypeCallbackGame::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineKeyboardButtonTypeCallbackGame");
    s.store_class_end();
  }
}

inlineKeyboardButtonTypeSwitchInline::inlineKeyboardButtonTypeSwitchInline()
  : query_()
  , target_chat_()
{}

inlineKeyboardButtonTypeSwitchInline::inlineKeyboardButtonTypeSwitchInline(string const &query_, object_ptr<TargetChat> &&target_chat_)
  : query_(query_)
  , target_chat_(std::move(target_chat_))
{}

const std::int32_t inlineKeyboardButtonTypeSwitchInline::ID;

void inlineKeyboardButtonTypeSwitchInline::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineKeyboardButtonTypeSwitchInline");
    s.store_field("query", query_);
    s.store_object_field("target_chat", static_cast<const BaseObject *>(target_chat_.get()));
    s.store_class_end();
  }
}

inlineKeyboardButtonTypeBuy::inlineKeyboardButtonTypeBuy() {
}

const std::int32_t inlineKeyboardButtonTypeBuy::ID;

void inlineKeyboardButtonTypeBuy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineKeyboardButtonTypeBuy");
    s.store_class_end();
  }
}

inlineKeyboardButtonTypeUser::inlineKeyboardButtonTypeUser()
  : user_id_()
{}

inlineKeyboardButtonTypeUser::inlineKeyboardButtonTypeUser(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t inlineKeyboardButtonTypeUser::ID;

void inlineKeyboardButtonTypeUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineKeyboardButtonTypeUser");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

inlineKeyboardButtonTypeCopyText::inlineKeyboardButtonTypeCopyText()
  : text_()
{}

inlineKeyboardButtonTypeCopyText::inlineKeyboardButtonTypeCopyText(string const &text_)
  : text_(text_)
{}

const std::int32_t inlineKeyboardButtonTypeCopyText::ID;

void inlineKeyboardButtonTypeCopyText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inlineKeyboardButtonTypeCopyText");
    s.store_field("text", text_);
    s.store_class_end();
  }
}

inputInlineQueryResultAnimation::inputInlineQueryResultAnimation()
  : id_()
  , title_()
  , thumbnail_url_()
  , thumbnail_mime_type_()
  , video_url_()
  , video_mime_type_()
  , video_duration_()
  , video_width_()
  , video_height_()
  , reply_markup_()
  , input_message_content_()
{}

inputInlineQueryResultAnimation::inputInlineQueryResultAnimation(string const &id_, string const &title_, string const &thumbnail_url_, string const &thumbnail_mime_type_, string const &video_url_, string const &video_mime_type_, int32 video_duration_, int32 video_width_, int32 video_height_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_)
  : id_(id_)
  , title_(title_)
  , thumbnail_url_(thumbnail_url_)
  , thumbnail_mime_type_(thumbnail_mime_type_)
  , video_url_(video_url_)
  , video_mime_type_(video_mime_type_)
  , video_duration_(video_duration_)
  , video_width_(video_width_)
  , video_height_(video_height_)
  , reply_markup_(std::move(reply_markup_))
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t inputInlineQueryResultAnimation::ID;

void inputInlineQueryResultAnimation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInlineQueryResultAnimation");
    s.store_field("id", id_);
    s.store_field("title", title_);
    s.store_field("thumbnail_url", thumbnail_url_);
    s.store_field("thumbnail_mime_type", thumbnail_mime_type_);
    s.store_field("video_url", video_url_);
    s.store_field("video_mime_type", video_mime_type_);
    s.store_field("video_duration", video_duration_);
    s.store_field("video_width", video_width_);
    s.store_field("video_height", video_height_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

inputInlineQueryResultArticle::inputInlineQueryResultArticle()
  : id_()
  , url_()
  , title_()
  , description_()
  , thumbnail_url_()
  , thumbnail_width_()
  , thumbnail_height_()
  , reply_markup_()
  , input_message_content_()
{}

inputInlineQueryResultArticle::inputInlineQueryResultArticle(string const &id_, string const &url_, string const &title_, string const &description_, string const &thumbnail_url_, int32 thumbnail_width_, int32 thumbnail_height_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_)
  : id_(id_)
  , url_(url_)
  , title_(title_)
  , description_(description_)
  , thumbnail_url_(thumbnail_url_)
  , thumbnail_width_(thumbnail_width_)
  , thumbnail_height_(thumbnail_height_)
  , reply_markup_(std::move(reply_markup_))
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t inputInlineQueryResultArticle::ID;

void inputInlineQueryResultArticle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInlineQueryResultArticle");
    s.store_field("id", id_);
    s.store_field("url", url_);
    s.store_field("title", title_);
    s.store_field("description", description_);
    s.store_field("thumbnail_url", thumbnail_url_);
    s.store_field("thumbnail_width", thumbnail_width_);
    s.store_field("thumbnail_height", thumbnail_height_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

inputInlineQueryResultAudio::inputInlineQueryResultAudio()
  : id_()
  , title_()
  , performer_()
  , audio_url_()
  , audio_duration_()
  , reply_markup_()
  , input_message_content_()
{}

inputInlineQueryResultAudio::inputInlineQueryResultAudio(string const &id_, string const &title_, string const &performer_, string const &audio_url_, int32 audio_duration_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_)
  : id_(id_)
  , title_(title_)
  , performer_(performer_)
  , audio_url_(audio_url_)
  , audio_duration_(audio_duration_)
  , reply_markup_(std::move(reply_markup_))
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t inputInlineQueryResultAudio::ID;

void inputInlineQueryResultAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInlineQueryResultAudio");
    s.store_field("id", id_);
    s.store_field("title", title_);
    s.store_field("performer", performer_);
    s.store_field("audio_url", audio_url_);
    s.store_field("audio_duration", audio_duration_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

inputInlineQueryResultContact::inputInlineQueryResultContact()
  : id_()
  , contact_()
  , thumbnail_url_()
  , thumbnail_width_()
  , thumbnail_height_()
  , reply_markup_()
  , input_message_content_()
{}

inputInlineQueryResultContact::inputInlineQueryResultContact(string const &id_, object_ptr<contact> &&contact_, string const &thumbnail_url_, int32 thumbnail_width_, int32 thumbnail_height_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_)
  : id_(id_)
  , contact_(std::move(contact_))
  , thumbnail_url_(thumbnail_url_)
  , thumbnail_width_(thumbnail_width_)
  , thumbnail_height_(thumbnail_height_)
  , reply_markup_(std::move(reply_markup_))
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t inputInlineQueryResultContact::ID;

void inputInlineQueryResultContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInlineQueryResultContact");
    s.store_field("id", id_);
    s.store_object_field("contact", static_cast<const BaseObject *>(contact_.get()));
    s.store_field("thumbnail_url", thumbnail_url_);
    s.store_field("thumbnail_width", thumbnail_width_);
    s.store_field("thumbnail_height", thumbnail_height_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

inputInlineQueryResultDocument::inputInlineQueryResultDocument()
  : id_()
  , title_()
  , description_()
  , document_url_()
  , mime_type_()
  , thumbnail_url_()
  , thumbnail_width_()
  , thumbnail_height_()
  , reply_markup_()
  , input_message_content_()
{}

inputInlineQueryResultDocument::inputInlineQueryResultDocument(string const &id_, string const &title_, string const &description_, string const &document_url_, string const &mime_type_, string const &thumbnail_url_, int32 thumbnail_width_, int32 thumbnail_height_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_)
  : id_(id_)
  , title_(title_)
  , description_(description_)
  , document_url_(document_url_)
  , mime_type_(mime_type_)
  , thumbnail_url_(thumbnail_url_)
  , thumbnail_width_(thumbnail_width_)
  , thumbnail_height_(thumbnail_height_)
  , reply_markup_(std::move(reply_markup_))
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t inputInlineQueryResultDocument::ID;

void inputInlineQueryResultDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInlineQueryResultDocument");
    s.store_field("id", id_);
    s.store_field("title", title_);
    s.store_field("description", description_);
    s.store_field("document_url", document_url_);
    s.store_field("mime_type", mime_type_);
    s.store_field("thumbnail_url", thumbnail_url_);
    s.store_field("thumbnail_width", thumbnail_width_);
    s.store_field("thumbnail_height", thumbnail_height_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

inputInlineQueryResultGame::inputInlineQueryResultGame()
  : id_()
  , game_short_name_()
  , reply_markup_()
{}

inputInlineQueryResultGame::inputInlineQueryResultGame(string const &id_, string const &game_short_name_, object_ptr<ReplyMarkup> &&reply_markup_)
  : id_(id_)
  , game_short_name_(game_short_name_)
  , reply_markup_(std::move(reply_markup_))
{}

const std::int32_t inputInlineQueryResultGame::ID;

void inputInlineQueryResultGame::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInlineQueryResultGame");
    s.store_field("id", id_);
    s.store_field("game_short_name", game_short_name_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_class_end();
  }
}

inputInlineQueryResultLocation::inputInlineQueryResultLocation()
  : id_()
  , location_()
  , live_period_()
  , title_()
  , thumbnail_url_()
  , thumbnail_width_()
  , thumbnail_height_()
  , reply_markup_()
  , input_message_content_()
{}

inputInlineQueryResultLocation::inputInlineQueryResultLocation(string const &id_, object_ptr<location> &&location_, int32 live_period_, string const &title_, string const &thumbnail_url_, int32 thumbnail_width_, int32 thumbnail_height_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_)
  : id_(id_)
  , location_(std::move(location_))
  , live_period_(live_period_)
  , title_(title_)
  , thumbnail_url_(thumbnail_url_)
  , thumbnail_width_(thumbnail_width_)
  , thumbnail_height_(thumbnail_height_)
  , reply_markup_(std::move(reply_markup_))
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t inputInlineQueryResultLocation::ID;

void inputInlineQueryResultLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInlineQueryResultLocation");
    s.store_field("id", id_);
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_field("live_period", live_period_);
    s.store_field("title", title_);
    s.store_field("thumbnail_url", thumbnail_url_);
    s.store_field("thumbnail_width", thumbnail_width_);
    s.store_field("thumbnail_height", thumbnail_height_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

inputInlineQueryResultPhoto::inputInlineQueryResultPhoto()
  : id_()
  , title_()
  , description_()
  , thumbnail_url_()
  , photo_url_()
  , photo_width_()
  , photo_height_()
  , reply_markup_()
  , input_message_content_()
{}

inputInlineQueryResultPhoto::inputInlineQueryResultPhoto(string const &id_, string const &title_, string const &description_, string const &thumbnail_url_, string const &photo_url_, int32 photo_width_, int32 photo_height_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_)
  : id_(id_)
  , title_(title_)
  , description_(description_)
  , thumbnail_url_(thumbnail_url_)
  , photo_url_(photo_url_)
  , photo_width_(photo_width_)
  , photo_height_(photo_height_)
  , reply_markup_(std::move(reply_markup_))
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t inputInlineQueryResultPhoto::ID;

void inputInlineQueryResultPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInlineQueryResultPhoto");
    s.store_field("id", id_);
    s.store_field("title", title_);
    s.store_field("description", description_);
    s.store_field("thumbnail_url", thumbnail_url_);
    s.store_field("photo_url", photo_url_);
    s.store_field("photo_width", photo_width_);
    s.store_field("photo_height", photo_height_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

inputInlineQueryResultSticker::inputInlineQueryResultSticker()
  : id_()
  , thumbnail_url_()
  , sticker_url_()
  , sticker_width_()
  , sticker_height_()
  , reply_markup_()
  , input_message_content_()
{}

inputInlineQueryResultSticker::inputInlineQueryResultSticker(string const &id_, string const &thumbnail_url_, string const &sticker_url_, int32 sticker_width_, int32 sticker_height_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_)
  : id_(id_)
  , thumbnail_url_(thumbnail_url_)
  , sticker_url_(sticker_url_)
  , sticker_width_(sticker_width_)
  , sticker_height_(sticker_height_)
  , reply_markup_(std::move(reply_markup_))
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t inputInlineQueryResultSticker::ID;

void inputInlineQueryResultSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInlineQueryResultSticker");
    s.store_field("id", id_);
    s.store_field("thumbnail_url", thumbnail_url_);
    s.store_field("sticker_url", sticker_url_);
    s.store_field("sticker_width", sticker_width_);
    s.store_field("sticker_height", sticker_height_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

inputInlineQueryResultVenue::inputInlineQueryResultVenue()
  : id_()
  , venue_()
  , thumbnail_url_()
  , thumbnail_width_()
  , thumbnail_height_()
  , reply_markup_()
  , input_message_content_()
{}

inputInlineQueryResultVenue::inputInlineQueryResultVenue(string const &id_, object_ptr<venue> &&venue_, string const &thumbnail_url_, int32 thumbnail_width_, int32 thumbnail_height_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_)
  : id_(id_)
  , venue_(std::move(venue_))
  , thumbnail_url_(thumbnail_url_)
  , thumbnail_width_(thumbnail_width_)
  , thumbnail_height_(thumbnail_height_)
  , reply_markup_(std::move(reply_markup_))
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t inputInlineQueryResultVenue::ID;

void inputInlineQueryResultVenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInlineQueryResultVenue");
    s.store_field("id", id_);
    s.store_object_field("venue", static_cast<const BaseObject *>(venue_.get()));
    s.store_field("thumbnail_url", thumbnail_url_);
    s.store_field("thumbnail_width", thumbnail_width_);
    s.store_field("thumbnail_height", thumbnail_height_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

inputInlineQueryResultVideo::inputInlineQueryResultVideo()
  : id_()
  , title_()
  , description_()
  , thumbnail_url_()
  , video_url_()
  , mime_type_()
  , video_width_()
  , video_height_()
  , video_duration_()
  , reply_markup_()
  , input_message_content_()
{}

inputInlineQueryResultVideo::inputInlineQueryResultVideo(string const &id_, string const &title_, string const &description_, string const &thumbnail_url_, string const &video_url_, string const &mime_type_, int32 video_width_, int32 video_height_, int32 video_duration_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_)
  : id_(id_)
  , title_(title_)
  , description_(description_)
  , thumbnail_url_(thumbnail_url_)
  , video_url_(video_url_)
  , mime_type_(mime_type_)
  , video_width_(video_width_)
  , video_height_(video_height_)
  , video_duration_(video_duration_)
  , reply_markup_(std::move(reply_markup_))
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t inputInlineQueryResultVideo::ID;

void inputInlineQueryResultVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInlineQueryResultVideo");
    s.store_field("id", id_);
    s.store_field("title", title_);
    s.store_field("description", description_);
    s.store_field("thumbnail_url", thumbnail_url_);
    s.store_field("video_url", video_url_);
    s.store_field("mime_type", mime_type_);
    s.store_field("video_width", video_width_);
    s.store_field("video_height", video_height_);
    s.store_field("video_duration", video_duration_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

inputInlineQueryResultVoiceNote::inputInlineQueryResultVoiceNote()
  : id_()
  , title_()
  , voice_note_url_()
  , voice_note_duration_()
  , reply_markup_()
  , input_message_content_()
{}

inputInlineQueryResultVoiceNote::inputInlineQueryResultVoiceNote(string const &id_, string const &title_, string const &voice_note_url_, int32 voice_note_duration_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_)
  : id_(id_)
  , title_(title_)
  , voice_note_url_(voice_note_url_)
  , voice_note_duration_(voice_note_duration_)
  , reply_markup_(std::move(reply_markup_))
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t inputInlineQueryResultVoiceNote::ID;

void inputInlineQueryResultVoiceNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputInlineQueryResultVoiceNote");
    s.store_field("id", id_);
    s.store_field("title", title_);
    s.store_field("voice_note_url", voice_note_url_);
    s.store_field("voice_note_duration", voice_note_duration_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

inputMessageReplyToMessage::inputMessageReplyToMessage()
  : message_id_()
  , quote_()
  , checklist_task_id_()
{}

inputMessageReplyToMessage::inputMessageReplyToMessage(int53 message_id_, object_ptr<inputTextQuote> &&quote_, int32 checklist_task_id_)
  : message_id_(message_id_)
  , quote_(std::move(quote_))
  , checklist_task_id_(checklist_task_id_)
{}

const std::int32_t inputMessageReplyToMessage::ID;

void inputMessageReplyToMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageReplyToMessage");
    s.store_field("message_id", message_id_);
    s.store_object_field("quote", static_cast<const BaseObject *>(quote_.get()));
    s.store_field("checklist_task_id", checklist_task_id_);
    s.store_class_end();
  }
}

inputMessageReplyToExternalMessage::inputMessageReplyToExternalMessage()
  : chat_id_()
  , message_id_()
  , quote_()
  , checklist_task_id_()
{}

inputMessageReplyToExternalMessage::inputMessageReplyToExternalMessage(int53 chat_id_, int53 message_id_, object_ptr<inputTextQuote> &&quote_, int32 checklist_task_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , quote_(std::move(quote_))
  , checklist_task_id_(checklist_task_id_)
{}

const std::int32_t inputMessageReplyToExternalMessage::ID;

void inputMessageReplyToExternalMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageReplyToExternalMessage");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("quote", static_cast<const BaseObject *>(quote_.get()));
    s.store_field("checklist_task_id", checklist_task_id_);
    s.store_class_end();
  }
}

inputMessageReplyToStory::inputMessageReplyToStory()
  : story_poster_chat_id_()
  , story_id_()
{}

inputMessageReplyToStory::inputMessageReplyToStory(int53 story_poster_chat_id_, int32 story_id_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , story_id_(story_id_)
{}

const std::int32_t inputMessageReplyToStory::ID;

void inputMessageReplyToStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageReplyToStory");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("story_id", story_id_);
    s.store_class_end();
  }
}

inviteGroupCallParticipantResultUserPrivacyRestricted::inviteGroupCallParticipantResultUserPrivacyRestricted() {
}

const std::int32_t inviteGroupCallParticipantResultUserPrivacyRestricted::ID;

void inviteGroupCallParticipantResultUserPrivacyRestricted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inviteGroupCallParticipantResultUserPrivacyRestricted");
    s.store_class_end();
  }
}

inviteGroupCallParticipantResultUserAlreadyParticipant::inviteGroupCallParticipantResultUserAlreadyParticipant() {
}

const std::int32_t inviteGroupCallParticipantResultUserAlreadyParticipant::ID;

void inviteGroupCallParticipantResultUserAlreadyParticipant::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inviteGroupCallParticipantResultUserAlreadyParticipant");
    s.store_class_end();
  }
}

inviteGroupCallParticipantResultUserWasBanned::inviteGroupCallParticipantResultUserWasBanned() {
}

const std::int32_t inviteGroupCallParticipantResultUserWasBanned::ID;

void inviteGroupCallParticipantResultUserWasBanned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inviteGroupCallParticipantResultUserWasBanned");
    s.store_class_end();
  }
}

inviteGroupCallParticipantResultSuccess::inviteGroupCallParticipantResultSuccess()
  : chat_id_()
  , message_id_()
{}

inviteGroupCallParticipantResultSuccess::inviteGroupCallParticipantResultSuccess(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t inviteGroupCallParticipantResultSuccess::ID;

void inviteGroupCallParticipantResultSuccess::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inviteGroupCallParticipantResultSuccess");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

loginUrlInfoOpen::loginUrlInfoOpen()
  : url_()
  , skip_confirmation_()
{}

loginUrlInfoOpen::loginUrlInfoOpen(string const &url_, bool skip_confirmation_)
  : url_(url_)
  , skip_confirmation_(skip_confirmation_)
{}

const std::int32_t loginUrlInfoOpen::ID;

void loginUrlInfoOpen::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "loginUrlInfoOpen");
    s.store_field("url", url_);
    s.store_field("skip_confirmation", skip_confirmation_);
    s.store_class_end();
  }
}

loginUrlInfoRequestConfirmation::loginUrlInfoRequestConfirmation()
  : url_()
  , domain_()
  , bot_user_id_()
  , request_write_access_()
{}

loginUrlInfoRequestConfirmation::loginUrlInfoRequestConfirmation(string const &url_, string const &domain_, int53 bot_user_id_, bool request_write_access_)
  : url_(url_)
  , domain_(domain_)
  , bot_user_id_(bot_user_id_)
  , request_write_access_(request_write_access_)
{}

const std::int32_t loginUrlInfoRequestConfirmation::ID;

void loginUrlInfoRequestConfirmation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "loginUrlInfoRequestConfirmation");
    s.store_field("url", url_);
    s.store_field("domain", domain_);
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("request_write_access", request_write_access_);
    s.store_class_end();
  }
}

messageAutoDeleteTime::messageAutoDeleteTime()
  : time_()
{}

messageAutoDeleteTime::messageAutoDeleteTime(int32 time_)
  : time_(time_)
{}

const std::int32_t messageAutoDeleteTime::ID;

void messageAutoDeleteTime::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageAutoDeleteTime");
    s.store_field("time", time_);
    s.store_class_end();
  }
}

messageReaction::messageReaction()
  : type_()
  , total_count_()
  , is_chosen_()
  , used_sender_id_()
  , recent_sender_ids_()
{}

messageReaction::messageReaction(object_ptr<ReactionType> &&type_, int32 total_count_, bool is_chosen_, object_ptr<MessageSender> &&used_sender_id_, array<object_ptr<MessageSender>> &&recent_sender_ids_)
  : type_(std::move(type_))
  , total_count_(total_count_)
  , is_chosen_(is_chosen_)
  , used_sender_id_(std::move(used_sender_id_))
  , recent_sender_ids_(std::move(recent_sender_ids_))
{}

const std::int32_t messageReaction::ID;

void messageReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageReaction");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("total_count", total_count_);
    s.store_field("is_chosen", is_chosen_);
    s.store_object_field("used_sender_id", static_cast<const BaseObject *>(used_sender_id_.get()));
    { s.store_vector_begin("recent_sender_ids", recent_sender_ids_.size()); for (const auto &_value : recent_sender_ids_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messageSenderUser::messageSenderUser()
  : user_id_()
{}

messageSenderUser::messageSenderUser(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t messageSenderUser::ID;

void messageSenderUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSenderUser");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

messageSenderChat::messageSenderChat()
  : chat_id_()
{}

messageSenderChat::messageSenderChat(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t messageSenderChat::ID;

void messageSenderChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSenderChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

notificationSound::notificationSound()
  : id_()
  , duration_()
  , date_()
  , title_()
  , data_()
  , sound_()
{}

notificationSound::notificationSound(int64 id_, int32 duration_, int32 date_, string const &title_, string const &data_, object_ptr<file> &&sound_)
  : id_(id_)
  , duration_(duration_)
  , date_(date_)
  , title_(title_)
  , data_(data_)
  , sound_(std::move(sound_))
{}

const std::int32_t notificationSound::ID;

void notificationSound::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "notificationSound");
    s.store_field("id", id_);
    s.store_field("duration", duration_);
    s.store_field("date", date_);
    s.store_field("title", title_);
    s.store_field("data", data_);
    s.store_object_field("sound", static_cast<const BaseObject *>(sound_.get()));
    s.store_class_end();
  }
}

passportElementPersonalDetails::passportElementPersonalDetails()
  : personal_details_()
{}

passportElementPersonalDetails::passportElementPersonalDetails(object_ptr<personalDetails> &&personal_details_)
  : personal_details_(std::move(personal_details_))
{}

const std::int32_t passportElementPersonalDetails::ID;

void passportElementPersonalDetails::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementPersonalDetails");
    s.store_object_field("personal_details", static_cast<const BaseObject *>(personal_details_.get()));
    s.store_class_end();
  }
}

passportElementPassport::passportElementPassport()
  : passport_()
{}

passportElementPassport::passportElementPassport(object_ptr<identityDocument> &&passport_)
  : passport_(std::move(passport_))
{}

const std::int32_t passportElementPassport::ID;

void passportElementPassport::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementPassport");
    s.store_object_field("passport", static_cast<const BaseObject *>(passport_.get()));
    s.store_class_end();
  }
}

passportElementDriverLicense::passportElementDriverLicense()
  : driver_license_()
{}

passportElementDriverLicense::passportElementDriverLicense(object_ptr<identityDocument> &&driver_license_)
  : driver_license_(std::move(driver_license_))
{}

const std::int32_t passportElementDriverLicense::ID;

void passportElementDriverLicense::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementDriverLicense");
    s.store_object_field("driver_license", static_cast<const BaseObject *>(driver_license_.get()));
    s.store_class_end();
  }
}

passportElementIdentityCard::passportElementIdentityCard()
  : identity_card_()
{}

passportElementIdentityCard::passportElementIdentityCard(object_ptr<identityDocument> &&identity_card_)
  : identity_card_(std::move(identity_card_))
{}

const std::int32_t passportElementIdentityCard::ID;

void passportElementIdentityCard::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementIdentityCard");
    s.store_object_field("identity_card", static_cast<const BaseObject *>(identity_card_.get()));
    s.store_class_end();
  }
}

passportElementInternalPassport::passportElementInternalPassport()
  : internal_passport_()
{}

passportElementInternalPassport::passportElementInternalPassport(object_ptr<identityDocument> &&internal_passport_)
  : internal_passport_(std::move(internal_passport_))
{}

const std::int32_t passportElementInternalPassport::ID;

void passportElementInternalPassport::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementInternalPassport");
    s.store_object_field("internal_passport", static_cast<const BaseObject *>(internal_passport_.get()));
    s.store_class_end();
  }
}

passportElementAddress::passportElementAddress()
  : address_()
{}

passportElementAddress::passportElementAddress(object_ptr<address> &&address_)
  : address_(std::move(address_))
{}

const std::int32_t passportElementAddress::ID;

void passportElementAddress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementAddress");
    s.store_object_field("address", static_cast<const BaseObject *>(address_.get()));
    s.store_class_end();
  }
}

passportElementUtilityBill::passportElementUtilityBill()
  : utility_bill_()
{}

passportElementUtilityBill::passportElementUtilityBill(object_ptr<personalDocument> &&utility_bill_)
  : utility_bill_(std::move(utility_bill_))
{}

const std::int32_t passportElementUtilityBill::ID;

void passportElementUtilityBill::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementUtilityBill");
    s.store_object_field("utility_bill", static_cast<const BaseObject *>(utility_bill_.get()));
    s.store_class_end();
  }
}

passportElementBankStatement::passportElementBankStatement()
  : bank_statement_()
{}

passportElementBankStatement::passportElementBankStatement(object_ptr<personalDocument> &&bank_statement_)
  : bank_statement_(std::move(bank_statement_))
{}

const std::int32_t passportElementBankStatement::ID;

void passportElementBankStatement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementBankStatement");
    s.store_object_field("bank_statement", static_cast<const BaseObject *>(bank_statement_.get()));
    s.store_class_end();
  }
}

passportElementRentalAgreement::passportElementRentalAgreement()
  : rental_agreement_()
{}

passportElementRentalAgreement::passportElementRentalAgreement(object_ptr<personalDocument> &&rental_agreement_)
  : rental_agreement_(std::move(rental_agreement_))
{}

const std::int32_t passportElementRentalAgreement::ID;

void passportElementRentalAgreement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementRentalAgreement");
    s.store_object_field("rental_agreement", static_cast<const BaseObject *>(rental_agreement_.get()));
    s.store_class_end();
  }
}

passportElementPassportRegistration::passportElementPassportRegistration()
  : passport_registration_()
{}

passportElementPassportRegistration::passportElementPassportRegistration(object_ptr<personalDocument> &&passport_registration_)
  : passport_registration_(std::move(passport_registration_))
{}

const std::int32_t passportElementPassportRegistration::ID;

void passportElementPassportRegistration::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementPassportRegistration");
    s.store_object_field("passport_registration", static_cast<const BaseObject *>(passport_registration_.get()));
    s.store_class_end();
  }
}

passportElementTemporaryRegistration::passportElementTemporaryRegistration()
  : temporary_registration_()
{}

passportElementTemporaryRegistration::passportElementTemporaryRegistration(object_ptr<personalDocument> &&temporary_registration_)
  : temporary_registration_(std::move(temporary_registration_))
{}

const std::int32_t passportElementTemporaryRegistration::ID;

void passportElementTemporaryRegistration::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementTemporaryRegistration");
    s.store_object_field("temporary_registration", static_cast<const BaseObject *>(temporary_registration_.get()));
    s.store_class_end();
  }
}

passportElementPhoneNumber::passportElementPhoneNumber()
  : phone_number_()
{}

passportElementPhoneNumber::passportElementPhoneNumber(string const &phone_number_)
  : phone_number_(phone_number_)
{}

const std::int32_t passportElementPhoneNumber::ID;

void passportElementPhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementPhoneNumber");
    s.store_field("phone_number", phone_number_);
    s.store_class_end();
  }
}

passportElementEmailAddress::passportElementEmailAddress()
  : email_address_()
{}

passportElementEmailAddress::passportElementEmailAddress(string const &email_address_)
  : email_address_(email_address_)
{}

const std::int32_t passportElementEmailAddress::ID;

void passportElementEmailAddress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementEmailAddress");
    s.store_field("email_address", email_address_);
    s.store_class_end();
  }
}

prepaidGiveaway::prepaidGiveaway()
  : id_()
  , winner_count_()
  , prize_()
  , boost_count_()
  , payment_date_()
{}

prepaidGiveaway::prepaidGiveaway(int64 id_, int32 winner_count_, object_ptr<GiveawayPrize> &&prize_, int32 boost_count_, int32 payment_date_)
  : id_(id_)
  , winner_count_(winner_count_)
  , prize_(std::move(prize_))
  , boost_count_(boost_count_)
  , payment_date_(payment_date_)
{}

const std::int32_t prepaidGiveaway::ID;

void prepaidGiveaway::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "prepaidGiveaway");
    s.store_field("id", id_);
    s.store_field("winner_count", winner_count_);
    s.store_object_field("prize", static_cast<const BaseObject *>(prize_.get()));
    s.store_field("boost_count", boost_count_);
    s.store_field("payment_date", payment_date_);
    s.store_class_end();
  }
}

pushReceiverId::pushReceiverId()
  : id_()
{}

pushReceiverId::pushReceiverId(int64 id_)
  : id_(id_)
{}

const std::int32_t pushReceiverId::ID;

void pushReceiverId::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pushReceiverId");
    s.store_field("id", id_);
    s.store_class_end();
  }
}

revenueWithdrawalStatePending::revenueWithdrawalStatePending() {
}

const std::int32_t revenueWithdrawalStatePending::ID;

void revenueWithdrawalStatePending::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "revenueWithdrawalStatePending");
    s.store_class_end();
  }
}

revenueWithdrawalStateSucceeded::revenueWithdrawalStateSucceeded()
  : date_()
  , url_()
{}

revenueWithdrawalStateSucceeded::revenueWithdrawalStateSucceeded(int32 date_, string const &url_)
  : date_(date_)
  , url_(url_)
{}

const std::int32_t revenueWithdrawalStateSucceeded::ID;

void revenueWithdrawalStateSucceeded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "revenueWithdrawalStateSucceeded");
    s.store_field("date", date_);
    s.store_field("url", url_);
    s.store_class_end();
  }
}

revenueWithdrawalStateFailed::revenueWithdrawalStateFailed() {
}

const std::int32_t revenueWithdrawalStateFailed::ID;

void revenueWithdrawalStateFailed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "revenueWithdrawalStateFailed");
    s.store_class_end();
  }
}

sentGiftRegular::sentGiftRegular()
  : gift_()
{}

sentGiftRegular::sentGiftRegular(object_ptr<gift> &&gift_)
  : gift_(std::move(gift_))
{}

const std::int32_t sentGiftRegular::ID;

void sentGiftRegular::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sentGiftRegular");
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_class_end();
  }
}

sentGiftUpgraded::sentGiftUpgraded()
  : gift_()
{}

sentGiftUpgraded::sentGiftUpgraded(object_ptr<upgradedGift> &&gift_)
  : gift_(std::move(gift_))
{}

const std::int32_t sentGiftUpgraded::ID;

void sentGiftUpgraded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sentGiftUpgraded");
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_class_end();
  }
}

session::session()
  : id_()
  , is_current_()
  , is_password_pending_()
  , is_unconfirmed_()
  , can_accept_secret_chats_()
  , can_accept_calls_()
  , type_()
  , api_id_()
  , application_name_()
  , application_version_()
  , is_official_application_()
  , device_model_()
  , platform_()
  , system_version_()
  , log_in_date_()
  , last_active_date_()
  , ip_address_()
  , location_()
{}

session::session(int64 id_, bool is_current_, bool is_password_pending_, bool is_unconfirmed_, bool can_accept_secret_chats_, bool can_accept_calls_, object_ptr<SessionType> &&type_, int32 api_id_, string const &application_name_, string const &application_version_, bool is_official_application_, string const &device_model_, string const &platform_, string const &system_version_, int32 log_in_date_, int32 last_active_date_, string const &ip_address_, string const &location_)
  : id_(id_)
  , is_current_(is_current_)
  , is_password_pending_(is_password_pending_)
  , is_unconfirmed_(is_unconfirmed_)
  , can_accept_secret_chats_(can_accept_secret_chats_)
  , can_accept_calls_(can_accept_calls_)
  , type_(std::move(type_))
  , api_id_(api_id_)
  , application_name_(application_name_)
  , application_version_(application_version_)
  , is_official_application_(is_official_application_)
  , device_model_(device_model_)
  , platform_(platform_)
  , system_version_(system_version_)
  , log_in_date_(log_in_date_)
  , last_active_date_(last_active_date_)
  , ip_address_(ip_address_)
  , location_(location_)
{}

const std::int32_t session::ID;

void session::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "session");
    s.store_field("id", id_);
    s.store_field("is_current", is_current_);
    s.store_field("is_password_pending", is_password_pending_);
    s.store_field("is_unconfirmed", is_unconfirmed_);
    s.store_field("can_accept_secret_chats", can_accept_secret_chats_);
    s.store_field("can_accept_calls", can_accept_calls_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("api_id", api_id_);
    s.store_field("application_name", application_name_);
    s.store_field("application_version", application_version_);
    s.store_field("is_official_application", is_official_application_);
    s.store_field("device_model", device_model_);
    s.store_field("platform", platform_);
    s.store_field("system_version", system_version_);
    s.store_field("log_in_date", log_in_date_);
    s.store_field("last_active_date", last_active_date_);
    s.store_field("ip_address", ip_address_);
    s.store_field("location", location_);
    s.store_class_end();
  }
}

storageStatistics::storageStatistics()
  : size_()
  , count_()
  , by_chat_()
{}

storageStatistics::storageStatistics(int53 size_, int32 count_, array<object_ptr<storageStatisticsByChat>> &&by_chat_)
  : size_(size_)
  , count_(count_)
  , by_chat_(std::move(by_chat_))
{}

const std::int32_t storageStatistics::ID;

void storageStatistics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storageStatistics");
    s.store_field("size", size_);
    s.store_field("count", count_);
    { s.store_vector_begin("by_chat", by_chat_.size()); for (const auto &_value : by_chat_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

storyAlbum::storyAlbum()
  : id_()
  , name_()
  , photo_icon_()
  , video_icon_()
{}

storyAlbum::storyAlbum(int32 id_, string const &name_, object_ptr<photo> &&photo_icon_, object_ptr<video> &&video_icon_)
  : id_(id_)
  , name_(name_)
  , photo_icon_(std::move(photo_icon_))
  , video_icon_(std::move(video_icon_))
{}

const std::int32_t storyAlbum::ID;

void storyAlbum::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyAlbum");
    s.store_field("id", id_);
    s.store_field("name", name_);
    s.store_object_field("photo_icon", static_cast<const BaseObject *>(photo_icon_.get()));
    s.store_object_field("video_icon", static_cast<const BaseObject *>(video_icon_.get()));
    s.store_class_end();
  }
}

supergroup::supergroup()
  : id_()
  , usernames_()
  , date_()
  , status_()
  , member_count_()
  , boost_level_()
  , has_automatic_translation_()
  , has_linked_chat_()
  , has_location_()
  , sign_messages_()
  , show_message_sender_()
  , join_to_send_messages_()
  , join_by_request_()
  , is_slow_mode_enabled_()
  , is_channel_()
  , is_broadcast_group_()
  , is_forum_()
  , is_direct_messages_group_()
  , is_administered_direct_messages_group_()
  , verification_status_()
  , has_direct_messages_group_()
  , has_forum_tabs_()
  , restriction_info_()
  , paid_message_star_count_()
  , has_active_stories_()
  , has_unread_active_stories_()
{}

supergroup::supergroup(int53 id_, object_ptr<usernames> &&usernames_, int32 date_, object_ptr<ChatMemberStatus> &&status_, int32 member_count_, int32 boost_level_, bool has_automatic_translation_, bool has_linked_chat_, bool has_location_, bool sign_messages_, bool show_message_sender_, bool join_to_send_messages_, bool join_by_request_, bool is_slow_mode_enabled_, bool is_channel_, bool is_broadcast_group_, bool is_forum_, bool is_direct_messages_group_, bool is_administered_direct_messages_group_, object_ptr<verificationStatus> &&verification_status_, bool has_direct_messages_group_, bool has_forum_tabs_, object_ptr<restrictionInfo> &&restriction_info_, int53 paid_message_star_count_, bool has_active_stories_, bool has_unread_active_stories_)
  : id_(id_)
  , usernames_(std::move(usernames_))
  , date_(date_)
  , status_(std::move(status_))
  , member_count_(member_count_)
  , boost_level_(boost_level_)
  , has_automatic_translation_(has_automatic_translation_)
  , has_linked_chat_(has_linked_chat_)
  , has_location_(has_location_)
  , sign_messages_(sign_messages_)
  , show_message_sender_(show_message_sender_)
  , join_to_send_messages_(join_to_send_messages_)
  , join_by_request_(join_by_request_)
  , is_slow_mode_enabled_(is_slow_mode_enabled_)
  , is_channel_(is_channel_)
  , is_broadcast_group_(is_broadcast_group_)
  , is_forum_(is_forum_)
  , is_direct_messages_group_(is_direct_messages_group_)
  , is_administered_direct_messages_group_(is_administered_direct_messages_group_)
  , verification_status_(std::move(verification_status_))
  , has_direct_messages_group_(has_direct_messages_group_)
  , has_forum_tabs_(has_forum_tabs_)
  , restriction_info_(std::move(restriction_info_))
  , paid_message_star_count_(paid_message_star_count_)
  , has_active_stories_(has_active_stories_)
  , has_unread_active_stories_(has_unread_active_stories_)
{}

const std::int32_t supergroup::ID;

void supergroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "supergroup");
    s.store_field("id", id_);
    s.store_object_field("usernames", static_cast<const BaseObject *>(usernames_.get()));
    s.store_field("date", date_);
    s.store_object_field("status", static_cast<const BaseObject *>(status_.get()));
    s.store_field("member_count", member_count_);
    s.store_field("boost_level", boost_level_);
    s.store_field("has_automatic_translation", has_automatic_translation_);
    s.store_field("has_linked_chat", has_linked_chat_);
    s.store_field("has_location", has_location_);
    s.store_field("sign_messages", sign_messages_);
    s.store_field("show_message_sender", show_message_sender_);
    s.store_field("join_to_send_messages", join_to_send_messages_);
    s.store_field("join_by_request", join_by_request_);
    s.store_field("is_slow_mode_enabled", is_slow_mode_enabled_);
    s.store_field("is_channel", is_channel_);
    s.store_field("is_broadcast_group", is_broadcast_group_);
    s.store_field("is_forum", is_forum_);
    s.store_field("is_direct_messages_group", is_direct_messages_group_);
    s.store_field("is_administered_direct_messages_group", is_administered_direct_messages_group_);
    s.store_object_field("verification_status", static_cast<const BaseObject *>(verification_status_.get()));
    s.store_field("has_direct_messages_group", has_direct_messages_group_);
    s.store_field("has_forum_tabs", has_forum_tabs_);
    s.store_object_field("restriction_info", static_cast<const BaseObject *>(restriction_info_.get()));
    s.store_field("paid_message_star_count", paid_message_star_count_);
    s.store_field("has_active_stories", has_active_stories_);
    s.store_field("has_unread_active_stories", has_unread_active_stories_);
    s.store_class_end();
  }
}

tMeUrls::tMeUrls()
  : urls_()
{}

tMeUrls::tMeUrls(array<object_ptr<tMeUrl>> &&urls_)
  : urls_(std::move(urls_))
{}

const std::int32_t tMeUrls::ID;

void tMeUrls::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "tMeUrls");
    { s.store_vector_begin("urls", urls_.size()); for (const auto &_value : urls_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

updates::updates()
  : updates_()
{}

updates::updates(array<object_ptr<Update>> &&updates_)
  : updates_(std::move(updates_))
{}

const std::int32_t updates::ID;

void updates::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "updates");
    { s.store_vector_begin("updates", updates_.size()); for (const auto &_value : updates_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

upgradedGiftModelCount::upgradedGiftModelCount()
  : model_()
  , total_count_()
{}

upgradedGiftModelCount::upgradedGiftModelCount(object_ptr<upgradedGiftModel> &&model_, int32 total_count_)
  : model_(std::move(model_))
  , total_count_(total_count_)
{}

const std::int32_t upgradedGiftModelCount::ID;

void upgradedGiftModelCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradedGiftModelCount");
    s.store_object_field("model", static_cast<const BaseObject *>(model_.get()));
    s.store_field("total_count", total_count_);
    s.store_class_end();
  }
}

userRating::userRating()
  : level_()
  , is_maximum_level_reached_()
  , rating_()
  , current_level_rating_()
  , next_level_rating_()
{}

userRating::userRating(int32 level_, bool is_maximum_level_reached_, int53 rating_, int53 current_level_rating_, int53 next_level_rating_)
  : level_(level_)
  , is_maximum_level_reached_(is_maximum_level_reached_)
  , rating_(rating_)
  , current_level_rating_(current_level_rating_)
  , next_level_rating_(next_level_rating_)
{}

const std::int32_t userRating::ID;

void userRating::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userRating");
    s.store_field("level", level_);
    s.store_field("is_maximum_level_reached", is_maximum_level_reached_);
    s.store_field("rating", rating_);
    s.store_field("current_level_rating", current_level_rating_);
    s.store_field("next_level_rating", next_level_rating_);
    s.store_class_end();
  }
}

users::users()
  : total_count_()
  , user_ids_()
{}

users::users(int32 total_count_, array<int53> &&user_ids_)
  : total_count_(total_count_)
  , user_ids_(std::move(user_ids_))
{}

const std::int32_t users::ID;

void users::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "users");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("user_ids", user_ids_.size()); for (const auto &_value : user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

videoChatStreams::videoChatStreams()
  : streams_()
{}

videoChatStreams::videoChatStreams(array<object_ptr<videoChatStream>> &&streams_)
  : streams_(std::move(streams_))
{}

const std::int32_t videoChatStreams::ID;

void videoChatStreams::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "videoChatStreams");
    { s.store_vector_begin("streams", streams_.size()); for (const auto &_value : streams_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

acceptCall::acceptCall()
  : call_id_()
  , protocol_()
{}

acceptCall::acceptCall(int32 call_id_, object_ptr<callProtocol> &&protocol_)
  : call_id_(call_id_)
  , protocol_(std::move(protocol_))
{}

const std::int32_t acceptCall::ID;

void acceptCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "acceptCall");
    s.store_field("call_id", call_id_);
    s.store_object_field("protocol", static_cast<const BaseObject *>(protocol_.get()));
    s.store_class_end();
  }
}

activateStoryStealthMode::activateStoryStealthMode() {
}

const std::int32_t activateStoryStealthMode::ID;

void activateStoryStealthMode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "activateStoryStealthMode");
    s.store_class_end();
  }
}

applyPremiumGiftCode::applyPremiumGiftCode()
  : code_()
{}

applyPremiumGiftCode::applyPremiumGiftCode(string const &code_)
  : code_(code_)
{}

const std::int32_t applyPremiumGiftCode::ID;

void applyPremiumGiftCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "applyPremiumGiftCode");
    s.store_field("code", code_);
    s.store_class_end();
  }
}

checkLoginEmailAddressCode::checkLoginEmailAddressCode()
  : code_()
{}

checkLoginEmailAddressCode::checkLoginEmailAddressCode(object_ptr<EmailAddressAuthentication> &&code_)
  : code_(std::move(code_))
{}

const std::int32_t checkLoginEmailAddressCode::ID;

void checkLoginEmailAddressCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkLoginEmailAddressCode");
    s.store_object_field("code", static_cast<const BaseObject *>(code_.get()));
    s.store_class_end();
  }
}

clearRecentlyFoundChats::clearRecentlyFoundChats() {
}

const std::int32_t clearRecentlyFoundChats::ID;

void clearRecentlyFoundChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "clearRecentlyFoundChats");
    s.store_class_end();
  }
}

clickPremiumSubscriptionButton::clickPremiumSubscriptionButton() {
}

const std::int32_t clickPremiumSubscriptionButton::ID;

void clickPremiumSubscriptionButton::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "clickPremiumSubscriptionButton");
    s.store_class_end();
  }
}

close::close() {
}

const std::int32_t close::ID;

void close::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "close");
    s.store_class_end();
  }
}

confirmQrCodeAuthentication::confirmQrCodeAuthentication()
  : link_()
{}

confirmQrCodeAuthentication::confirmQrCodeAuthentication(string const &link_)
  : link_(link_)
{}

const std::int32_t confirmQrCodeAuthentication::ID;

void confirmQrCodeAuthentication::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "confirmQrCodeAuthentication");
    s.store_field("link", link_);
    s.store_class_end();
  }
}

createBusinessChatLink::createBusinessChatLink()
  : link_info_()
{}

createBusinessChatLink::createBusinessChatLink(object_ptr<inputBusinessChatLink> &&link_info_)
  : link_info_(std::move(link_info_))
{}

const std::int32_t createBusinessChatLink::ID;

void createBusinessChatLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createBusinessChatLink");
    s.store_object_field("link_info", static_cast<const BaseObject *>(link_info_.get()));
    s.store_class_end();
  }
}

createForumTopic::createForumTopic()
  : chat_id_()
  , name_()
  , is_name_implicit_()
  , icon_()
{}

createForumTopic::createForumTopic(int53 chat_id_, string const &name_, bool is_name_implicit_, object_ptr<forumTopicIcon> &&icon_)
  : chat_id_(chat_id_)
  , name_(name_)
  , is_name_implicit_(is_name_implicit_)
  , icon_(std::move(icon_))
{}

const std::int32_t createForumTopic::ID;

void createForumTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createForumTopic");
    s.store_field("chat_id", chat_id_);
    s.store_field("name", name_);
    s.store_field("is_name_implicit", is_name_implicit_);
    s.store_object_field("icon", static_cast<const BaseObject *>(icon_.get()));
    s.store_class_end();
  }
}

createNewSecretChat::createNewSecretChat()
  : user_id_()
{}

createNewSecretChat::createNewSecretChat(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t createNewSecretChat::ID;

void createNewSecretChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createNewSecretChat");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

createTemporaryPassword::createTemporaryPassword()
  : password_()
  , valid_for_()
{}

createTemporaryPassword::createTemporaryPassword(string const &password_, int32 valid_for_)
  : password_(password_)
  , valid_for_(valid_for_)
{}

const std::int32_t createTemporaryPassword::ID;

void createTemporaryPassword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createTemporaryPassword");
    s.store_field("password", password_);
    s.store_field("valid_for", valid_for_);
    s.store_class_end();
  }
}

deleteChatFolderInviteLink::deleteChatFolderInviteLink()
  : chat_folder_id_()
  , invite_link_()
{}

deleteChatFolderInviteLink::deleteChatFolderInviteLink(int32 chat_folder_id_, string const &invite_link_)
  : chat_folder_id_(chat_folder_id_)
  , invite_link_(invite_link_)
{}

const std::int32_t deleteChatFolderInviteLink::ID;

void deleteChatFolderInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteChatFolderInviteLink");
    s.store_field("chat_folder_id", chat_folder_id_);
    s.store_field("invite_link", invite_link_);
    s.store_class_end();
  }
}

deleteChatMessagesByDate::deleteChatMessagesByDate()
  : chat_id_()
  , min_date_()
  , max_date_()
  , revoke_()
{}

deleteChatMessagesByDate::deleteChatMessagesByDate(int53 chat_id_, int32 min_date_, int32 max_date_, bool revoke_)
  : chat_id_(chat_id_)
  , min_date_(min_date_)
  , max_date_(max_date_)
  , revoke_(revoke_)
{}

const std::int32_t deleteChatMessagesByDate::ID;

void deleteChatMessagesByDate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteChatMessagesByDate");
    s.store_field("chat_id", chat_id_);
    s.store_field("min_date", min_date_);
    s.store_field("max_date", max_date_);
    s.store_field("revoke", revoke_);
    s.store_class_end();
  }
}

deleteProfilePhoto::deleteProfilePhoto()
  : profile_photo_id_()
{}

deleteProfilePhoto::deleteProfilePhoto(int64 profile_photo_id_)
  : profile_photo_id_(profile_photo_id_)
{}

const std::int32_t deleteProfilePhoto::ID;

void deleteProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteProfilePhoto");
    s.store_field("profile_photo_id", profile_photo_id_);
    s.store_class_end();
  }
}

deleteQuickReplyShortcut::deleteQuickReplyShortcut()
  : shortcut_id_()
{}

deleteQuickReplyShortcut::deleteQuickReplyShortcut(int32 shortcut_id_)
  : shortcut_id_(shortcut_id_)
{}

const std::int32_t deleteQuickReplyShortcut::ID;

void deleteQuickReplyShortcut::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteQuickReplyShortcut");
    s.store_field("shortcut_id", shortcut_id_);
    s.store_class_end();
  }
}

editQuickReplyMessage::editQuickReplyMessage()
  : shortcut_id_()
  , message_id_()
  , input_message_content_()
{}

editQuickReplyMessage::editQuickReplyMessage(int32 shortcut_id_, int53 message_id_, object_ptr<InputMessageContent> &&input_message_content_)
  : shortcut_id_(shortcut_id_)
  , message_id_(message_id_)
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t editQuickReplyMessage::ID;

void editQuickReplyMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editQuickReplyMessage");
    s.store_field("shortcut_id", shortcut_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

editUserStarSubscription::editUserStarSubscription()
  : user_id_()
  , telegram_payment_charge_id_()
  , is_canceled_()
{}

editUserStarSubscription::editUserStarSubscription(int53 user_id_, string const &telegram_payment_charge_id_, bool is_canceled_)
  : user_id_(user_id_)
  , telegram_payment_charge_id_(telegram_payment_charge_id_)
  , is_canceled_(is_canceled_)
{}

const std::int32_t editUserStarSubscription::ID;

void editUserStarSubscription::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editUserStarSubscription");
    s.store_field("user_id", user_id_);
    s.store_field("telegram_payment_charge_id", telegram_payment_charge_id_);
    s.store_field("is_canceled", is_canceled_);
    s.store_class_end();
  }
}

getApplicationConfig::getApplicationConfig() {
}

const std::int32_t getApplicationConfig::ID;

void getApplicationConfig::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getApplicationConfig");
    s.store_class_end();
  }
}

getArchivedStickerSets::getArchivedStickerSets()
  : sticker_type_()
  , offset_sticker_set_id_()
  , limit_()
{}

getArchivedStickerSets::getArchivedStickerSets(object_ptr<StickerType> &&sticker_type_, int64 offset_sticker_set_id_, int32 limit_)
  : sticker_type_(std::move(sticker_type_))
  , offset_sticker_set_id_(offset_sticker_set_id_)
  , limit_(limit_)
{}

const std::int32_t getArchivedStickerSets::ID;

void getArchivedStickerSets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getArchivedStickerSets");
    s.store_object_field("sticker_type", static_cast<const BaseObject *>(sticker_type_.get()));
    s.store_field("offset_sticker_set_id", offset_sticker_set_id_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getAutoDownloadSettingsPresets::getAutoDownloadSettingsPresets() {
}

const std::int32_t getAutoDownloadSettingsPresets::ID;

void getAutoDownloadSettingsPresets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getAutoDownloadSettingsPresets");
    s.store_class_end();
  }
}

getBotMediaPreviews::getBotMediaPreviews()
  : bot_user_id_()
{}

getBotMediaPreviews::getBotMediaPreviews(int53 bot_user_id_)
  : bot_user_id_(bot_user_id_)
{}

const std::int32_t getBotMediaPreviews::ID;

void getBotMediaPreviews::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getBotMediaPreviews");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_class_end();
  }
}

getBotSimilarBots::getBotSimilarBots()
  : bot_user_id_()
{}

getBotSimilarBots::getBotSimilarBots(int53 bot_user_id_)
  : bot_user_id_(bot_user_id_)
{}

const std::int32_t getBotSimilarBots::ID;

void getBotSimilarBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getBotSimilarBots");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_class_end();
  }
}

getBusinessConnectedBot::getBusinessConnectedBot() {
}

const std::int32_t getBusinessConnectedBot::ID;

void getBusinessConnectedBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getBusinessConnectedBot");
    s.store_class_end();
  }
}

getCallbackQueryMessage::getCallbackQueryMessage()
  : chat_id_()
  , message_id_()
  , callback_query_id_()
{}

getCallbackQueryMessage::getCallbackQueryMessage(int53 chat_id_, int53 message_id_, int64 callback_query_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , callback_query_id_(callback_query_id_)
{}

const std::int32_t getCallbackQueryMessage::ID;

void getCallbackQueryMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getCallbackQueryMessage");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("callback_query_id", callback_query_id_);
    s.store_class_end();
  }
}

getChatFolderInviteLinks::getChatFolderInviteLinks()
  : chat_folder_id_()
{}

getChatFolderInviteLinks::getChatFolderInviteLinks(int32 chat_folder_id_)
  : chat_folder_id_(chat_folder_id_)
{}

const std::int32_t getChatFolderInviteLinks::ID;

void getChatFolderInviteLinks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatFolderInviteLinks");
    s.store_field("chat_folder_id", chat_folder_id_);
    s.store_class_end();
  }
}

getChatMember::getChatMember()
  : chat_id_()
  , member_id_()
{}

getChatMember::getChatMember(int53 chat_id_, object_ptr<MessageSender> &&member_id_)
  : chat_id_(chat_id_)
  , member_id_(std::move(member_id_))
{}

const std::int32_t getChatMember::ID;

void getChatMember::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatMember");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("member_id", static_cast<const BaseObject *>(member_id_.get()));
    s.store_class_end();
  }
}

getDisallowedChatEmojiStatuses::getDisallowedChatEmojiStatuses() {
}

const std::int32_t getDisallowedChatEmojiStatuses::ID;

void getDisallowedChatEmojiStatuses::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getDisallowedChatEmojiStatuses");
    s.store_class_end();
  }
}

getGiftChatThemes::getGiftChatThemes()
  : offset_()
  , limit_()
{}

getGiftChatThemes::getGiftChatThemes(string const &offset_, int32 limit_)
  : offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getGiftChatThemes::ID;

void getGiftChatThemes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getGiftChatThemes");
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getInstalledStickerSets::getInstalledStickerSets()
  : sticker_type_()
{}

getInstalledStickerSets::getInstalledStickerSets(object_ptr<StickerType> &&sticker_type_)
  : sticker_type_(std::move(sticker_type_))
{}

const std::int32_t getInstalledStickerSets::ID;

void getInstalledStickerSets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getInstalledStickerSets");
    s.store_object_field("sticker_type", static_cast<const BaseObject *>(sticker_type_.get()));
    s.store_class_end();
  }
}

getMessagePublicForwards::getMessagePublicForwards()
  : chat_id_()
  , message_id_()
  , offset_()
  , limit_()
{}

getMessagePublicForwards::getMessagePublicForwards(int53 chat_id_, int53 message_id_, string const &offset_, int32 limit_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getMessagePublicForwards::ID;

void getMessagePublicForwards::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMessagePublicForwards");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getPassportElement::getPassportElement()
  : type_()
  , password_()
{}

getPassportElement::getPassportElement(object_ptr<PassportElementType> &&type_, string const &password_)
  : type_(std::move(type_))
  , password_(password_)
{}

const std::int32_t getPassportElement::ID;

void getPassportElement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPassportElement");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("password", password_);
    s.store_class_end();
  }
}

getPasswordState::getPasswordState() {
}

const std::int32_t getPasswordState::ID;

void getPasswordState::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPasswordState");
    s.store_class_end();
  }
}

getPollVoters::getPollVoters()
  : chat_id_()
  , message_id_()
  , option_id_()
  , offset_()
  , limit_()
{}

getPollVoters::getPollVoters(int53 chat_id_, int53 message_id_, int32 option_id_, int32 offset_, int32 limit_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , option_id_(option_id_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getPollVoters::ID;

void getPollVoters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPollVoters");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("option_id", option_id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getProxies::getProxies() {
}

const std::int32_t getProxies::ID;

void getProxies::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getProxies");
    s.store_class_end();
  }
}

getReceivedGifts::getReceivedGifts()
  : business_connection_id_()
  , owner_id_()
  , collection_id_()
  , exclude_unsaved_()
  , exclude_saved_()
  , exclude_unlimited_()
  , exclude_upgradable_()
  , exclude_non_upgradable_()
  , exclude_upgraded_()
  , exclude_without_colors_()
  , exclude_hosted_()
  , sort_by_price_()
  , offset_()
  , limit_()
{}

getReceivedGifts::getReceivedGifts(string const &business_connection_id_, object_ptr<MessageSender> &&owner_id_, int32 collection_id_, bool exclude_unsaved_, bool exclude_saved_, bool exclude_unlimited_, bool exclude_upgradable_, bool exclude_non_upgradable_, bool exclude_upgraded_, bool exclude_without_colors_, bool exclude_hosted_, bool sort_by_price_, string const &offset_, int32 limit_)
  : business_connection_id_(business_connection_id_)
  , owner_id_(std::move(owner_id_))
  , collection_id_(collection_id_)
  , exclude_unsaved_(exclude_unsaved_)
  , exclude_saved_(exclude_saved_)
  , exclude_unlimited_(exclude_unlimited_)
  , exclude_upgradable_(exclude_upgradable_)
  , exclude_non_upgradable_(exclude_non_upgradable_)
  , exclude_upgraded_(exclude_upgraded_)
  , exclude_without_colors_(exclude_without_colors_)
  , exclude_hosted_(exclude_hosted_)
  , sort_by_price_(sort_by_price_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getReceivedGifts::ID;

void getReceivedGifts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getReceivedGifts");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_field("collection_id", collection_id_);
    s.store_field("exclude_unsaved", exclude_unsaved_);
    s.store_field("exclude_saved", exclude_saved_);
    s.store_field("exclude_unlimited", exclude_unlimited_);
    s.store_field("exclude_upgradable", exclude_upgradable_);
    s.store_field("exclude_non_upgradable", exclude_non_upgradable_);
    s.store_field("exclude_upgraded", exclude_upgraded_);
    s.store_field("exclude_without_colors", exclude_without_colors_);
    s.store_field("exclude_hosted", exclude_hosted_);
    s.store_field("sort_by_price", sort_by_price_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getRecentlyOpenedChats::getRecentlyOpenedChats()
  : limit_()
{}

getRecentlyOpenedChats::getRecentlyOpenedChats(int32 limit_)
  : limit_(limit_)
{}

const std::int32_t getRecentlyOpenedChats::ID;

void getRecentlyOpenedChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getRecentlyOpenedChats");
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getSavedMessagesTopicHistory::getSavedMessagesTopicHistory()
  : saved_messages_topic_id_()
  , from_message_id_()
  , offset_()
  , limit_()
{}

getSavedMessagesTopicHistory::getSavedMessagesTopicHistory(int53 saved_messages_topic_id_, int53 from_message_id_, int32 offset_, int32 limit_)
  : saved_messages_topic_id_(saved_messages_topic_id_)
  , from_message_id_(from_message_id_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getSavedMessagesTopicHistory::ID;

void getSavedMessagesTopicHistory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getSavedMessagesTopicHistory");
    s.store_field("saved_messages_topic_id", saved_messages_topic_id_);
    s.store_field("from_message_id", from_message_id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getSearchSponsoredChats::getSearchSponsoredChats()
  : query_()
{}

getSearchSponsoredChats::getSearchSponsoredChats(string const &query_)
  : query_(query_)
{}

const std::int32_t getSearchSponsoredChats::ID;

void getSearchSponsoredChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getSearchSponsoredChats");
    s.store_field("query", query_);
    s.store_class_end();
  }
}

getStickerSetName::getStickerSetName()
  : set_id_()
{}

getStickerSetName::getStickerSetName(int64 set_id_)
  : set_id_(set_id_)
{}

const std::int32_t getStickerSetName::ID;

void getStickerSetName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStickerSetName");
    s.store_field("set_id", set_id_);
    s.store_class_end();
  }
}

getStoryInteractions::getStoryInteractions()
  : story_id_()
  , query_()
  , only_contacts_()
  , prefer_forwards_()
  , prefer_with_reaction_()
  , offset_()
  , limit_()
{}

getStoryInteractions::getStoryInteractions(int32 story_id_, string const &query_, bool only_contacts_, bool prefer_forwards_, bool prefer_with_reaction_, string const &offset_, int32 limit_)
  : story_id_(story_id_)
  , query_(query_)
  , only_contacts_(only_contacts_)
  , prefer_forwards_(prefer_forwards_)
  , prefer_with_reaction_(prefer_with_reaction_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getStoryInteractions::ID;

void getStoryInteractions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStoryInteractions");
    s.store_field("story_id", story_id_);
    s.store_field("query", query_);
    s.store_field("only_contacts", only_contacts_);
    s.store_field("prefer_forwards", prefer_forwards_);
    s.store_field("prefer_with_reaction", prefer_with_reaction_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getUserFullInfo::getUserFullInfo()
  : user_id_()
{}

getUserFullInfo::getUserFullInfo(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t getUserFullInfo::ID;

void getUserFullInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getUserFullInfo");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

getVideoChatInviteLink::getVideoChatInviteLink()
  : group_call_id_()
  , can_self_unmute_()
{}

getVideoChatInviteLink::getVideoChatInviteLink(int32 group_call_id_, bool can_self_unmute_)
  : group_call_id_(group_call_id_)
  , can_self_unmute_(can_self_unmute_)
{}

const std::int32_t getVideoChatInviteLink::ID;

void getVideoChatInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getVideoChatInviteLink");
    s.store_field("group_call_id", group_call_id_);
    s.store_field("can_self_unmute", can_self_unmute_);
    s.store_class_end();
  }
}

getWebAppLinkUrl::getWebAppLinkUrl()
  : chat_id_()
  , bot_user_id_()
  , web_app_short_name_()
  , start_parameter_()
  , allow_write_access_()
  , parameters_()
{}

getWebAppLinkUrl::getWebAppLinkUrl(int53 chat_id_, int53 bot_user_id_, string const &web_app_short_name_, string const &start_parameter_, bool allow_write_access_, object_ptr<webAppOpenParameters> &&parameters_)
  : chat_id_(chat_id_)
  , bot_user_id_(bot_user_id_)
  , web_app_short_name_(web_app_short_name_)
  , start_parameter_(start_parameter_)
  , allow_write_access_(allow_write_access_)
  , parameters_(std::move(parameters_))
{}

const std::int32_t getWebAppLinkUrl::ID;

void getWebAppLinkUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getWebAppLinkUrl");
    s.store_field("chat_id", chat_id_);
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("web_app_short_name", web_app_short_name_);
    s.store_field("start_parameter", start_parameter_);
    s.store_field("allow_write_access", allow_write_access_);
    s.store_object_field("parameters", static_cast<const BaseObject *>(parameters_.get()));
    s.store_class_end();
  }
}

getWebAppUrl::getWebAppUrl()
  : bot_user_id_()
  , url_()
  , parameters_()
{}

getWebAppUrl::getWebAppUrl(int53 bot_user_id_, string const &url_, object_ptr<webAppOpenParameters> &&parameters_)
  : bot_user_id_(bot_user_id_)
  , url_(url_)
  , parameters_(std::move(parameters_))
{}

const std::int32_t getWebAppUrl::ID;

void getWebAppUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getWebAppUrl");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("url", url_);
    s.store_object_field("parameters", static_cast<const BaseObject *>(parameters_.get()));
    s.store_class_end();
  }
}

loadQuickReplyShortcuts::loadQuickReplyShortcuts() {
}

const std::int32_t loadQuickReplyShortcuts::ID;

void loadQuickReplyShortcuts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "loadQuickReplyShortcuts");
    s.store_class_end();
  }
}

parseMarkdown::parseMarkdown()
  : text_()
{}

parseMarkdown::parseMarkdown(object_ptr<formattedText> &&text_)
  : text_(std::move(text_))
{}

const std::int32_t parseMarkdown::ID;

void parseMarkdown::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "parseMarkdown");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

pingProxy::pingProxy()
  : proxy_id_()
{}

pingProxy::pingProxy(int32 proxy_id_)
  : proxy_id_(proxy_id_)
{}

const std::int32_t pingProxy::ID;

void pingProxy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pingProxy");
    s.store_field("proxy_id", proxy_id_);
    s.store_class_end();
  }
}

readAllChatMentions::readAllChatMentions()
  : chat_id_()
{}

readAllChatMentions::readAllChatMentions(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t readAllChatMentions::ID;

void readAllChatMentions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "readAllChatMentions");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

removeBusinessConnectedBotFromChat::removeBusinessConnectedBotFromChat()
  : chat_id_()
{}

removeBusinessConnectedBotFromChat::removeBusinessConnectedBotFromChat(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t removeBusinessConnectedBotFromChat::ID;

void removeBusinessConnectedBotFromChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeBusinessConnectedBotFromChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

removeMessageReaction::removeMessageReaction()
  : chat_id_()
  , message_id_()
  , reaction_type_()
{}

removeMessageReaction::removeMessageReaction(int53 chat_id_, int53 message_id_, object_ptr<ReactionType> &&reaction_type_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , reaction_type_(std::move(reaction_type_))
{}

const std::int32_t removeMessageReaction::ID;

void removeMessageReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeMessageReaction");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("reaction_type", static_cast<const BaseObject *>(reaction_type_.get()));
    s.store_class_end();
  }
}

reorderGiftCollections::reorderGiftCollections()
  : owner_id_()
  , collection_ids_()
{}

reorderGiftCollections::reorderGiftCollections(object_ptr<MessageSender> &&owner_id_, array<int32> &&collection_ids_)
  : owner_id_(std::move(owner_id_))
  , collection_ids_(std::move(collection_ids_))
{}

const std::int32_t reorderGiftCollections::ID;

void reorderGiftCollections::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reorderGiftCollections");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    { s.store_vector_begin("collection_ids", collection_ids_.size()); for (const auto &_value : collection_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

reorderStoryAlbums::reorderStoryAlbums()
  : chat_id_()
  , story_album_ids_()
{}

reorderStoryAlbums::reorderStoryAlbums(int53 chat_id_, array<int32> &&story_album_ids_)
  : chat_id_(chat_id_)
  , story_album_ids_(std::move(story_album_ids_))
{}

const std::int32_t reorderStoryAlbums::ID;

void reorderStoryAlbums::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reorderStoryAlbums");
    s.store_field("chat_id", chat_id_);
    { s.store_vector_begin("story_album_ids", story_album_ids_.size()); for (const auto &_value : story_album_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

replaceStickerInSet::replaceStickerInSet()
  : user_id_()
  , name_()
  , old_sticker_()
  , new_sticker_()
{}

replaceStickerInSet::replaceStickerInSet(int53 user_id_, string const &name_, object_ptr<InputFile> &&old_sticker_, object_ptr<inputSticker> &&new_sticker_)
  : user_id_(user_id_)
  , name_(name_)
  , old_sticker_(std::move(old_sticker_))
  , new_sticker_(std::move(new_sticker_))
{}

const std::int32_t replaceStickerInSet::ID;

void replaceStickerInSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "replaceStickerInSet");
    s.store_field("user_id", user_id_);
    s.store_field("name", name_);
    s.store_object_field("old_sticker", static_cast<const BaseObject *>(old_sticker_.get()));
    s.store_object_field("new_sticker", static_cast<const BaseObject *>(new_sticker_.get()));
    s.store_class_end();
  }
}

revokeGroupCallInviteLink::revokeGroupCallInviteLink()
  : group_call_id_()
{}

revokeGroupCallInviteLink::revokeGroupCallInviteLink(int32 group_call_id_)
  : group_call_id_(group_call_id_)
{}

const std::int32_t revokeGroupCallInviteLink::ID;

void revokeGroupCallInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "revokeGroupCallInviteLink");
    s.store_field("group_call_id", group_call_id_);
    s.store_class_end();
  }
}

searchCallMessages::searchCallMessages()
  : offset_()
  , limit_()
  , only_missed_()
{}

searchCallMessages::searchCallMessages(string const &offset_, int32 limit_, bool only_missed_)
  : offset_(offset_)
  , limit_(limit_)
  , only_missed_(only_missed_)
{}

const std::int32_t searchCallMessages::ID;

void searchCallMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchCallMessages");
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_field("only_missed", only_missed_);
    s.store_class_end();
  }
}

searchPublicChats::searchPublicChats()
  : query_()
{}

searchPublicChats::searchPublicChats(string const &query_)
  : query_(query_)
{}

const std::int32_t searchPublicChats::ID;

void searchPublicChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchPublicChats");
    s.store_field("query", query_);
    s.store_class_end();
  }
}

searchPublicStoriesByLocation::searchPublicStoriesByLocation()
  : address_()
  , offset_()
  , limit_()
{}

searchPublicStoriesByLocation::searchPublicStoriesByLocation(object_ptr<locationAddress> &&address_, string const &offset_, int32 limit_)
  : address_(std::move(address_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t searchPublicStoriesByLocation::ID;

void searchPublicStoriesByLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchPublicStoriesByLocation");
    s.store_object_field("address", static_cast<const BaseObject *>(address_.get()));
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

searchSecretMessages::searchSecretMessages()
  : chat_id_()
  , query_()
  , offset_()
  , limit_()
  , filter_()
{}

searchSecretMessages::searchSecretMessages(int53 chat_id_, string const &query_, string const &offset_, int32 limit_, object_ptr<SearchMessagesFilter> &&filter_)
  : chat_id_(chat_id_)
  , query_(query_)
  , offset_(offset_)
  , limit_(limit_)
  , filter_(std::move(filter_))
{}

const std::int32_t searchSecretMessages::ID;

void searchSecretMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchSecretMessages");
    s.store_field("chat_id", chat_id_);
    s.store_field("query", query_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get()));
    s.store_class_end();
  }
}

sendMessageAlbum::sendMessageAlbum()
  : chat_id_()
  , topic_id_()
  , reply_to_()
  , options_()
  , input_message_contents_()
{}

sendMessageAlbum::sendMessageAlbum(int53 chat_id_, object_ptr<MessageTopic> &&topic_id_, object_ptr<InputMessageReplyTo> &&reply_to_, object_ptr<messageSendOptions> &&options_, array<object_ptr<InputMessageContent>> &&input_message_contents_)
  : chat_id_(chat_id_)
  , topic_id_(std::move(topic_id_))
  , reply_to_(std::move(reply_to_))
  , options_(std::move(options_))
  , input_message_contents_(std::move(input_message_contents_))
{}

const std::int32_t sendMessageAlbum::ID;

void sendMessageAlbum::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendMessageAlbum");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("topic_id", static_cast<const BaseObject *>(topic_id_.get()));
    s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get()));
    s.store_object_field("options", static_cast<const BaseObject *>(options_.get()));
    { s.store_vector_begin("input_message_contents", input_message_contents_.size()); for (const auto &_value : input_message_contents_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

sendPaymentForm::sendPaymentForm()
  : input_invoice_()
  , payment_form_id_()
  , order_info_id_()
  , shipping_option_id_()
  , credentials_()
  , tip_amount_()
{}

sendPaymentForm::sendPaymentForm(object_ptr<InputInvoice> &&input_invoice_, int64 payment_form_id_, string const &order_info_id_, string const &shipping_option_id_, object_ptr<InputCredentials> &&credentials_, int53 tip_amount_)
  : input_invoice_(std::move(input_invoice_))
  , payment_form_id_(payment_form_id_)
  , order_info_id_(order_info_id_)
  , shipping_option_id_(shipping_option_id_)
  , credentials_(std::move(credentials_))
  , tip_amount_(tip_amount_)
{}

const std::int32_t sendPaymentForm::ID;

void sendPaymentForm::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendPaymentForm");
    s.store_object_field("input_invoice", static_cast<const BaseObject *>(input_invoice_.get()));
    s.store_field("payment_form_id", payment_form_id_);
    s.store_field("order_info_id", order_info_id_);
    s.store_field("shipping_option_id", shipping_option_id_);
    s.store_object_field("credentials", static_cast<const BaseObject *>(credentials_.get()));
    s.store_field("tip_amount", tip_amount_);
    s.store_class_end();
  }
}

setBotName::setBotName()
  : bot_user_id_()
  , language_code_()
  , name_()
{}

setBotName::setBotName(int53 bot_user_id_, string const &language_code_, string const &name_)
  : bot_user_id_(bot_user_id_)
  , language_code_(language_code_)
  , name_(name_)
{}

const std::int32_t setBotName::ID;

void setBotName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setBotName");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("language_code", language_code_);
    s.store_field("name", name_);
    s.store_class_end();
  }
}

setChatBackground::setChatBackground()
  : chat_id_()
  , background_()
  , type_()
  , dark_theme_dimming_()
  , only_for_self_()
{}

setChatBackground::setChatBackground(int53 chat_id_, object_ptr<InputBackground> &&background_, object_ptr<BackgroundType> &&type_, int32 dark_theme_dimming_, bool only_for_self_)
  : chat_id_(chat_id_)
  , background_(std::move(background_))
  , type_(std::move(type_))
  , dark_theme_dimming_(dark_theme_dimming_)
  , only_for_self_(only_for_self_)
{}

const std::int32_t setChatBackground::ID;

void setChatBackground::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatBackground");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("background", static_cast<const BaseObject *>(background_.get()));
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("dark_theme_dimming", dark_theme_dimming_);
    s.store_field("only_for_self", only_for_self_);
    s.store_class_end();
  }
}

setChatDirectMessagesGroup::setChatDirectMessagesGroup()
  : chat_id_()
  , is_enabled_()
  , paid_message_star_count_()
{}

setChatDirectMessagesGroup::setChatDirectMessagesGroup(int53 chat_id_, bool is_enabled_, int53 paid_message_star_count_)
  : chat_id_(chat_id_)
  , is_enabled_(is_enabled_)
  , paid_message_star_count_(paid_message_star_count_)
{}

const std::int32_t setChatDirectMessagesGroup::ID;

void setChatDirectMessagesGroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatDirectMessagesGroup");
    s.store_field("chat_id", chat_id_);
    s.store_field("is_enabled", is_enabled_);
    s.store_field("paid_message_star_count", paid_message_star_count_);
    s.store_class_end();
  }
}

setChatMemberStatus::setChatMemberStatus()
  : chat_id_()
  , member_id_()
  , status_()
{}

setChatMemberStatus::setChatMemberStatus(int53 chat_id_, object_ptr<MessageSender> &&member_id_, object_ptr<ChatMemberStatus> &&status_)
  : chat_id_(chat_id_)
  , member_id_(std::move(member_id_))
  , status_(std::move(status_))
{}

const std::int32_t setChatMemberStatus::ID;

void setChatMemberStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatMemberStatus");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("member_id", static_cast<const BaseObject *>(member_id_.get()));
    s.store_object_field("status", static_cast<const BaseObject *>(status_.get()));
    s.store_class_end();
  }
}

setChatMessageSender::setChatMessageSender()
  : chat_id_()
  , message_sender_id_()
{}

setChatMessageSender::setChatMessageSender(int53 chat_id_, object_ptr<MessageSender> &&message_sender_id_)
  : chat_id_(chat_id_)
  , message_sender_id_(std::move(message_sender_id_))
{}

const std::int32_t setChatMessageSender::ID;

void setChatMessageSender::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatMessageSender");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("message_sender_id", static_cast<const BaseObject *>(message_sender_id_.get()));
    s.store_class_end();
  }
}

setChatNotificationSettings::setChatNotificationSettings()
  : chat_id_()
  , notification_settings_()
{}

setChatNotificationSettings::setChatNotificationSettings(int53 chat_id_, object_ptr<chatNotificationSettings> &&notification_settings_)
  : chat_id_(chat_id_)
  , notification_settings_(std::move(notification_settings_))
{}

const std::int32_t setChatNotificationSettings::ID;

void setChatNotificationSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatNotificationSettings");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("notification_settings", static_cast<const BaseObject *>(notification_settings_.get()));
    s.store_class_end();
  }
}

setChatPermissions::setChatPermissions()
  : chat_id_()
  , permissions_()
{}

setChatPermissions::setChatPermissions(int53 chat_id_, object_ptr<chatPermissions> &&permissions_)
  : chat_id_(chat_id_)
  , permissions_(std::move(permissions_))
{}

const std::int32_t setChatPermissions::ID;

void setChatPermissions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatPermissions");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("permissions", static_cast<const BaseObject *>(permissions_.get()));
    s.store_class_end();
  }
}

setChatTitle::setChatTitle()
  : chat_id_()
  , title_()
{}

setChatTitle::setChatTitle(int53 chat_id_, string const &title_)
  : chat_id_(chat_id_)
  , title_(title_)
{}

const std::int32_t setChatTitle::ID;

void setChatTitle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatTitle");
    s.store_field("chat_id", chat_id_);
    s.store_field("title", title_);
    s.store_class_end();
  }
}

setCommands::setCommands()
  : scope_()
  , language_code_()
  , commands_()
{}

setCommands::setCommands(object_ptr<BotCommandScope> &&scope_, string const &language_code_, array<object_ptr<botCommand>> &&commands_)
  : scope_(std::move(scope_))
  , language_code_(language_code_)
  , commands_(std::move(commands_))
{}

const std::int32_t setCommands::ID;

void setCommands::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setCommands");
    s.store_object_field("scope", static_cast<const BaseObject *>(scope_.get()));
    s.store_field("language_code", language_code_);
    { s.store_vector_begin("commands", commands_.size()); for (const auto &_value : commands_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

setCustomLanguagePack::setCustomLanguagePack()
  : info_()
  , strings_()
{}

setCustomLanguagePack::setCustomLanguagePack(object_ptr<languagePackInfo> &&info_, array<object_ptr<languagePackString>> &&strings_)
  : info_(std::move(info_))
  , strings_(std::move(strings_))
{}

const std::int32_t setCustomLanguagePack::ID;

void setCustomLanguagePack::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setCustomLanguagePack");
    s.store_object_field("info", static_cast<const BaseObject *>(info_.get()));
    { s.store_vector_begin("strings", strings_.size()); for (const auto &_value : strings_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

setDefaultChannelAdministratorRights::setDefaultChannelAdministratorRights()
  : default_channel_administrator_rights_()
{}

setDefaultChannelAdministratorRights::setDefaultChannelAdministratorRights(object_ptr<chatAdministratorRights> &&default_channel_administrator_rights_)
  : default_channel_administrator_rights_(std::move(default_channel_administrator_rights_))
{}

const std::int32_t setDefaultChannelAdministratorRights::ID;

void setDefaultChannelAdministratorRights::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setDefaultChannelAdministratorRights");
    s.store_object_field("default_channel_administrator_rights", static_cast<const BaseObject *>(default_channel_administrator_rights_.get()));
    s.store_class_end();
  }
}

setEmojiStatus::setEmojiStatus()
  : emoji_status_()
{}

setEmojiStatus::setEmojiStatus(object_ptr<emojiStatus> &&emoji_status_)
  : emoji_status_(std::move(emoji_status_))
{}

const std::int32_t setEmojiStatus::ID;

void setEmojiStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setEmojiStatus");
    s.store_object_field("emoji_status", static_cast<const BaseObject *>(emoji_status_.get()));
    s.store_class_end();
  }
}

setQuickReplyShortcutName::setQuickReplyShortcutName()
  : shortcut_id_()
  , name_()
{}

setQuickReplyShortcutName::setQuickReplyShortcutName(int32 shortcut_id_, string const &name_)
  : shortcut_id_(shortcut_id_)
  , name_(name_)
{}

const std::int32_t setQuickReplyShortcutName::ID;

void setQuickReplyShortcutName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setQuickReplyShortcutName");
    s.store_field("shortcut_id", shortcut_id_);
    s.store_field("name", name_);
    s.store_class_end();
  }
}

setSupergroupCustomEmojiStickerSet::setSupergroupCustomEmojiStickerSet()
  : supergroup_id_()
  , custom_emoji_sticker_set_id_()
{}

setSupergroupCustomEmojiStickerSet::setSupergroupCustomEmojiStickerSet(int53 supergroup_id_, int64 custom_emoji_sticker_set_id_)
  : supergroup_id_(supergroup_id_)
  , custom_emoji_sticker_set_id_(custom_emoji_sticker_set_id_)
{}

const std::int32_t setSupergroupCustomEmojiStickerSet::ID;

void setSupergroupCustomEmojiStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setSupergroupCustomEmojiStickerSet");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_field("custom_emoji_sticker_set_id", custom_emoji_sticker_set_id_);
    s.store_class_end();
  }
}

setUpgradedGiftColors::setUpgradedGiftColors()
  : upgraded_gift_colors_id_()
{}

setUpgradedGiftColors::setUpgradedGiftColors(int64 upgraded_gift_colors_id_)
  : upgraded_gift_colors_id_(upgraded_gift_colors_id_)
{}

const std::int32_t setUpgradedGiftColors::ID;

void setUpgradedGiftColors::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setUpgradedGiftColors");
    s.store_field("upgraded_gift_colors_id", upgraded_gift_colors_id_);
    s.store_class_end();
  }
}

setUsername::setUsername()
  : username_()
{}

setUsername::setUsername(string const &username_)
  : username_(username_)
{}

const std::int32_t setUsername::ID;

void setUsername::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setUsername");
    s.store_field("username", username_);
    s.store_class_end();
  }
}

shareChatWithBot::shareChatWithBot()
  : chat_id_()
  , message_id_()
  , button_id_()
  , shared_chat_id_()
  , only_check_()
{}

shareChatWithBot::shareChatWithBot(int53 chat_id_, int53 message_id_, int32 button_id_, int53 shared_chat_id_, bool only_check_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , button_id_(button_id_)
  , shared_chat_id_(shared_chat_id_)
  , only_check_(only_check_)
{}

const std::int32_t shareChatWithBot::ID;

void shareChatWithBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "shareChatWithBot");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("button_id", button_id_);
    s.store_field("shared_chat_id", shared_chat_id_);
    s.store_field("only_check", only_check_);
    s.store_class_end();
  }
}

sharePhoneNumber::sharePhoneNumber()
  : user_id_()
{}

sharePhoneNumber::sharePhoneNumber(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t sharePhoneNumber::ID;

void sharePhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sharePhoneNumber");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

startGroupCallRecording::startGroupCallRecording()
  : group_call_id_()
  , title_()
  , record_video_()
  , use_portrait_orientation_()
{}

startGroupCallRecording::startGroupCallRecording(int32 group_call_id_, string const &title_, bool record_video_, bool use_portrait_orientation_)
  : group_call_id_(group_call_id_)
  , title_(title_)
  , record_video_(record_video_)
  , use_portrait_orientation_(use_portrait_orientation_)
{}

const std::int32_t startGroupCallRecording::ID;

void startGroupCallRecording::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "startGroupCallRecording");
    s.store_field("group_call_id", group_call_id_);
    s.store_field("title", title_);
    s.store_field("record_video", record_video_);
    s.store_field("use_portrait_orientation", use_portrait_orientation_);
    s.store_class_end();
  }
}

testCallVectorInt::testCallVectorInt()
  : x_()
{}

testCallVectorInt::testCallVectorInt(array<int32> &&x_)
  : x_(std::move(x_))
{}

const std::int32_t testCallVectorInt::ID;

void testCallVectorInt::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testCallVectorInt");
    { s.store_vector_begin("x", x_.size()); for (const auto &_value : x_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

testSquareInt::testSquareInt()
  : x_()
{}

testSquareInt::testSquareInt(int32 x_)
  : x_(x_)
{}

const std::int32_t testSquareInt::ID;

void testSquareInt::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testSquareInt");
    s.store_field("x", x_);
    s.store_class_end();
  }
}

testUseUpdate::testUseUpdate() {
}

const std::int32_t testUseUpdate::ID;

void testUseUpdate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testUseUpdate");
    s.store_class_end();
  }
}

toggleChatIsPinned::toggleChatIsPinned()
  : chat_list_()
  , chat_id_()
  , is_pinned_()
{}

toggleChatIsPinned::toggleChatIsPinned(object_ptr<ChatList> &&chat_list_, int53 chat_id_, bool is_pinned_)
  : chat_list_(std::move(chat_list_))
  , chat_id_(chat_id_)
  , is_pinned_(is_pinned_)
{}

const std::int32_t toggleChatIsPinned::ID;

void toggleChatIsPinned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleChatIsPinned");
    s.store_object_field("chat_list", static_cast<const BaseObject *>(chat_list_.get()));
    s.store_field("chat_id", chat_id_);
    s.store_field("is_pinned", is_pinned_);
    s.store_class_end();
  }
}

toggleHasSponsoredMessagesEnabled::toggleHasSponsoredMessagesEnabled()
  : has_sponsored_messages_enabled_()
{}

toggleHasSponsoredMessagesEnabled::toggleHasSponsoredMessagesEnabled(bool has_sponsored_messages_enabled_)
  : has_sponsored_messages_enabled_(has_sponsored_messages_enabled_)
{}

const std::int32_t toggleHasSponsoredMessagesEnabled::ID;

void toggleHasSponsoredMessagesEnabled::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleHasSponsoredMessagesEnabled");
    s.store_field("has_sponsored_messages_enabled", has_sponsored_messages_enabled_);
    s.store_class_end();
  }
}

toggleSupergroupHasAutomaticTranslation::toggleSupergroupHasAutomaticTranslation()
  : supergroup_id_()
  , has_automatic_translation_()
{}

toggleSupergroupHasAutomaticTranslation::toggleSupergroupHasAutomaticTranslation(int53 supergroup_id_, bool has_automatic_translation_)
  : supergroup_id_(supergroup_id_)
  , has_automatic_translation_(has_automatic_translation_)
{}

const std::int32_t toggleSupergroupHasAutomaticTranslation::ID;

void toggleSupergroupHasAutomaticTranslation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleSupergroupHasAutomaticTranslation");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_field("has_automatic_translation", has_automatic_translation_);
    s.store_class_end();
  }
}

upgradeBasicGroupChatToSupergroupChat::upgradeBasicGroupChatToSupergroupChat()
  : chat_id_()
{}

upgradeBasicGroupChatToSupergroupChat::upgradeBasicGroupChatToSupergroupChat(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t upgradeBasicGroupChatToSupergroupChat::ID;

void upgradeBasicGroupChatToSupergroupChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradeBasicGroupChatToSupergroupChat");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

upgradeGift::upgradeGift()
  : business_connection_id_()
  , received_gift_id_()
  , keep_original_details_()
  , star_count_()
{}

upgradeGift::upgradeGift(string const &business_connection_id_, string const &received_gift_id_, bool keep_original_details_, int53 star_count_)
  : business_connection_id_(business_connection_id_)
  , received_gift_id_(received_gift_id_)
  , keep_original_details_(keep_original_details_)
  , star_count_(star_count_)
{}

const std::int32_t upgradeGift::ID;

void upgradeGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradeGift");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_field("received_gift_id", received_gift_id_);
    s.store_field("keep_original_details", keep_original_details_);
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

viewVideoMessageAdvertisement::viewVideoMessageAdvertisement()
  : advertisement_unique_id_()
{}

viewVideoMessageAdvertisement::viewVideoMessageAdvertisement(int53 advertisement_unique_id_)
  : advertisement_unique_id_(advertisement_unique_id_)
{}

const std::int32_t viewVideoMessageAdvertisement::ID;

void viewVideoMessageAdvertisement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "viewVideoMessageAdvertisement");
    s.store_field("advertisement_unique_id", advertisement_unique_id_);
    s.store_class_end();
  }
}
}  // namespace td_api
}  // namespace td
