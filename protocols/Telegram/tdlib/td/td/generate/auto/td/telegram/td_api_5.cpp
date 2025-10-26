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


animation::animation()
  : duration_()
  , width_()
  , height_()
  , file_name_()
  , mime_type_()
  , has_stickers_()
  , minithumbnail_()
  , thumbnail_()
  , animation_()
{}

animation::animation(int32 duration_, int32 width_, int32 height_, string const &file_name_, string const &mime_type_, bool has_stickers_, object_ptr<minithumbnail> &&minithumbnail_, object_ptr<thumbnail> &&thumbnail_, object_ptr<file> &&animation_)
  : duration_(duration_)
  , width_(width_)
  , height_(height_)
  , file_name_(file_name_)
  , mime_type_(mime_type_)
  , has_stickers_(has_stickers_)
  , minithumbnail_(std::move(minithumbnail_))
  , thumbnail_(std::move(thumbnail_))
  , animation_(std::move(animation_))
{}

const std::int32_t animation::ID;

void animation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "animation");
    s.store_field("duration", duration_);
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_field("file_name", file_name_);
    s.store_field("mime_type", mime_type_);
    s.store_field("has_stickers", has_stickers_);
    s.store_object_field("minithumbnail", static_cast<const BaseObject *>(minithumbnail_.get()));
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_object_field("animation", static_cast<const BaseObject *>(animation_.get()));
    s.store_class_end();
  }
}

authorizationStateWaitTdlibParameters::authorizationStateWaitTdlibParameters() {
}

const std::int32_t authorizationStateWaitTdlibParameters::ID;

void authorizationStateWaitTdlibParameters::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authorizationStateWaitTdlibParameters");
    s.store_class_end();
  }
}

authorizationStateWaitPhoneNumber::authorizationStateWaitPhoneNumber() {
}

const std::int32_t authorizationStateWaitPhoneNumber::ID;

void authorizationStateWaitPhoneNumber::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authorizationStateWaitPhoneNumber");
    s.store_class_end();
  }
}

authorizationStateWaitPremiumPurchase::authorizationStateWaitPremiumPurchase()
  : store_product_id_()
  , support_email_address_()
  , support_email_subject_()
{}

authorizationStateWaitPremiumPurchase::authorizationStateWaitPremiumPurchase(string const &store_product_id_, string const &support_email_address_, string const &support_email_subject_)
  : store_product_id_(store_product_id_)
  , support_email_address_(support_email_address_)
  , support_email_subject_(support_email_subject_)
{}

const std::int32_t authorizationStateWaitPremiumPurchase::ID;

void authorizationStateWaitPremiumPurchase::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authorizationStateWaitPremiumPurchase");
    s.store_field("store_product_id", store_product_id_);
    s.store_field("support_email_address", support_email_address_);
    s.store_field("support_email_subject", support_email_subject_);
    s.store_class_end();
  }
}

authorizationStateWaitEmailAddress::authorizationStateWaitEmailAddress()
  : allow_apple_id_()
  , allow_google_id_()
{}

authorizationStateWaitEmailAddress::authorizationStateWaitEmailAddress(bool allow_apple_id_, bool allow_google_id_)
  : allow_apple_id_(allow_apple_id_)
  , allow_google_id_(allow_google_id_)
{}

const std::int32_t authorizationStateWaitEmailAddress::ID;

void authorizationStateWaitEmailAddress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authorizationStateWaitEmailAddress");
    s.store_field("allow_apple_id", allow_apple_id_);
    s.store_field("allow_google_id", allow_google_id_);
    s.store_class_end();
  }
}

authorizationStateWaitEmailCode::authorizationStateWaitEmailCode()
  : allow_apple_id_()
  , allow_google_id_()
  , code_info_()
  , email_address_reset_state_()
{}

authorizationStateWaitEmailCode::authorizationStateWaitEmailCode(bool allow_apple_id_, bool allow_google_id_, object_ptr<emailAddressAuthenticationCodeInfo> &&code_info_, object_ptr<EmailAddressResetState> &&email_address_reset_state_)
  : allow_apple_id_(allow_apple_id_)
  , allow_google_id_(allow_google_id_)
  , code_info_(std::move(code_info_))
  , email_address_reset_state_(std::move(email_address_reset_state_))
{}

const std::int32_t authorizationStateWaitEmailCode::ID;

void authorizationStateWaitEmailCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authorizationStateWaitEmailCode");
    s.store_field("allow_apple_id", allow_apple_id_);
    s.store_field("allow_google_id", allow_google_id_);
    s.store_object_field("code_info", static_cast<const BaseObject *>(code_info_.get()));
    s.store_object_field("email_address_reset_state", static_cast<const BaseObject *>(email_address_reset_state_.get()));
    s.store_class_end();
  }
}

authorizationStateWaitCode::authorizationStateWaitCode()
  : code_info_()
{}

authorizationStateWaitCode::authorizationStateWaitCode(object_ptr<authenticationCodeInfo> &&code_info_)
  : code_info_(std::move(code_info_))
{}

const std::int32_t authorizationStateWaitCode::ID;

void authorizationStateWaitCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authorizationStateWaitCode");
    s.store_object_field("code_info", static_cast<const BaseObject *>(code_info_.get()));
    s.store_class_end();
  }
}

authorizationStateWaitOtherDeviceConfirmation::authorizationStateWaitOtherDeviceConfirmation()
  : link_()
{}

authorizationStateWaitOtherDeviceConfirmation::authorizationStateWaitOtherDeviceConfirmation(string const &link_)
  : link_(link_)
{}

const std::int32_t authorizationStateWaitOtherDeviceConfirmation::ID;

void authorizationStateWaitOtherDeviceConfirmation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authorizationStateWaitOtherDeviceConfirmation");
    s.store_field("link", link_);
    s.store_class_end();
  }
}

authorizationStateWaitRegistration::authorizationStateWaitRegistration()
  : terms_of_service_()
{}

authorizationStateWaitRegistration::authorizationStateWaitRegistration(object_ptr<termsOfService> &&terms_of_service_)
  : terms_of_service_(std::move(terms_of_service_))
{}

const std::int32_t authorizationStateWaitRegistration::ID;

void authorizationStateWaitRegistration::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authorizationStateWaitRegistration");
    s.store_object_field("terms_of_service", static_cast<const BaseObject *>(terms_of_service_.get()));
    s.store_class_end();
  }
}

authorizationStateWaitPassword::authorizationStateWaitPassword()
  : password_hint_()
  , has_recovery_email_address_()
  , has_passport_data_()
  , recovery_email_address_pattern_()
{}

authorizationStateWaitPassword::authorizationStateWaitPassword(string const &password_hint_, bool has_recovery_email_address_, bool has_passport_data_, string const &recovery_email_address_pattern_)
  : password_hint_(password_hint_)
  , has_recovery_email_address_(has_recovery_email_address_)
  , has_passport_data_(has_passport_data_)
  , recovery_email_address_pattern_(recovery_email_address_pattern_)
{}

const std::int32_t authorizationStateWaitPassword::ID;

void authorizationStateWaitPassword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authorizationStateWaitPassword");
    s.store_field("password_hint", password_hint_);
    s.store_field("has_recovery_email_address", has_recovery_email_address_);
    s.store_field("has_passport_data", has_passport_data_);
    s.store_field("recovery_email_address_pattern", recovery_email_address_pattern_);
    s.store_class_end();
  }
}

authorizationStateReady::authorizationStateReady() {
}

const std::int32_t authorizationStateReady::ID;

void authorizationStateReady::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authorizationStateReady");
    s.store_class_end();
  }
}

authorizationStateLoggingOut::authorizationStateLoggingOut() {
}

const std::int32_t authorizationStateLoggingOut::ID;

void authorizationStateLoggingOut::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authorizationStateLoggingOut");
    s.store_class_end();
  }
}

authorizationStateClosing::authorizationStateClosing() {
}

const std::int32_t authorizationStateClosing::ID;

void authorizationStateClosing::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authorizationStateClosing");
    s.store_class_end();
  }
}

authorizationStateClosed::authorizationStateClosed() {
}

const std::int32_t authorizationStateClosed::ID;

void authorizationStateClosed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authorizationStateClosed");
    s.store_class_end();
  }
}

autoDownloadSettings::autoDownloadSettings()
  : is_auto_download_enabled_()
  , max_photo_file_size_()
  , max_video_file_size_()
  , max_other_file_size_()
  , video_upload_bitrate_()
  , preload_large_videos_()
  , preload_next_audio_()
  , preload_stories_()
  , use_less_data_for_calls_()
{}

autoDownloadSettings::autoDownloadSettings(bool is_auto_download_enabled_, int32 max_photo_file_size_, int53 max_video_file_size_, int53 max_other_file_size_, int32 video_upload_bitrate_, bool preload_large_videos_, bool preload_next_audio_, bool preload_stories_, bool use_less_data_for_calls_)
  : is_auto_download_enabled_(is_auto_download_enabled_)
  , max_photo_file_size_(max_photo_file_size_)
  , max_video_file_size_(max_video_file_size_)
  , max_other_file_size_(max_other_file_size_)
  , video_upload_bitrate_(video_upload_bitrate_)
  , preload_large_videos_(preload_large_videos_)
  , preload_next_audio_(preload_next_audio_)
  , preload_stories_(preload_stories_)
  , use_less_data_for_calls_(use_less_data_for_calls_)
{}

const std::int32_t autoDownloadSettings::ID;

void autoDownloadSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "autoDownloadSettings");
    s.store_field("is_auto_download_enabled", is_auto_download_enabled_);
    s.store_field("max_photo_file_size", max_photo_file_size_);
    s.store_field("max_video_file_size", max_video_file_size_);
    s.store_field("max_other_file_size", max_other_file_size_);
    s.store_field("video_upload_bitrate", video_upload_bitrate_);
    s.store_field("preload_large_videos", preload_large_videos_);
    s.store_field("preload_next_audio", preload_next_audio_);
    s.store_field("preload_stories", preload_stories_);
    s.store_field("use_less_data_for_calls", use_less_data_for_calls_);
    s.store_class_end();
  }
}

botVerification::botVerification()
  : bot_user_id_()
  , icon_custom_emoji_id_()
  , custom_description_()
{}

botVerification::botVerification(int53 bot_user_id_, int64 icon_custom_emoji_id_, object_ptr<formattedText> &&custom_description_)
  : bot_user_id_(bot_user_id_)
  , icon_custom_emoji_id_(icon_custom_emoji_id_)
  , custom_description_(std::move(custom_description_))
{}

const std::int32_t botVerification::ID;

void botVerification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "botVerification");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("icon_custom_emoji_id", icon_custom_emoji_id_);
    s.store_object_field("custom_description", static_cast<const BaseObject *>(custom_description_.get()));
    s.store_class_end();
  }
}

businessAwayMessageSettings::businessAwayMessageSettings()
  : shortcut_id_()
  , recipients_()
  , schedule_()
  , offline_only_()
{}

businessAwayMessageSettings::businessAwayMessageSettings(int32 shortcut_id_, object_ptr<businessRecipients> &&recipients_, object_ptr<BusinessAwayMessageSchedule> &&schedule_, bool offline_only_)
  : shortcut_id_(shortcut_id_)
  , recipients_(std::move(recipients_))
  , schedule_(std::move(schedule_))
  , offline_only_(offline_only_)
{}

const std::int32_t businessAwayMessageSettings::ID;

void businessAwayMessageSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessAwayMessageSettings");
    s.store_field("shortcut_id", shortcut_id_);
    s.store_object_field("recipients", static_cast<const BaseObject *>(recipients_.get()));
    s.store_object_field("schedule", static_cast<const BaseObject *>(schedule_.get()));
    s.store_field("offline_only", offline_only_);
    s.store_class_end();
  }
}

businessChatLinkInfo::businessChatLinkInfo()
  : chat_id_()
  , text_()
{}

businessChatLinkInfo::businessChatLinkInfo(int53 chat_id_, object_ptr<formattedText> &&text_)
  : chat_id_(chat_id_)
  , text_(std::move(text_))
{}

const std::int32_t businessChatLinkInfo::ID;

void businessChatLinkInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessChatLinkInfo");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

canSendMessageToUserResultOk::canSendMessageToUserResultOk() {
}

const std::int32_t canSendMessageToUserResultOk::ID;

void canSendMessageToUserResultOk::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canSendMessageToUserResultOk");
    s.store_class_end();
  }
}

canSendMessageToUserResultUserHasPaidMessages::canSendMessageToUserResultUserHasPaidMessages()
  : outgoing_paid_message_star_count_()
{}

canSendMessageToUserResultUserHasPaidMessages::canSendMessageToUserResultUserHasPaidMessages(int53 outgoing_paid_message_star_count_)
  : outgoing_paid_message_star_count_(outgoing_paid_message_star_count_)
{}

const std::int32_t canSendMessageToUserResultUserHasPaidMessages::ID;

void canSendMessageToUserResultUserHasPaidMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canSendMessageToUserResultUserHasPaidMessages");
    s.store_field("outgoing_paid_message_star_count", outgoing_paid_message_star_count_);
    s.store_class_end();
  }
}

canSendMessageToUserResultUserIsDeleted::canSendMessageToUserResultUserIsDeleted() {
}

const std::int32_t canSendMessageToUserResultUserIsDeleted::ID;

void canSendMessageToUserResultUserIsDeleted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canSendMessageToUserResultUserIsDeleted");
    s.store_class_end();
  }
}

canSendMessageToUserResultUserRestrictsNewChats::canSendMessageToUserResultUserRestrictsNewChats() {
}

const std::int32_t canSendMessageToUserResultUserRestrictsNewChats::ID;

void canSendMessageToUserResultUserRestrictsNewChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "canSendMessageToUserResultUserRestrictsNewChats");
    s.store_class_end();
  }
}

chatBoostFeatures::chatBoostFeatures()
  : features_()
  , min_profile_background_custom_emoji_boost_level_()
  , min_background_custom_emoji_boost_level_()
  , min_emoji_status_boost_level_()
  , min_chat_theme_background_boost_level_()
  , min_custom_background_boost_level_()
  , min_custom_emoji_sticker_set_boost_level_()
  , min_automatic_translation_boost_level_()
  , min_speech_recognition_boost_level_()
  , min_sponsored_message_disable_boost_level_()
{}

chatBoostFeatures::chatBoostFeatures(array<object_ptr<chatBoostLevelFeatures>> &&features_, int32 min_profile_background_custom_emoji_boost_level_, int32 min_background_custom_emoji_boost_level_, int32 min_emoji_status_boost_level_, int32 min_chat_theme_background_boost_level_, int32 min_custom_background_boost_level_, int32 min_custom_emoji_sticker_set_boost_level_, int32 min_automatic_translation_boost_level_, int32 min_speech_recognition_boost_level_, int32 min_sponsored_message_disable_boost_level_)
  : features_(std::move(features_))
  , min_profile_background_custom_emoji_boost_level_(min_profile_background_custom_emoji_boost_level_)
  , min_background_custom_emoji_boost_level_(min_background_custom_emoji_boost_level_)
  , min_emoji_status_boost_level_(min_emoji_status_boost_level_)
  , min_chat_theme_background_boost_level_(min_chat_theme_background_boost_level_)
  , min_custom_background_boost_level_(min_custom_background_boost_level_)
  , min_custom_emoji_sticker_set_boost_level_(min_custom_emoji_sticker_set_boost_level_)
  , min_automatic_translation_boost_level_(min_automatic_translation_boost_level_)
  , min_speech_recognition_boost_level_(min_speech_recognition_boost_level_)
  , min_sponsored_message_disable_boost_level_(min_sponsored_message_disable_boost_level_)
{}

const std::int32_t chatBoostFeatures::ID;

void chatBoostFeatures::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatBoostFeatures");
    { s.store_vector_begin("features", features_.size()); for (const auto &_value : features_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("min_profile_background_custom_emoji_boost_level", min_profile_background_custom_emoji_boost_level_);
    s.store_field("min_background_custom_emoji_boost_level", min_background_custom_emoji_boost_level_);
    s.store_field("min_emoji_status_boost_level", min_emoji_status_boost_level_);
    s.store_field("min_chat_theme_background_boost_level", min_chat_theme_background_boost_level_);
    s.store_field("min_custom_background_boost_level", min_custom_background_boost_level_);
    s.store_field("min_custom_emoji_sticker_set_boost_level", min_custom_emoji_sticker_set_boost_level_);
    s.store_field("min_automatic_translation_boost_level", min_automatic_translation_boost_level_);
    s.store_field("min_speech_recognition_boost_level", min_speech_recognition_boost_level_);
    s.store_field("min_sponsored_message_disable_boost_level", min_sponsored_message_disable_boost_level_);
    s.store_class_end();
  }
}

chatBoostLevelFeatures::chatBoostLevelFeatures()
  : level_()
  , story_per_day_count_()
  , custom_emoji_reaction_count_()
  , title_color_count_()
  , profile_accent_color_count_()
  , can_set_profile_background_custom_emoji_()
  , accent_color_count_()
  , can_set_background_custom_emoji_()
  , can_set_emoji_status_()
  , chat_theme_background_count_()
  , can_set_custom_background_()
  , can_set_custom_emoji_sticker_set_()
  , can_enable_automatic_translation_()
  , can_recognize_speech_()
  , can_disable_sponsored_messages_()
{}

chatBoostLevelFeatures::chatBoostLevelFeatures(int32 level_, int32 story_per_day_count_, int32 custom_emoji_reaction_count_, int32 title_color_count_, int32 profile_accent_color_count_, bool can_set_profile_background_custom_emoji_, int32 accent_color_count_, bool can_set_background_custom_emoji_, bool can_set_emoji_status_, int32 chat_theme_background_count_, bool can_set_custom_background_, bool can_set_custom_emoji_sticker_set_, bool can_enable_automatic_translation_, bool can_recognize_speech_, bool can_disable_sponsored_messages_)
  : level_(level_)
  , story_per_day_count_(story_per_day_count_)
  , custom_emoji_reaction_count_(custom_emoji_reaction_count_)
  , title_color_count_(title_color_count_)
  , profile_accent_color_count_(profile_accent_color_count_)
  , can_set_profile_background_custom_emoji_(can_set_profile_background_custom_emoji_)
  , accent_color_count_(accent_color_count_)
  , can_set_background_custom_emoji_(can_set_background_custom_emoji_)
  , can_set_emoji_status_(can_set_emoji_status_)
  , chat_theme_background_count_(chat_theme_background_count_)
  , can_set_custom_background_(can_set_custom_background_)
  , can_set_custom_emoji_sticker_set_(can_set_custom_emoji_sticker_set_)
  , can_enable_automatic_translation_(can_enable_automatic_translation_)
  , can_recognize_speech_(can_recognize_speech_)
  , can_disable_sponsored_messages_(can_disable_sponsored_messages_)
{}

const std::int32_t chatBoostLevelFeatures::ID;

void chatBoostLevelFeatures::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatBoostLevelFeatures");
    s.store_field("level", level_);
    s.store_field("story_per_day_count", story_per_day_count_);
    s.store_field("custom_emoji_reaction_count", custom_emoji_reaction_count_);
    s.store_field("title_color_count", title_color_count_);
    s.store_field("profile_accent_color_count", profile_accent_color_count_);
    s.store_field("can_set_profile_background_custom_emoji", can_set_profile_background_custom_emoji_);
    s.store_field("accent_color_count", accent_color_count_);
    s.store_field("can_set_background_custom_emoji", can_set_background_custom_emoji_);
    s.store_field("can_set_emoji_status", can_set_emoji_status_);
    s.store_field("chat_theme_background_count", chat_theme_background_count_);
    s.store_field("can_set_custom_background", can_set_custom_background_);
    s.store_field("can_set_custom_emoji_sticker_set", can_set_custom_emoji_sticker_set_);
    s.store_field("can_enable_automatic_translation", can_enable_automatic_translation_);
    s.store_field("can_recognize_speech", can_recognize_speech_);
    s.store_field("can_disable_sponsored_messages", can_disable_sponsored_messages_);
    s.store_class_end();
  }
}

chatEvent::chatEvent()
  : id_()
  , date_()
  , member_id_()
  , action_()
{}

chatEvent::chatEvent(int64 id_, int32 date_, object_ptr<MessageSender> &&member_id_, object_ptr<ChatEventAction> &&action_)
  : id_(id_)
  , date_(date_)
  , member_id_(std::move(member_id_))
  , action_(std::move(action_))
{}

const std::int32_t chatEvent::ID;

void chatEvent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatEvent");
    s.store_field("id", id_);
    s.store_field("date", date_);
    s.store_object_field("member_id", static_cast<const BaseObject *>(member_id_.get()));
    s.store_object_field("action", static_cast<const BaseObject *>(action_.get()));
    s.store_class_end();
  }
}

chatInviteLinkCount::chatInviteLinkCount()
  : user_id_()
  , invite_link_count_()
  , revoked_invite_link_count_()
{}

chatInviteLinkCount::chatInviteLinkCount(int53 user_id_, int32 invite_link_count_, int32 revoked_invite_link_count_)
  : user_id_(user_id_)
  , invite_link_count_(invite_link_count_)
  , revoked_invite_link_count_(revoked_invite_link_count_)
{}

const std::int32_t chatInviteLinkCount::ID;

void chatInviteLinkCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatInviteLinkCount");
    s.store_field("user_id", user_id_);
    s.store_field("invite_link_count", invite_link_count_);
    s.store_field("revoked_invite_link_count", revoked_invite_link_count_);
    s.store_class_end();
  }
}

chatJoinRequests::chatJoinRequests()
  : total_count_()
  , requests_()
{}

chatJoinRequests::chatJoinRequests(int32 total_count_, array<object_ptr<chatJoinRequest>> &&requests_)
  : total_count_(total_count_)
  , requests_(std::move(requests_))
{}

const std::int32_t chatJoinRequests::ID;

void chatJoinRequests::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatJoinRequests");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("requests", requests_.size()); for (const auto &_value : requests_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatPhotoInfo::chatPhotoInfo()
  : small_()
  , big_()
  , minithumbnail_()
  , has_animation_()
  , is_personal_()
{}

chatPhotoInfo::chatPhotoInfo(object_ptr<file> &&small_, object_ptr<file> &&big_, object_ptr<minithumbnail> &&minithumbnail_, bool has_animation_, bool is_personal_)
  : small_(std::move(small_))
  , big_(std::move(big_))
  , minithumbnail_(std::move(minithumbnail_))
  , has_animation_(has_animation_)
  , is_personal_(is_personal_)
{}

const std::int32_t chatPhotoInfo::ID;

void chatPhotoInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatPhotoInfo");
    s.store_object_field("small", static_cast<const BaseObject *>(small_.get()));
    s.store_object_field("big", static_cast<const BaseObject *>(big_.get()));
    s.store_object_field("minithumbnail", static_cast<const BaseObject *>(minithumbnail_.get()));
    s.store_field("has_animation", has_animation_);
    s.store_field("is_personal", is_personal_);
    s.store_class_end();
  }
}

chatStatisticsInviterInfo::chatStatisticsInviterInfo()
  : user_id_()
  , added_member_count_()
{}

chatStatisticsInviterInfo::chatStatisticsInviterInfo(int53 user_id_, int32 added_member_count_)
  : user_id_(user_id_)
  , added_member_count_(added_member_count_)
{}

const std::int32_t chatStatisticsInviterInfo::ID;

void chatStatisticsInviterInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatStatisticsInviterInfo");
    s.store_field("user_id", user_id_);
    s.store_field("added_member_count", added_member_count_);
    s.store_class_end();
  }
}

contact::contact()
  : phone_number_()
  , first_name_()
  , last_name_()
  , vcard_()
  , user_id_()
{}

contact::contact(string const &phone_number_, string const &first_name_, string const &last_name_, string const &vcard_, int53 user_id_)
  : phone_number_(phone_number_)
  , first_name_(first_name_)
  , last_name_(last_name_)
  , vcard_(vcard_)
  , user_id_(user_id_)
{}

const std::int32_t contact::ID;

void contact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "contact");
    s.store_field("phone_number", phone_number_);
    s.store_field("first_name", first_name_);
    s.store_field("last_name", last_name_);
    s.store_field("vcard", vcard_);
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

countryInfo::countryInfo()
  : country_code_()
  , name_()
  , english_name_()
  , is_hidden_()
  , calling_codes_()
{}

countryInfo::countryInfo(string const &country_code_, string const &name_, string const &english_name_, bool is_hidden_, array<string> &&calling_codes_)
  : country_code_(country_code_)
  , name_(name_)
  , english_name_(english_name_)
  , is_hidden_(is_hidden_)
  , calling_codes_(std::move(calling_codes_))
{}

const std::int32_t countryInfo::ID;

void countryInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "countryInfo");
    s.store_field("country_code", country_code_);
    s.store_field("name", name_);
    s.store_field("english_name", english_name_);
    s.store_field("is_hidden", is_hidden_);
    { s.store_vector_begin("calling_codes", calling_codes_.size()); for (const auto &_value : calling_codes_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

directMessagesChatTopic::directMessagesChatTopic()
  : chat_id_()
  , id_()
  , sender_id_()
  , order_()
  , can_send_unpaid_messages_()
  , is_marked_as_unread_()
  , unread_count_()
  , last_read_inbox_message_id_()
  , last_read_outbox_message_id_()
  , unread_reaction_count_()
  , last_message_()
  , draft_message_()
{}

directMessagesChatTopic::directMessagesChatTopic(int53 chat_id_, int53 id_, object_ptr<MessageSender> &&sender_id_, int64 order_, bool can_send_unpaid_messages_, bool is_marked_as_unread_, int53 unread_count_, int53 last_read_inbox_message_id_, int53 last_read_outbox_message_id_, int53 unread_reaction_count_, object_ptr<message> &&last_message_, object_ptr<draftMessage> &&draft_message_)
  : chat_id_(chat_id_)
  , id_(id_)
  , sender_id_(std::move(sender_id_))
  , order_(order_)
  , can_send_unpaid_messages_(can_send_unpaid_messages_)
  , is_marked_as_unread_(is_marked_as_unread_)
  , unread_count_(unread_count_)
  , last_read_inbox_message_id_(last_read_inbox_message_id_)
  , last_read_outbox_message_id_(last_read_outbox_message_id_)
  , unread_reaction_count_(unread_reaction_count_)
  , last_message_(std::move(last_message_))
  , draft_message_(std::move(draft_message_))
{}

const std::int32_t directMessagesChatTopic::ID;

void directMessagesChatTopic::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "directMessagesChatTopic");
    s.store_field("chat_id", chat_id_);
    s.store_field("id", id_);
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_field("order", order_);
    s.store_field("can_send_unpaid_messages", can_send_unpaid_messages_);
    s.store_field("is_marked_as_unread", is_marked_as_unread_);
    s.store_field("unread_count", unread_count_);
    s.store_field("last_read_inbox_message_id", last_read_inbox_message_id_);
    s.store_field("last_read_outbox_message_id", last_read_outbox_message_id_);
    s.store_field("unread_reaction_count", unread_reaction_count_);
    s.store_object_field("last_message", static_cast<const BaseObject *>(last_message_.get()));
    s.store_object_field("draft_message", static_cast<const BaseObject *>(draft_message_.get()));
    s.store_class_end();
  }
}

emojiCategory::emojiCategory()
  : name_()
  , icon_()
  , source_()
  , is_greeting_()
{}

emojiCategory::emojiCategory(string const &name_, object_ptr<sticker> &&icon_, object_ptr<EmojiCategorySource> &&source_, bool is_greeting_)
  : name_(name_)
  , icon_(std::move(icon_))
  , source_(std::move(source_))
  , is_greeting_(is_greeting_)
{}

const std::int32_t emojiCategory::ID;

void emojiCategory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiCategory");
    s.store_field("name", name_);
    s.store_object_field("icon", static_cast<const BaseObject *>(icon_.get()));
    s.store_object_field("source", static_cast<const BaseObject *>(source_.get()));
    s.store_field("is_greeting", is_greeting_);
    s.store_class_end();
  }
}

emojiCategoryTypeDefault::emojiCategoryTypeDefault() {
}

const std::int32_t emojiCategoryTypeDefault::ID;

void emojiCategoryTypeDefault::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiCategoryTypeDefault");
    s.store_class_end();
  }
}

emojiCategoryTypeRegularStickers::emojiCategoryTypeRegularStickers() {
}

const std::int32_t emojiCategoryTypeRegularStickers::ID;

void emojiCategoryTypeRegularStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiCategoryTypeRegularStickers");
    s.store_class_end();
  }
}

emojiCategoryTypeEmojiStatus::emojiCategoryTypeEmojiStatus() {
}

const std::int32_t emojiCategoryTypeEmojiStatus::ID;

void emojiCategoryTypeEmojiStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiCategoryTypeEmojiStatus");
    s.store_class_end();
  }
}

emojiCategoryTypeChatPhoto::emojiCategoryTypeChatPhoto() {
}

const std::int32_t emojiCategoryTypeChatPhoto::ID;

void emojiCategoryTypeChatPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiCategoryTypeChatPhoto");
    s.store_class_end();
  }
}

encryptedCredentials::encryptedCredentials()
  : data_()
  , hash_()
  , secret_()
{}

encryptedCredentials::encryptedCredentials(bytes const &data_, bytes const &hash_, bytes const &secret_)
  : data_(std::move(data_))
  , hash_(std::move(hash_))
  , secret_(std::move(secret_))
{}

const std::int32_t encryptedCredentials::ID;

void encryptedCredentials::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "encryptedCredentials");
    s.store_bytes_field("data", data_);
    s.store_bytes_field("hash", hash_);
    s.store_bytes_field("secret", secret_);
    s.store_class_end();
  }
}

forumTopicInfo::forumTopicInfo()
  : chat_id_()
  , forum_topic_id_()
  , name_()
  , icon_()
  , creation_date_()
  , creator_id_()
  , is_general_()
  , is_outgoing_()
  , is_closed_()
  , is_hidden_()
  , is_name_implicit_()
{}

forumTopicInfo::forumTopicInfo(int53 chat_id_, int32 forum_topic_id_, string const &name_, object_ptr<forumTopicIcon> &&icon_, int32 creation_date_, object_ptr<MessageSender> &&creator_id_, bool is_general_, bool is_outgoing_, bool is_closed_, bool is_hidden_, bool is_name_implicit_)
  : chat_id_(chat_id_)
  , forum_topic_id_(forum_topic_id_)
  , name_(name_)
  , icon_(std::move(icon_))
  , creation_date_(creation_date_)
  , creator_id_(std::move(creator_id_))
  , is_general_(is_general_)
  , is_outgoing_(is_outgoing_)
  , is_closed_(is_closed_)
  , is_hidden_(is_hidden_)
  , is_name_implicit_(is_name_implicit_)
{}

const std::int32_t forumTopicInfo::ID;

void forumTopicInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "forumTopicInfo");
    s.store_field("chat_id", chat_id_);
    s.store_field("forum_topic_id", forum_topic_id_);
    s.store_field("name", name_);
    s.store_object_field("icon", static_cast<const BaseObject *>(icon_.get()));
    s.store_field("creation_date", creation_date_);
    s.store_object_field("creator_id", static_cast<const BaseObject *>(creator_id_.get()));
    s.store_field("is_general", is_general_);
    s.store_field("is_outgoing", is_outgoing_);
    s.store_field("is_closed", is_closed_);
    s.store_field("is_hidden", is_hidden_);
    s.store_field("is_name_implicit", is_name_implicit_);
    s.store_class_end();
  }
}

foundAffiliatePrograms::foundAffiliatePrograms()
  : total_count_()
  , programs_()
  , next_offset_()
{}

foundAffiliatePrograms::foundAffiliatePrograms(int32 total_count_, array<object_ptr<foundAffiliateProgram>> &&programs_, string const &next_offset_)
  : total_count_(total_count_)
  , programs_(std::move(programs_))
  , next_offset_(next_offset_)
{}

const std::int32_t foundAffiliatePrograms::ID;

void foundAffiliatePrograms::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "foundAffiliatePrograms");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("programs", programs_.size()); for (const auto &_value : programs_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

foundPublicPosts::foundPublicPosts()
  : messages_()
  , next_offset_()
  , search_limits_()
  , are_limits_exceeded_()
{}

foundPublicPosts::foundPublicPosts(array<object_ptr<message>> &&messages_, string const &next_offset_, object_ptr<publicPostSearchLimits> &&search_limits_, bool are_limits_exceeded_)
  : messages_(std::move(messages_))
  , next_offset_(next_offset_)
  , search_limits_(std::move(search_limits_))
  , are_limits_exceeded_(are_limits_exceeded_)
{}

const std::int32_t foundPublicPosts::ID;

void foundPublicPosts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "foundPublicPosts");
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("next_offset", next_offset_);
    s.store_object_field("search_limits", static_cast<const BaseObject *>(search_limits_.get()));
    s.store_field("are_limits_exceeded", are_limits_exceeded_);
    s.store_class_end();
  }
}

foundUsers::foundUsers()
  : user_ids_()
  , next_offset_()
{}

foundUsers::foundUsers(array<int53> &&user_ids_, string const &next_offset_)
  : user_ids_(std::move(user_ids_))
  , next_offset_(next_offset_)
{}

const std::int32_t foundUsers::ID;

void foundUsers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "foundUsers");
    { s.store_vector_begin("user_ids", user_ids_.size()); for (const auto &_value : user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

gameHighScore::gameHighScore()
  : position_()
  , user_id_()
  , score_()
{}

gameHighScore::gameHighScore(int32 position_, int53 user_id_, int32 score_)
  : position_(position_)
  , user_id_(user_id_)
  , score_(score_)
{}

const std::int32_t gameHighScore::ID;

void gameHighScore::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "gameHighScore");
    s.store_field("position", position_);
    s.store_field("user_id", user_id_);
    s.store_field("score", score_);
    s.store_class_end();
  }
}

giftsForResale::giftsForResale()
  : total_count_()
  , gifts_()
  , models_()
  , symbols_()
  , backdrops_()
  , next_offset_()
{}

giftsForResale::giftsForResale(int32 total_count_, array<object_ptr<giftForResale>> &&gifts_, array<object_ptr<upgradedGiftModelCount>> &&models_, array<object_ptr<upgradedGiftSymbolCount>> &&symbols_, array<object_ptr<upgradedGiftBackdropCount>> &&backdrops_, string const &next_offset_)
  : total_count_(total_count_)
  , gifts_(std::move(gifts_))
  , models_(std::move(models_))
  , symbols_(std::move(symbols_))
  , backdrops_(std::move(backdrops_))
  , next_offset_(next_offset_)
{}

const std::int32_t giftsForResale::ID;

void giftsForResale::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giftsForResale");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("gifts", gifts_.size()); for (const auto &_value : gifts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("models", models_.size()); for (const auto &_value : models_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("symbols", symbols_.size()); for (const auto &_value : symbols_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("backdrops", backdrops_.size()); for (const auto &_value : backdrops_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

inputBackgroundLocal::inputBackgroundLocal()
  : background_()
{}

inputBackgroundLocal::inputBackgroundLocal(object_ptr<InputFile> &&background_)
  : background_(std::move(background_))
{}

const std::int32_t inputBackgroundLocal::ID;

void inputBackgroundLocal::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBackgroundLocal");
    s.store_object_field("background", static_cast<const BaseObject *>(background_.get()));
    s.store_class_end();
  }
}

inputBackgroundRemote::inputBackgroundRemote()
  : background_id_()
{}

inputBackgroundRemote::inputBackgroundRemote(int64 background_id_)
  : background_id_(background_id_)
{}

const std::int32_t inputBackgroundRemote::ID;

void inputBackgroundRemote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBackgroundRemote");
    s.store_field("background_id", background_id_);
    s.store_class_end();
  }
}

inputBackgroundPrevious::inputBackgroundPrevious()
  : message_id_()
{}

inputBackgroundPrevious::inputBackgroundPrevious(int53 message_id_)
  : message_id_(message_id_)
{}

const std::int32_t inputBackgroundPrevious::ID;

void inputBackgroundPrevious::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBackgroundPrevious");
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

inputBusinessChatLink::inputBusinessChatLink()
  : text_()
  , title_()
{}

inputBusinessChatLink::inputBusinessChatLink(object_ptr<formattedText> &&text_, string const &title_)
  : text_(std::move(text_))
  , title_(title_)
{}

const std::int32_t inputBusinessChatLink::ID;

void inputBusinessChatLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputBusinessChatLink");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("title", title_);
    s.store_class_end();
  }
}

inputMessageText::inputMessageText()
  : text_()
  , link_preview_options_()
  , clear_draft_()
{}

inputMessageText::inputMessageText(object_ptr<formattedText> &&text_, object_ptr<linkPreviewOptions> &&link_preview_options_, bool clear_draft_)
  : text_(std::move(text_))
  , link_preview_options_(std::move(link_preview_options_))
  , clear_draft_(clear_draft_)
{}

const std::int32_t inputMessageText::ID;

void inputMessageText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageText");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_object_field("link_preview_options", static_cast<const BaseObject *>(link_preview_options_.get()));
    s.store_field("clear_draft", clear_draft_);
    s.store_class_end();
  }
}

inputMessageAnimation::inputMessageAnimation()
  : animation_()
  , thumbnail_()
  , added_sticker_file_ids_()
  , duration_()
  , width_()
  , height_()
  , caption_()
  , show_caption_above_media_()
  , has_spoiler_()
{}

inputMessageAnimation::inputMessageAnimation(object_ptr<InputFile> &&animation_, object_ptr<inputThumbnail> &&thumbnail_, array<int32> &&added_sticker_file_ids_, int32 duration_, int32 width_, int32 height_, object_ptr<formattedText> &&caption_, bool show_caption_above_media_, bool has_spoiler_)
  : animation_(std::move(animation_))
  , thumbnail_(std::move(thumbnail_))
  , added_sticker_file_ids_(std::move(added_sticker_file_ids_))
  , duration_(duration_)
  , width_(width_)
  , height_(height_)
  , caption_(std::move(caption_))
  , show_caption_above_media_(show_caption_above_media_)
  , has_spoiler_(has_spoiler_)
{}

const std::int32_t inputMessageAnimation::ID;

void inputMessageAnimation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageAnimation");
    s.store_object_field("animation", static_cast<const BaseObject *>(animation_.get()));
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    { s.store_vector_begin("added_sticker_file_ids", added_sticker_file_ids_.size()); for (const auto &_value : added_sticker_file_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("duration", duration_);
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_field("show_caption_above_media", show_caption_above_media_);
    s.store_field("has_spoiler", has_spoiler_);
    s.store_class_end();
  }
}

inputMessageAudio::inputMessageAudio()
  : audio_()
  , album_cover_thumbnail_()
  , duration_()
  , title_()
  , performer_()
  , caption_()
{}

inputMessageAudio::inputMessageAudio(object_ptr<InputFile> &&audio_, object_ptr<inputThumbnail> &&album_cover_thumbnail_, int32 duration_, string const &title_, string const &performer_, object_ptr<formattedText> &&caption_)
  : audio_(std::move(audio_))
  , album_cover_thumbnail_(std::move(album_cover_thumbnail_))
  , duration_(duration_)
  , title_(title_)
  , performer_(performer_)
  , caption_(std::move(caption_))
{}

const std::int32_t inputMessageAudio::ID;

void inputMessageAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageAudio");
    s.store_object_field("audio", static_cast<const BaseObject *>(audio_.get()));
    s.store_object_field("album_cover_thumbnail", static_cast<const BaseObject *>(album_cover_thumbnail_.get()));
    s.store_field("duration", duration_);
    s.store_field("title", title_);
    s.store_field("performer", performer_);
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

inputMessageDocument::inputMessageDocument()
  : document_()
  , thumbnail_()
  , disable_content_type_detection_()
  , caption_()
{}

inputMessageDocument::inputMessageDocument(object_ptr<InputFile> &&document_, object_ptr<inputThumbnail> &&thumbnail_, bool disable_content_type_detection_, object_ptr<formattedText> &&caption_)
  : document_(std::move(document_))
  , thumbnail_(std::move(thumbnail_))
  , disable_content_type_detection_(disable_content_type_detection_)
  , caption_(std::move(caption_))
{}

const std::int32_t inputMessageDocument::ID;

void inputMessageDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageDocument");
    s.store_object_field("document", static_cast<const BaseObject *>(document_.get()));
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_field("disable_content_type_detection", disable_content_type_detection_);
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

inputMessagePaidMedia::inputMessagePaidMedia()
  : star_count_()
  , paid_media_()
  , caption_()
  , show_caption_above_media_()
  , payload_()
{}

inputMessagePaidMedia::inputMessagePaidMedia(int53 star_count_, array<object_ptr<inputPaidMedia>> &&paid_media_, object_ptr<formattedText> &&caption_, bool show_caption_above_media_, string const &payload_)
  : star_count_(star_count_)
  , paid_media_(std::move(paid_media_))
  , caption_(std::move(caption_))
  , show_caption_above_media_(show_caption_above_media_)
  , payload_(payload_)
{}

const std::int32_t inputMessagePaidMedia::ID;

void inputMessagePaidMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagePaidMedia");
    s.store_field("star_count", star_count_);
    { s.store_vector_begin("paid_media", paid_media_.size()); for (const auto &_value : paid_media_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_field("show_caption_above_media", show_caption_above_media_);
    s.store_field("payload", payload_);
    s.store_class_end();
  }
}

inputMessagePhoto::inputMessagePhoto()
  : photo_()
  , thumbnail_()
  , added_sticker_file_ids_()
  , width_()
  , height_()
  , caption_()
  , show_caption_above_media_()
  , self_destruct_type_()
  , has_spoiler_()
{}

inputMessagePhoto::inputMessagePhoto(object_ptr<InputFile> &&photo_, object_ptr<inputThumbnail> &&thumbnail_, array<int32> &&added_sticker_file_ids_, int32 width_, int32 height_, object_ptr<formattedText> &&caption_, bool show_caption_above_media_, object_ptr<MessageSelfDestructType> &&self_destruct_type_, bool has_spoiler_)
  : photo_(std::move(photo_))
  , thumbnail_(std::move(thumbnail_))
  , added_sticker_file_ids_(std::move(added_sticker_file_ids_))
  , width_(width_)
  , height_(height_)
  , caption_(std::move(caption_))
  , show_caption_above_media_(show_caption_above_media_)
  , self_destruct_type_(std::move(self_destruct_type_))
  , has_spoiler_(has_spoiler_)
{}

const std::int32_t inputMessagePhoto::ID;

void inputMessagePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagePhoto");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    { s.store_vector_begin("added_sticker_file_ids", added_sticker_file_ids_.size()); for (const auto &_value : added_sticker_file_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_field("show_caption_above_media", show_caption_above_media_);
    s.store_object_field("self_destruct_type", static_cast<const BaseObject *>(self_destruct_type_.get()));
    s.store_field("has_spoiler", has_spoiler_);
    s.store_class_end();
  }
}

inputMessageSticker::inputMessageSticker()
  : sticker_()
  , thumbnail_()
  , width_()
  , height_()
  , emoji_()
{}

inputMessageSticker::inputMessageSticker(object_ptr<InputFile> &&sticker_, object_ptr<inputThumbnail> &&thumbnail_, int32 width_, int32 height_, string const &emoji_)
  : sticker_(std::move(sticker_))
  , thumbnail_(std::move(thumbnail_))
  , width_(width_)
  , height_(height_)
  , emoji_(emoji_)
{}

const std::int32_t inputMessageSticker::ID;

void inputMessageSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageSticker");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_field("emoji", emoji_);
    s.store_class_end();
  }
}

inputMessageVideo::inputMessageVideo()
  : video_()
  , thumbnail_()
  , cover_()
  , start_timestamp_()
  , added_sticker_file_ids_()
  , duration_()
  , width_()
  , height_()
  , supports_streaming_()
  , caption_()
  , show_caption_above_media_()
  , self_destruct_type_()
  , has_spoiler_()
{}

inputMessageVideo::inputMessageVideo(object_ptr<InputFile> &&video_, object_ptr<inputThumbnail> &&thumbnail_, object_ptr<InputFile> &&cover_, int32 start_timestamp_, array<int32> &&added_sticker_file_ids_, int32 duration_, int32 width_, int32 height_, bool supports_streaming_, object_ptr<formattedText> &&caption_, bool show_caption_above_media_, object_ptr<MessageSelfDestructType> &&self_destruct_type_, bool has_spoiler_)
  : video_(std::move(video_))
  , thumbnail_(std::move(thumbnail_))
  , cover_(std::move(cover_))
  , start_timestamp_(start_timestamp_)
  , added_sticker_file_ids_(std::move(added_sticker_file_ids_))
  , duration_(duration_)
  , width_(width_)
  , height_(height_)
  , supports_streaming_(supports_streaming_)
  , caption_(std::move(caption_))
  , show_caption_above_media_(show_caption_above_media_)
  , self_destruct_type_(std::move(self_destruct_type_))
  , has_spoiler_(has_spoiler_)
{}

const std::int32_t inputMessageVideo::ID;

void inputMessageVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageVideo");
    s.store_object_field("video", static_cast<const BaseObject *>(video_.get()));
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_object_field("cover", static_cast<const BaseObject *>(cover_.get()));
    s.store_field("start_timestamp", start_timestamp_);
    { s.store_vector_begin("added_sticker_file_ids", added_sticker_file_ids_.size()); for (const auto &_value : added_sticker_file_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("duration", duration_);
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_field("supports_streaming", supports_streaming_);
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_field("show_caption_above_media", show_caption_above_media_);
    s.store_object_field("self_destruct_type", static_cast<const BaseObject *>(self_destruct_type_.get()));
    s.store_field("has_spoiler", has_spoiler_);
    s.store_class_end();
  }
}

inputMessageVideoNote::inputMessageVideoNote()
  : video_note_()
  , thumbnail_()
  , duration_()
  , length_()
  , self_destruct_type_()
{}

inputMessageVideoNote::inputMessageVideoNote(object_ptr<InputFile> &&video_note_, object_ptr<inputThumbnail> &&thumbnail_, int32 duration_, int32 length_, object_ptr<MessageSelfDestructType> &&self_destruct_type_)
  : video_note_(std::move(video_note_))
  , thumbnail_(std::move(thumbnail_))
  , duration_(duration_)
  , length_(length_)
  , self_destruct_type_(std::move(self_destruct_type_))
{}

const std::int32_t inputMessageVideoNote::ID;

void inputMessageVideoNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageVideoNote");
    s.store_object_field("video_note", static_cast<const BaseObject *>(video_note_.get()));
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_field("duration", duration_);
    s.store_field("length", length_);
    s.store_object_field("self_destruct_type", static_cast<const BaseObject *>(self_destruct_type_.get()));
    s.store_class_end();
  }
}

inputMessageVoiceNote::inputMessageVoiceNote()
  : voice_note_()
  , duration_()
  , waveform_()
  , caption_()
  , self_destruct_type_()
{}

inputMessageVoiceNote::inputMessageVoiceNote(object_ptr<InputFile> &&voice_note_, int32 duration_, bytes const &waveform_, object_ptr<formattedText> &&caption_, object_ptr<MessageSelfDestructType> &&self_destruct_type_)
  : voice_note_(std::move(voice_note_))
  , duration_(duration_)
  , waveform_(std::move(waveform_))
  , caption_(std::move(caption_))
  , self_destruct_type_(std::move(self_destruct_type_))
{}

const std::int32_t inputMessageVoiceNote::ID;

void inputMessageVoiceNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageVoiceNote");
    s.store_object_field("voice_note", static_cast<const BaseObject *>(voice_note_.get()));
    s.store_field("duration", duration_);
    s.store_bytes_field("waveform", waveform_);
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_object_field("self_destruct_type", static_cast<const BaseObject *>(self_destruct_type_.get()));
    s.store_class_end();
  }
}

inputMessageLocation::inputMessageLocation()
  : location_()
  , live_period_()
  , heading_()
  , proximity_alert_radius_()
{}

inputMessageLocation::inputMessageLocation(object_ptr<location> &&location_, int32 live_period_, int32 heading_, int32 proximity_alert_radius_)
  : location_(std::move(location_))
  , live_period_(live_period_)
  , heading_(heading_)
  , proximity_alert_radius_(proximity_alert_radius_)
{}

const std::int32_t inputMessageLocation::ID;

void inputMessageLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageLocation");
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_field("live_period", live_period_);
    s.store_field("heading", heading_);
    s.store_field("proximity_alert_radius", proximity_alert_radius_);
    s.store_class_end();
  }
}

inputMessageVenue::inputMessageVenue()
  : venue_()
{}

inputMessageVenue::inputMessageVenue(object_ptr<venue> &&venue_)
  : venue_(std::move(venue_))
{}

const std::int32_t inputMessageVenue::ID;

void inputMessageVenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageVenue");
    s.store_object_field("venue", static_cast<const BaseObject *>(venue_.get()));
    s.store_class_end();
  }
}

inputMessageContact::inputMessageContact()
  : contact_()
{}

inputMessageContact::inputMessageContact(object_ptr<contact> &&contact_)
  : contact_(std::move(contact_))
{}

const std::int32_t inputMessageContact::ID;

void inputMessageContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageContact");
    s.store_object_field("contact", static_cast<const BaseObject *>(contact_.get()));
    s.store_class_end();
  }
}

inputMessageDice::inputMessageDice()
  : emoji_()
  , clear_draft_()
{}

inputMessageDice::inputMessageDice(string const &emoji_, bool clear_draft_)
  : emoji_(emoji_)
  , clear_draft_(clear_draft_)
{}

const std::int32_t inputMessageDice::ID;

void inputMessageDice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageDice");
    s.store_field("emoji", emoji_);
    s.store_field("clear_draft", clear_draft_);
    s.store_class_end();
  }
}

inputMessageGame::inputMessageGame()
  : bot_user_id_()
  , game_short_name_()
{}

inputMessageGame::inputMessageGame(int53 bot_user_id_, string const &game_short_name_)
  : bot_user_id_(bot_user_id_)
  , game_short_name_(game_short_name_)
{}

const std::int32_t inputMessageGame::ID;

void inputMessageGame::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageGame");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("game_short_name", game_short_name_);
    s.store_class_end();
  }
}

inputMessageInvoice::inputMessageInvoice()
  : invoice_()
  , title_()
  , description_()
  , photo_url_()
  , photo_size_()
  , photo_width_()
  , photo_height_()
  , payload_()
  , provider_token_()
  , provider_data_()
  , start_parameter_()
  , paid_media_()
  , paid_media_caption_()
{}

inputMessageInvoice::inputMessageInvoice(object_ptr<invoice> &&invoice_, string const &title_, string const &description_, string const &photo_url_, int32 photo_size_, int32 photo_width_, int32 photo_height_, bytes const &payload_, string const &provider_token_, string const &provider_data_, string const &start_parameter_, object_ptr<inputPaidMedia> &&paid_media_, object_ptr<formattedText> &&paid_media_caption_)
  : invoice_(std::move(invoice_))
  , title_(title_)
  , description_(description_)
  , photo_url_(photo_url_)
  , photo_size_(photo_size_)
  , photo_width_(photo_width_)
  , photo_height_(photo_height_)
  , payload_(std::move(payload_))
  , provider_token_(provider_token_)
  , provider_data_(provider_data_)
  , start_parameter_(start_parameter_)
  , paid_media_(std::move(paid_media_))
  , paid_media_caption_(std::move(paid_media_caption_))
{}

const std::int32_t inputMessageInvoice::ID;

void inputMessageInvoice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageInvoice");
    s.store_object_field("invoice", static_cast<const BaseObject *>(invoice_.get()));
    s.store_field("title", title_);
    s.store_field("description", description_);
    s.store_field("photo_url", photo_url_);
    s.store_field("photo_size", photo_size_);
    s.store_field("photo_width", photo_width_);
    s.store_field("photo_height", photo_height_);
    s.store_bytes_field("payload", payload_);
    s.store_field("provider_token", provider_token_);
    s.store_field("provider_data", provider_data_);
    s.store_field("start_parameter", start_parameter_);
    s.store_object_field("paid_media", static_cast<const BaseObject *>(paid_media_.get()));
    s.store_object_field("paid_media_caption", static_cast<const BaseObject *>(paid_media_caption_.get()));
    s.store_class_end();
  }
}

inputMessagePoll::inputMessagePoll()
  : question_()
  , options_()
  , is_anonymous_()
  , type_()
  , open_period_()
  , close_date_()
  , is_closed_()
{}

inputMessagePoll::inputMessagePoll(object_ptr<formattedText> &&question_, array<object_ptr<formattedText>> &&options_, bool is_anonymous_, object_ptr<PollType> &&type_, int32 open_period_, int32 close_date_, bool is_closed_)
  : question_(std::move(question_))
  , options_(std::move(options_))
  , is_anonymous_(is_anonymous_)
  , type_(std::move(type_))
  , open_period_(open_period_)
  , close_date_(close_date_)
  , is_closed_(is_closed_)
{}

const std::int32_t inputMessagePoll::ID;

void inputMessagePoll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessagePoll");
    s.store_object_field("question", static_cast<const BaseObject *>(question_.get()));
    { s.store_vector_begin("options", options_.size()); for (const auto &_value : options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("is_anonymous", is_anonymous_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("open_period", open_period_);
    s.store_field("close_date", close_date_);
    s.store_field("is_closed", is_closed_);
    s.store_class_end();
  }
}

inputMessageStory::inputMessageStory()
  : story_poster_chat_id_()
  , story_id_()
{}

inputMessageStory::inputMessageStory(int53 story_poster_chat_id_, int32 story_id_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , story_id_(story_id_)
{}

const std::int32_t inputMessageStory::ID;

void inputMessageStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageStory");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("story_id", story_id_);
    s.store_class_end();
  }
}

inputMessageChecklist::inputMessageChecklist()
  : checklist_()
{}

inputMessageChecklist::inputMessageChecklist(object_ptr<inputChecklist> &&checklist_)
  : checklist_(std::move(checklist_))
{}

const std::int32_t inputMessageChecklist::ID;

void inputMessageChecklist::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageChecklist");
    s.store_object_field("checklist", static_cast<const BaseObject *>(checklist_.get()));
    s.store_class_end();
  }
}

inputMessageForwarded::inputMessageForwarded()
  : from_chat_id_()
  , message_id_()
  , in_game_share_()
  , replace_video_start_timestamp_()
  , new_video_start_timestamp_()
  , copy_options_()
{}

inputMessageForwarded::inputMessageForwarded(int53 from_chat_id_, int53 message_id_, bool in_game_share_, bool replace_video_start_timestamp_, int32 new_video_start_timestamp_, object_ptr<messageCopyOptions> &&copy_options_)
  : from_chat_id_(from_chat_id_)
  , message_id_(message_id_)
  , in_game_share_(in_game_share_)
  , replace_video_start_timestamp_(replace_video_start_timestamp_)
  , new_video_start_timestamp_(new_video_start_timestamp_)
  , copy_options_(std::move(copy_options_))
{}

const std::int32_t inputMessageForwarded::ID;

void inputMessageForwarded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputMessageForwarded");
    s.store_field("from_chat_id", from_chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("in_game_share", in_game_share_);
    s.store_field("replace_video_start_timestamp", replace_video_start_timestamp_);
    s.store_field("new_video_start_timestamp", new_video_start_timestamp_);
    s.store_object_field("copy_options", static_cast<const BaseObject *>(copy_options_.get()));
    s.store_class_end();
  }
}

inputPaidMedia::inputPaidMedia()
  : type_()
  , media_()
  , thumbnail_()
  , added_sticker_file_ids_()
  , width_()
  , height_()
{}

inputPaidMedia::inputPaidMedia(object_ptr<InputPaidMediaType> &&type_, object_ptr<InputFile> &&media_, object_ptr<inputThumbnail> &&thumbnail_, array<int32> &&added_sticker_file_ids_, int32 width_, int32 height_)
  : type_(std::move(type_))
  , media_(std::move(media_))
  , thumbnail_(std::move(thumbnail_))
  , added_sticker_file_ids_(std::move(added_sticker_file_ids_))
  , width_(width_)
  , height_(height_)
{}

const std::int32_t inputPaidMedia::ID;

void inputPaidMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPaidMedia");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_object_field("media", static_cast<const BaseObject *>(media_.get()));
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    { s.store_vector_begin("added_sticker_file_ids", added_sticker_file_ids_.size()); for (const auto &_value : added_sticker_file_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_class_end();
  }
}

inputPaidMediaTypePhoto::inputPaidMediaTypePhoto() {
}

const std::int32_t inputPaidMediaTypePhoto::ID;

void inputPaidMediaTypePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPaidMediaTypePhoto");
    s.store_class_end();
  }
}

inputPaidMediaTypeVideo::inputPaidMediaTypeVideo()
  : cover_()
  , start_timestamp_()
  , duration_()
  , supports_streaming_()
{}

inputPaidMediaTypeVideo::inputPaidMediaTypeVideo(object_ptr<InputFile> &&cover_, int32 start_timestamp_, int32 duration_, bool supports_streaming_)
  : cover_(std::move(cover_))
  , start_timestamp_(start_timestamp_)
  , duration_(duration_)
  , supports_streaming_(supports_streaming_)
{}

const std::int32_t inputPaidMediaTypeVideo::ID;

void inputPaidMediaTypeVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPaidMediaTypeVideo");
    s.store_object_field("cover", static_cast<const BaseObject *>(cover_.get()));
    s.store_field("start_timestamp", start_timestamp_);
    s.store_field("duration", duration_);
    s.store_field("supports_streaming", supports_streaming_);
    s.store_class_end();
  }
}

inputPassportElementError::inputPassportElementError()
  : type_()
  , message_()
  , source_()
{}

inputPassportElementError::inputPassportElementError(object_ptr<PassportElementType> &&type_, string const &message_, object_ptr<InputPassportElementErrorSource> &&source_)
  : type_(std::move(type_))
  , message_(message_)
  , source_(std::move(source_))
{}

const std::int32_t inputPassportElementError::ID;

void inputPassportElementError::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputPassportElementError");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("message", message_);
    s.store_object_field("source", static_cast<const BaseObject *>(source_.get()));
    s.store_class_end();
  }
}

inputStoryArea::inputStoryArea()
  : position_()
  , type_()
{}

inputStoryArea::inputStoryArea(object_ptr<storyAreaPosition> &&position_, object_ptr<InputStoryAreaType> &&type_)
  : position_(std::move(position_))
  , type_(std::move(type_))
{}

const std::int32_t inputStoryArea::ID;

void inputStoryArea::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputStoryArea");
    s.store_object_field("position", static_cast<const BaseObject *>(position_.get()));
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

inputThumbnail::inputThumbnail()
  : thumbnail_()
  , width_()
  , height_()
{}

inputThumbnail::inputThumbnail(object_ptr<InputFile> &&thumbnail_, int32 width_, int32 height_)
  : thumbnail_(std::move(thumbnail_))
  , width_(width_)
  , height_(height_)
{}

const std::int32_t inputThumbnail::ID;

void inputThumbnail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputThumbnail");
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_class_end();
  }
}

languagePackString::languagePackString()
  : key_()
  , value_()
{}

languagePackString::languagePackString(string const &key_, object_ptr<LanguagePackStringValue> &&value_)
  : key_(key_)
  , value_(std::move(value_))
{}

const std::int32_t languagePackString::ID;

void languagePackString::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "languagePackString");
    s.store_field("key", key_);
    s.store_object_field("value", static_cast<const BaseObject *>(value_.get()));
    s.store_class_end();
  }
}

linkPreview::linkPreview()
  : url_()
  , display_url_()
  , site_name_()
  , title_()
  , description_()
  , author_()
  , type_()
  , has_large_media_()
  , show_large_media_()
  , show_media_above_description_()
  , skip_confirmation_()
  , show_above_text_()
  , instant_view_version_()
{}

linkPreview::linkPreview(string const &url_, string const &display_url_, string const &site_name_, string const &title_, object_ptr<formattedText> &&description_, string const &author_, object_ptr<LinkPreviewType> &&type_, bool has_large_media_, bool show_large_media_, bool show_media_above_description_, bool skip_confirmation_, bool show_above_text_, int32 instant_view_version_)
  : url_(url_)
  , display_url_(display_url_)
  , site_name_(site_name_)
  , title_(title_)
  , description_(std::move(description_))
  , author_(author_)
  , type_(std::move(type_))
  , has_large_media_(has_large_media_)
  , show_large_media_(show_large_media_)
  , show_media_above_description_(show_media_above_description_)
  , skip_confirmation_(skip_confirmation_)
  , show_above_text_(show_above_text_)
  , instant_view_version_(instant_view_version_)
{}

const std::int32_t linkPreview::ID;

void linkPreview::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreview");
    s.store_field("url", url_);
    s.store_field("display_url", display_url_);
    s.store_field("site_name", site_name_);
    s.store_field("title", title_);
    s.store_object_field("description", static_cast<const BaseObject *>(description_.get()));
    s.store_field("author", author_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("has_large_media", has_large_media_);
    s.store_field("show_large_media", show_large_media_);
    s.store_field("show_media_above_description", show_media_above_description_);
    s.store_field("skip_confirmation", skip_confirmation_);
    s.store_field("show_above_text", show_above_text_);
    s.store_field("instant_view_version", instant_view_version_);
    s.store_class_end();
  }
}

linkPreviewTypeAlbum::linkPreviewTypeAlbum()
  : media_()
  , caption_()
{}

linkPreviewTypeAlbum::linkPreviewTypeAlbum(array<object_ptr<LinkPreviewAlbumMedia>> &&media_, string const &caption_)
  : media_(std::move(media_))
  , caption_(caption_)
{}

const std::int32_t linkPreviewTypeAlbum::ID;

void linkPreviewTypeAlbum::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeAlbum");
    { s.store_vector_begin("media", media_.size()); for (const auto &_value : media_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("caption", caption_);
    s.store_class_end();
  }
}

linkPreviewTypeAnimation::linkPreviewTypeAnimation()
  : animation_()
{}

linkPreviewTypeAnimation::linkPreviewTypeAnimation(object_ptr<animation> &&animation_)
  : animation_(std::move(animation_))
{}

const std::int32_t linkPreviewTypeAnimation::ID;

void linkPreviewTypeAnimation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeAnimation");
    s.store_object_field("animation", static_cast<const BaseObject *>(animation_.get()));
    s.store_class_end();
  }
}

linkPreviewTypeApp::linkPreviewTypeApp()
  : photo_()
{}

linkPreviewTypeApp::linkPreviewTypeApp(object_ptr<photo> &&photo_)
  : photo_(std::move(photo_))
{}

const std::int32_t linkPreviewTypeApp::ID;

void linkPreviewTypeApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeApp");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

linkPreviewTypeArticle::linkPreviewTypeArticle()
  : photo_()
{}

linkPreviewTypeArticle::linkPreviewTypeArticle(object_ptr<photo> &&photo_)
  : photo_(std::move(photo_))
{}

const std::int32_t linkPreviewTypeArticle::ID;

void linkPreviewTypeArticle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeArticle");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

linkPreviewTypeAudio::linkPreviewTypeAudio()
  : audio_()
{}

linkPreviewTypeAudio::linkPreviewTypeAudio(object_ptr<audio> &&audio_)
  : audio_(std::move(audio_))
{}

const std::int32_t linkPreviewTypeAudio::ID;

void linkPreviewTypeAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeAudio");
    s.store_object_field("audio", static_cast<const BaseObject *>(audio_.get()));
    s.store_class_end();
  }
}

linkPreviewTypeBackground::linkPreviewTypeBackground()
  : document_()
  , background_type_()
{}

linkPreviewTypeBackground::linkPreviewTypeBackground(object_ptr<document> &&document_, object_ptr<BackgroundType> &&background_type_)
  : document_(std::move(document_))
  , background_type_(std::move(background_type_))
{}

const std::int32_t linkPreviewTypeBackground::ID;

void linkPreviewTypeBackground::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeBackground");
    s.store_object_field("document", static_cast<const BaseObject *>(document_.get()));
    s.store_object_field("background_type", static_cast<const BaseObject *>(background_type_.get()));
    s.store_class_end();
  }
}

linkPreviewTypeChannelBoost::linkPreviewTypeChannelBoost()
  : photo_()
{}

linkPreviewTypeChannelBoost::linkPreviewTypeChannelBoost(object_ptr<chatPhoto> &&photo_)
  : photo_(std::move(photo_))
{}

const std::int32_t linkPreviewTypeChannelBoost::ID;

void linkPreviewTypeChannelBoost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeChannelBoost");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

linkPreviewTypeChat::linkPreviewTypeChat()
  : type_()
  , photo_()
  , creates_join_request_()
{}

linkPreviewTypeChat::linkPreviewTypeChat(object_ptr<InviteLinkChatType> &&type_, object_ptr<chatPhoto> &&photo_, bool creates_join_request_)
  : type_(std::move(type_))
  , photo_(std::move(photo_))
  , creates_join_request_(creates_join_request_)
{}

const std::int32_t linkPreviewTypeChat::ID;

void linkPreviewTypeChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeChat");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_field("creates_join_request", creates_join_request_);
    s.store_class_end();
  }
}

linkPreviewTypeDirectMessagesChat::linkPreviewTypeDirectMessagesChat()
  : photo_()
{}

linkPreviewTypeDirectMessagesChat::linkPreviewTypeDirectMessagesChat(object_ptr<chatPhoto> &&photo_)
  : photo_(std::move(photo_))
{}

const std::int32_t linkPreviewTypeDirectMessagesChat::ID;

void linkPreviewTypeDirectMessagesChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeDirectMessagesChat");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

linkPreviewTypeDocument::linkPreviewTypeDocument()
  : document_()
{}

linkPreviewTypeDocument::linkPreviewTypeDocument(object_ptr<document> &&document_)
  : document_(std::move(document_))
{}

const std::int32_t linkPreviewTypeDocument::ID;

void linkPreviewTypeDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeDocument");
    s.store_object_field("document", static_cast<const BaseObject *>(document_.get()));
    s.store_class_end();
  }
}

linkPreviewTypeEmbeddedAnimationPlayer::linkPreviewTypeEmbeddedAnimationPlayer()
  : url_()
  , thumbnail_()
  , duration_()
  , width_()
  , height_()
{}

linkPreviewTypeEmbeddedAnimationPlayer::linkPreviewTypeEmbeddedAnimationPlayer(string const &url_, object_ptr<photo> &&thumbnail_, int32 duration_, int32 width_, int32 height_)
  : url_(url_)
  , thumbnail_(std::move(thumbnail_))
  , duration_(duration_)
  , width_(width_)
  , height_(height_)
{}

const std::int32_t linkPreviewTypeEmbeddedAnimationPlayer::ID;

void linkPreviewTypeEmbeddedAnimationPlayer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeEmbeddedAnimationPlayer");
    s.store_field("url", url_);
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_field("duration", duration_);
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_class_end();
  }
}

linkPreviewTypeEmbeddedAudioPlayer::linkPreviewTypeEmbeddedAudioPlayer()
  : url_()
  , thumbnail_()
  , duration_()
  , width_()
  , height_()
{}

linkPreviewTypeEmbeddedAudioPlayer::linkPreviewTypeEmbeddedAudioPlayer(string const &url_, object_ptr<photo> &&thumbnail_, int32 duration_, int32 width_, int32 height_)
  : url_(url_)
  , thumbnail_(std::move(thumbnail_))
  , duration_(duration_)
  , width_(width_)
  , height_(height_)
{}

const std::int32_t linkPreviewTypeEmbeddedAudioPlayer::ID;

void linkPreviewTypeEmbeddedAudioPlayer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeEmbeddedAudioPlayer");
    s.store_field("url", url_);
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_field("duration", duration_);
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_class_end();
  }
}

linkPreviewTypeEmbeddedVideoPlayer::linkPreviewTypeEmbeddedVideoPlayer()
  : url_()
  , thumbnail_()
  , duration_()
  , width_()
  , height_()
{}

linkPreviewTypeEmbeddedVideoPlayer::linkPreviewTypeEmbeddedVideoPlayer(string const &url_, object_ptr<photo> &&thumbnail_, int32 duration_, int32 width_, int32 height_)
  : url_(url_)
  , thumbnail_(std::move(thumbnail_))
  , duration_(duration_)
  , width_(width_)
  , height_(height_)
{}

const std::int32_t linkPreviewTypeEmbeddedVideoPlayer::ID;

void linkPreviewTypeEmbeddedVideoPlayer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeEmbeddedVideoPlayer");
    s.store_field("url", url_);
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_field("duration", duration_);
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_class_end();
  }
}

linkPreviewTypeExternalAudio::linkPreviewTypeExternalAudio()
  : url_()
  , mime_type_()
  , duration_()
{}

linkPreviewTypeExternalAudio::linkPreviewTypeExternalAudio(string const &url_, string const &mime_type_, int32 duration_)
  : url_(url_)
  , mime_type_(mime_type_)
  , duration_(duration_)
{}

const std::int32_t linkPreviewTypeExternalAudio::ID;

void linkPreviewTypeExternalAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeExternalAudio");
    s.store_field("url", url_);
    s.store_field("mime_type", mime_type_);
    s.store_field("duration", duration_);
    s.store_class_end();
  }
}

linkPreviewTypeExternalVideo::linkPreviewTypeExternalVideo()
  : url_()
  , mime_type_()
  , width_()
  , height_()
  , duration_()
{}

linkPreviewTypeExternalVideo::linkPreviewTypeExternalVideo(string const &url_, string const &mime_type_, int32 width_, int32 height_, int32 duration_)
  : url_(url_)
  , mime_type_(mime_type_)
  , width_(width_)
  , height_(height_)
  , duration_(duration_)
{}

const std::int32_t linkPreviewTypeExternalVideo::ID;

void linkPreviewTypeExternalVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeExternalVideo");
    s.store_field("url", url_);
    s.store_field("mime_type", mime_type_);
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_field("duration", duration_);
    s.store_class_end();
  }
}

linkPreviewTypeGiftCollection::linkPreviewTypeGiftCollection()
  : icons_()
{}

linkPreviewTypeGiftCollection::linkPreviewTypeGiftCollection(array<object_ptr<sticker>> &&icons_)
  : icons_(std::move(icons_))
{}

const std::int32_t linkPreviewTypeGiftCollection::ID;

void linkPreviewTypeGiftCollection::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeGiftCollection");
    { s.store_vector_begin("icons", icons_.size()); for (const auto &_value : icons_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

linkPreviewTypeGroupCall::linkPreviewTypeGroupCall() {
}

const std::int32_t linkPreviewTypeGroupCall::ID;

void linkPreviewTypeGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeGroupCall");
    s.store_class_end();
  }
}

linkPreviewTypeInvoice::linkPreviewTypeInvoice() {
}

const std::int32_t linkPreviewTypeInvoice::ID;

void linkPreviewTypeInvoice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeInvoice");
    s.store_class_end();
  }
}

linkPreviewTypeMessage::linkPreviewTypeMessage() {
}

const std::int32_t linkPreviewTypeMessage::ID;

void linkPreviewTypeMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeMessage");
    s.store_class_end();
  }
}

linkPreviewTypePhoto::linkPreviewTypePhoto()
  : photo_()
{}

linkPreviewTypePhoto::linkPreviewTypePhoto(object_ptr<photo> &&photo_)
  : photo_(std::move(photo_))
{}

const std::int32_t linkPreviewTypePhoto::ID;

void linkPreviewTypePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypePhoto");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

linkPreviewTypePremiumGiftCode::linkPreviewTypePremiumGiftCode() {
}

const std::int32_t linkPreviewTypePremiumGiftCode::ID;

void linkPreviewTypePremiumGiftCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypePremiumGiftCode");
    s.store_class_end();
  }
}

linkPreviewTypeShareableChatFolder::linkPreviewTypeShareableChatFolder() {
}

const std::int32_t linkPreviewTypeShareableChatFolder::ID;

void linkPreviewTypeShareableChatFolder::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeShareableChatFolder");
    s.store_class_end();
  }
}

linkPreviewTypeSticker::linkPreviewTypeSticker()
  : sticker_()
{}

linkPreviewTypeSticker::linkPreviewTypeSticker(object_ptr<sticker> &&sticker_)
  : sticker_(std::move(sticker_))
{}

const std::int32_t linkPreviewTypeSticker::ID;

void linkPreviewTypeSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeSticker");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

linkPreviewTypeStickerSet::linkPreviewTypeStickerSet()
  : stickers_()
{}

linkPreviewTypeStickerSet::linkPreviewTypeStickerSet(array<object_ptr<sticker>> &&stickers_)
  : stickers_(std::move(stickers_))
{}

const std::int32_t linkPreviewTypeStickerSet::ID;

void linkPreviewTypeStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeStickerSet");
    { s.store_vector_begin("stickers", stickers_.size()); for (const auto &_value : stickers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

linkPreviewTypeStory::linkPreviewTypeStory()
  : story_poster_chat_id_()
  , story_id_()
{}

linkPreviewTypeStory::linkPreviewTypeStory(int53 story_poster_chat_id_, int32 story_id_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , story_id_(story_id_)
{}

const std::int32_t linkPreviewTypeStory::ID;

void linkPreviewTypeStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeStory");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("story_id", story_id_);
    s.store_class_end();
  }
}

linkPreviewTypeStoryAlbum::linkPreviewTypeStoryAlbum()
  : photo_icon_()
  , video_icon_()
{}

linkPreviewTypeStoryAlbum::linkPreviewTypeStoryAlbum(object_ptr<photo> &&photo_icon_, object_ptr<video> &&video_icon_)
  : photo_icon_(std::move(photo_icon_))
  , video_icon_(std::move(video_icon_))
{}

const std::int32_t linkPreviewTypeStoryAlbum::ID;

void linkPreviewTypeStoryAlbum::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeStoryAlbum");
    s.store_object_field("photo_icon", static_cast<const BaseObject *>(photo_icon_.get()));
    s.store_object_field("video_icon", static_cast<const BaseObject *>(video_icon_.get()));
    s.store_class_end();
  }
}

linkPreviewTypeSupergroupBoost::linkPreviewTypeSupergroupBoost()
  : photo_()
{}

linkPreviewTypeSupergroupBoost::linkPreviewTypeSupergroupBoost(object_ptr<chatPhoto> &&photo_)
  : photo_(std::move(photo_))
{}

const std::int32_t linkPreviewTypeSupergroupBoost::ID;

void linkPreviewTypeSupergroupBoost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeSupergroupBoost");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

linkPreviewTypeTheme::linkPreviewTypeTheme()
  : documents_()
  , settings_()
{}

linkPreviewTypeTheme::linkPreviewTypeTheme(array<object_ptr<document>> &&documents_, object_ptr<themeSettings> &&settings_)
  : documents_(std::move(documents_))
  , settings_(std::move(settings_))
{}

const std::int32_t linkPreviewTypeTheme::ID;

void linkPreviewTypeTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeTheme");
    { s.store_vector_begin("documents", documents_.size()); for (const auto &_value : documents_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

linkPreviewTypeUnsupported::linkPreviewTypeUnsupported() {
}

const std::int32_t linkPreviewTypeUnsupported::ID;

void linkPreviewTypeUnsupported::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeUnsupported");
    s.store_class_end();
  }
}

linkPreviewTypeUpgradedGift::linkPreviewTypeUpgradedGift()
  : gift_()
{}

linkPreviewTypeUpgradedGift::linkPreviewTypeUpgradedGift(object_ptr<upgradedGift> &&gift_)
  : gift_(std::move(gift_))
{}

const std::int32_t linkPreviewTypeUpgradedGift::ID;

void linkPreviewTypeUpgradedGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeUpgradedGift");
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_class_end();
  }
}

linkPreviewTypeUser::linkPreviewTypeUser()
  : photo_()
  , is_bot_()
{}

linkPreviewTypeUser::linkPreviewTypeUser(object_ptr<chatPhoto> &&photo_, bool is_bot_)
  : photo_(std::move(photo_))
  , is_bot_(is_bot_)
{}

const std::int32_t linkPreviewTypeUser::ID;

void linkPreviewTypeUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeUser");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_field("is_bot", is_bot_);
    s.store_class_end();
  }
}

linkPreviewTypeVideo::linkPreviewTypeVideo()
  : video_()
  , cover_()
  , start_timestamp_()
{}

linkPreviewTypeVideo::linkPreviewTypeVideo(object_ptr<video> &&video_, object_ptr<photo> &&cover_, int32 start_timestamp_)
  : video_(std::move(video_))
  , cover_(std::move(cover_))
  , start_timestamp_(start_timestamp_)
{}

const std::int32_t linkPreviewTypeVideo::ID;

void linkPreviewTypeVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeVideo");
    s.store_object_field("video", static_cast<const BaseObject *>(video_.get()));
    s.store_object_field("cover", static_cast<const BaseObject *>(cover_.get()));
    s.store_field("start_timestamp", start_timestamp_);
    s.store_class_end();
  }
}

linkPreviewTypeVideoChat::linkPreviewTypeVideoChat()
  : photo_()
  , is_live_stream_()
  , joins_as_speaker_()
{}

linkPreviewTypeVideoChat::linkPreviewTypeVideoChat(object_ptr<chatPhoto> &&photo_, bool is_live_stream_, bool joins_as_speaker_)
  : photo_(std::move(photo_))
  , is_live_stream_(is_live_stream_)
  , joins_as_speaker_(joins_as_speaker_)
{}

const std::int32_t linkPreviewTypeVideoChat::ID;

void linkPreviewTypeVideoChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeVideoChat");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_field("is_live_stream", is_live_stream_);
    s.store_field("joins_as_speaker", joins_as_speaker_);
    s.store_class_end();
  }
}

linkPreviewTypeVideoNote::linkPreviewTypeVideoNote()
  : video_note_()
{}

linkPreviewTypeVideoNote::linkPreviewTypeVideoNote(object_ptr<videoNote> &&video_note_)
  : video_note_(std::move(video_note_))
{}

const std::int32_t linkPreviewTypeVideoNote::ID;

void linkPreviewTypeVideoNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeVideoNote");
    s.store_object_field("video_note", static_cast<const BaseObject *>(video_note_.get()));
    s.store_class_end();
  }
}

linkPreviewTypeVoiceNote::linkPreviewTypeVoiceNote()
  : voice_note_()
{}

linkPreviewTypeVoiceNote::linkPreviewTypeVoiceNote(object_ptr<voiceNote> &&voice_note_)
  : voice_note_(std::move(voice_note_))
{}

const std::int32_t linkPreviewTypeVoiceNote::ID;

void linkPreviewTypeVoiceNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeVoiceNote");
    s.store_object_field("voice_note", static_cast<const BaseObject *>(voice_note_.get()));
    s.store_class_end();
  }
}

linkPreviewTypeWebApp::linkPreviewTypeWebApp()
  : photo_()
{}

linkPreviewTypeWebApp::linkPreviewTypeWebApp(object_ptr<photo> &&photo_)
  : photo_(std::move(photo_))
{}

const std::int32_t linkPreviewTypeWebApp::ID;

void linkPreviewTypeWebApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "linkPreviewTypeWebApp");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

logStreamDefault::logStreamDefault() {
}

const std::int32_t logStreamDefault::ID;

void logStreamDefault::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "logStreamDefault");
    s.store_class_end();
  }
}

logStreamFile::logStreamFile()
  : path_()
  , max_file_size_()
  , redirect_stderr_()
{}

logStreamFile::logStreamFile(string const &path_, int53 max_file_size_, bool redirect_stderr_)
  : path_(path_)
  , max_file_size_(max_file_size_)
  , redirect_stderr_(redirect_stderr_)
{}

const std::int32_t logStreamFile::ID;

void logStreamFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "logStreamFile");
    s.store_field("path", path_);
    s.store_field("max_file_size", max_file_size_);
    s.store_field("redirect_stderr", redirect_stderr_);
    s.store_class_end();
  }
}

logStreamEmpty::logStreamEmpty() {
}

const std::int32_t logStreamEmpty::ID;

void logStreamEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "logStreamEmpty");
    s.store_class_end();
  }
}

messageText::messageText()
  : text_()
  , link_preview_()
  , link_preview_options_()
{}

messageText::messageText(object_ptr<formattedText> &&text_, object_ptr<linkPreview> &&link_preview_, object_ptr<linkPreviewOptions> &&link_preview_options_)
  : text_(std::move(text_))
  , link_preview_(std::move(link_preview_))
  , link_preview_options_(std::move(link_preview_options_))
{}

const std::int32_t messageText::ID;

void messageText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageText");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_object_field("link_preview", static_cast<const BaseObject *>(link_preview_.get()));
    s.store_object_field("link_preview_options", static_cast<const BaseObject *>(link_preview_options_.get()));
    s.store_class_end();
  }
}

messageAnimation::messageAnimation()
  : animation_()
  , caption_()
  , show_caption_above_media_()
  , has_spoiler_()
  , is_secret_()
{}

messageAnimation::messageAnimation(object_ptr<animation> &&animation_, object_ptr<formattedText> &&caption_, bool show_caption_above_media_, bool has_spoiler_, bool is_secret_)
  : animation_(std::move(animation_))
  , caption_(std::move(caption_))
  , show_caption_above_media_(show_caption_above_media_)
  , has_spoiler_(has_spoiler_)
  , is_secret_(is_secret_)
{}

const std::int32_t messageAnimation::ID;

void messageAnimation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageAnimation");
    s.store_object_field("animation", static_cast<const BaseObject *>(animation_.get()));
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_field("show_caption_above_media", show_caption_above_media_);
    s.store_field("has_spoiler", has_spoiler_);
    s.store_field("is_secret", is_secret_);
    s.store_class_end();
  }
}

messageAudio::messageAudio()
  : audio_()
  , caption_()
{}

messageAudio::messageAudio(object_ptr<audio> &&audio_, object_ptr<formattedText> &&caption_)
  : audio_(std::move(audio_))
  , caption_(std::move(caption_))
{}

const std::int32_t messageAudio::ID;

void messageAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageAudio");
    s.store_object_field("audio", static_cast<const BaseObject *>(audio_.get()));
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

messageDocument::messageDocument()
  : document_()
  , caption_()
{}

messageDocument::messageDocument(object_ptr<document> &&document_, object_ptr<formattedText> &&caption_)
  : document_(std::move(document_))
  , caption_(std::move(caption_))
{}

const std::int32_t messageDocument::ID;

void messageDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageDocument");
    s.store_object_field("document", static_cast<const BaseObject *>(document_.get()));
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

messagePaidMedia::messagePaidMedia()
  : star_count_()
  , media_()
  , caption_()
  , show_caption_above_media_()
{}

messagePaidMedia::messagePaidMedia(int53 star_count_, array<object_ptr<PaidMedia>> &&media_, object_ptr<formattedText> &&caption_, bool show_caption_above_media_)
  : star_count_(star_count_)
  , media_(std::move(media_))
  , caption_(std::move(caption_))
  , show_caption_above_media_(show_caption_above_media_)
{}

const std::int32_t messagePaidMedia::ID;

void messagePaidMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messagePaidMedia");
    s.store_field("star_count", star_count_);
    { s.store_vector_begin("media", media_.size()); for (const auto &_value : media_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_field("show_caption_above_media", show_caption_above_media_);
    s.store_class_end();
  }
}

messagePhoto::messagePhoto()
  : photo_()
  , caption_()
  , show_caption_above_media_()
  , has_spoiler_()
  , is_secret_()
{}

messagePhoto::messagePhoto(object_ptr<photo> &&photo_, object_ptr<formattedText> &&caption_, bool show_caption_above_media_, bool has_spoiler_, bool is_secret_)
  : photo_(std::move(photo_))
  , caption_(std::move(caption_))
  , show_caption_above_media_(show_caption_above_media_)
  , has_spoiler_(has_spoiler_)
  , is_secret_(is_secret_)
{}

const std::int32_t messagePhoto::ID;

void messagePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messagePhoto");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_field("show_caption_above_media", show_caption_above_media_);
    s.store_field("has_spoiler", has_spoiler_);
    s.store_field("is_secret", is_secret_);
    s.store_class_end();
  }
}

messageSticker::messageSticker()
  : sticker_()
  , is_premium_()
{}

messageSticker::messageSticker(object_ptr<sticker> &&sticker_, bool is_premium_)
  : sticker_(std::move(sticker_))
  , is_premium_(is_premium_)
{}

const std::int32_t messageSticker::ID;

void messageSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSticker");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_field("is_premium", is_premium_);
    s.store_class_end();
  }
}

messageVideo::messageVideo()
  : video_()
  , alternative_videos_()
  , storyboards_()
  , cover_()
  , start_timestamp_()
  , caption_()
  , show_caption_above_media_()
  , has_spoiler_()
  , is_secret_()
{}

messageVideo::messageVideo(object_ptr<video> &&video_, array<object_ptr<alternativeVideo>> &&alternative_videos_, array<object_ptr<videoStoryboard>> &&storyboards_, object_ptr<photo> &&cover_, int32 start_timestamp_, object_ptr<formattedText> &&caption_, bool show_caption_above_media_, bool has_spoiler_, bool is_secret_)
  : video_(std::move(video_))
  , alternative_videos_(std::move(alternative_videos_))
  , storyboards_(std::move(storyboards_))
  , cover_(std::move(cover_))
  , start_timestamp_(start_timestamp_)
  , caption_(std::move(caption_))
  , show_caption_above_media_(show_caption_above_media_)
  , has_spoiler_(has_spoiler_)
  , is_secret_(is_secret_)
{}

const std::int32_t messageVideo::ID;

void messageVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageVideo");
    s.store_object_field("video", static_cast<const BaseObject *>(video_.get()));
    { s.store_vector_begin("alternative_videos", alternative_videos_.size()); for (const auto &_value : alternative_videos_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("storyboards", storyboards_.size()); for (const auto &_value : storyboards_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("cover", static_cast<const BaseObject *>(cover_.get()));
    s.store_field("start_timestamp", start_timestamp_);
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_field("show_caption_above_media", show_caption_above_media_);
    s.store_field("has_spoiler", has_spoiler_);
    s.store_field("is_secret", is_secret_);
    s.store_class_end();
  }
}

messageVideoNote::messageVideoNote()
  : video_note_()
  , is_viewed_()
  , is_secret_()
{}

messageVideoNote::messageVideoNote(object_ptr<videoNote> &&video_note_, bool is_viewed_, bool is_secret_)
  : video_note_(std::move(video_note_))
  , is_viewed_(is_viewed_)
  , is_secret_(is_secret_)
{}

const std::int32_t messageVideoNote::ID;

void messageVideoNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageVideoNote");
    s.store_object_field("video_note", static_cast<const BaseObject *>(video_note_.get()));
    s.store_field("is_viewed", is_viewed_);
    s.store_field("is_secret", is_secret_);
    s.store_class_end();
  }
}

messageVoiceNote::messageVoiceNote()
  : voice_note_()
  , caption_()
  , is_listened_()
{}

messageVoiceNote::messageVoiceNote(object_ptr<voiceNote> &&voice_note_, object_ptr<formattedText> &&caption_, bool is_listened_)
  : voice_note_(std::move(voice_note_))
  , caption_(std::move(caption_))
  , is_listened_(is_listened_)
{}

const std::int32_t messageVoiceNote::ID;

void messageVoiceNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageVoiceNote");
    s.store_object_field("voice_note", static_cast<const BaseObject *>(voice_note_.get()));
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_field("is_listened", is_listened_);
    s.store_class_end();
  }
}

messageExpiredPhoto::messageExpiredPhoto() {
}

const std::int32_t messageExpiredPhoto::ID;

void messageExpiredPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageExpiredPhoto");
    s.store_class_end();
  }
}

messageExpiredVideo::messageExpiredVideo() {
}

const std::int32_t messageExpiredVideo::ID;

void messageExpiredVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageExpiredVideo");
    s.store_class_end();
  }
}

messageExpiredVideoNote::messageExpiredVideoNote() {
}

const std::int32_t messageExpiredVideoNote::ID;

void messageExpiredVideoNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageExpiredVideoNote");
    s.store_class_end();
  }
}

messageExpiredVoiceNote::messageExpiredVoiceNote() {
}

const std::int32_t messageExpiredVoiceNote::ID;

void messageExpiredVoiceNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageExpiredVoiceNote");
    s.store_class_end();
  }
}

messageLocation::messageLocation()
  : location_()
  , live_period_()
  , expires_in_()
  , heading_()
  , proximity_alert_radius_()
{}

messageLocation::messageLocation(object_ptr<location> &&location_, int32 live_period_, int32 expires_in_, int32 heading_, int32 proximity_alert_radius_)
  : location_(std::move(location_))
  , live_period_(live_period_)
  , expires_in_(expires_in_)
  , heading_(heading_)
  , proximity_alert_radius_(proximity_alert_radius_)
{}

const std::int32_t messageLocation::ID;

void messageLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageLocation");
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_field("live_period", live_period_);
    s.store_field("expires_in", expires_in_);
    s.store_field("heading", heading_);
    s.store_field("proximity_alert_radius", proximity_alert_radius_);
    s.store_class_end();
  }
}

messageVenue::messageVenue()
  : venue_()
{}

messageVenue::messageVenue(object_ptr<venue> &&venue_)
  : venue_(std::move(venue_))
{}

const std::int32_t messageVenue::ID;

void messageVenue::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageVenue");
    s.store_object_field("venue", static_cast<const BaseObject *>(venue_.get()));
    s.store_class_end();
  }
}

messageContact::messageContact()
  : contact_()
{}

messageContact::messageContact(object_ptr<contact> &&contact_)
  : contact_(std::move(contact_))
{}

const std::int32_t messageContact::ID;

void messageContact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageContact");
    s.store_object_field("contact", static_cast<const BaseObject *>(contact_.get()));
    s.store_class_end();
  }
}

messageAnimatedEmoji::messageAnimatedEmoji()
  : animated_emoji_()
  , emoji_()
{}

messageAnimatedEmoji::messageAnimatedEmoji(object_ptr<animatedEmoji> &&animated_emoji_, string const &emoji_)
  : animated_emoji_(std::move(animated_emoji_))
  , emoji_(emoji_)
{}

const std::int32_t messageAnimatedEmoji::ID;

void messageAnimatedEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageAnimatedEmoji");
    s.store_object_field("animated_emoji", static_cast<const BaseObject *>(animated_emoji_.get()));
    s.store_field("emoji", emoji_);
    s.store_class_end();
  }
}

messageDice::messageDice()
  : initial_state_()
  , final_state_()
  , emoji_()
  , value_()
  , success_animation_frame_number_()
{}

messageDice::messageDice(object_ptr<DiceStickers> &&initial_state_, object_ptr<DiceStickers> &&final_state_, string const &emoji_, int32 value_, int32 success_animation_frame_number_)
  : initial_state_(std::move(initial_state_))
  , final_state_(std::move(final_state_))
  , emoji_(emoji_)
  , value_(value_)
  , success_animation_frame_number_(success_animation_frame_number_)
{}

const std::int32_t messageDice::ID;

void messageDice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageDice");
    s.store_object_field("initial_state", static_cast<const BaseObject *>(initial_state_.get()));
    s.store_object_field("final_state", static_cast<const BaseObject *>(final_state_.get()));
    s.store_field("emoji", emoji_);
    s.store_field("value", value_);
    s.store_field("success_animation_frame_number", success_animation_frame_number_);
    s.store_class_end();
  }
}

messageGame::messageGame()
  : game_()
{}

messageGame::messageGame(object_ptr<game> &&game_)
  : game_(std::move(game_))
{}

const std::int32_t messageGame::ID;

void messageGame::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageGame");
    s.store_object_field("game", static_cast<const BaseObject *>(game_.get()));
    s.store_class_end();
  }
}

messagePoll::messagePoll()
  : poll_()
{}

messagePoll::messagePoll(object_ptr<poll> &&poll_)
  : poll_(std::move(poll_))
{}

const std::int32_t messagePoll::ID;

void messagePoll::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messagePoll");
    s.store_object_field("poll", static_cast<const BaseObject *>(poll_.get()));
    s.store_class_end();
  }
}

messageStory::messageStory()
  : story_poster_chat_id_()
  , story_id_()
  , via_mention_()
{}

messageStory::messageStory(int53 story_poster_chat_id_, int32 story_id_, bool via_mention_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , story_id_(story_id_)
  , via_mention_(via_mention_)
{}

const std::int32_t messageStory::ID;

void messageStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageStory");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("story_id", story_id_);
    s.store_field("via_mention", via_mention_);
    s.store_class_end();
  }
}

messageChecklist::messageChecklist()
  : list_()
{}

messageChecklist::messageChecklist(object_ptr<checklist> &&list_)
  : list_(std::move(list_))
{}

const std::int32_t messageChecklist::ID;

void messageChecklist::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageChecklist");
    s.store_object_field("list", static_cast<const BaseObject *>(list_.get()));
    s.store_class_end();
  }
}

messageInvoice::messageInvoice()
  : product_info_()
  , currency_()
  , total_amount_()
  , start_parameter_()
  , is_test_()
  , need_shipping_address_()
  , receipt_message_id_()
  , paid_media_()
  , paid_media_caption_()
{}

messageInvoice::messageInvoice(object_ptr<productInfo> &&product_info_, string const &currency_, int53 total_amount_, string const &start_parameter_, bool is_test_, bool need_shipping_address_, int53 receipt_message_id_, object_ptr<PaidMedia> &&paid_media_, object_ptr<formattedText> &&paid_media_caption_)
  : product_info_(std::move(product_info_))
  , currency_(currency_)
  , total_amount_(total_amount_)
  , start_parameter_(start_parameter_)
  , is_test_(is_test_)
  , need_shipping_address_(need_shipping_address_)
  , receipt_message_id_(receipt_message_id_)
  , paid_media_(std::move(paid_media_))
  , paid_media_caption_(std::move(paid_media_caption_))
{}

const std::int32_t messageInvoice::ID;

void messageInvoice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageInvoice");
    s.store_object_field("product_info", static_cast<const BaseObject *>(product_info_.get()));
    s.store_field("currency", currency_);
    s.store_field("total_amount", total_amount_);
    s.store_field("start_parameter", start_parameter_);
    s.store_field("is_test", is_test_);
    s.store_field("need_shipping_address", need_shipping_address_);
    s.store_field("receipt_message_id", receipt_message_id_);
    s.store_object_field("paid_media", static_cast<const BaseObject *>(paid_media_.get()));
    s.store_object_field("paid_media_caption", static_cast<const BaseObject *>(paid_media_caption_.get()));
    s.store_class_end();
  }
}

messageCall::messageCall()
  : is_video_()
  , discard_reason_()
  , duration_()
{}

messageCall::messageCall(bool is_video_, object_ptr<CallDiscardReason> &&discard_reason_, int32 duration_)
  : is_video_(is_video_)
  , discard_reason_(std::move(discard_reason_))
  , duration_(duration_)
{}

const std::int32_t messageCall::ID;

void messageCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageCall");
    s.store_field("is_video", is_video_);
    s.store_object_field("discard_reason", static_cast<const BaseObject *>(discard_reason_.get()));
    s.store_field("duration", duration_);
    s.store_class_end();
  }
}

messageGroupCall::messageGroupCall()
  : is_active_()
  , was_missed_()
  , is_video_()
  , duration_()
  , other_participant_ids_()
{}

messageGroupCall::messageGroupCall(bool is_active_, bool was_missed_, bool is_video_, int32 duration_, array<object_ptr<MessageSender>> &&other_participant_ids_)
  : is_active_(is_active_)
  , was_missed_(was_missed_)
  , is_video_(is_video_)
  , duration_(duration_)
  , other_participant_ids_(std::move(other_participant_ids_))
{}

const std::int32_t messageGroupCall::ID;

void messageGroupCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageGroupCall");
    s.store_field("is_active", is_active_);
    s.store_field("was_missed", was_missed_);
    s.store_field("is_video", is_video_);
    s.store_field("duration", duration_);
    { s.store_vector_begin("other_participant_ids", other_participant_ids_.size()); for (const auto &_value : other_participant_ids_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messageVideoChatScheduled::messageVideoChatScheduled()
  : group_call_id_()
  , start_date_()
{}

messageVideoChatScheduled::messageVideoChatScheduled(int32 group_call_id_, int32 start_date_)
  : group_call_id_(group_call_id_)
  , start_date_(start_date_)
{}

const std::int32_t messageVideoChatScheduled::ID;

void messageVideoChatScheduled::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageVideoChatScheduled");
    s.store_field("group_call_id", group_call_id_);
    s.store_field("start_date", start_date_);
    s.store_class_end();
  }
}

messageVideoChatStarted::messageVideoChatStarted()
  : group_call_id_()
{}

messageVideoChatStarted::messageVideoChatStarted(int32 group_call_id_)
  : group_call_id_(group_call_id_)
{}

const std::int32_t messageVideoChatStarted::ID;

void messageVideoChatStarted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageVideoChatStarted");
    s.store_field("group_call_id", group_call_id_);
    s.store_class_end();
  }
}

messageVideoChatEnded::messageVideoChatEnded()
  : duration_()
{}

messageVideoChatEnded::messageVideoChatEnded(int32 duration_)
  : duration_(duration_)
{}

const std::int32_t messageVideoChatEnded::ID;

void messageVideoChatEnded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageVideoChatEnded");
    s.store_field("duration", duration_);
    s.store_class_end();
  }
}

messageInviteVideoChatParticipants::messageInviteVideoChatParticipants()
  : group_call_id_()
  , user_ids_()
{}

messageInviteVideoChatParticipants::messageInviteVideoChatParticipants(int32 group_call_id_, array<int53> &&user_ids_)
  : group_call_id_(group_call_id_)
  , user_ids_(std::move(user_ids_))
{}

const std::int32_t messageInviteVideoChatParticipants::ID;

void messageInviteVideoChatParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageInviteVideoChatParticipants");
    s.store_field("group_call_id", group_call_id_);
    { s.store_vector_begin("user_ids", user_ids_.size()); for (const auto &_value : user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messageBasicGroupChatCreate::messageBasicGroupChatCreate()
  : title_()
  , member_user_ids_()
{}

messageBasicGroupChatCreate::messageBasicGroupChatCreate(string const &title_, array<int53> &&member_user_ids_)
  : title_(title_)
  , member_user_ids_(std::move(member_user_ids_))
{}

const std::int32_t messageBasicGroupChatCreate::ID;

void messageBasicGroupChatCreate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageBasicGroupChatCreate");
    s.store_field("title", title_);
    { s.store_vector_begin("member_user_ids", member_user_ids_.size()); for (const auto &_value : member_user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messageSupergroupChatCreate::messageSupergroupChatCreate()
  : title_()
{}

messageSupergroupChatCreate::messageSupergroupChatCreate(string const &title_)
  : title_(title_)
{}

const std::int32_t messageSupergroupChatCreate::ID;

void messageSupergroupChatCreate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSupergroupChatCreate");
    s.store_field("title", title_);
    s.store_class_end();
  }
}

messageChatChangeTitle::messageChatChangeTitle()
  : title_()
{}

messageChatChangeTitle::messageChatChangeTitle(string const &title_)
  : title_(title_)
{}

const std::int32_t messageChatChangeTitle::ID;

void messageChatChangeTitle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageChatChangeTitle");
    s.store_field("title", title_);
    s.store_class_end();
  }
}

messageChatChangePhoto::messageChatChangePhoto()
  : photo_()
{}

messageChatChangePhoto::messageChatChangePhoto(object_ptr<chatPhoto> &&photo_)
  : photo_(std::move(photo_))
{}

const std::int32_t messageChatChangePhoto::ID;

void messageChatChangePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageChatChangePhoto");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

messageChatDeletePhoto::messageChatDeletePhoto() {
}

const std::int32_t messageChatDeletePhoto::ID;

void messageChatDeletePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageChatDeletePhoto");
    s.store_class_end();
  }
}

messageChatAddMembers::messageChatAddMembers()
  : member_user_ids_()
{}

messageChatAddMembers::messageChatAddMembers(array<int53> &&member_user_ids_)
  : member_user_ids_(std::move(member_user_ids_))
{}

const std::int32_t messageChatAddMembers::ID;

void messageChatAddMembers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageChatAddMembers");
    { s.store_vector_begin("member_user_ids", member_user_ids_.size()); for (const auto &_value : member_user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messageChatJoinByLink::messageChatJoinByLink() {
}

const std::int32_t messageChatJoinByLink::ID;

void messageChatJoinByLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageChatJoinByLink");
    s.store_class_end();
  }
}

messageChatJoinByRequest::messageChatJoinByRequest() {
}

const std::int32_t messageChatJoinByRequest::ID;

void messageChatJoinByRequest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageChatJoinByRequest");
    s.store_class_end();
  }
}

messageChatDeleteMember::messageChatDeleteMember()
  : user_id_()
{}

messageChatDeleteMember::messageChatDeleteMember(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t messageChatDeleteMember::ID;

void messageChatDeleteMember::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageChatDeleteMember");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

messageChatUpgradeTo::messageChatUpgradeTo()
  : supergroup_id_()
{}

messageChatUpgradeTo::messageChatUpgradeTo(int53 supergroup_id_)
  : supergroup_id_(supergroup_id_)
{}

const std::int32_t messageChatUpgradeTo::ID;

void messageChatUpgradeTo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageChatUpgradeTo");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_class_end();
  }
}

messageChatUpgradeFrom::messageChatUpgradeFrom()
  : title_()
  , basic_group_id_()
{}

messageChatUpgradeFrom::messageChatUpgradeFrom(string const &title_, int53 basic_group_id_)
  : title_(title_)
  , basic_group_id_(basic_group_id_)
{}

const std::int32_t messageChatUpgradeFrom::ID;

void messageChatUpgradeFrom::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageChatUpgradeFrom");
    s.store_field("title", title_);
    s.store_field("basic_group_id", basic_group_id_);
    s.store_class_end();
  }
}

messagePinMessage::messagePinMessage()
  : message_id_()
{}

messagePinMessage::messagePinMessage(int53 message_id_)
  : message_id_(message_id_)
{}

const std::int32_t messagePinMessage::ID;

void messagePinMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messagePinMessage");
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

messageScreenshotTaken::messageScreenshotTaken() {
}

const std::int32_t messageScreenshotTaken::ID;

void messageScreenshotTaken::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageScreenshotTaken");
    s.store_class_end();
  }
}

messageChatSetBackground::messageChatSetBackground()
  : old_background_message_id_()
  , background_()
  , only_for_self_()
{}

messageChatSetBackground::messageChatSetBackground(int53 old_background_message_id_, object_ptr<chatBackground> &&background_, bool only_for_self_)
  : old_background_message_id_(old_background_message_id_)
  , background_(std::move(background_))
  , only_for_self_(only_for_self_)
{}

const std::int32_t messageChatSetBackground::ID;

void messageChatSetBackground::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageChatSetBackground");
    s.store_field("old_background_message_id", old_background_message_id_);
    s.store_object_field("background", static_cast<const BaseObject *>(background_.get()));
    s.store_field("only_for_self", only_for_self_);
    s.store_class_end();
  }
}

messageChatSetTheme::messageChatSetTheme()
  : theme_()
{}

messageChatSetTheme::messageChatSetTheme(object_ptr<ChatTheme> &&theme_)
  : theme_(std::move(theme_))
{}

const std::int32_t messageChatSetTheme::ID;

void messageChatSetTheme::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageChatSetTheme");
    s.store_object_field("theme", static_cast<const BaseObject *>(theme_.get()));
    s.store_class_end();
  }
}

messageChatSetMessageAutoDeleteTime::messageChatSetMessageAutoDeleteTime()
  : message_auto_delete_time_()
  , from_user_id_()
{}

messageChatSetMessageAutoDeleteTime::messageChatSetMessageAutoDeleteTime(int32 message_auto_delete_time_, int53 from_user_id_)
  : message_auto_delete_time_(message_auto_delete_time_)
  , from_user_id_(from_user_id_)
{}

const std::int32_t messageChatSetMessageAutoDeleteTime::ID;

void messageChatSetMessageAutoDeleteTime::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageChatSetMessageAutoDeleteTime");
    s.store_field("message_auto_delete_time", message_auto_delete_time_);
    s.store_field("from_user_id", from_user_id_);
    s.store_class_end();
  }
}

messageChatBoost::messageChatBoost()
  : boost_count_()
{}

messageChatBoost::messageChatBoost(int32 boost_count_)
  : boost_count_(boost_count_)
{}

const std::int32_t messageChatBoost::ID;

void messageChatBoost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageChatBoost");
    s.store_field("boost_count", boost_count_);
    s.store_class_end();
  }
}

messageForumTopicCreated::messageForumTopicCreated()
  : name_()
  , is_name_implicit_()
  , icon_()
{}

messageForumTopicCreated::messageForumTopicCreated(string const &name_, bool is_name_implicit_, object_ptr<forumTopicIcon> &&icon_)
  : name_(name_)
  , is_name_implicit_(is_name_implicit_)
  , icon_(std::move(icon_))
{}

const std::int32_t messageForumTopicCreated::ID;

void messageForumTopicCreated::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageForumTopicCreated");
    s.store_field("name", name_);
    s.store_field("is_name_implicit", is_name_implicit_);
    s.store_object_field("icon", static_cast<const BaseObject *>(icon_.get()));
    s.store_class_end();
  }
}

messageForumTopicEdited::messageForumTopicEdited()
  : name_()
  , edit_icon_custom_emoji_id_()
  , icon_custom_emoji_id_()
{}

messageForumTopicEdited::messageForumTopicEdited(string const &name_, bool edit_icon_custom_emoji_id_, int64 icon_custom_emoji_id_)
  : name_(name_)
  , edit_icon_custom_emoji_id_(edit_icon_custom_emoji_id_)
  , icon_custom_emoji_id_(icon_custom_emoji_id_)
{}

const std::int32_t messageForumTopicEdited::ID;

void messageForumTopicEdited::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageForumTopicEdited");
    s.store_field("name", name_);
    s.store_field("edit_icon_custom_emoji_id", edit_icon_custom_emoji_id_);
    s.store_field("icon_custom_emoji_id", icon_custom_emoji_id_);
    s.store_class_end();
  }
}

messageForumTopicIsClosedToggled::messageForumTopicIsClosedToggled()
  : is_closed_()
{}

messageForumTopicIsClosedToggled::messageForumTopicIsClosedToggled(bool is_closed_)
  : is_closed_(is_closed_)
{}

const std::int32_t messageForumTopicIsClosedToggled::ID;

void messageForumTopicIsClosedToggled::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageForumTopicIsClosedToggled");
    s.store_field("is_closed", is_closed_);
    s.store_class_end();
  }
}

messageForumTopicIsHiddenToggled::messageForumTopicIsHiddenToggled()
  : is_hidden_()
{}

messageForumTopicIsHiddenToggled::messageForumTopicIsHiddenToggled(bool is_hidden_)
  : is_hidden_(is_hidden_)
{}

const std::int32_t messageForumTopicIsHiddenToggled::ID;

void messageForumTopicIsHiddenToggled::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageForumTopicIsHiddenToggled");
    s.store_field("is_hidden", is_hidden_);
    s.store_class_end();
  }
}

messageSuggestProfilePhoto::messageSuggestProfilePhoto()
  : photo_()
{}

messageSuggestProfilePhoto::messageSuggestProfilePhoto(object_ptr<chatPhoto> &&photo_)
  : photo_(std::move(photo_))
{}

const std::int32_t messageSuggestProfilePhoto::ID;

void messageSuggestProfilePhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSuggestProfilePhoto");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

messageSuggestBirthdate::messageSuggestBirthdate()
  : birthdate_()
{}

messageSuggestBirthdate::messageSuggestBirthdate(object_ptr<birthdate> &&birthdate_)
  : birthdate_(std::move(birthdate_))
{}

const std::int32_t messageSuggestBirthdate::ID;

void messageSuggestBirthdate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSuggestBirthdate");
    s.store_object_field("birthdate", static_cast<const BaseObject *>(birthdate_.get()));
    s.store_class_end();
  }
}

messageCustomServiceAction::messageCustomServiceAction()
  : text_()
{}

messageCustomServiceAction::messageCustomServiceAction(string const &text_)
  : text_(text_)
{}

const std::int32_t messageCustomServiceAction::ID;

void messageCustomServiceAction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageCustomServiceAction");
    s.store_field("text", text_);
    s.store_class_end();
  }
}

messageGameScore::messageGameScore()
  : game_message_id_()
  , game_id_()
  , score_()
{}

messageGameScore::messageGameScore(int53 game_message_id_, int64 game_id_, int32 score_)
  : game_message_id_(game_message_id_)
  , game_id_(game_id_)
  , score_(score_)
{}

const std::int32_t messageGameScore::ID;

void messageGameScore::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageGameScore");
    s.store_field("game_message_id", game_message_id_);
    s.store_field("game_id", game_id_);
    s.store_field("score", score_);
    s.store_class_end();
  }
}

messagePaymentSuccessful::messagePaymentSuccessful()
  : invoice_chat_id_()
  , invoice_message_id_()
  , currency_()
  , total_amount_()
  , subscription_until_date_()
  , is_recurring_()
  , is_first_recurring_()
  , invoice_name_()
{}

messagePaymentSuccessful::messagePaymentSuccessful(int53 invoice_chat_id_, int53 invoice_message_id_, string const &currency_, int53 total_amount_, int32 subscription_until_date_, bool is_recurring_, bool is_first_recurring_, string const &invoice_name_)
  : invoice_chat_id_(invoice_chat_id_)
  , invoice_message_id_(invoice_message_id_)
  , currency_(currency_)
  , total_amount_(total_amount_)
  , subscription_until_date_(subscription_until_date_)
  , is_recurring_(is_recurring_)
  , is_first_recurring_(is_first_recurring_)
  , invoice_name_(invoice_name_)
{}

const std::int32_t messagePaymentSuccessful::ID;

void messagePaymentSuccessful::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messagePaymentSuccessful");
    s.store_field("invoice_chat_id", invoice_chat_id_);
    s.store_field("invoice_message_id", invoice_message_id_);
    s.store_field("currency", currency_);
    s.store_field("total_amount", total_amount_);
    s.store_field("subscription_until_date", subscription_until_date_);
    s.store_field("is_recurring", is_recurring_);
    s.store_field("is_first_recurring", is_first_recurring_);
    s.store_field("invoice_name", invoice_name_);
    s.store_class_end();
  }
}

messagePaymentSuccessfulBot::messagePaymentSuccessfulBot()
  : currency_()
  , total_amount_()
  , subscription_until_date_()
  , is_recurring_()
  , is_first_recurring_()
  , invoice_payload_()
  , shipping_option_id_()
  , order_info_()
  , telegram_payment_charge_id_()
  , provider_payment_charge_id_()
{}

messagePaymentSuccessfulBot::messagePaymentSuccessfulBot(string const &currency_, int53 total_amount_, int32 subscription_until_date_, bool is_recurring_, bool is_first_recurring_, bytes const &invoice_payload_, string const &shipping_option_id_, object_ptr<orderInfo> &&order_info_, string const &telegram_payment_charge_id_, string const &provider_payment_charge_id_)
  : currency_(currency_)
  , total_amount_(total_amount_)
  , subscription_until_date_(subscription_until_date_)
  , is_recurring_(is_recurring_)
  , is_first_recurring_(is_first_recurring_)
  , invoice_payload_(std::move(invoice_payload_))
  , shipping_option_id_(shipping_option_id_)
  , order_info_(std::move(order_info_))
  , telegram_payment_charge_id_(telegram_payment_charge_id_)
  , provider_payment_charge_id_(provider_payment_charge_id_)
{}

const std::int32_t messagePaymentSuccessfulBot::ID;

void messagePaymentSuccessfulBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messagePaymentSuccessfulBot");
    s.store_field("currency", currency_);
    s.store_field("total_amount", total_amount_);
    s.store_field("subscription_until_date", subscription_until_date_);
    s.store_field("is_recurring", is_recurring_);
    s.store_field("is_first_recurring", is_first_recurring_);
    s.store_bytes_field("invoice_payload", invoice_payload_);
    s.store_field("shipping_option_id", shipping_option_id_);
    s.store_object_field("order_info", static_cast<const BaseObject *>(order_info_.get()));
    s.store_field("telegram_payment_charge_id", telegram_payment_charge_id_);
    s.store_field("provider_payment_charge_id", provider_payment_charge_id_);
    s.store_class_end();
  }
}

messagePaymentRefunded::messagePaymentRefunded()
  : owner_id_()
  , currency_()
  , total_amount_()
  , invoice_payload_()
  , telegram_payment_charge_id_()
  , provider_payment_charge_id_()
{}

messagePaymentRefunded::messagePaymentRefunded(object_ptr<MessageSender> &&owner_id_, string const &currency_, int53 total_amount_, bytes const &invoice_payload_, string const &telegram_payment_charge_id_, string const &provider_payment_charge_id_)
  : owner_id_(std::move(owner_id_))
  , currency_(currency_)
  , total_amount_(total_amount_)
  , invoice_payload_(std::move(invoice_payload_))
  , telegram_payment_charge_id_(telegram_payment_charge_id_)
  , provider_payment_charge_id_(provider_payment_charge_id_)
{}

const std::int32_t messagePaymentRefunded::ID;

void messagePaymentRefunded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messagePaymentRefunded");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_field("currency", currency_);
    s.store_field("total_amount", total_amount_);
    s.store_bytes_field("invoice_payload", invoice_payload_);
    s.store_field("telegram_payment_charge_id", telegram_payment_charge_id_);
    s.store_field("provider_payment_charge_id", provider_payment_charge_id_);
    s.store_class_end();
  }
}

messageGiftedPremium::messageGiftedPremium()
  : gifter_user_id_()
  , receiver_user_id_()
  , text_()
  , currency_()
  , amount_()
  , cryptocurrency_()
  , cryptocurrency_amount_()
  , month_count_()
  , sticker_()
{}

messageGiftedPremium::messageGiftedPremium(int53 gifter_user_id_, int53 receiver_user_id_, object_ptr<formattedText> &&text_, string const &currency_, int53 amount_, string const &cryptocurrency_, int64 cryptocurrency_amount_, int32 month_count_, object_ptr<sticker> &&sticker_)
  : gifter_user_id_(gifter_user_id_)
  , receiver_user_id_(receiver_user_id_)
  , text_(std::move(text_))
  , currency_(currency_)
  , amount_(amount_)
  , cryptocurrency_(cryptocurrency_)
  , cryptocurrency_amount_(cryptocurrency_amount_)
  , month_count_(month_count_)
  , sticker_(std::move(sticker_))
{}

const std::int32_t messageGiftedPremium::ID;

void messageGiftedPremium::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageGiftedPremium");
    s.store_field("gifter_user_id", gifter_user_id_);
    s.store_field("receiver_user_id", receiver_user_id_);
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("cryptocurrency", cryptocurrency_);
    s.store_field("cryptocurrency_amount", cryptocurrency_amount_);
    s.store_field("month_count", month_count_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

messagePremiumGiftCode::messagePremiumGiftCode()
  : creator_id_()
  , text_()
  , is_from_giveaway_()
  , is_unclaimed_()
  , currency_()
  , amount_()
  , cryptocurrency_()
  , cryptocurrency_amount_()
  , month_count_()
  , sticker_()
  , code_()
{}

messagePremiumGiftCode::messagePremiumGiftCode(object_ptr<MessageSender> &&creator_id_, object_ptr<formattedText> &&text_, bool is_from_giveaway_, bool is_unclaimed_, string const &currency_, int53 amount_, string const &cryptocurrency_, int64 cryptocurrency_amount_, int32 month_count_, object_ptr<sticker> &&sticker_, string const &code_)
  : creator_id_(std::move(creator_id_))
  , text_(std::move(text_))
  , is_from_giveaway_(is_from_giveaway_)
  , is_unclaimed_(is_unclaimed_)
  , currency_(currency_)
  , amount_(amount_)
  , cryptocurrency_(cryptocurrency_)
  , cryptocurrency_amount_(cryptocurrency_amount_)
  , month_count_(month_count_)
  , sticker_(std::move(sticker_))
  , code_(code_)
{}

const std::int32_t messagePremiumGiftCode::ID;

void messagePremiumGiftCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messagePremiumGiftCode");
    s.store_object_field("creator_id", static_cast<const BaseObject *>(creator_id_.get()));
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("is_from_giveaway", is_from_giveaway_);
    s.store_field("is_unclaimed", is_unclaimed_);
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("cryptocurrency", cryptocurrency_);
    s.store_field("cryptocurrency_amount", cryptocurrency_amount_);
    s.store_field("month_count", month_count_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_field("code", code_);
    s.store_class_end();
  }
}

messageGiveawayCreated::messageGiveawayCreated()
  : star_count_()
{}

messageGiveawayCreated::messageGiveawayCreated(int53 star_count_)
  : star_count_(star_count_)
{}

const std::int32_t messageGiveawayCreated::ID;

void messageGiveawayCreated::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageGiveawayCreated");
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

messageGiveaway::messageGiveaway()
  : parameters_()
  , winner_count_()
  , prize_()
  , sticker_()
{}

messageGiveaway::messageGiveaway(object_ptr<giveawayParameters> &&parameters_, int32 winner_count_, object_ptr<GiveawayPrize> &&prize_, object_ptr<sticker> &&sticker_)
  : parameters_(std::move(parameters_))
  , winner_count_(winner_count_)
  , prize_(std::move(prize_))
  , sticker_(std::move(sticker_))
{}

const std::int32_t messageGiveaway::ID;

void messageGiveaway::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageGiveaway");
    s.store_object_field("parameters", static_cast<const BaseObject *>(parameters_.get()));
    s.store_field("winner_count", winner_count_);
    s.store_object_field("prize", static_cast<const BaseObject *>(prize_.get()));
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

messageGiveawayCompleted::messageGiveawayCompleted()
  : giveaway_message_id_()
  , winner_count_()
  , is_star_giveaway_()
  , unclaimed_prize_count_()
{}

messageGiveawayCompleted::messageGiveawayCompleted(int53 giveaway_message_id_, int32 winner_count_, bool is_star_giveaway_, int32 unclaimed_prize_count_)
  : giveaway_message_id_(giveaway_message_id_)
  , winner_count_(winner_count_)
  , is_star_giveaway_(is_star_giveaway_)
  , unclaimed_prize_count_(unclaimed_prize_count_)
{}

const std::int32_t messageGiveawayCompleted::ID;

void messageGiveawayCompleted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageGiveawayCompleted");
    s.store_field("giveaway_message_id", giveaway_message_id_);
    s.store_field("winner_count", winner_count_);
    s.store_field("is_star_giveaway", is_star_giveaway_);
    s.store_field("unclaimed_prize_count", unclaimed_prize_count_);
    s.store_class_end();
  }
}

messageGiveawayWinners::messageGiveawayWinners()
  : boosted_chat_id_()
  , giveaway_message_id_()
  , additional_chat_count_()
  , actual_winners_selection_date_()
  , only_new_members_()
  , was_refunded_()
  , prize_()
  , prize_description_()
  , winner_count_()
  , winner_user_ids_()
  , unclaimed_prize_count_()
{}

messageGiveawayWinners::messageGiveawayWinners(int53 boosted_chat_id_, int53 giveaway_message_id_, int32 additional_chat_count_, int32 actual_winners_selection_date_, bool only_new_members_, bool was_refunded_, object_ptr<GiveawayPrize> &&prize_, string const &prize_description_, int32 winner_count_, array<int53> &&winner_user_ids_, int32 unclaimed_prize_count_)
  : boosted_chat_id_(boosted_chat_id_)
  , giveaway_message_id_(giveaway_message_id_)
  , additional_chat_count_(additional_chat_count_)
  , actual_winners_selection_date_(actual_winners_selection_date_)
  , only_new_members_(only_new_members_)
  , was_refunded_(was_refunded_)
  , prize_(std::move(prize_))
  , prize_description_(prize_description_)
  , winner_count_(winner_count_)
  , winner_user_ids_(std::move(winner_user_ids_))
  , unclaimed_prize_count_(unclaimed_prize_count_)
{}

const std::int32_t messageGiveawayWinners::ID;

void messageGiveawayWinners::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageGiveawayWinners");
    s.store_field("boosted_chat_id", boosted_chat_id_);
    s.store_field("giveaway_message_id", giveaway_message_id_);
    s.store_field("additional_chat_count", additional_chat_count_);
    s.store_field("actual_winners_selection_date", actual_winners_selection_date_);
    s.store_field("only_new_members", only_new_members_);
    s.store_field("was_refunded", was_refunded_);
    s.store_object_field("prize", static_cast<const BaseObject *>(prize_.get()));
    s.store_field("prize_description", prize_description_);
    s.store_field("winner_count", winner_count_);
    { s.store_vector_begin("winner_user_ids", winner_user_ids_.size()); for (const auto &_value : winner_user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("unclaimed_prize_count", unclaimed_prize_count_);
    s.store_class_end();
  }
}

messageGiftedStars::messageGiftedStars()
  : gifter_user_id_()
  , receiver_user_id_()
  , currency_()
  , amount_()
  , cryptocurrency_()
  , cryptocurrency_amount_()
  , star_count_()
  , transaction_id_()
  , sticker_()
{}

messageGiftedStars::messageGiftedStars(int53 gifter_user_id_, int53 receiver_user_id_, string const &currency_, int53 amount_, string const &cryptocurrency_, int64 cryptocurrency_amount_, int53 star_count_, string const &transaction_id_, object_ptr<sticker> &&sticker_)
  : gifter_user_id_(gifter_user_id_)
  , receiver_user_id_(receiver_user_id_)
  , currency_(currency_)
  , amount_(amount_)
  , cryptocurrency_(cryptocurrency_)
  , cryptocurrency_amount_(cryptocurrency_amount_)
  , star_count_(star_count_)
  , transaction_id_(transaction_id_)
  , sticker_(std::move(sticker_))
{}

const std::int32_t messageGiftedStars::ID;

void messageGiftedStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageGiftedStars");
    s.store_field("gifter_user_id", gifter_user_id_);
    s.store_field("receiver_user_id", receiver_user_id_);
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("cryptocurrency", cryptocurrency_);
    s.store_field("cryptocurrency_amount", cryptocurrency_amount_);
    s.store_field("star_count", star_count_);
    s.store_field("transaction_id", transaction_id_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

messageGiftedTon::messageGiftedTon()
  : gifter_user_id_()
  , receiver_user_id_()
  , ton_amount_()
  , transaction_id_()
  , sticker_()
{}

messageGiftedTon::messageGiftedTon(int53 gifter_user_id_, int53 receiver_user_id_, int53 ton_amount_, string const &transaction_id_, object_ptr<sticker> &&sticker_)
  : gifter_user_id_(gifter_user_id_)
  , receiver_user_id_(receiver_user_id_)
  , ton_amount_(ton_amount_)
  , transaction_id_(transaction_id_)
  , sticker_(std::move(sticker_))
{}

const std::int32_t messageGiftedTon::ID;

void messageGiftedTon::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageGiftedTon");
    s.store_field("gifter_user_id", gifter_user_id_);
    s.store_field("receiver_user_id", receiver_user_id_);
    s.store_field("ton_amount", ton_amount_);
    s.store_field("transaction_id", transaction_id_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

messageGiveawayPrizeStars::messageGiveawayPrizeStars()
  : star_count_()
  , transaction_id_()
  , boosted_chat_id_()
  , giveaway_message_id_()
  , is_unclaimed_()
  , sticker_()
{}

messageGiveawayPrizeStars::messageGiveawayPrizeStars(int53 star_count_, string const &transaction_id_, int53 boosted_chat_id_, int53 giveaway_message_id_, bool is_unclaimed_, object_ptr<sticker> &&sticker_)
  : star_count_(star_count_)
  , transaction_id_(transaction_id_)
  , boosted_chat_id_(boosted_chat_id_)
  , giveaway_message_id_(giveaway_message_id_)
  , is_unclaimed_(is_unclaimed_)
  , sticker_(std::move(sticker_))
{}

const std::int32_t messageGiveawayPrizeStars::ID;

void messageGiveawayPrizeStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageGiveawayPrizeStars");
    s.store_field("star_count", star_count_);
    s.store_field("transaction_id", transaction_id_);
    s.store_field("boosted_chat_id", boosted_chat_id_);
    s.store_field("giveaway_message_id", giveaway_message_id_);
    s.store_field("is_unclaimed", is_unclaimed_);
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

messageGift::messageGift()
  : gift_()
  , sender_id_()
  , receiver_id_()
  , received_gift_id_()
  , text_()
  , sell_star_count_()
  , prepaid_upgrade_star_count_()
  , is_upgrade_separate_()
  , is_private_()
  , is_saved_()
  , is_prepaid_upgrade_()
  , can_be_upgraded_()
  , was_converted_()
  , was_upgraded_()
  , was_refunded_()
  , upgraded_received_gift_id_()
  , prepaid_upgrade_hash_()
{}

messageGift::messageGift(object_ptr<gift> &&gift_, object_ptr<MessageSender> &&sender_id_, object_ptr<MessageSender> &&receiver_id_, string const &received_gift_id_, object_ptr<formattedText> &&text_, int53 sell_star_count_, int53 prepaid_upgrade_star_count_, bool is_upgrade_separate_, bool is_private_, bool is_saved_, bool is_prepaid_upgrade_, bool can_be_upgraded_, bool was_converted_, bool was_upgraded_, bool was_refunded_, string const &upgraded_received_gift_id_, string const &prepaid_upgrade_hash_)
  : gift_(std::move(gift_))
  , sender_id_(std::move(sender_id_))
  , receiver_id_(std::move(receiver_id_))
  , received_gift_id_(received_gift_id_)
  , text_(std::move(text_))
  , sell_star_count_(sell_star_count_)
  , prepaid_upgrade_star_count_(prepaid_upgrade_star_count_)
  , is_upgrade_separate_(is_upgrade_separate_)
  , is_private_(is_private_)
  , is_saved_(is_saved_)
  , is_prepaid_upgrade_(is_prepaid_upgrade_)
  , can_be_upgraded_(can_be_upgraded_)
  , was_converted_(was_converted_)
  , was_upgraded_(was_upgraded_)
  , was_refunded_(was_refunded_)
  , upgraded_received_gift_id_(upgraded_received_gift_id_)
  , prepaid_upgrade_hash_(prepaid_upgrade_hash_)
{}

const std::int32_t messageGift::ID;

void messageGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageGift");
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_object_field("receiver_id", static_cast<const BaseObject *>(receiver_id_.get()));
    s.store_field("received_gift_id", received_gift_id_);
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("sell_star_count", sell_star_count_);
    s.store_field("prepaid_upgrade_star_count", prepaid_upgrade_star_count_);
    s.store_field("is_upgrade_separate", is_upgrade_separate_);
    s.store_field("is_private", is_private_);
    s.store_field("is_saved", is_saved_);
    s.store_field("is_prepaid_upgrade", is_prepaid_upgrade_);
    s.store_field("can_be_upgraded", can_be_upgraded_);
    s.store_field("was_converted", was_converted_);
    s.store_field("was_upgraded", was_upgraded_);
    s.store_field("was_refunded", was_refunded_);
    s.store_field("upgraded_received_gift_id", upgraded_received_gift_id_);
    s.store_field("prepaid_upgrade_hash", prepaid_upgrade_hash_);
    s.store_class_end();
  }
}

messageUpgradedGift::messageUpgradedGift()
  : gift_()
  , sender_id_()
  , receiver_id_()
  , origin_()
  , received_gift_id_()
  , is_saved_()
  , can_be_transferred_()
  , was_transferred_()
  , transfer_star_count_()
  , drop_original_details_star_count_()
  , next_transfer_date_()
  , next_resale_date_()
  , export_date_()
{}

messageUpgradedGift::messageUpgradedGift(object_ptr<upgradedGift> &&gift_, object_ptr<MessageSender> &&sender_id_, object_ptr<MessageSender> &&receiver_id_, object_ptr<UpgradedGiftOrigin> &&origin_, string const &received_gift_id_, bool is_saved_, bool can_be_transferred_, bool was_transferred_, int53 transfer_star_count_, int53 drop_original_details_star_count_, int32 next_transfer_date_, int32 next_resale_date_, int32 export_date_)
  : gift_(std::move(gift_))
  , sender_id_(std::move(sender_id_))
  , receiver_id_(std::move(receiver_id_))
  , origin_(std::move(origin_))
  , received_gift_id_(received_gift_id_)
  , is_saved_(is_saved_)
  , can_be_transferred_(can_be_transferred_)
  , was_transferred_(was_transferred_)
  , transfer_star_count_(transfer_star_count_)
  , drop_original_details_star_count_(drop_original_details_star_count_)
  , next_transfer_date_(next_transfer_date_)
  , next_resale_date_(next_resale_date_)
  , export_date_(export_date_)
{}

const std::int32_t messageUpgradedGift::ID;

void messageUpgradedGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageUpgradedGift");
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_object_field("receiver_id", static_cast<const BaseObject *>(receiver_id_.get()));
    s.store_object_field("origin", static_cast<const BaseObject *>(origin_.get()));
    s.store_field("received_gift_id", received_gift_id_);
    s.store_field("is_saved", is_saved_);
    s.store_field("can_be_transferred", can_be_transferred_);
    s.store_field("was_transferred", was_transferred_);
    s.store_field("transfer_star_count", transfer_star_count_);
    s.store_field("drop_original_details_star_count", drop_original_details_star_count_);
    s.store_field("next_transfer_date", next_transfer_date_);
    s.store_field("next_resale_date", next_resale_date_);
    s.store_field("export_date", export_date_);
    s.store_class_end();
  }
}

messageRefundedUpgradedGift::messageRefundedUpgradedGift()
  : gift_()
  , sender_id_()
  , receiver_id_()
  , origin_()
{}

messageRefundedUpgradedGift::messageRefundedUpgradedGift(object_ptr<gift> &&gift_, object_ptr<MessageSender> &&sender_id_, object_ptr<MessageSender> &&receiver_id_, object_ptr<UpgradedGiftOrigin> &&origin_)
  : gift_(std::move(gift_))
  , sender_id_(std::move(sender_id_))
  , receiver_id_(std::move(receiver_id_))
  , origin_(std::move(origin_))
{}

const std::int32_t messageRefundedUpgradedGift::ID;

void messageRefundedUpgradedGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageRefundedUpgradedGift");
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_object_field("receiver_id", static_cast<const BaseObject *>(receiver_id_.get()));
    s.store_object_field("origin", static_cast<const BaseObject *>(origin_.get()));
    s.store_class_end();
  }
}

messagePaidMessagesRefunded::messagePaidMessagesRefunded()
  : message_count_()
  , star_count_()
{}

messagePaidMessagesRefunded::messagePaidMessagesRefunded(int32 message_count_, int53 star_count_)
  : message_count_(message_count_)
  , star_count_(star_count_)
{}

const std::int32_t messagePaidMessagesRefunded::ID;

void messagePaidMessagesRefunded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messagePaidMessagesRefunded");
    s.store_field("message_count", message_count_);
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

messagePaidMessagePriceChanged::messagePaidMessagePriceChanged()
  : paid_message_star_count_()
{}

messagePaidMessagePriceChanged::messagePaidMessagePriceChanged(int53 paid_message_star_count_)
  : paid_message_star_count_(paid_message_star_count_)
{}

const std::int32_t messagePaidMessagePriceChanged::ID;

void messagePaidMessagePriceChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messagePaidMessagePriceChanged");
    s.store_field("paid_message_star_count", paid_message_star_count_);
    s.store_class_end();
  }
}

messageDirectMessagePriceChanged::messageDirectMessagePriceChanged()
  : is_enabled_()
  , paid_message_star_count_()
{}

messageDirectMessagePriceChanged::messageDirectMessagePriceChanged(bool is_enabled_, int53 paid_message_star_count_)
  : is_enabled_(is_enabled_)
  , paid_message_star_count_(paid_message_star_count_)
{}

const std::int32_t messageDirectMessagePriceChanged::ID;

void messageDirectMessagePriceChanged::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageDirectMessagePriceChanged");
    s.store_field("is_enabled", is_enabled_);
    s.store_field("paid_message_star_count", paid_message_star_count_);
    s.store_class_end();
  }
}

messageChecklistTasksDone::messageChecklistTasksDone()
  : checklist_message_id_()
  , marked_as_done_task_ids_()
  , marked_as_not_done_task_ids_()
{}

messageChecklistTasksDone::messageChecklistTasksDone(int53 checklist_message_id_, array<int32> &&marked_as_done_task_ids_, array<int32> &&marked_as_not_done_task_ids_)
  : checklist_message_id_(checklist_message_id_)
  , marked_as_done_task_ids_(std::move(marked_as_done_task_ids_))
  , marked_as_not_done_task_ids_(std::move(marked_as_not_done_task_ids_))
{}

const std::int32_t messageChecklistTasksDone::ID;

void messageChecklistTasksDone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageChecklistTasksDone");
    s.store_field("checklist_message_id", checklist_message_id_);
    { s.store_vector_begin("marked_as_done_task_ids", marked_as_done_task_ids_.size()); for (const auto &_value : marked_as_done_task_ids_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("marked_as_not_done_task_ids", marked_as_not_done_task_ids_.size()); for (const auto &_value : marked_as_not_done_task_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

messageChecklistTasksAdded::messageChecklistTasksAdded()
  : checklist_message_id_()
  , tasks_()
{}

messageChecklistTasksAdded::messageChecklistTasksAdded(int53 checklist_message_id_, array<object_ptr<checklistTask>> &&tasks_)
  : checklist_message_id_(checklist_message_id_)
  , tasks_(std::move(tasks_))
{}

const std::int32_t messageChecklistTasksAdded::ID;

void messageChecklistTasksAdded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageChecklistTasksAdded");
    s.store_field("checklist_message_id", checklist_message_id_);
    { s.store_vector_begin("tasks", tasks_.size()); for (const auto &_value : tasks_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messageSuggestedPostApprovalFailed::messageSuggestedPostApprovalFailed()
  : suggested_post_message_id_()
  , price_()
{}

messageSuggestedPostApprovalFailed::messageSuggestedPostApprovalFailed(int53 suggested_post_message_id_, object_ptr<SuggestedPostPrice> &&price_)
  : suggested_post_message_id_(suggested_post_message_id_)
  , price_(std::move(price_))
{}

const std::int32_t messageSuggestedPostApprovalFailed::ID;

void messageSuggestedPostApprovalFailed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSuggestedPostApprovalFailed");
    s.store_field("suggested_post_message_id", suggested_post_message_id_);
    s.store_object_field("price", static_cast<const BaseObject *>(price_.get()));
    s.store_class_end();
  }
}

messageSuggestedPostApproved::messageSuggestedPostApproved()
  : suggested_post_message_id_()
  , price_()
  , send_date_()
{}

messageSuggestedPostApproved::messageSuggestedPostApproved(int53 suggested_post_message_id_, object_ptr<SuggestedPostPrice> &&price_, int32 send_date_)
  : suggested_post_message_id_(suggested_post_message_id_)
  , price_(std::move(price_))
  , send_date_(send_date_)
{}

const std::int32_t messageSuggestedPostApproved::ID;

void messageSuggestedPostApproved::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSuggestedPostApproved");
    s.store_field("suggested_post_message_id", suggested_post_message_id_);
    s.store_object_field("price", static_cast<const BaseObject *>(price_.get()));
    s.store_field("send_date", send_date_);
    s.store_class_end();
  }
}

messageSuggestedPostDeclined::messageSuggestedPostDeclined()
  : suggested_post_message_id_()
  , comment_()
{}

messageSuggestedPostDeclined::messageSuggestedPostDeclined(int53 suggested_post_message_id_, string const &comment_)
  : suggested_post_message_id_(suggested_post_message_id_)
  , comment_(comment_)
{}

const std::int32_t messageSuggestedPostDeclined::ID;

void messageSuggestedPostDeclined::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSuggestedPostDeclined");
    s.store_field("suggested_post_message_id", suggested_post_message_id_);
    s.store_field("comment", comment_);
    s.store_class_end();
  }
}

messageSuggestedPostPaid::messageSuggestedPostPaid()
  : suggested_post_message_id_()
  , star_amount_()
  , ton_amount_()
{}

messageSuggestedPostPaid::messageSuggestedPostPaid(int53 suggested_post_message_id_, object_ptr<starAmount> &&star_amount_, int53 ton_amount_)
  : suggested_post_message_id_(suggested_post_message_id_)
  , star_amount_(std::move(star_amount_))
  , ton_amount_(ton_amount_)
{}

const std::int32_t messageSuggestedPostPaid::ID;

void messageSuggestedPostPaid::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSuggestedPostPaid");
    s.store_field("suggested_post_message_id", suggested_post_message_id_);
    s.store_object_field("star_amount", static_cast<const BaseObject *>(star_amount_.get()));
    s.store_field("ton_amount", ton_amount_);
    s.store_class_end();
  }
}

messageSuggestedPostRefunded::messageSuggestedPostRefunded()
  : suggested_post_message_id_()
  , reason_()
{}

messageSuggestedPostRefunded::messageSuggestedPostRefunded(int53 suggested_post_message_id_, object_ptr<SuggestedPostRefundReason> &&reason_)
  : suggested_post_message_id_(suggested_post_message_id_)
  , reason_(std::move(reason_))
{}

const std::int32_t messageSuggestedPostRefunded::ID;

void messageSuggestedPostRefunded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSuggestedPostRefunded");
    s.store_field("suggested_post_message_id", suggested_post_message_id_);
    s.store_object_field("reason", static_cast<const BaseObject *>(reason_.get()));
    s.store_class_end();
  }
}

messageContactRegistered::messageContactRegistered() {
}

const std::int32_t messageContactRegistered::ID;

void messageContactRegistered::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageContactRegistered");
    s.store_class_end();
  }
}

messageUsersShared::messageUsersShared()
  : users_()
  , button_id_()
{}

messageUsersShared::messageUsersShared(array<object_ptr<sharedUser>> &&users_, int32 button_id_)
  : users_(std::move(users_))
  , button_id_(button_id_)
{}

const std::int32_t messageUsersShared::ID;

void messageUsersShared::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageUsersShared");
    { s.store_vector_begin("users", users_.size()); for (const auto &_value : users_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("button_id", button_id_);
    s.store_class_end();
  }
}

messageChatShared::messageChatShared()
  : chat_()
  , button_id_()
{}

messageChatShared::messageChatShared(object_ptr<sharedChat> &&chat_, int32 button_id_)
  : chat_(std::move(chat_))
  , button_id_(button_id_)
{}

const std::int32_t messageChatShared::ID;

void messageChatShared::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageChatShared");
    s.store_object_field("chat", static_cast<const BaseObject *>(chat_.get()));
    s.store_field("button_id", button_id_);
    s.store_class_end();
  }
}

messageBotWriteAccessAllowed::messageBotWriteAccessAllowed()
  : reason_()
{}

messageBotWriteAccessAllowed::messageBotWriteAccessAllowed(object_ptr<BotWriteAccessAllowReason> &&reason_)
  : reason_(std::move(reason_))
{}

const std::int32_t messageBotWriteAccessAllowed::ID;

void messageBotWriteAccessAllowed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageBotWriteAccessAllowed");
    s.store_object_field("reason", static_cast<const BaseObject *>(reason_.get()));
    s.store_class_end();
  }
}

messageWebAppDataSent::messageWebAppDataSent()
  : button_text_()
{}

messageWebAppDataSent::messageWebAppDataSent(string const &button_text_)
  : button_text_(button_text_)
{}

const std::int32_t messageWebAppDataSent::ID;

void messageWebAppDataSent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageWebAppDataSent");
    s.store_field("button_text", button_text_);
    s.store_class_end();
  }
}

messageWebAppDataReceived::messageWebAppDataReceived()
  : button_text_()
  , data_()
{}

messageWebAppDataReceived::messageWebAppDataReceived(string const &button_text_, string const &data_)
  : button_text_(button_text_)
  , data_(data_)
{}

const std::int32_t messageWebAppDataReceived::ID;

void messageWebAppDataReceived::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageWebAppDataReceived");
    s.store_field("button_text", button_text_);
    s.store_field("data", data_);
    s.store_class_end();
  }
}

messagePassportDataSent::messagePassportDataSent()
  : types_()
{}

messagePassportDataSent::messagePassportDataSent(array<object_ptr<PassportElementType>> &&types_)
  : types_(std::move(types_))
{}

const std::int32_t messagePassportDataSent::ID;

void messagePassportDataSent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messagePassportDataSent");
    { s.store_vector_begin("types", types_.size()); for (const auto &_value : types_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messagePassportDataReceived::messagePassportDataReceived()
  : elements_()
  , credentials_()
{}

messagePassportDataReceived::messagePassportDataReceived(array<object_ptr<encryptedPassportElement>> &&elements_, object_ptr<encryptedCredentials> &&credentials_)
  : elements_(std::move(elements_))
  , credentials_(std::move(credentials_))
{}

const std::int32_t messagePassportDataReceived::ID;

void messagePassportDataReceived::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messagePassportDataReceived");
    { s.store_vector_begin("elements", elements_.size()); for (const auto &_value : elements_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("credentials", static_cast<const BaseObject *>(credentials_.get()));
    s.store_class_end();
  }
}

messageProximityAlertTriggered::messageProximityAlertTriggered()
  : traveler_id_()
  , watcher_id_()
  , distance_()
{}

messageProximityAlertTriggered::messageProximityAlertTriggered(object_ptr<MessageSender> &&traveler_id_, object_ptr<MessageSender> &&watcher_id_, int32 distance_)
  : traveler_id_(std::move(traveler_id_))
  , watcher_id_(std::move(watcher_id_))
  , distance_(distance_)
{}

const std::int32_t messageProximityAlertTriggered::ID;

void messageProximityAlertTriggered::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageProximityAlertTriggered");
    s.store_object_field("traveler_id", static_cast<const BaseObject *>(traveler_id_.get()));
    s.store_object_field("watcher_id", static_cast<const BaseObject *>(watcher_id_.get()));
    s.store_field("distance", distance_);
    s.store_class_end();
  }
}

messageUnsupported::messageUnsupported() {
}

const std::int32_t messageUnsupported::ID;

void messageUnsupported::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageUnsupported");
    s.store_class_end();
  }
}

messageCopyOptions::messageCopyOptions()
  : send_copy_()
  , replace_caption_()
  , new_caption_()
  , new_show_caption_above_media_()
{}

messageCopyOptions::messageCopyOptions(bool send_copy_, bool replace_caption_, object_ptr<formattedText> &&new_caption_, bool new_show_caption_above_media_)
  : send_copy_(send_copy_)
  , replace_caption_(replace_caption_)
  , new_caption_(std::move(new_caption_))
  , new_show_caption_above_media_(new_show_caption_above_media_)
{}

const std::int32_t messageCopyOptions::ID;

void messageCopyOptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageCopyOptions");
    s.store_field("send_copy", send_copy_);
    s.store_field("replace_caption", replace_caption_);
    s.store_object_field("new_caption", static_cast<const BaseObject *>(new_caption_.get()));
    s.store_field("new_show_caption_above_media", new_show_caption_above_media_);
    s.store_class_end();
  }
}

messageEffectTypeEmojiReaction::messageEffectTypeEmojiReaction()
  : select_animation_()
  , effect_animation_()
{}

messageEffectTypeEmojiReaction::messageEffectTypeEmojiReaction(object_ptr<sticker> &&select_animation_, object_ptr<sticker> &&effect_animation_)
  : select_animation_(std::move(select_animation_))
  , effect_animation_(std::move(effect_animation_))
{}

const std::int32_t messageEffectTypeEmojiReaction::ID;

void messageEffectTypeEmojiReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEffectTypeEmojiReaction");
    s.store_object_field("select_animation", static_cast<const BaseObject *>(select_animation_.get()));
    s.store_object_field("effect_animation", static_cast<const BaseObject *>(effect_animation_.get()));
    s.store_class_end();
  }
}

messageEffectTypePremiumSticker::messageEffectTypePremiumSticker()
  : sticker_()
{}

messageEffectTypePremiumSticker::messageEffectTypePremiumSticker(object_ptr<sticker> &&sticker_)
  : sticker_(std::move(sticker_))
{}

const std::int32_t messageEffectTypePremiumSticker::ID;

void messageEffectTypePremiumSticker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageEffectTypePremiumSticker");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_class_end();
  }
}

messageSchedulingStateSendAtDate::messageSchedulingStateSendAtDate()
  : send_date_()
{}

messageSchedulingStateSendAtDate::messageSchedulingStateSendAtDate(int32 send_date_)
  : send_date_(send_date_)
{}

const std::int32_t messageSchedulingStateSendAtDate::ID;

void messageSchedulingStateSendAtDate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSchedulingStateSendAtDate");
    s.store_field("send_date", send_date_);
    s.store_class_end();
  }
}

messageSchedulingStateSendWhenOnline::messageSchedulingStateSendWhenOnline() {
}

const std::int32_t messageSchedulingStateSendWhenOnline::ID;

void messageSchedulingStateSendWhenOnline::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSchedulingStateSendWhenOnline");
    s.store_class_end();
  }
}

messageSchedulingStateSendWhenVideoProcessed::messageSchedulingStateSendWhenVideoProcessed()
  : send_date_()
{}

messageSchedulingStateSendWhenVideoProcessed::messageSchedulingStateSendWhenVideoProcessed(int32 send_date_)
  : send_date_(send_date_)
{}

const std::int32_t messageSchedulingStateSendWhenVideoProcessed::ID;

void messageSchedulingStateSendWhenVideoProcessed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageSchedulingStateSendWhenVideoProcessed");
    s.store_field("send_date", send_date_);
    s.store_class_end();
  }
}

networkTypeNone::networkTypeNone() {
}

const std::int32_t networkTypeNone::ID;

void networkTypeNone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "networkTypeNone");
    s.store_class_end();
  }
}

networkTypeMobile::networkTypeMobile() {
}

const std::int32_t networkTypeMobile::ID;

void networkTypeMobile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "networkTypeMobile");
    s.store_class_end();
  }
}

networkTypeMobileRoaming::networkTypeMobileRoaming() {
}

const std::int32_t networkTypeMobileRoaming::ID;

void networkTypeMobileRoaming::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "networkTypeMobileRoaming");
    s.store_class_end();
  }
}

networkTypeWiFi::networkTypeWiFi() {
}

const std::int32_t networkTypeWiFi::ID;

void networkTypeWiFi::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "networkTypeWiFi");
    s.store_class_end();
  }
}

networkTypeOther::networkTypeOther() {
}

const std::int32_t networkTypeOther::ID;

void networkTypeOther::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "networkTypeOther");
    s.store_class_end();
  }
}

pageBlockTitle::pageBlockTitle()
  : title_()
{}

pageBlockTitle::pageBlockTitle(object_ptr<RichText> &&title_)
  : title_(std::move(title_))
{}

const std::int32_t pageBlockTitle::ID;

void pageBlockTitle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockTitle");
    s.store_object_field("title", static_cast<const BaseObject *>(title_.get()));
    s.store_class_end();
  }
}

pageBlockSubtitle::pageBlockSubtitle()
  : subtitle_()
{}

pageBlockSubtitle::pageBlockSubtitle(object_ptr<RichText> &&subtitle_)
  : subtitle_(std::move(subtitle_))
{}

const std::int32_t pageBlockSubtitle::ID;

void pageBlockSubtitle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockSubtitle");
    s.store_object_field("subtitle", static_cast<const BaseObject *>(subtitle_.get()));
    s.store_class_end();
  }
}

pageBlockAuthorDate::pageBlockAuthorDate()
  : author_()
  , publish_date_()
{}

pageBlockAuthorDate::pageBlockAuthorDate(object_ptr<RichText> &&author_, int32 publish_date_)
  : author_(std::move(author_))
  , publish_date_(publish_date_)
{}

const std::int32_t pageBlockAuthorDate::ID;

void pageBlockAuthorDate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockAuthorDate");
    s.store_object_field("author", static_cast<const BaseObject *>(author_.get()));
    s.store_field("publish_date", publish_date_);
    s.store_class_end();
  }
}

pageBlockHeader::pageBlockHeader()
  : header_()
{}

pageBlockHeader::pageBlockHeader(object_ptr<RichText> &&header_)
  : header_(std::move(header_))
{}

const std::int32_t pageBlockHeader::ID;

void pageBlockHeader::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockHeader");
    s.store_object_field("header", static_cast<const BaseObject *>(header_.get()));
    s.store_class_end();
  }
}

pageBlockSubheader::pageBlockSubheader()
  : subheader_()
{}

pageBlockSubheader::pageBlockSubheader(object_ptr<RichText> &&subheader_)
  : subheader_(std::move(subheader_))
{}

const std::int32_t pageBlockSubheader::ID;

void pageBlockSubheader::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockSubheader");
    s.store_object_field("subheader", static_cast<const BaseObject *>(subheader_.get()));
    s.store_class_end();
  }
}

pageBlockKicker::pageBlockKicker()
  : kicker_()
{}

pageBlockKicker::pageBlockKicker(object_ptr<RichText> &&kicker_)
  : kicker_(std::move(kicker_))
{}

const std::int32_t pageBlockKicker::ID;

void pageBlockKicker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockKicker");
    s.store_object_field("kicker", static_cast<const BaseObject *>(kicker_.get()));
    s.store_class_end();
  }
}

pageBlockParagraph::pageBlockParagraph()
  : text_()
{}

pageBlockParagraph::pageBlockParagraph(object_ptr<RichText> &&text_)
  : text_(std::move(text_))
{}

const std::int32_t pageBlockParagraph::ID;

void pageBlockParagraph::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockParagraph");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

pageBlockPreformatted::pageBlockPreformatted()
  : text_()
  , language_()
{}

pageBlockPreformatted::pageBlockPreformatted(object_ptr<RichText> &&text_, string const &language_)
  : text_(std::move(text_))
  , language_(language_)
{}

const std::int32_t pageBlockPreformatted::ID;

void pageBlockPreformatted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockPreformatted");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("language", language_);
    s.store_class_end();
  }
}

pageBlockFooter::pageBlockFooter()
  : footer_()
{}

pageBlockFooter::pageBlockFooter(object_ptr<RichText> &&footer_)
  : footer_(std::move(footer_))
{}

const std::int32_t pageBlockFooter::ID;

void pageBlockFooter::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockFooter");
    s.store_object_field("footer", static_cast<const BaseObject *>(footer_.get()));
    s.store_class_end();
  }
}

pageBlockDivider::pageBlockDivider() {
}

const std::int32_t pageBlockDivider::ID;

void pageBlockDivider::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockDivider");
    s.store_class_end();
  }
}

pageBlockAnchor::pageBlockAnchor()
  : name_()
{}

pageBlockAnchor::pageBlockAnchor(string const &name_)
  : name_(name_)
{}

const std::int32_t pageBlockAnchor::ID;

void pageBlockAnchor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockAnchor");
    s.store_field("name", name_);
    s.store_class_end();
  }
}

pageBlockList::pageBlockList()
  : items_()
{}

pageBlockList::pageBlockList(array<object_ptr<pageBlockListItem>> &&items_)
  : items_(std::move(items_))
{}

const std::int32_t pageBlockList::ID;

void pageBlockList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockList");
    { s.store_vector_begin("items", items_.size()); for (const auto &_value : items_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

pageBlockBlockQuote::pageBlockBlockQuote()
  : text_()
  , credit_()
{}

pageBlockBlockQuote::pageBlockBlockQuote(object_ptr<RichText> &&text_, object_ptr<RichText> &&credit_)
  : text_(std::move(text_))
  , credit_(std::move(credit_))
{}

const std::int32_t pageBlockBlockQuote::ID;

void pageBlockBlockQuote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockBlockQuote");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_object_field("credit", static_cast<const BaseObject *>(credit_.get()));
    s.store_class_end();
  }
}

pageBlockPullQuote::pageBlockPullQuote()
  : text_()
  , credit_()
{}

pageBlockPullQuote::pageBlockPullQuote(object_ptr<RichText> &&text_, object_ptr<RichText> &&credit_)
  : text_(std::move(text_))
  , credit_(std::move(credit_))
{}

const std::int32_t pageBlockPullQuote::ID;

void pageBlockPullQuote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockPullQuote");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_object_field("credit", static_cast<const BaseObject *>(credit_.get()));
    s.store_class_end();
  }
}

pageBlockAnimation::pageBlockAnimation()
  : animation_()
  , caption_()
  , need_autoplay_()
{}

pageBlockAnimation::pageBlockAnimation(object_ptr<animation> &&animation_, object_ptr<pageBlockCaption> &&caption_, bool need_autoplay_)
  : animation_(std::move(animation_))
  , caption_(std::move(caption_))
  , need_autoplay_(need_autoplay_)
{}

const std::int32_t pageBlockAnimation::ID;

void pageBlockAnimation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockAnimation");
    s.store_object_field("animation", static_cast<const BaseObject *>(animation_.get()));
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_field("need_autoplay", need_autoplay_);
    s.store_class_end();
  }
}

pageBlockAudio::pageBlockAudio()
  : audio_()
  , caption_()
{}

pageBlockAudio::pageBlockAudio(object_ptr<audio> &&audio_, object_ptr<pageBlockCaption> &&caption_)
  : audio_(std::move(audio_))
  , caption_(std::move(caption_))
{}

const std::int32_t pageBlockAudio::ID;

void pageBlockAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockAudio");
    s.store_object_field("audio", static_cast<const BaseObject *>(audio_.get()));
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

pageBlockPhoto::pageBlockPhoto()
  : photo_()
  , caption_()
  , url_()
{}

pageBlockPhoto::pageBlockPhoto(object_ptr<photo> &&photo_, object_ptr<pageBlockCaption> &&caption_, string const &url_)
  : photo_(std::move(photo_))
  , caption_(std::move(caption_))
  , url_(url_)
{}

const std::int32_t pageBlockPhoto::ID;

void pageBlockPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockPhoto");
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_field("url", url_);
    s.store_class_end();
  }
}

pageBlockVideo::pageBlockVideo()
  : video_()
  , caption_()
  , need_autoplay_()
  , is_looped_()
{}

pageBlockVideo::pageBlockVideo(object_ptr<video> &&video_, object_ptr<pageBlockCaption> &&caption_, bool need_autoplay_, bool is_looped_)
  : video_(std::move(video_))
  , caption_(std::move(caption_))
  , need_autoplay_(need_autoplay_)
  , is_looped_(is_looped_)
{}

const std::int32_t pageBlockVideo::ID;

void pageBlockVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockVideo");
    s.store_object_field("video", static_cast<const BaseObject *>(video_.get()));
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_field("need_autoplay", need_autoplay_);
    s.store_field("is_looped", is_looped_);
    s.store_class_end();
  }
}

pageBlockVoiceNote::pageBlockVoiceNote()
  : voice_note_()
  , caption_()
{}

pageBlockVoiceNote::pageBlockVoiceNote(object_ptr<voiceNote> &&voice_note_, object_ptr<pageBlockCaption> &&caption_)
  : voice_note_(std::move(voice_note_))
  , caption_(std::move(caption_))
{}

const std::int32_t pageBlockVoiceNote::ID;

void pageBlockVoiceNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockVoiceNote");
    s.store_object_field("voice_note", static_cast<const BaseObject *>(voice_note_.get()));
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

pageBlockCover::pageBlockCover()
  : cover_()
{}

pageBlockCover::pageBlockCover(object_ptr<PageBlock> &&cover_)
  : cover_(std::move(cover_))
{}

const std::int32_t pageBlockCover::ID;

void pageBlockCover::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockCover");
    s.store_object_field("cover", static_cast<const BaseObject *>(cover_.get()));
    s.store_class_end();
  }
}

pageBlockEmbedded::pageBlockEmbedded()
  : url_()
  , html_()
  , poster_photo_()
  , width_()
  , height_()
  , caption_()
  , is_full_width_()
  , allow_scrolling_()
{}

pageBlockEmbedded::pageBlockEmbedded(string const &url_, string const &html_, object_ptr<photo> &&poster_photo_, int32 width_, int32 height_, object_ptr<pageBlockCaption> &&caption_, bool is_full_width_, bool allow_scrolling_)
  : url_(url_)
  , html_(html_)
  , poster_photo_(std::move(poster_photo_))
  , width_(width_)
  , height_(height_)
  , caption_(std::move(caption_))
  , is_full_width_(is_full_width_)
  , allow_scrolling_(allow_scrolling_)
{}

const std::int32_t pageBlockEmbedded::ID;

void pageBlockEmbedded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockEmbedded");
    s.store_field("url", url_);
    s.store_field("html", html_);
    s.store_object_field("poster_photo", static_cast<const BaseObject *>(poster_photo_.get()));
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_field("is_full_width", is_full_width_);
    s.store_field("allow_scrolling", allow_scrolling_);
    s.store_class_end();
  }
}

pageBlockEmbeddedPost::pageBlockEmbeddedPost()
  : url_()
  , author_()
  , author_photo_()
  , date_()
  , page_blocks_()
  , caption_()
{}

pageBlockEmbeddedPost::pageBlockEmbeddedPost(string const &url_, string const &author_, object_ptr<photo> &&author_photo_, int32 date_, array<object_ptr<PageBlock>> &&page_blocks_, object_ptr<pageBlockCaption> &&caption_)
  : url_(url_)
  , author_(author_)
  , author_photo_(std::move(author_photo_))
  , date_(date_)
  , page_blocks_(std::move(page_blocks_))
  , caption_(std::move(caption_))
{}

const std::int32_t pageBlockEmbeddedPost::ID;

void pageBlockEmbeddedPost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockEmbeddedPost");
    s.store_field("url", url_);
    s.store_field("author", author_);
    s.store_object_field("author_photo", static_cast<const BaseObject *>(author_photo_.get()));
    s.store_field("date", date_);
    { s.store_vector_begin("page_blocks", page_blocks_.size()); for (const auto &_value : page_blocks_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

pageBlockCollage::pageBlockCollage()
  : page_blocks_()
  , caption_()
{}

pageBlockCollage::pageBlockCollage(array<object_ptr<PageBlock>> &&page_blocks_, object_ptr<pageBlockCaption> &&caption_)
  : page_blocks_(std::move(page_blocks_))
  , caption_(std::move(caption_))
{}

const std::int32_t pageBlockCollage::ID;

void pageBlockCollage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockCollage");
    { s.store_vector_begin("page_blocks", page_blocks_.size()); for (const auto &_value : page_blocks_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

pageBlockSlideshow::pageBlockSlideshow()
  : page_blocks_()
  , caption_()
{}

pageBlockSlideshow::pageBlockSlideshow(array<object_ptr<PageBlock>> &&page_blocks_, object_ptr<pageBlockCaption> &&caption_)
  : page_blocks_(std::move(page_blocks_))
  , caption_(std::move(caption_))
{}

const std::int32_t pageBlockSlideshow::ID;

void pageBlockSlideshow::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockSlideshow");
    { s.store_vector_begin("page_blocks", page_blocks_.size()); for (const auto &_value : page_blocks_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

pageBlockChatLink::pageBlockChatLink()
  : title_()
  , photo_()
  , accent_color_id_()
  , username_()
{}

pageBlockChatLink::pageBlockChatLink(string const &title_, object_ptr<chatPhotoInfo> &&photo_, int32 accent_color_id_, string const &username_)
  : title_(title_)
  , photo_(std::move(photo_))
  , accent_color_id_(accent_color_id_)
  , username_(username_)
{}

const std::int32_t pageBlockChatLink::ID;

void pageBlockChatLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockChatLink");
    s.store_field("title", title_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_field("accent_color_id", accent_color_id_);
    s.store_field("username", username_);
    s.store_class_end();
  }
}

pageBlockTable::pageBlockTable()
  : caption_()
  , cells_()
  , is_bordered_()
  , is_striped_()
{}

pageBlockTable::pageBlockTable(object_ptr<RichText> &&caption_, array<array<object_ptr<pageBlockTableCell>>> &&cells_, bool is_bordered_, bool is_striped_)
  : caption_(std::move(caption_))
  , cells_(std::move(cells_))
  , is_bordered_(is_bordered_)
  , is_striped_(is_striped_)
{}

const std::int32_t pageBlockTable::ID;

void pageBlockTable::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockTable");
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    { s.store_vector_begin("cells", cells_.size()); for (const auto &_value : cells_) { { s.store_vector_begin("", _value.size()); for (const auto &_value2 : _value) { s.store_object_field("", static_cast<const BaseObject *>(_value2.get())); } s.store_class_end(); } } s.store_class_end(); }
    s.store_field("is_bordered", is_bordered_);
    s.store_field("is_striped", is_striped_);
    s.store_class_end();
  }
}

pageBlockDetails::pageBlockDetails()
  : header_()
  , page_blocks_()
  , is_open_()
{}

pageBlockDetails::pageBlockDetails(object_ptr<RichText> &&header_, array<object_ptr<PageBlock>> &&page_blocks_, bool is_open_)
  : header_(std::move(header_))
  , page_blocks_(std::move(page_blocks_))
  , is_open_(is_open_)
{}

const std::int32_t pageBlockDetails::ID;

void pageBlockDetails::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockDetails");
    s.store_object_field("header", static_cast<const BaseObject *>(header_.get()));
    { s.store_vector_begin("page_blocks", page_blocks_.size()); for (const auto &_value : page_blocks_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("is_open", is_open_);
    s.store_class_end();
  }
}

pageBlockRelatedArticles::pageBlockRelatedArticles()
  : header_()
  , articles_()
{}

pageBlockRelatedArticles::pageBlockRelatedArticles(object_ptr<RichText> &&header_, array<object_ptr<pageBlockRelatedArticle>> &&articles_)
  : header_(std::move(header_))
  , articles_(std::move(articles_))
{}

const std::int32_t pageBlockRelatedArticles::ID;

void pageBlockRelatedArticles::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockRelatedArticles");
    s.store_object_field("header", static_cast<const BaseObject *>(header_.get()));
    { s.store_vector_begin("articles", articles_.size()); for (const auto &_value : articles_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

pageBlockMap::pageBlockMap()
  : location_()
  , zoom_()
  , width_()
  , height_()
  , caption_()
{}

pageBlockMap::pageBlockMap(object_ptr<location> &&location_, int32 zoom_, int32 width_, int32 height_, object_ptr<pageBlockCaption> &&caption_)
  : location_(std::move(location_))
  , zoom_(zoom_)
  , width_(width_)
  , height_(height_)
  , caption_(std::move(caption_))
{}

const std::int32_t pageBlockMap::ID;

void pageBlockMap::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pageBlockMap");
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_field("zoom", zoom_);
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_object_field("caption", static_cast<const BaseObject *>(caption_.get()));
    s.store_class_end();
  }
}

paidReactor::paidReactor()
  : sender_id_()
  , star_count_()
  , is_top_()
  , is_me_()
  , is_anonymous_()
{}

paidReactor::paidReactor(object_ptr<MessageSender> &&sender_id_, int32 star_count_, bool is_top_, bool is_me_, bool is_anonymous_)
  : sender_id_(std::move(sender_id_))
  , star_count_(star_count_)
  , is_top_(is_top_)
  , is_me_(is_me_)
  , is_anonymous_(is_anonymous_)
{}

const std::int32_t paidReactor::ID;

void paidReactor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paidReactor");
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_field("star_count", star_count_);
    s.store_field("is_top", is_top_);
    s.store_field("is_me", is_me_);
    s.store_field("is_anonymous", is_anonymous_);
    s.store_class_end();
  }
}

passportElementsWithErrors::passportElementsWithErrors()
  : elements_()
  , errors_()
{}

passportElementsWithErrors::passportElementsWithErrors(array<object_ptr<PassportElement>> &&elements_, array<object_ptr<passportElementError>> &&errors_)
  : elements_(std::move(elements_))
  , errors_(std::move(errors_))
{}

const std::int32_t passportElementsWithErrors::ID;

void passportElementsWithErrors::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementsWithErrors");
    { s.store_vector_begin("elements", elements_.size()); for (const auto &_value : elements_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("errors", errors_.size()); for (const auto &_value : errors_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

paymentForm::paymentForm()
  : id_()
  , type_()
  , seller_bot_user_id_()
  , product_info_()
{}

paymentForm::paymentForm(int64 id_, object_ptr<PaymentFormType> &&type_, int53 seller_bot_user_id_, object_ptr<productInfo> &&product_info_)
  : id_(id_)
  , type_(std::move(type_))
  , seller_bot_user_id_(seller_bot_user_id_)
  , product_info_(std::move(product_info_))
{}

const std::int32_t paymentForm::ID;

void paymentForm::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paymentForm");
    s.store_field("id", id_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("seller_bot_user_id", seller_bot_user_id_);
    s.store_object_field("product_info", static_cast<const BaseObject *>(product_info_.get()));
    s.store_class_end();
  }
}

paymentFormTypeRegular::paymentFormTypeRegular()
  : invoice_()
  , payment_provider_user_id_()
  , payment_provider_()
  , additional_payment_options_()
  , saved_order_info_()
  , saved_credentials_()
  , can_save_credentials_()
  , need_password_()
{}

paymentFormTypeRegular::paymentFormTypeRegular(object_ptr<invoice> &&invoice_, int53 payment_provider_user_id_, object_ptr<PaymentProvider> &&payment_provider_, array<object_ptr<paymentOption>> &&additional_payment_options_, object_ptr<orderInfo> &&saved_order_info_, array<object_ptr<savedCredentials>> &&saved_credentials_, bool can_save_credentials_, bool need_password_)
  : invoice_(std::move(invoice_))
  , payment_provider_user_id_(payment_provider_user_id_)
  , payment_provider_(std::move(payment_provider_))
  , additional_payment_options_(std::move(additional_payment_options_))
  , saved_order_info_(std::move(saved_order_info_))
  , saved_credentials_(std::move(saved_credentials_))
  , can_save_credentials_(can_save_credentials_)
  , need_password_(need_password_)
{}

const std::int32_t paymentFormTypeRegular::ID;

void paymentFormTypeRegular::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paymentFormTypeRegular");
    s.store_object_field("invoice", static_cast<const BaseObject *>(invoice_.get()));
    s.store_field("payment_provider_user_id", payment_provider_user_id_);
    s.store_object_field("payment_provider", static_cast<const BaseObject *>(payment_provider_.get()));
    { s.store_vector_begin("additional_payment_options", additional_payment_options_.size()); for (const auto &_value : additional_payment_options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_object_field("saved_order_info", static_cast<const BaseObject *>(saved_order_info_.get()));
    { s.store_vector_begin("saved_credentials", saved_credentials_.size()); for (const auto &_value : saved_credentials_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("can_save_credentials", can_save_credentials_);
    s.store_field("need_password", need_password_);
    s.store_class_end();
  }
}

paymentFormTypeStars::paymentFormTypeStars()
  : star_count_()
{}

paymentFormTypeStars::paymentFormTypeStars(int53 star_count_)
  : star_count_(star_count_)
{}

const std::int32_t paymentFormTypeStars::ID;

void paymentFormTypeStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paymentFormTypeStars");
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

paymentFormTypeStarSubscription::paymentFormTypeStarSubscription()
  : pricing_()
{}

paymentFormTypeStarSubscription::paymentFormTypeStarSubscription(object_ptr<starSubscriptionPricing> &&pricing_)
  : pricing_(std::move(pricing_))
{}

const std::int32_t paymentFormTypeStarSubscription::ID;

void paymentFormTypeStarSubscription::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paymentFormTypeStarSubscription");
    s.store_object_field("pricing", static_cast<const BaseObject *>(pricing_.get()));
    s.store_class_end();
  }
}

personalDetails::personalDetails()
  : first_name_()
  , middle_name_()
  , last_name_()
  , native_first_name_()
  , native_middle_name_()
  , native_last_name_()
  , birthdate_()
  , gender_()
  , country_code_()
  , residence_country_code_()
{}

personalDetails::personalDetails(string const &first_name_, string const &middle_name_, string const &last_name_, string const &native_first_name_, string const &native_middle_name_, string const &native_last_name_, object_ptr<date> &&birthdate_, string const &gender_, string const &country_code_, string const &residence_country_code_)
  : first_name_(first_name_)
  , middle_name_(middle_name_)
  , last_name_(last_name_)
  , native_first_name_(native_first_name_)
  , native_middle_name_(native_middle_name_)
  , native_last_name_(native_last_name_)
  , birthdate_(std::move(birthdate_))
  , gender_(gender_)
  , country_code_(country_code_)
  , residence_country_code_(residence_country_code_)
{}

const std::int32_t personalDetails::ID;

void personalDetails::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "personalDetails");
    s.store_field("first_name", first_name_);
    s.store_field("middle_name", middle_name_);
    s.store_field("last_name", last_name_);
    s.store_field("native_first_name", native_first_name_);
    s.store_field("native_middle_name", native_middle_name_);
    s.store_field("native_last_name", native_last_name_);
    s.store_object_field("birthdate", static_cast<const BaseObject *>(birthdate_.get()));
    s.store_field("gender", gender_);
    s.store_field("country_code", country_code_);
    s.store_field("residence_country_code", residence_country_code_);
    s.store_class_end();
  }
}

phoneNumberAuthenticationSettings::phoneNumberAuthenticationSettings()
  : allow_flash_call_()
  , allow_missed_call_()
  , is_current_phone_number_()
  , has_unknown_phone_number_()
  , allow_sms_retriever_api_()
  , firebase_authentication_settings_()
  , authentication_tokens_()
{}

phoneNumberAuthenticationSettings::phoneNumberAuthenticationSettings(bool allow_flash_call_, bool allow_missed_call_, bool is_current_phone_number_, bool has_unknown_phone_number_, bool allow_sms_retriever_api_, object_ptr<FirebaseAuthenticationSettings> &&firebase_authentication_settings_, array<string> &&authentication_tokens_)
  : allow_flash_call_(allow_flash_call_)
  , allow_missed_call_(allow_missed_call_)
  , is_current_phone_number_(is_current_phone_number_)
  , has_unknown_phone_number_(has_unknown_phone_number_)
  , allow_sms_retriever_api_(allow_sms_retriever_api_)
  , firebase_authentication_settings_(std::move(firebase_authentication_settings_))
  , authentication_tokens_(std::move(authentication_tokens_))
{}

const std::int32_t phoneNumberAuthenticationSettings::ID;

void phoneNumberAuthenticationSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phoneNumberAuthenticationSettings");
    s.store_field("allow_flash_call", allow_flash_call_);
    s.store_field("allow_missed_call", allow_missed_call_);
    s.store_field("is_current_phone_number", is_current_phone_number_);
    s.store_field("has_unknown_phone_number", has_unknown_phone_number_);
    s.store_field("allow_sms_retriever_api", allow_sms_retriever_api_);
    s.store_object_field("firebase_authentication_settings", static_cast<const BaseObject *>(firebase_authentication_settings_.get()));
    { s.store_vector_begin("authentication_tokens", authentication_tokens_.size()); for (const auto &_value : authentication_tokens_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

phoneNumberCodeTypeChange::phoneNumberCodeTypeChange() {
}

const std::int32_t phoneNumberCodeTypeChange::ID;

void phoneNumberCodeTypeChange::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phoneNumberCodeTypeChange");
    s.store_class_end();
  }
}

phoneNumberCodeTypeVerify::phoneNumberCodeTypeVerify() {
}

const std::int32_t phoneNumberCodeTypeVerify::ID;

void phoneNumberCodeTypeVerify::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phoneNumberCodeTypeVerify");
    s.store_class_end();
  }
}

phoneNumberCodeTypeConfirmOwnership::phoneNumberCodeTypeConfirmOwnership()
  : hash_()
{}

phoneNumberCodeTypeConfirmOwnership::phoneNumberCodeTypeConfirmOwnership(string const &hash_)
  : hash_(hash_)
{}

const std::int32_t phoneNumberCodeTypeConfirmOwnership::ID;

void phoneNumberCodeTypeConfirmOwnership::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "phoneNumberCodeTypeConfirmOwnership");
    s.store_field("hash", hash_);
    s.store_class_end();
  }
}

premiumPaymentOption::premiumPaymentOption()
  : currency_()
  , amount_()
  , discount_percentage_()
  , month_count_()
  , store_product_id_()
  , payment_link_()
{}

premiumPaymentOption::premiumPaymentOption(string const &currency_, int53 amount_, int32 discount_percentage_, int32 month_count_, string const &store_product_id_, object_ptr<InternalLinkType> &&payment_link_)
  : currency_(currency_)
  , amount_(amount_)
  , discount_percentage_(discount_percentage_)
  , month_count_(month_count_)
  , store_product_id_(store_product_id_)
  , payment_link_(std::move(payment_link_))
{}

const std::int32_t premiumPaymentOption::ID;

void premiumPaymentOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumPaymentOption");
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("discount_percentage", discount_percentage_);
    s.store_field("month_count", month_count_);
    s.store_field("store_product_id", store_product_id_);
    s.store_object_field("payment_link", static_cast<const BaseObject *>(payment_link_.get()));
    s.store_class_end();
  }
}

premiumState::premiumState()
  : state_()
  , payment_options_()
  , animations_()
  , business_animations_()
{}

premiumState::premiumState(object_ptr<formattedText> &&state_, array<object_ptr<premiumStatePaymentOption>> &&payment_options_, array<object_ptr<premiumFeaturePromotionAnimation>> &&animations_, array<object_ptr<businessFeaturePromotionAnimation>> &&business_animations_)
  : state_(std::move(state_))
  , payment_options_(std::move(payment_options_))
  , animations_(std::move(animations_))
  , business_animations_(std::move(business_animations_))
{}

const std::int32_t premiumState::ID;

void premiumState::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "premiumState");
    s.store_object_field("state", static_cast<const BaseObject *>(state_.get()));
    { s.store_vector_begin("payment_options", payment_options_.size()); for (const auto &_value : payment_options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("animations", animations_.size()); for (const auto &_value : animations_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("business_animations", business_animations_.size()); for (const auto &_value : business_animations_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

preparedInlineMessageId::preparedInlineMessageId()
  : id_()
  , expiration_date_()
{}

preparedInlineMessageId::preparedInlineMessageId(string const &id_, int32 expiration_date_)
  : id_(id_)
  , expiration_date_(expiration_date_)
{}

const std::int32_t preparedInlineMessageId::ID;

void preparedInlineMessageId::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "preparedInlineMessageId");
    s.store_field("id", id_);
    s.store_field("expiration_date", expiration_date_);
    s.store_class_end();
  }
}

proxy::proxy()
  : id_()
  , server_()
  , port_()
  , last_used_date_()
  , is_enabled_()
  , type_()
{}

proxy::proxy(int32 id_, string const &server_, int32 port_, int32 last_used_date_, bool is_enabled_, object_ptr<ProxyType> &&type_)
  : id_(id_)
  , server_(server_)
  , port_(port_)
  , last_used_date_(last_used_date_)
  , is_enabled_(is_enabled_)
  , type_(std::move(type_))
{}

const std::int32_t proxy::ID;

void proxy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "proxy");
    s.store_field("id", id_);
    s.store_field("server", server_);
    s.store_field("port", port_);
    s.store_field("last_used_date", last_used_date_);
    s.store_field("is_enabled", is_enabled_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

receivedGifts::receivedGifts()
  : total_count_()
  , gifts_()
  , are_notifications_enabled_()
  , next_offset_()
{}

receivedGifts::receivedGifts(int32 total_count_, array<object_ptr<receivedGift>> &&gifts_, bool are_notifications_enabled_, string const &next_offset_)
  : total_count_(total_count_)
  , gifts_(std::move(gifts_))
  , are_notifications_enabled_(are_notifications_enabled_)
  , next_offset_(next_offset_)
{}

const std::int32_t receivedGifts::ID;

void receivedGifts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "receivedGifts");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("gifts", gifts_.size()); for (const auto &_value : gifts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("are_notifications_enabled", are_notifications_enabled_);
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

reportReasonSpam::reportReasonSpam() {
}

const std::int32_t reportReasonSpam::ID;

void reportReasonSpam::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportReasonSpam");
    s.store_class_end();
  }
}

reportReasonViolence::reportReasonViolence() {
}

const std::int32_t reportReasonViolence::ID;

void reportReasonViolence::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportReasonViolence");
    s.store_class_end();
  }
}

reportReasonPornography::reportReasonPornography() {
}

const std::int32_t reportReasonPornography::ID;

void reportReasonPornography::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportReasonPornography");
    s.store_class_end();
  }
}

reportReasonChildAbuse::reportReasonChildAbuse() {
}

const std::int32_t reportReasonChildAbuse::ID;

void reportReasonChildAbuse::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportReasonChildAbuse");
    s.store_class_end();
  }
}

reportReasonCopyright::reportReasonCopyright() {
}

const std::int32_t reportReasonCopyright::ID;

void reportReasonCopyright::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportReasonCopyright");
    s.store_class_end();
  }
}

reportReasonUnrelatedLocation::reportReasonUnrelatedLocation() {
}

const std::int32_t reportReasonUnrelatedLocation::ID;

void reportReasonUnrelatedLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportReasonUnrelatedLocation");
    s.store_class_end();
  }
}

reportReasonFake::reportReasonFake() {
}

const std::int32_t reportReasonFake::ID;

void reportReasonFake::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportReasonFake");
    s.store_class_end();
  }
}

reportReasonIllegalDrugs::reportReasonIllegalDrugs() {
}

const std::int32_t reportReasonIllegalDrugs::ID;

void reportReasonIllegalDrugs::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportReasonIllegalDrugs");
    s.store_class_end();
  }
}

reportReasonPersonalDetails::reportReasonPersonalDetails() {
}

const std::int32_t reportReasonPersonalDetails::ID;

void reportReasonPersonalDetails::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportReasonPersonalDetails");
    s.store_class_end();
  }
}

reportReasonCustom::reportReasonCustom() {
}

const std::int32_t reportReasonCustom::ID;

void reportReasonCustom::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportReasonCustom");
    s.store_class_end();
  }
}

restrictionInfo::restrictionInfo()
  : restriction_reason_()
  , has_sensitive_content_()
{}

restrictionInfo::restrictionInfo(string const &restriction_reason_, bool has_sensitive_content_)
  : restriction_reason_(restriction_reason_)
  , has_sensitive_content_(has_sensitive_content_)
{}

const std::int32_t restrictionInfo::ID;

void restrictionInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "restrictionInfo");
    s.store_field("restriction_reason", restriction_reason_);
    s.store_field("has_sensitive_content", has_sensitive_content_);
    s.store_class_end();
  }
}

savedMessagesTags::savedMessagesTags()
  : tags_()
{}

savedMessagesTags::savedMessagesTags(array<object_ptr<savedMessagesTag>> &&tags_)
  : tags_(std::move(tags_))
{}

const std::int32_t savedMessagesTags::ID;

void savedMessagesTags::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "savedMessagesTags");
    { s.store_vector_begin("tags", tags_.size()); for (const auto &_value : tags_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

scopeNotificationSettings::scopeNotificationSettings()
  : mute_for_()
  , sound_id_()
  , show_preview_()
  , use_default_mute_stories_()
  , mute_stories_()
  , story_sound_id_()
  , show_story_poster_()
  , disable_pinned_message_notifications_()
  , disable_mention_notifications_()
{}

scopeNotificationSettings::scopeNotificationSettings(int32 mute_for_, int64 sound_id_, bool show_preview_, bool use_default_mute_stories_, bool mute_stories_, int64 story_sound_id_, bool show_story_poster_, bool disable_pinned_message_notifications_, bool disable_mention_notifications_)
  : mute_for_(mute_for_)
  , sound_id_(sound_id_)
  , show_preview_(show_preview_)
  , use_default_mute_stories_(use_default_mute_stories_)
  , mute_stories_(mute_stories_)
  , story_sound_id_(story_sound_id_)
  , show_story_poster_(show_story_poster_)
  , disable_pinned_message_notifications_(disable_pinned_message_notifications_)
  , disable_mention_notifications_(disable_mention_notifications_)
{}

const std::int32_t scopeNotificationSettings::ID;

void scopeNotificationSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "scopeNotificationSettings");
    s.store_field("mute_for", mute_for_);
    s.store_field("sound_id", sound_id_);
    s.store_field("show_preview", show_preview_);
    s.store_field("use_default_mute_stories", use_default_mute_stories_);
    s.store_field("mute_stories", mute_stories_);
    s.store_field("story_sound_id", story_sound_id_);
    s.store_field("show_story_poster", show_story_poster_);
    s.store_field("disable_pinned_message_notifications", disable_pinned_message_notifications_);
    s.store_field("disable_mention_notifications", disable_mention_notifications_);
    s.store_class_end();
  }
}

searchMessagesFilterEmpty::searchMessagesFilterEmpty() {
}

const std::int32_t searchMessagesFilterEmpty::ID;

void searchMessagesFilterEmpty::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesFilterEmpty");
    s.store_class_end();
  }
}

searchMessagesFilterAnimation::searchMessagesFilterAnimation() {
}

const std::int32_t searchMessagesFilterAnimation::ID;

void searchMessagesFilterAnimation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesFilterAnimation");
    s.store_class_end();
  }
}

searchMessagesFilterAudio::searchMessagesFilterAudio() {
}

const std::int32_t searchMessagesFilterAudio::ID;

void searchMessagesFilterAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesFilterAudio");
    s.store_class_end();
  }
}

searchMessagesFilterDocument::searchMessagesFilterDocument() {
}

const std::int32_t searchMessagesFilterDocument::ID;

void searchMessagesFilterDocument::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesFilterDocument");
    s.store_class_end();
  }
}

searchMessagesFilterPhoto::searchMessagesFilterPhoto() {
}

const std::int32_t searchMessagesFilterPhoto::ID;

void searchMessagesFilterPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesFilterPhoto");
    s.store_class_end();
  }
}

searchMessagesFilterVideo::searchMessagesFilterVideo() {
}

const std::int32_t searchMessagesFilterVideo::ID;

void searchMessagesFilterVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesFilterVideo");
    s.store_class_end();
  }
}

searchMessagesFilterVoiceNote::searchMessagesFilterVoiceNote() {
}

const std::int32_t searchMessagesFilterVoiceNote::ID;

void searchMessagesFilterVoiceNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesFilterVoiceNote");
    s.store_class_end();
  }
}

searchMessagesFilterPhotoAndVideo::searchMessagesFilterPhotoAndVideo() {
}

const std::int32_t searchMessagesFilterPhotoAndVideo::ID;

void searchMessagesFilterPhotoAndVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesFilterPhotoAndVideo");
    s.store_class_end();
  }
}

searchMessagesFilterUrl::searchMessagesFilterUrl() {
}

const std::int32_t searchMessagesFilterUrl::ID;

void searchMessagesFilterUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesFilterUrl");
    s.store_class_end();
  }
}

searchMessagesFilterChatPhoto::searchMessagesFilterChatPhoto() {
}

const std::int32_t searchMessagesFilterChatPhoto::ID;

void searchMessagesFilterChatPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesFilterChatPhoto");
    s.store_class_end();
  }
}

searchMessagesFilterVideoNote::searchMessagesFilterVideoNote() {
}

const std::int32_t searchMessagesFilterVideoNote::ID;

void searchMessagesFilterVideoNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesFilterVideoNote");
    s.store_class_end();
  }
}

searchMessagesFilterVoiceAndVideoNote::searchMessagesFilterVoiceAndVideoNote() {
}

const std::int32_t searchMessagesFilterVoiceAndVideoNote::ID;

void searchMessagesFilterVoiceAndVideoNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesFilterVoiceAndVideoNote");
    s.store_class_end();
  }
}

searchMessagesFilterMention::searchMessagesFilterMention() {
}

const std::int32_t searchMessagesFilterMention::ID;

void searchMessagesFilterMention::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesFilterMention");
    s.store_class_end();
  }
}

searchMessagesFilterUnreadMention::searchMessagesFilterUnreadMention() {
}

const std::int32_t searchMessagesFilterUnreadMention::ID;

void searchMessagesFilterUnreadMention::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesFilterUnreadMention");
    s.store_class_end();
  }
}

searchMessagesFilterUnreadReaction::searchMessagesFilterUnreadReaction() {
}

const std::int32_t searchMessagesFilterUnreadReaction::ID;

void searchMessagesFilterUnreadReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesFilterUnreadReaction");
    s.store_class_end();
  }
}

searchMessagesFilterFailedToSend::searchMessagesFilterFailedToSend() {
}

const std::int32_t searchMessagesFilterFailedToSend::ID;

void searchMessagesFilterFailedToSend::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesFilterFailedToSend");
    s.store_class_end();
  }
}

searchMessagesFilterPinned::searchMessagesFilterPinned() {
}

const std::int32_t searchMessagesFilterPinned::ID;

void searchMessagesFilterPinned::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchMessagesFilterPinned");
    s.store_class_end();
  }
}

sessionTypeAndroid::sessionTypeAndroid() {
}

const std::int32_t sessionTypeAndroid::ID;

void sessionTypeAndroid::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sessionTypeAndroid");
    s.store_class_end();
  }
}

sessionTypeApple::sessionTypeApple() {
}

const std::int32_t sessionTypeApple::ID;

void sessionTypeApple::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sessionTypeApple");
    s.store_class_end();
  }
}

sessionTypeBrave::sessionTypeBrave() {
}

const std::int32_t sessionTypeBrave::ID;

void sessionTypeBrave::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sessionTypeBrave");
    s.store_class_end();
  }
}

sessionTypeChrome::sessionTypeChrome() {
}

const std::int32_t sessionTypeChrome::ID;

void sessionTypeChrome::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sessionTypeChrome");
    s.store_class_end();
  }
}

sessionTypeEdge::sessionTypeEdge() {
}

const std::int32_t sessionTypeEdge::ID;

void sessionTypeEdge::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sessionTypeEdge");
    s.store_class_end();
  }
}

sessionTypeFirefox::sessionTypeFirefox() {
}

const std::int32_t sessionTypeFirefox::ID;

void sessionTypeFirefox::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sessionTypeFirefox");
    s.store_class_end();
  }
}

sessionTypeIpad::sessionTypeIpad() {
}

const std::int32_t sessionTypeIpad::ID;

void sessionTypeIpad::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sessionTypeIpad");
    s.store_class_end();
  }
}

sessionTypeIphone::sessionTypeIphone() {
}

const std::int32_t sessionTypeIphone::ID;

void sessionTypeIphone::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sessionTypeIphone");
    s.store_class_end();
  }
}

sessionTypeLinux::sessionTypeLinux() {
}

const std::int32_t sessionTypeLinux::ID;

void sessionTypeLinux::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sessionTypeLinux");
    s.store_class_end();
  }
}

sessionTypeMac::sessionTypeMac() {
}

const std::int32_t sessionTypeMac::ID;

void sessionTypeMac::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sessionTypeMac");
    s.store_class_end();
  }
}

sessionTypeOpera::sessionTypeOpera() {
}

const std::int32_t sessionTypeOpera::ID;

void sessionTypeOpera::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sessionTypeOpera");
    s.store_class_end();
  }
}

sessionTypeSafari::sessionTypeSafari() {
}

const std::int32_t sessionTypeSafari::ID;

void sessionTypeSafari::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sessionTypeSafari");
    s.store_class_end();
  }
}

sessionTypeUbuntu::sessionTypeUbuntu() {
}

const std::int32_t sessionTypeUbuntu::ID;

void sessionTypeUbuntu::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sessionTypeUbuntu");
    s.store_class_end();
  }
}

sessionTypeUnknown::sessionTypeUnknown() {
}

const std::int32_t sessionTypeUnknown::ID;

void sessionTypeUnknown::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sessionTypeUnknown");
    s.store_class_end();
  }
}

sessionTypeVivaldi::sessionTypeVivaldi() {
}

const std::int32_t sessionTypeVivaldi::ID;

void sessionTypeVivaldi::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sessionTypeVivaldi");
    s.store_class_end();
  }
}

sessionTypeWindows::sessionTypeWindows() {
}

const std::int32_t sessionTypeWindows::ID;

void sessionTypeWindows::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sessionTypeWindows");
    s.store_class_end();
  }
}

sessionTypeXbox::sessionTypeXbox() {
}

const std::int32_t sessionTypeXbox::ID;

void sessionTypeXbox::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sessionTypeXbox");
    s.store_class_end();
  }
}

speechRecognitionResultPending::speechRecognitionResultPending()
  : partial_text_()
{}

speechRecognitionResultPending::speechRecognitionResultPending(string const &partial_text_)
  : partial_text_(partial_text_)
{}

const std::int32_t speechRecognitionResultPending::ID;

void speechRecognitionResultPending::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "speechRecognitionResultPending");
    s.store_field("partial_text", partial_text_);
    s.store_class_end();
  }
}

speechRecognitionResultText::speechRecognitionResultText()
  : text_()
{}

speechRecognitionResultText::speechRecognitionResultText(string const &text_)
  : text_(text_)
{}

const std::int32_t speechRecognitionResultText::ID;

void speechRecognitionResultText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "speechRecognitionResultText");
    s.store_field("text", text_);
    s.store_class_end();
  }
}

speechRecognitionResultError::speechRecognitionResultError()
  : error_()
{}

speechRecognitionResultError::speechRecognitionResultError(object_ptr<error> &&error_)
  : error_(std::move(error_))
{}

const std::int32_t speechRecognitionResultError::ID;

void speechRecognitionResultError::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "speechRecognitionResultError");
    s.store_object_field("error", static_cast<const BaseObject *>(error_.get()));
    s.store_class_end();
  }
}

sponsoredChats::sponsoredChats()
  : chats_()
{}

sponsoredChats::sponsoredChats(array<object_ptr<sponsoredChat>> &&chats_)
  : chats_(std::move(chats_))
{}

const std::int32_t sponsoredChats::ID;

void sponsoredChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sponsoredChats");
    { s.store_vector_begin("chats", chats_.size()); for (const auto &_value : chats_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

starGiveawayPaymentOption::starGiveawayPaymentOption()
  : currency_()
  , amount_()
  , star_count_()
  , store_product_id_()
  , yearly_boost_count_()
  , winner_options_()
  , is_default_()
  , is_additional_()
{}

starGiveawayPaymentOption::starGiveawayPaymentOption(string const &currency_, int53 amount_, int53 star_count_, string const &store_product_id_, int32 yearly_boost_count_, array<object_ptr<starGiveawayWinnerOption>> &&winner_options_, bool is_default_, bool is_additional_)
  : currency_(currency_)
  , amount_(amount_)
  , star_count_(star_count_)
  , store_product_id_(store_product_id_)
  , yearly_boost_count_(yearly_boost_count_)
  , winner_options_(std::move(winner_options_))
  , is_default_(is_default_)
  , is_additional_(is_additional_)
{}

const std::int32_t starGiveawayPaymentOption::ID;

void starGiveawayPaymentOption::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "starGiveawayPaymentOption");
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("star_count", star_count_);
    s.store_field("store_product_id", store_product_id_);
    s.store_field("yearly_boost_count", yearly_boost_count_);
    { s.store_vector_begin("winner_options", winner_options_.size()); for (const auto &_value : winner_options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("is_default", is_default_);
    s.store_field("is_additional", is_additional_);
    s.store_class_end();
  }
}

storyStatistics::storyStatistics()
  : story_interaction_graph_()
  , story_reaction_graph_()
{}

storyStatistics::storyStatistics(object_ptr<StatisticalGraph> &&story_interaction_graph_, object_ptr<StatisticalGraph> &&story_reaction_graph_)
  : story_interaction_graph_(std::move(story_interaction_graph_))
  , story_reaction_graph_(std::move(story_reaction_graph_))
{}

const std::int32_t storyStatistics::ID;

void storyStatistics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyStatistics");
    s.store_object_field("story_interaction_graph", static_cast<const BaseObject *>(story_interaction_graph_.get()));
    s.store_object_field("story_reaction_graph", static_cast<const BaseObject *>(story_reaction_graph_.get()));
    s.store_class_end();
  }
}

storyVideo::storyVideo()
  : duration_()
  , width_()
  , height_()
  , has_stickers_()
  , is_animation_()
  , minithumbnail_()
  , thumbnail_()
  , preload_prefix_size_()
  , cover_frame_timestamp_()
  , video_()
{}

storyVideo::storyVideo(double duration_, int32 width_, int32 height_, bool has_stickers_, bool is_animation_, object_ptr<minithumbnail> &&minithumbnail_, object_ptr<thumbnail> &&thumbnail_, int32 preload_prefix_size_, double cover_frame_timestamp_, object_ptr<file> &&video_)
  : duration_(duration_)
  , width_(width_)
  , height_(height_)
  , has_stickers_(has_stickers_)
  , is_animation_(is_animation_)
  , minithumbnail_(std::move(minithumbnail_))
  , thumbnail_(std::move(thumbnail_))
  , preload_prefix_size_(preload_prefix_size_)
  , cover_frame_timestamp_(cover_frame_timestamp_)
  , video_(std::move(video_))
{}

const std::int32_t storyVideo::ID;

void storyVideo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyVideo");
    s.store_field("duration", duration_);
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_field("has_stickers", has_stickers_);
    s.store_field("is_animation", is_animation_);
    s.store_object_field("minithumbnail", static_cast<const BaseObject *>(minithumbnail_.get()));
    s.store_object_field("thumbnail", static_cast<const BaseObject *>(thumbnail_.get()));
    s.store_field("preload_prefix_size", preload_prefix_size_);
    s.store_field("cover_frame_timestamp", cover_frame_timestamp_);
    s.store_object_field("video", static_cast<const BaseObject *>(video_.get()));
    s.store_class_end();
  }
}

themeSettings::themeSettings()
  : base_theme_()
  , accent_color_()
  , background_()
  , outgoing_message_fill_()
  , animate_outgoing_message_fill_()
  , outgoing_message_accent_color_()
{}

themeSettings::themeSettings(object_ptr<BuiltInTheme> &&base_theme_, int32 accent_color_, object_ptr<background> &&background_, object_ptr<BackgroundFill> &&outgoing_message_fill_, bool animate_outgoing_message_fill_, int32 outgoing_message_accent_color_)
  : base_theme_(std::move(base_theme_))
  , accent_color_(accent_color_)
  , background_(std::move(background_))
  , outgoing_message_fill_(std::move(outgoing_message_fill_))
  , animate_outgoing_message_fill_(animate_outgoing_message_fill_)
  , outgoing_message_accent_color_(outgoing_message_accent_color_)
{}

const std::int32_t themeSettings::ID;

void themeSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "themeSettings");
    s.store_object_field("base_theme", static_cast<const BaseObject *>(base_theme_.get()));
    s.store_field("accent_color", accent_color_);
    s.store_object_field("background", static_cast<const BaseObject *>(background_.get()));
    s.store_object_field("outgoing_message_fill", static_cast<const BaseObject *>(outgoing_message_fill_.get()));
    s.store_field("animate_outgoing_message_fill", animate_outgoing_message_fill_);
    s.store_field("outgoing_message_accent_color", outgoing_message_accent_color_);
    s.store_class_end();
  }
}

thumbnail::thumbnail()
  : format_()
  , width_()
  , height_()
  , file_()
{}

thumbnail::thumbnail(object_ptr<ThumbnailFormat> &&format_, int32 width_, int32 height_, object_ptr<file> &&file_)
  : format_(std::move(format_))
  , width_(width_)
  , height_(height_)
  , file_(std::move(file_))
{}

const std::int32_t thumbnail::ID;

void thumbnail::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "thumbnail");
    s.store_object_field("format", static_cast<const BaseObject *>(format_.get()));
    s.store_field("width", width_);
    s.store_field("height", height_);
    s.store_object_field("file", static_cast<const BaseObject *>(file_.get()));
    s.store_class_end();
  }
}

upgradedGiftOriginUpgrade::upgradedGiftOriginUpgrade()
  : gift_message_id_()
{}

upgradedGiftOriginUpgrade::upgradedGiftOriginUpgrade(int53 gift_message_id_)
  : gift_message_id_(gift_message_id_)
{}

const std::int32_t upgradedGiftOriginUpgrade::ID;

void upgradedGiftOriginUpgrade::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradedGiftOriginUpgrade");
    s.store_field("gift_message_id", gift_message_id_);
    s.store_class_end();
  }
}

upgradedGiftOriginTransfer::upgradedGiftOriginTransfer() {
}

const std::int32_t upgradedGiftOriginTransfer::ID;

void upgradedGiftOriginTransfer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradedGiftOriginTransfer");
    s.store_class_end();
  }
}

upgradedGiftOriginResale::upgradedGiftOriginResale()
  : price_()
{}

upgradedGiftOriginResale::upgradedGiftOriginResale(object_ptr<GiftResalePrice> &&price_)
  : price_(std::move(price_))
{}

const std::int32_t upgradedGiftOriginResale::ID;

void upgradedGiftOriginResale::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradedGiftOriginResale");
    s.store_object_field("price", static_cast<const BaseObject *>(price_.get()));
    s.store_class_end();
  }
}

upgradedGiftOriginBlockchain::upgradedGiftOriginBlockchain() {
}

const std::int32_t upgradedGiftOriginBlockchain::ID;

void upgradedGiftOriginBlockchain::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradedGiftOriginBlockchain");
    s.store_class_end();
  }
}

upgradedGiftOriginPrepaidUpgrade::upgradedGiftOriginPrepaidUpgrade() {
}

const std::int32_t upgradedGiftOriginPrepaidUpgrade::ID;

void upgradedGiftOriginPrepaidUpgrade::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradedGiftOriginPrepaidUpgrade");
    s.store_class_end();
  }
}

voiceNote::voiceNote()
  : duration_()
  , waveform_()
  , mime_type_()
  , speech_recognition_result_()
  , voice_()
{}

voiceNote::voiceNote(int32 duration_, bytes const &waveform_, string const &mime_type_, object_ptr<SpeechRecognitionResult> &&speech_recognition_result_, object_ptr<file> &&voice_)
  : duration_(duration_)
  , waveform_(std::move(waveform_))
  , mime_type_(mime_type_)
  , speech_recognition_result_(std::move(speech_recognition_result_))
  , voice_(std::move(voice_))
{}

const std::int32_t voiceNote::ID;

void voiceNote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "voiceNote");
    s.store_field("duration", duration_);
    s.store_bytes_field("waveform", waveform_);
    s.store_field("mime_type", mime_type_);
    s.store_object_field("speech_recognition_result", static_cast<const BaseObject *>(speech_recognition_result_.get()));
    s.store_object_field("voice", static_cast<const BaseObject *>(voice_.get()));
    s.store_class_end();
  }
}

webAppOpenModeCompact::webAppOpenModeCompact() {
}

const std::int32_t webAppOpenModeCompact::ID;

void webAppOpenModeCompact::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webAppOpenModeCompact");
    s.store_class_end();
  }
}

webAppOpenModeFullSize::webAppOpenModeFullSize() {
}

const std::int32_t webAppOpenModeFullSize::ID;

void webAppOpenModeFullSize::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webAppOpenModeFullSize");
    s.store_class_end();
  }
}

webAppOpenModeFullScreen::webAppOpenModeFullScreen() {
}

const std::int32_t webAppOpenModeFullScreen::ID;

void webAppOpenModeFullScreen::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webAppOpenModeFullScreen");
    s.store_class_end();
  }
}

addBotMediaPreview::addBotMediaPreview()
  : bot_user_id_()
  , language_code_()
  , content_()
{}

addBotMediaPreview::addBotMediaPreview(int53 bot_user_id_, string const &language_code_, object_ptr<InputStoryContent> &&content_)
  : bot_user_id_(bot_user_id_)
  , language_code_(language_code_)
  , content_(std::move(content_))
{}

const std::int32_t addBotMediaPreview::ID;

void addBotMediaPreview::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addBotMediaPreview");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("language_code", language_code_);
    s.store_object_field("content", static_cast<const BaseObject *>(content_.get()));
    s.store_class_end();
  }
}

addGiftCollectionGifts::addGiftCollectionGifts()
  : owner_id_()
  , collection_id_()
  , received_gift_ids_()
{}

addGiftCollectionGifts::addGiftCollectionGifts(object_ptr<MessageSender> &&owner_id_, int32 collection_id_, array<string> &&received_gift_ids_)
  : owner_id_(std::move(owner_id_))
  , collection_id_(collection_id_)
  , received_gift_ids_(std::move(received_gift_ids_))
{}

const std::int32_t addGiftCollectionGifts::ID;

void addGiftCollectionGifts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addGiftCollectionGifts");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_field("collection_id", collection_id_);
    { s.store_vector_begin("received_gift_ids", received_gift_ids_.size()); for (const auto &_value : received_gift_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

addNetworkStatistics::addNetworkStatistics()
  : entry_()
{}

addNetworkStatistics::addNetworkStatistics(object_ptr<NetworkStatisticsEntry> &&entry_)
  : entry_(std::move(entry_))
{}

const std::int32_t addNetworkStatistics::ID;

void addNetworkStatistics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addNetworkStatistics");
    s.store_object_field("entry", static_cast<const BaseObject *>(entry_.get()));
    s.store_class_end();
  }
}

answerShippingQuery::answerShippingQuery()
  : shipping_query_id_()
  , shipping_options_()
  , error_message_()
{}

answerShippingQuery::answerShippingQuery(int64 shipping_query_id_, array<object_ptr<shippingOption>> &&shipping_options_, string const &error_message_)
  : shipping_query_id_(shipping_query_id_)
  , shipping_options_(std::move(shipping_options_))
  , error_message_(error_message_)
{}

const std::int32_t answerShippingQuery::ID;

void answerShippingQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "answerShippingQuery");
    s.store_field("shipping_query_id", shipping_query_id_);
    { s.store_vector_begin("shipping_options", shipping_options_.size()); for (const auto &_value : shipping_options_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("error_message", error_message_);
    s.store_class_end();
  }
}

checkChatUsername::checkChatUsername()
  : chat_id_()
  , username_()
{}

checkChatUsername::checkChatUsername(int53 chat_id_, string const &username_)
  : chat_id_(chat_id_)
  , username_(username_)
{}

const std::int32_t checkChatUsername::ID;

void checkChatUsername::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkChatUsername");
    s.store_field("chat_id", chat_id_);
    s.store_field("username", username_);
    s.store_class_end();
  }
}

checkPhoneNumberCode::checkPhoneNumberCode()
  : code_()
{}

checkPhoneNumberCode::checkPhoneNumberCode(string const &code_)
  : code_(code_)
{}

const std::int32_t checkPhoneNumberCode::ID;

void checkPhoneNumberCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkPhoneNumberCode");
    s.store_field("code", code_);
    s.store_class_end();
  }
}

checkStickerSetName::checkStickerSetName()
  : name_()
{}

checkStickerSetName::checkStickerSetName(string const &name_)
  : name_(name_)
{}

const std::int32_t checkStickerSetName::ID;

void checkStickerSetName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkStickerSetName");
    s.store_field("name", name_);
    s.store_class_end();
  }
}

cleanFileName::cleanFileName()
  : file_name_()
{}

cleanFileName::cleanFileName(string const &file_name_)
  : file_name_(file_name_)
{}

const std::int32_t cleanFileName::ID;

void cleanFileName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "cleanFileName");
    s.store_field("file_name", file_name_);
    s.store_class_end();
  }
}

clearAutosaveSettingsExceptions::clearAutosaveSettingsExceptions() {
}

const std::int32_t clearAutosaveSettingsExceptions::ID;

void clearAutosaveSettingsExceptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "clearAutosaveSettingsExceptions");
    s.store_class_end();
  }
}

clearImportedContacts::clearImportedContacts() {
}

const std::int32_t clearImportedContacts::ID;

void clearImportedContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "clearImportedContacts");
    s.store_class_end();
  }
}

clearRecentReactions::clearRecentReactions() {
}

const std::int32_t clearRecentReactions::ID;

void clearRecentReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "clearRecentReactions");
    s.store_class_end();
  }
}

clearRecentStickers::clearRecentStickers()
  : is_attached_()
{}

clearRecentStickers::clearRecentStickers(bool is_attached_)
  : is_attached_(is_attached_)
{}

const std::int32_t clearRecentStickers::ID;

void clearRecentStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "clearRecentStickers");
    s.store_field("is_attached", is_attached_);
    s.store_class_end();
  }
}

createGiftCollection::createGiftCollection()
  : owner_id_()
  , name_()
  , received_gift_ids_()
{}

createGiftCollection::createGiftCollection(object_ptr<MessageSender> &&owner_id_, string const &name_, array<string> &&received_gift_ids_)
  : owner_id_(std::move(owner_id_))
  , name_(name_)
  , received_gift_ids_(std::move(received_gift_ids_))
{}

const std::int32_t createGiftCollection::ID;

void createGiftCollection::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createGiftCollection");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_field("name", name_);
    { s.store_vector_begin("received_gift_ids", received_gift_ids_.size()); for (const auto &_value : received_gift_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

createStoryAlbum::createStoryAlbum()
  : story_poster_chat_id_()
  , name_()
  , story_ids_()
{}

createStoryAlbum::createStoryAlbum(int53 story_poster_chat_id_, string const &name_, array<int32> &&story_ids_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , name_(name_)
  , story_ids_(std::move(story_ids_))
{}

const std::int32_t createStoryAlbum::ID;

void createStoryAlbum::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createStoryAlbum");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("name", name_);
    { s.store_vector_begin("story_ids", story_ids_.size()); for (const auto &_value : story_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

deleteBusinessStory::deleteBusinessStory()
  : business_connection_id_()
  , story_id_()
{}

deleteBusinessStory::deleteBusinessStory(string const &business_connection_id_, int32 story_id_)
  : business_connection_id_(business_connection_id_)
  , story_id_(story_id_)
{}

const std::int32_t deleteBusinessStory::ID;

void deleteBusinessStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteBusinessStory");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_field("story_id", story_id_);
    s.store_class_end();
  }
}

deleteDirectMessagesChatTopicMessagesByDate::deleteDirectMessagesChatTopicMessagesByDate()
  : chat_id_()
  , topic_id_()
  , min_date_()
  , max_date_()
{}

deleteDirectMessagesChatTopicMessagesByDate::deleteDirectMessagesChatTopicMessagesByDate(int53 chat_id_, int53 topic_id_, int32 min_date_, int32 max_date_)
  : chat_id_(chat_id_)
  , topic_id_(topic_id_)
  , min_date_(min_date_)
  , max_date_(max_date_)
{}

const std::int32_t deleteDirectMessagesChatTopicMessagesByDate::ID;

void deleteDirectMessagesChatTopicMessagesByDate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteDirectMessagesChatTopicMessagesByDate");
    s.store_field("chat_id", chat_id_);
    s.store_field("topic_id", topic_id_);
    s.store_field("min_date", min_date_);
    s.store_field("max_date", max_date_);
    s.store_class_end();
  }
}

deleteSavedMessagesTopicMessagesByDate::deleteSavedMessagesTopicMessagesByDate()
  : saved_messages_topic_id_()
  , min_date_()
  , max_date_()
{}

deleteSavedMessagesTopicMessagesByDate::deleteSavedMessagesTopicMessagesByDate(int53 saved_messages_topic_id_, int32 min_date_, int32 max_date_)
  : saved_messages_topic_id_(saved_messages_topic_id_)
  , min_date_(min_date_)
  , max_date_(max_date_)
{}

const std::int32_t deleteSavedMessagesTopicMessagesByDate::ID;

void deleteSavedMessagesTopicMessagesByDate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteSavedMessagesTopicMessagesByDate");
    s.store_field("saved_messages_topic_id", saved_messages_topic_id_);
    s.store_field("min_date", min_date_);
    s.store_field("max_date", max_date_);
    s.store_class_end();
  }
}

deleteStory::deleteStory()
  : story_poster_chat_id_()
  , story_id_()
{}

deleteStory::deleteStory(int53 story_poster_chat_id_, int32 story_id_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , story_id_(story_id_)
{}

const std::int32_t deleteStory::ID;

void deleteStory::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteStory");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("story_id", story_id_);
    s.store_class_end();
  }
}

disableProxy::disableProxy() {
}

const std::int32_t disableProxy::ID;

void disableProxy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "disableProxy");
    s.store_class_end();
  }
}

discardCall::discardCall()
  : call_id_()
  , is_disconnected_()
  , invite_link_()
  , duration_()
  , is_video_()
  , connection_id_()
{}

discardCall::discardCall(int32 call_id_, bool is_disconnected_, string const &invite_link_, int32 duration_, bool is_video_, int64 connection_id_)
  : call_id_(call_id_)
  , is_disconnected_(is_disconnected_)
  , invite_link_(invite_link_)
  , duration_(duration_)
  , is_video_(is_video_)
  , connection_id_(connection_id_)
{}

const std::int32_t discardCall::ID;

void discardCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "discardCall");
    s.store_field("call_id", call_id_);
    s.store_field("is_disconnected", is_disconnected_);
    s.store_field("invite_link", invite_link_);
    s.store_field("duration", duration_);
    s.store_field("is_video", is_video_);
    s.store_field("connection_id", connection_id_);
    s.store_class_end();
  }
}

editInlineMessageMedia::editInlineMessageMedia()
  : inline_message_id_()
  , reply_markup_()
  , input_message_content_()
{}

editInlineMessageMedia::editInlineMessageMedia(string const &inline_message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_)
  : inline_message_id_(inline_message_id_)
  , reply_markup_(std::move(reply_markup_))
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t editInlineMessageMedia::ID;

void editInlineMessageMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editInlineMessageMedia");
    s.store_field("inline_message_id", inline_message_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

editMessageSchedulingState::editMessageSchedulingState()
  : chat_id_()
  , message_id_()
  , scheduling_state_()
{}

editMessageSchedulingState::editMessageSchedulingState(int53 chat_id_, int53 message_id_, object_ptr<MessageSchedulingState> &&scheduling_state_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , scheduling_state_(std::move(scheduling_state_))
{}

const std::int32_t editMessageSchedulingState::ID;

void editMessageSchedulingState::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editMessageSchedulingState");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("scheduling_state", static_cast<const BaseObject *>(scheduling_state_.get()));
    s.store_class_end();
  }
}

editMessageText::editMessageText()
  : chat_id_()
  , message_id_()
  , reply_markup_()
  , input_message_content_()
{}

editMessageText::editMessageText(int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , reply_markup_(std::move(reply_markup_))
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t editMessageText::ID;

void editMessageText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editMessageText");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

editProxy::editProxy()
  : proxy_id_()
  , server_()
  , port_()
  , enable_()
  , type_()
{}

editProxy::editProxy(int32 proxy_id_, string const &server_, int32 port_, bool enable_, object_ptr<ProxyType> &&type_)
  : proxy_id_(proxy_id_)
  , server_(server_)
  , port_(port_)
  , enable_(enable_)
  , type_(std::move(type_))
{}

const std::int32_t editProxy::ID;

void editProxy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editProxy");
    s.store_field("proxy_id", proxy_id_);
    s.store_field("server", server_);
    s.store_field("port", port_);
    s.store_field("enable", enable_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

getAllPassportElements::getAllPassportElements()
  : password_()
{}

getAllPassportElements::getAllPassportElements(string const &password_)
  : password_(password_)
{}

const std::int32_t getAllPassportElements::ID;

void getAllPassportElements::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getAllPassportElements");
    s.store_field("password", password_);
    s.store_class_end();
  }
}

getAttachedStickerSets::getAttachedStickerSets()
  : file_id_()
{}

getAttachedStickerSets::getAttachedStickerSets(int32 file_id_)
  : file_id_(file_id_)
{}

const std::int32_t getAttachedStickerSets::ID;

void getAttachedStickerSets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getAttachedStickerSets");
    s.store_field("file_id", file_id_);
    s.store_class_end();
  }
}

getBasicGroupFullInfo::getBasicGroupFullInfo()
  : basic_group_id_()
{}

getBasicGroupFullInfo::getBasicGroupFullInfo(int53 basic_group_id_)
  : basic_group_id_(basic_group_id_)
{}

const std::int32_t getBasicGroupFullInfo::ID;

void getBasicGroupFullInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getBasicGroupFullInfo");
    s.store_field("basic_group_id", basic_group_id_);
    s.store_class_end();
  }
}

getChatAvailableMessageSenders::getChatAvailableMessageSenders()
  : chat_id_()
{}

getChatAvailableMessageSenders::getChatAvailableMessageSenders(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t getChatAvailableMessageSenders::ID;

void getChatAvailableMessageSenders::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatAvailableMessageSenders");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

getChatBoostFeatures::getChatBoostFeatures()
  : is_channel_()
{}

getChatBoostFeatures::getChatBoostFeatures(bool is_channel_)
  : is_channel_(is_channel_)
{}

const std::int32_t getChatBoostFeatures::ID;

void getChatBoostFeatures::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatBoostFeatures");
    s.store_field("is_channel", is_channel_);
    s.store_class_end();
  }
}

getChatBoostLevelFeatures::getChatBoostLevelFeatures()
  : is_channel_()
  , level_()
{}

getChatBoostLevelFeatures::getChatBoostLevelFeatures(bool is_channel_, int32 level_)
  : is_channel_(is_channel_)
  , level_(level_)
{}

const std::int32_t getChatBoostLevelFeatures::ID;

void getChatBoostLevelFeatures::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatBoostLevelFeatures");
    s.store_field("is_channel", is_channel_);
    s.store_field("level", level_);
    s.store_class_end();
  }
}

getChatInviteLinkMembers::getChatInviteLinkMembers()
  : chat_id_()
  , invite_link_()
  , only_with_expired_subscription_()
  , offset_member_()
  , limit_()
{}

getChatInviteLinkMembers::getChatInviteLinkMembers(int53 chat_id_, string const &invite_link_, bool only_with_expired_subscription_, object_ptr<chatInviteLinkMember> &&offset_member_, int32 limit_)
  : chat_id_(chat_id_)
  , invite_link_(invite_link_)
  , only_with_expired_subscription_(only_with_expired_subscription_)
  , offset_member_(std::move(offset_member_))
  , limit_(limit_)
{}

const std::int32_t getChatInviteLinkMembers::ID;

void getChatInviteLinkMembers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatInviteLinkMembers");
    s.store_field("chat_id", chat_id_);
    s.store_field("invite_link", invite_link_);
    s.store_field("only_with_expired_subscription", only_with_expired_subscription_);
    s.store_object_field("offset_member", static_cast<const BaseObject *>(offset_member_.get()));
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getChatInviteLinks::getChatInviteLinks()
  : chat_id_()
  , creator_user_id_()
  , is_revoked_()
  , offset_date_()
  , offset_invite_link_()
  , limit_()
{}

getChatInviteLinks::getChatInviteLinks(int53 chat_id_, int53 creator_user_id_, bool is_revoked_, int32 offset_date_, string const &offset_invite_link_, int32 limit_)
  : chat_id_(chat_id_)
  , creator_user_id_(creator_user_id_)
  , is_revoked_(is_revoked_)
  , offset_date_(offset_date_)
  , offset_invite_link_(offset_invite_link_)
  , limit_(limit_)
{}

const std::int32_t getChatInviteLinks::ID;

void getChatInviteLinks::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatInviteLinks");
    s.store_field("chat_id", chat_id_);
    s.store_field("creator_user_id", creator_user_id_);
    s.store_field("is_revoked", is_revoked_);
    s.store_field("offset_date", offset_date_);
    s.store_field("offset_invite_link", offset_invite_link_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getChatMessageCount::getChatMessageCount()
  : chat_id_()
  , topic_id_()
  , filter_()
  , return_local_()
{}

getChatMessageCount::getChatMessageCount(int53 chat_id_, object_ptr<MessageTopic> &&topic_id_, object_ptr<SearchMessagesFilter> &&filter_, bool return_local_)
  : chat_id_(chat_id_)
  , topic_id_(std::move(topic_id_))
  , filter_(std::move(filter_))
  , return_local_(return_local_)
{}

const std::int32_t getChatMessageCount::ID;

void getChatMessageCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatMessageCount");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("topic_id", static_cast<const BaseObject *>(topic_id_.get()));
    s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get()));
    s.store_field("return_local", return_local_);
    s.store_class_end();
  }
}

getChatNotificationSettingsExceptions::getChatNotificationSettingsExceptions()
  : scope_()
  , compare_sound_()
{}

getChatNotificationSettingsExceptions::getChatNotificationSettingsExceptions(object_ptr<NotificationSettingsScope> &&scope_, bool compare_sound_)
  : scope_(std::move(scope_))
  , compare_sound_(compare_sound_)
{}

const std::int32_t getChatNotificationSettingsExceptions::ID;

void getChatNotificationSettingsExceptions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatNotificationSettingsExceptions");
    s.store_object_field("scope", static_cast<const BaseObject *>(scope_.get()));
    s.store_field("compare_sound", compare_sound_);
    s.store_class_end();
  }
}

getCommands::getCommands()
  : scope_()
  , language_code_()
{}

getCommands::getCommands(object_ptr<BotCommandScope> &&scope_, string const &language_code_)
  : scope_(std::move(scope_))
  , language_code_(language_code_)
{}

const std::int32_t getCommands::ID;

void getCommands::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getCommands");
    s.store_object_field("scope", static_cast<const BaseObject *>(scope_.get()));
    s.store_field("language_code", language_code_);
    s.store_class_end();
  }
}

getCountryFlagEmoji::getCountryFlagEmoji()
  : country_code_()
{}

getCountryFlagEmoji::getCountryFlagEmoji(string const &country_code_)
  : country_code_(country_code_)
{}

const std::int32_t getCountryFlagEmoji::ID;

void getCountryFlagEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getCountryFlagEmoji");
    s.store_field("country_code", country_code_);
    s.store_class_end();
  }
}

getCreatedPublicChats::getCreatedPublicChats()
  : type_()
{}

getCreatedPublicChats::getCreatedPublicChats(object_ptr<PublicChatType> &&type_)
  : type_(std::move(type_))
{}

const std::int32_t getCreatedPublicChats::ID;

void getCreatedPublicChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getCreatedPublicChats");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

getDatabaseStatistics::getDatabaseStatistics() {
}

const std::int32_t getDatabaseStatistics::ID;

void getDatabaseStatistics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getDatabaseStatistics");
    s.store_class_end();
  }
}

getEmojiSuggestionsUrl::getEmojiSuggestionsUrl()
  : language_code_()
{}

getEmojiSuggestionsUrl::getEmojiSuggestionsUrl(string const &language_code_)
  : language_code_(language_code_)
{}

const std::int32_t getEmojiSuggestionsUrl::ID;

void getEmojiSuggestionsUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getEmojiSuggestionsUrl");
    s.store_field("language_code", language_code_);
    s.store_class_end();
  }
}

getFavoriteStickers::getFavoriteStickers() {
}

const std::int32_t getFavoriteStickers::ID;

void getFavoriteStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getFavoriteStickers");
    s.store_class_end();
  }
}

getGroupsInCommon::getGroupsInCommon()
  : user_id_()
  , offset_chat_id_()
  , limit_()
{}

getGroupsInCommon::getGroupsInCommon(int53 user_id_, int53 offset_chat_id_, int32 limit_)
  : user_id_(user_id_)
  , offset_chat_id_(offset_chat_id_)
  , limit_(limit_)
{}

const std::int32_t getGroupsInCommon::ID;

void getGroupsInCommon::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getGroupsInCommon");
    s.store_field("user_id", user_id_);
    s.store_field("offset_chat_id", offset_chat_id_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getKeywordEmojis::getKeywordEmojis()
  : text_()
  , input_language_codes_()
{}

getKeywordEmojis::getKeywordEmojis(string const &text_, array<string> &&input_language_codes_)
  : text_(text_)
  , input_language_codes_(std::move(input_language_codes_))
{}

const std::int32_t getKeywordEmojis::ID;

void getKeywordEmojis::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getKeywordEmojis");
    s.store_field("text", text_);
    { s.store_vector_begin("input_language_codes", input_language_codes_.size()); for (const auto &_value : input_language_codes_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

getLinkPreview::getLinkPreview()
  : text_()
  , link_preview_options_()
{}

getLinkPreview::getLinkPreview(object_ptr<formattedText> &&text_, object_ptr<linkPreviewOptions> &&link_preview_options_)
  : text_(std::move(text_))
  , link_preview_options_(std::move(link_preview_options_))
{}

const std::int32_t getLinkPreview::ID;

void getLinkPreview::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getLinkPreview");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_object_field("link_preview_options", static_cast<const BaseObject *>(link_preview_options_.get()));
    s.store_class_end();
  }
}

getMainWebApp::getMainWebApp()
  : chat_id_()
  , bot_user_id_()
  , start_parameter_()
  , parameters_()
{}

getMainWebApp::getMainWebApp(int53 chat_id_, int53 bot_user_id_, string const &start_parameter_, object_ptr<webAppOpenParameters> &&parameters_)
  : chat_id_(chat_id_)
  , bot_user_id_(bot_user_id_)
  , start_parameter_(start_parameter_)
  , parameters_(std::move(parameters_))
{}

const std::int32_t getMainWebApp::ID;

void getMainWebApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMainWebApp");
    s.store_field("chat_id", chat_id_);
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("start_parameter", start_parameter_);
    s.store_object_field("parameters", static_cast<const BaseObject *>(parameters_.get()));
    s.store_class_end();
  }
}

getMessageAvailableReactions::getMessageAvailableReactions()
  : chat_id_()
  , message_id_()
  , row_size_()
{}

getMessageAvailableReactions::getMessageAvailableReactions(int53 chat_id_, int53 message_id_, int32 row_size_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , row_size_(row_size_)
{}

const std::int32_t getMessageAvailableReactions::ID;

void getMessageAvailableReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMessageAvailableReactions");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("row_size", row_size_);
    s.store_class_end();
  }
}

getMessageImportConfirmationText::getMessageImportConfirmationText()
  : chat_id_()
{}

getMessageImportConfirmationText::getMessageImportConfirmationText(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t getMessageImportConfirmationText::ID;

void getMessageImportConfirmationText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMessageImportConfirmationText");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

getPaymentForm::getPaymentForm()
  : input_invoice_()
  , theme_()
{}

getPaymentForm::getPaymentForm(object_ptr<InputInvoice> &&input_invoice_, object_ptr<themeParameters> &&theme_)
  : input_invoice_(std::move(input_invoice_))
  , theme_(std::move(theme_))
{}

const std::int32_t getPaymentForm::ID;

void getPaymentForm::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPaymentForm");
    s.store_object_field("input_invoice", static_cast<const BaseObject *>(input_invoice_.get()));
    s.store_object_field("theme", static_cast<const BaseObject *>(theme_.get()));
    s.store_class_end();
  }
}

getPremiumLimit::getPremiumLimit()
  : limit_type_()
{}

getPremiumLimit::getPremiumLimit(object_ptr<PremiumLimitType> &&limit_type_)
  : limit_type_(std::move(limit_type_))
{}

const std::int32_t getPremiumLimit::ID;

void getPremiumLimit::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPremiumLimit");
    s.store_object_field("limit_type", static_cast<const BaseObject *>(limit_type_.get()));
    s.store_class_end();
  }
}

getPremiumState::getPremiumState() {
}

const std::int32_t getPremiumState::ID;

void getPremiumState::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPremiumState");
    s.store_class_end();
  }
}

getRecentlyVisitedTMeUrls::getRecentlyVisitedTMeUrls()
  : referrer_()
{}

getRecentlyVisitedTMeUrls::getRecentlyVisitedTMeUrls(string const &referrer_)
  : referrer_(referrer_)
{}

const std::int32_t getRecentlyVisitedTMeUrls::ID;

void getRecentlyVisitedTMeUrls::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getRecentlyVisitedTMeUrls");
    s.store_field("referrer", referrer_);
    s.store_class_end();
  }
}

getRecommendedChatFolders::getRecommendedChatFolders() {
}

const std::int32_t getRecommendedChatFolders::ID;

void getRecommendedChatFolders::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getRecommendedChatFolders");
    s.store_class_end();
  }
}

getSavedMessagesTags::getSavedMessagesTags()
  : saved_messages_topic_id_()
{}

getSavedMessagesTags::getSavedMessagesTags(int53 saved_messages_topic_id_)
  : saved_messages_topic_id_(saved_messages_topic_id_)
{}

const std::int32_t getSavedMessagesTags::ID;

void getSavedMessagesTags::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getSavedMessagesTags");
    s.store_field("saved_messages_topic_id", saved_messages_topic_id_);
    s.store_class_end();
  }
}

getSavedNotificationSounds::getSavedNotificationSounds() {
}

const std::int32_t getSavedNotificationSounds::ID;

void getSavedNotificationSounds::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getSavedNotificationSounds");
    s.store_class_end();
  }
}

getStoryPublicForwards::getStoryPublicForwards()
  : story_poster_chat_id_()
  , story_id_()
  , offset_()
  , limit_()
{}

getStoryPublicForwards::getStoryPublicForwards(int53 story_poster_chat_id_, int32 story_id_, string const &offset_, int32 limit_)
  : story_poster_chat_id_(story_poster_chat_id_)
  , story_id_(story_id_)
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getStoryPublicForwards::ID;

void getStoryPublicForwards::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStoryPublicForwards");
    s.store_field("story_poster_chat_id", story_poster_chat_id_);
    s.store_field("story_id", story_id_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getSuggestedFileName::getSuggestedFileName()
  : file_id_()
  , directory_()
{}

getSuggestedFileName::getSuggestedFileName(int32 file_id_, string const &directory_)
  : file_id_(file_id_)
  , directory_(directory_)
{}

const std::int32_t getSuggestedFileName::ID;

void getSuggestedFileName::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getSuggestedFileName");
    s.store_field("file_id", file_id_);
    s.store_field("directory", directory_);
    s.store_class_end();
  }
}

getSuitablePersonalChats::getSuitablePersonalChats() {
}

const std::int32_t getSuitablePersonalChats::ID;

void getSuitablePersonalChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getSuitablePersonalChats");
    s.store_class_end();
  }
}

getTextEntities::getTextEntities()
  : text_()
{}

getTextEntities::getTextEntities(string const &text_)
  : text_(text_)
{}

const std::int32_t getTextEntities::ID;

void getTextEntities::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getTextEntities");
    s.store_field("text", text_);
    s.store_class_end();
  }
}

getUpgradedGift::getUpgradedGift()
  : name_()
{}

getUpgradedGift::getUpgradedGift(string const &name_)
  : name_(name_)
{}

const std::int32_t getUpgradedGift::ID;

void getUpgradedGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getUpgradedGift");
    s.store_field("name", name_);
    s.store_class_end();
  }
}

getUserPrivacySettingRules::getUserPrivacySettingRules()
  : setting_()
{}

getUserPrivacySettingRules::getUserPrivacySettingRules(object_ptr<UserPrivacySetting> &&setting_)
  : setting_(std::move(setting_))
{}

const std::int32_t getUserPrivacySettingRules::ID;

void getUserPrivacySettingRules::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getUserPrivacySettingRules");
    s.store_object_field("setting", static_cast<const BaseObject *>(setting_.get()));
    s.store_class_end();
  }
}

getVideoChatStreams::getVideoChatStreams()
  : group_call_id_()
{}

getVideoChatStreams::getVideoChatStreams(int32 group_call_id_)
  : group_call_id_(group_call_id_)
{}

const std::int32_t getVideoChatStreams::ID;

void getVideoChatStreams::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getVideoChatStreams");
    s.store_field("group_call_id", group_call_id_);
    s.store_class_end();
  }
}

giftPremiumWithStars::giftPremiumWithStars()
  : user_id_()
  , star_count_()
  , month_count_()
  , text_()
{}

giftPremiumWithStars::giftPremiumWithStars(int53 user_id_, int53 star_count_, int32 month_count_, object_ptr<formattedText> &&text_)
  : user_id_(user_id_)
  , star_count_(star_count_)
  , month_count_(month_count_)
  , text_(std::move(text_))
{}

const std::int32_t giftPremiumWithStars::ID;

void giftPremiumWithStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giftPremiumWithStars");
    s.store_field("user_id", user_id_);
    s.store_field("star_count", star_count_);
    s.store_field("month_count", month_count_);
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

inviteVideoChatParticipants::inviteVideoChatParticipants()
  : group_call_id_()
  , user_ids_()
{}

inviteVideoChatParticipants::inviteVideoChatParticipants(int32 group_call_id_, array<int53> &&user_ids_)
  : group_call_id_(group_call_id_)
  , user_ids_(std::move(user_ids_))
{}

const std::int32_t inviteVideoChatParticipants::ID;

void inviteVideoChatParticipants::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inviteVideoChatParticipants");
    s.store_field("group_call_id", group_call_id_);
    { s.store_vector_begin("user_ids", user_ids_.size()); for (const auto &_value : user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

openMessageContent::openMessageContent()
  : chat_id_()
  , message_id_()
{}

openMessageContent::openMessageContent(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t openMessageContent::ID;

void openMessageContent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "openMessageContent");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

optimizeStorage::optimizeStorage()
  : size_()
  , ttl_()
  , count_()
  , immunity_delay_()
  , file_types_()
  , chat_ids_()
  , exclude_chat_ids_()
  , return_deleted_file_statistics_()
  , chat_limit_()
{}

optimizeStorage::optimizeStorage(int53 size_, int32 ttl_, int32 count_, int32 immunity_delay_, array<object_ptr<FileType>> &&file_types_, array<int53> &&chat_ids_, array<int53> &&exclude_chat_ids_, bool return_deleted_file_statistics_, int32 chat_limit_)
  : size_(size_)
  , ttl_(ttl_)
  , count_(count_)
  , immunity_delay_(immunity_delay_)
  , file_types_(std::move(file_types_))
  , chat_ids_(std::move(chat_ids_))
  , exclude_chat_ids_(std::move(exclude_chat_ids_))
  , return_deleted_file_statistics_(return_deleted_file_statistics_)
  , chat_limit_(chat_limit_)
{}

const std::int32_t optimizeStorage::ID;

void optimizeStorage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "optimizeStorage");
    s.store_field("size", size_);
    s.store_field("ttl", ttl_);
    s.store_field("count", count_);
    s.store_field("immunity_delay", immunity_delay_);
    { s.store_vector_begin("file_types", file_types_.size()); for (const auto &_value : file_types_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("chat_ids", chat_ids_.size()); for (const auto &_value : chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("exclude_chat_ids", exclude_chat_ids_.size()); for (const auto &_value : exclude_chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("return_deleted_file_statistics", return_deleted_file_statistics_);
    s.store_field("chat_limit", chat_limit_);
    s.store_class_end();
  }
}

pinChatMessage::pinChatMessage()
  : chat_id_()
  , message_id_()
  , disable_notification_()
  , only_for_self_()
{}

pinChatMessage::pinChatMessage(int53 chat_id_, int53 message_id_, bool disable_notification_, bool only_for_self_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , disable_notification_(disable_notification_)
  , only_for_self_(only_for_self_)
{}

const std::int32_t pinChatMessage::ID;

void pinChatMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pinChatMessage");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("disable_notification", disable_notification_);
    s.store_field("only_for_self", only_for_self_);
    s.store_class_end();
  }
}

processChatFolderNewChats::processChatFolderNewChats()
  : chat_folder_id_()
  , added_chat_ids_()
{}

processChatFolderNewChats::processChatFolderNewChats(int32 chat_folder_id_, array<int53> &&added_chat_ids_)
  : chat_folder_id_(chat_folder_id_)
  , added_chat_ids_(std::move(added_chat_ids_))
{}

const std::int32_t processChatFolderNewChats::ID;

void processChatFolderNewChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "processChatFolderNewChats");
    s.store_field("chat_folder_id", chat_folder_id_);
    { s.store_vector_begin("added_chat_ids", added_chat_ids_.size()); for (const auto &_value : added_chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

refundStarPayment::refundStarPayment()
  : user_id_()
  , telegram_payment_charge_id_()
{}

refundStarPayment::refundStarPayment(int53 user_id_, string const &telegram_payment_charge_id_)
  : user_id_(user_id_)
  , telegram_payment_charge_id_(telegram_payment_charge_id_)
{}

const std::int32_t refundStarPayment::ID;

void refundStarPayment::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "refundStarPayment");
    s.store_field("user_id", user_id_);
    s.store_field("telegram_payment_charge_id", telegram_payment_charge_id_);
    s.store_class_end();
  }
}

registerUser::registerUser()
  : first_name_()
  , last_name_()
  , disable_notification_()
{}

registerUser::registerUser(string const &first_name_, string const &last_name_, bool disable_notification_)
  : first_name_(first_name_)
  , last_name_(last_name_)
  , disable_notification_(disable_notification_)
{}

const std::int32_t registerUser::ID;

void registerUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "registerUser");
    s.store_field("first_name", first_name_);
    s.store_field("last_name", last_name_);
    s.store_field("disable_notification", disable_notification_);
    s.store_class_end();
  }
}

removeProxy::removeProxy()
  : proxy_id_()
{}

removeProxy::removeProxy(int32 proxy_id_)
  : proxy_id_(proxy_id_)
{}

const std::int32_t removeProxy::ID;

void removeProxy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeProxy");
    s.store_field("proxy_id", proxy_id_);
    s.store_class_end();
  }
}

reorderSupergroupActiveUsernames::reorderSupergroupActiveUsernames()
  : supergroup_id_()
  , usernames_()
{}

reorderSupergroupActiveUsernames::reorderSupergroupActiveUsernames(int53 supergroup_id_, array<string> &&usernames_)
  : supergroup_id_(supergroup_id_)
  , usernames_(std::move(usernames_))
{}

const std::int32_t reorderSupergroupActiveUsernames::ID;

void reorderSupergroupActiveUsernames::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reorderSupergroupActiveUsernames");
    s.store_field("supergroup_id", supergroup_id_);
    { s.store_vector_begin("usernames", usernames_.size()); for (const auto &_value : usernames_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

reportChatPhoto::reportChatPhoto()
  : chat_id_()
  , file_id_()
  , reason_()
  , text_()
{}

reportChatPhoto::reportChatPhoto(int53 chat_id_, int32 file_id_, object_ptr<ReportReason> &&reason_, string const &text_)
  : chat_id_(chat_id_)
  , file_id_(file_id_)
  , reason_(std::move(reason_))
  , text_(text_)
{}

const std::int32_t reportChatPhoto::ID;

void reportChatPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reportChatPhoto");
    s.store_field("chat_id", chat_id_);
    s.store_field("file_id", file_id_);
    s.store_object_field("reason", static_cast<const BaseObject *>(reason_.get()));
    s.store_field("text", text_);
    s.store_class_end();
  }
}

resendAuthenticationCode::resendAuthenticationCode()
  : reason_()
{}

resendAuthenticationCode::resendAuthenticationCode(object_ptr<ResendCodeReason> &&reason_)
  : reason_(std::move(reason_))
{}

const std::int32_t resendAuthenticationCode::ID;

void resendAuthenticationCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "resendAuthenticationCode");
    s.store_object_field("reason", static_cast<const BaseObject *>(reason_.get()));
    s.store_class_end();
  }
}

resetAllNotificationSettings::resetAllNotificationSettings() {
}

const std::int32_t resetAllNotificationSettings::ID;

void resetAllNotificationSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "resetAllNotificationSettings");
    s.store_class_end();
  }
}

searchAffiliatePrograms::searchAffiliatePrograms()
  : affiliate_()
  , sort_order_()
  , offset_()
  , limit_()
{}

searchAffiliatePrograms::searchAffiliatePrograms(object_ptr<AffiliateType> &&affiliate_, object_ptr<AffiliateProgramSortOrder> &&sort_order_, string const &offset_, int32 limit_)
  : affiliate_(std::move(affiliate_))
  , sort_order_(std::move(sort_order_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t searchAffiliatePrograms::ID;

void searchAffiliatePrograms::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchAffiliatePrograms");
    s.store_object_field("affiliate", static_cast<const BaseObject *>(affiliate_.get()));
    s.store_object_field("sort_order", static_cast<const BaseObject *>(sort_order_.get()));
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

searchPublicPosts::searchPublicPosts()
  : query_()
  , offset_()
  , limit_()
  , star_count_()
{}

searchPublicPosts::searchPublicPosts(string const &query_, string const &offset_, int32 limit_, int53 star_count_)
  : query_(query_)
  , offset_(offset_)
  , limit_(limit_)
  , star_count_(star_count_)
{}

const std::int32_t searchPublicPosts::ID;

void searchPublicPosts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchPublicPosts");
    s.store_field("query", query_);
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

searchStickers::searchStickers()
  : sticker_type_()
  , emojis_()
  , query_()
  , input_language_codes_()
  , offset_()
  , limit_()
{}

searchStickers::searchStickers(object_ptr<StickerType> &&sticker_type_, string const &emojis_, string const &query_, array<string> &&input_language_codes_, int32 offset_, int32 limit_)
  : sticker_type_(std::move(sticker_type_))
  , emojis_(emojis_)
  , query_(query_)
  , input_language_codes_(std::move(input_language_codes_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t searchStickers::ID;

void searchStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "searchStickers");
    s.store_object_field("sticker_type", static_cast<const BaseObject *>(sticker_type_.get()));
    s.store_field("emojis", emojis_);
    s.store_field("query", query_);
    { s.store_vector_begin("input_language_codes", input_language_codes_.size()); for (const auto &_value : input_language_codes_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

sendCallSignalingData::sendCallSignalingData()
  : call_id_()
  , data_()
{}

sendCallSignalingData::sendCallSignalingData(int32 call_id_, bytes const &data_)
  : call_id_(call_id_)
  , data_(std::move(data_))
{}

const std::int32_t sendCallSignalingData::ID;

void sendCallSignalingData::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendCallSignalingData");
    s.store_field("call_id", call_id_);
    s.store_bytes_field("data", data_);
    s.store_class_end();
  }
}

setAuthenticationEmailAddress::setAuthenticationEmailAddress()
  : email_address_()
{}

setAuthenticationEmailAddress::setAuthenticationEmailAddress(string const &email_address_)
  : email_address_(email_address_)
{}

const std::int32_t setAuthenticationEmailAddress::ID;

void setAuthenticationEmailAddress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setAuthenticationEmailAddress");
    s.store_field("email_address", email_address_);
    s.store_class_end();
  }
}

setBio::setBio()
  : bio_()
{}

setBio::setBio(string const &bio_)
  : bio_(bio_)
{}

const std::int32_t setBio::ID;

void setBio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setBio");
    s.store_field("bio", bio_);
    s.store_class_end();
  }
}

setBusinessAccountGiftSettings::setBusinessAccountGiftSettings()
  : business_connection_id_()
  , settings_()
{}

setBusinessAccountGiftSettings::setBusinessAccountGiftSettings(string const &business_connection_id_, object_ptr<giftSettings> &&settings_)
  : business_connection_id_(business_connection_id_)
  , settings_(std::move(settings_))
{}

const std::int32_t setBusinessAccountGiftSettings::ID;

void setBusinessAccountGiftSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setBusinessAccountGiftSettings");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

setChatPhoto::setChatPhoto()
  : chat_id_()
  , photo_()
{}

setChatPhoto::setChatPhoto(int53 chat_id_, object_ptr<InputChatPhoto> &&photo_)
  : chat_id_(chat_id_)
  , photo_(std::move(photo_))
{}

const std::int32_t setChatPhoto::ID;

void setChatPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatPhoto");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

setDefaultGroupAdministratorRights::setDefaultGroupAdministratorRights()
  : default_group_administrator_rights_()
{}

setDefaultGroupAdministratorRights::setDefaultGroupAdministratorRights(object_ptr<chatAdministratorRights> &&default_group_administrator_rights_)
  : default_group_administrator_rights_(std::move(default_group_administrator_rights_))
{}

const std::int32_t setDefaultGroupAdministratorRights::ID;

void setDefaultGroupAdministratorRights::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setDefaultGroupAdministratorRights");
    s.store_object_field("default_group_administrator_rights", static_cast<const BaseObject *>(default_group_administrator_rights_.get()));
    s.store_class_end();
  }
}

setMessageSenderBotVerification::setMessageSenderBotVerification()
  : bot_user_id_()
  , verified_id_()
  , custom_description_()
{}

setMessageSenderBotVerification::setMessageSenderBotVerification(int53 bot_user_id_, object_ptr<MessageSender> &&verified_id_, string const &custom_description_)
  : bot_user_id_(bot_user_id_)
  , verified_id_(std::move(verified_id_))
  , custom_description_(custom_description_)
{}

const std::int32_t setMessageSenderBotVerification::ID;

void setMessageSenderBotVerification::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setMessageSenderBotVerification");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_object_field("verified_id", static_cast<const BaseObject *>(verified_id_.get()));
    s.store_field("custom_description", custom_description_);
    s.store_class_end();
  }
}

setNetworkType::setNetworkType()
  : type_()
{}

setNetworkType::setNetworkType(object_ptr<NetworkType> &&type_)
  : type_(std::move(type_))
{}

const std::int32_t setNetworkType::ID;

void setNetworkType::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setNetworkType");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

setProfileAccentColor::setProfileAccentColor()
  : profile_accent_color_id_()
  , profile_background_custom_emoji_id_()
{}

setProfileAccentColor::setProfileAccentColor(int32 profile_accent_color_id_, int64 profile_background_custom_emoji_id_)
  : profile_accent_color_id_(profile_accent_color_id_)
  , profile_background_custom_emoji_id_(profile_background_custom_emoji_id_)
{}

const std::int32_t setProfileAccentColor::ID;

void setProfileAccentColor::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setProfileAccentColor");
    s.store_field("profile_accent_color_id", profile_accent_color_id_);
    s.store_field("profile_background_custom_emoji_id", profile_background_custom_emoji_id_);
    s.store_class_end();
  }
}

testProxy::testProxy()
  : server_()
  , port_()
  , type_()
  , dc_id_()
  , timeout_()
{}

testProxy::testProxy(string const &server_, int32 port_, object_ptr<ProxyType> &&type_, int32 dc_id_, double timeout_)
  : server_(server_)
  , port_(port_)
  , type_(std::move(type_))
  , dc_id_(dc_id_)
  , timeout_(timeout_)
{}

const std::int32_t testProxy::ID;

void testProxy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testProxy");
    s.store_field("server", server_);
    s.store_field("port", port_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("dc_id", dc_id_);
    s.store_field("timeout", timeout_);
    s.store_class_end();
  }
}

toggleAllDownloadsArePaused::toggleAllDownloadsArePaused()
  : are_paused_()
{}

toggleAllDownloadsArePaused::toggleAllDownloadsArePaused(bool are_paused_)
  : are_paused_(are_paused_)
{}

const std::int32_t toggleAllDownloadsArePaused::ID;

void toggleAllDownloadsArePaused::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleAllDownloadsArePaused");
    s.store_field("are_paused", are_paused_);
    s.store_class_end();
  }
}

toggleBotIsAddedToAttachmentMenu::toggleBotIsAddedToAttachmentMenu()
  : bot_user_id_()
  , is_added_()
  , allow_write_access_()
{}

toggleBotIsAddedToAttachmentMenu::toggleBotIsAddedToAttachmentMenu(int53 bot_user_id_, bool is_added_, bool allow_write_access_)
  : bot_user_id_(bot_user_id_)
  , is_added_(is_added_)
  , allow_write_access_(allow_write_access_)
{}

const std::int32_t toggleBotIsAddedToAttachmentMenu::ID;

void toggleBotIsAddedToAttachmentMenu::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleBotIsAddedToAttachmentMenu");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("is_added", is_added_);
    s.store_field("allow_write_access", allow_write_access_);
    s.store_class_end();
  }
}

toggleChatGiftNotifications::toggleChatGiftNotifications()
  : chat_id_()
  , are_enabled_()
{}

toggleChatGiftNotifications::toggleChatGiftNotifications(int53 chat_id_, bool are_enabled_)
  : chat_id_(chat_id_)
  , are_enabled_(are_enabled_)
{}

const std::int32_t toggleChatGiftNotifications::ID;

void toggleChatGiftNotifications::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleChatGiftNotifications");
    s.store_field("chat_id", chat_id_);
    s.store_field("are_enabled", are_enabled_);
    s.store_class_end();
  }
}

toggleForumTopicIsClosed::toggleForumTopicIsClosed()
  : chat_id_()
  , forum_topic_id_()
  , is_closed_()
{}

toggleForumTopicIsClosed::toggleForumTopicIsClosed(int53 chat_id_, int32 forum_topic_id_, bool is_closed_)
  : chat_id_(chat_id_)
  , forum_topic_id_(forum_topic_id_)
  , is_closed_(is_closed_)
{}

const std::int32_t toggleForumTopicIsClosed::ID;

void toggleForumTopicIsClosed::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleForumTopicIsClosed");
    s.store_field("chat_id", chat_id_);
    s.store_field("forum_topic_id", forum_topic_id_);
    s.store_field("is_closed", is_closed_);
    s.store_class_end();
  }
}

toggleSupergroupJoinByRequest::toggleSupergroupJoinByRequest()
  : supergroup_id_()
  , join_by_request_()
{}

toggleSupergroupJoinByRequest::toggleSupergroupJoinByRequest(int53 supergroup_id_, bool join_by_request_)
  : supergroup_id_(supergroup_id_)
  , join_by_request_(join_by_request_)
{}

const std::int32_t toggleSupergroupJoinByRequest::ID;

void toggleSupergroupJoinByRequest::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleSupergroupJoinByRequest");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_field("join_by_request", join_by_request_);
    s.store_class_end();
  }
}

toggleSupergroupUsernameIsActive::toggleSupergroupUsernameIsActive()
  : supergroup_id_()
  , username_()
  , is_active_()
{}

toggleSupergroupUsernameIsActive::toggleSupergroupUsernameIsActive(int53 supergroup_id_, string const &username_, bool is_active_)
  : supergroup_id_(supergroup_id_)
  , username_(username_)
  , is_active_(is_active_)
{}

const std::int32_t toggleSupergroupUsernameIsActive::ID;

void toggleSupergroupUsernameIsActive::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleSupergroupUsernameIsActive");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_field("username", username_);
    s.store_field("is_active", is_active_);
    s.store_class_end();
  }
}
}  // namespace td_api
}  // namespace td
