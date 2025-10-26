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


addedReaction::addedReaction()
  : type_()
  , sender_id_()
  , is_outgoing_()
  , date_()
{}

addedReaction::addedReaction(object_ptr<ReactionType> &&type_, object_ptr<MessageSender> &&sender_id_, bool is_outgoing_, int32 date_)
  : type_(std::move(type_))
  , sender_id_(std::move(sender_id_))
  , is_outgoing_(is_outgoing_)
  , date_(date_)
{}

const std::int32_t addedReaction::ID;

void addedReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addedReaction");
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_field("is_outgoing", is_outgoing_);
    s.store_field("date", date_);
    s.store_class_end();
  }
}

animatedChatPhoto::animatedChatPhoto()
  : length_()
  , file_()
  , main_frame_timestamp_()
{}

animatedChatPhoto::animatedChatPhoto(int32 length_, object_ptr<file> &&file_, double main_frame_timestamp_)
  : length_(length_)
  , file_(std::move(file_))
  , main_frame_timestamp_(main_frame_timestamp_)
{}

const std::int32_t animatedChatPhoto::ID;

void animatedChatPhoto::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "animatedChatPhoto");
    s.store_field("length", length_);
    s.store_object_field("file", static_cast<const BaseObject *>(file_.get()));
    s.store_field("main_frame_timestamp", main_frame_timestamp_);
    s.store_class_end();
  }
}

authenticationCodeTypeTelegramMessage::authenticationCodeTypeTelegramMessage()
  : length_()
{}

authenticationCodeTypeTelegramMessage::authenticationCodeTypeTelegramMessage(int32 length_)
  : length_(length_)
{}

const std::int32_t authenticationCodeTypeTelegramMessage::ID;

void authenticationCodeTypeTelegramMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authenticationCodeTypeTelegramMessage");
    s.store_field("length", length_);
    s.store_class_end();
  }
}

authenticationCodeTypeSms::authenticationCodeTypeSms()
  : length_()
{}

authenticationCodeTypeSms::authenticationCodeTypeSms(int32 length_)
  : length_(length_)
{}

const std::int32_t authenticationCodeTypeSms::ID;

void authenticationCodeTypeSms::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authenticationCodeTypeSms");
    s.store_field("length", length_);
    s.store_class_end();
  }
}

authenticationCodeTypeSmsWord::authenticationCodeTypeSmsWord()
  : first_letter_()
{}

authenticationCodeTypeSmsWord::authenticationCodeTypeSmsWord(string const &first_letter_)
  : first_letter_(first_letter_)
{}

const std::int32_t authenticationCodeTypeSmsWord::ID;

void authenticationCodeTypeSmsWord::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authenticationCodeTypeSmsWord");
    s.store_field("first_letter", first_letter_);
    s.store_class_end();
  }
}

authenticationCodeTypeSmsPhrase::authenticationCodeTypeSmsPhrase()
  : first_word_()
{}

authenticationCodeTypeSmsPhrase::authenticationCodeTypeSmsPhrase(string const &first_word_)
  : first_word_(first_word_)
{}

const std::int32_t authenticationCodeTypeSmsPhrase::ID;

void authenticationCodeTypeSmsPhrase::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authenticationCodeTypeSmsPhrase");
    s.store_field("first_word", first_word_);
    s.store_class_end();
  }
}

authenticationCodeTypeCall::authenticationCodeTypeCall()
  : length_()
{}

authenticationCodeTypeCall::authenticationCodeTypeCall(int32 length_)
  : length_(length_)
{}

const std::int32_t authenticationCodeTypeCall::ID;

void authenticationCodeTypeCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authenticationCodeTypeCall");
    s.store_field("length", length_);
    s.store_class_end();
  }
}

authenticationCodeTypeFlashCall::authenticationCodeTypeFlashCall()
  : pattern_()
{}

authenticationCodeTypeFlashCall::authenticationCodeTypeFlashCall(string const &pattern_)
  : pattern_(pattern_)
{}

const std::int32_t authenticationCodeTypeFlashCall::ID;

void authenticationCodeTypeFlashCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authenticationCodeTypeFlashCall");
    s.store_field("pattern", pattern_);
    s.store_class_end();
  }
}

authenticationCodeTypeMissedCall::authenticationCodeTypeMissedCall()
  : phone_number_prefix_()
  , length_()
{}

authenticationCodeTypeMissedCall::authenticationCodeTypeMissedCall(string const &phone_number_prefix_, int32 length_)
  : phone_number_prefix_(phone_number_prefix_)
  , length_(length_)
{}

const std::int32_t authenticationCodeTypeMissedCall::ID;

void authenticationCodeTypeMissedCall::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authenticationCodeTypeMissedCall");
    s.store_field("phone_number_prefix", phone_number_prefix_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

authenticationCodeTypeFragment::authenticationCodeTypeFragment()
  : url_()
  , length_()
{}

authenticationCodeTypeFragment::authenticationCodeTypeFragment(string const &url_, int32 length_)
  : url_(url_)
  , length_(length_)
{}

const std::int32_t authenticationCodeTypeFragment::ID;

void authenticationCodeTypeFragment::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authenticationCodeTypeFragment");
    s.store_field("url", url_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

authenticationCodeTypeFirebaseAndroid::authenticationCodeTypeFirebaseAndroid()
  : device_verification_parameters_()
  , length_()
{}

authenticationCodeTypeFirebaseAndroid::authenticationCodeTypeFirebaseAndroid(object_ptr<FirebaseDeviceVerificationParameters> &&device_verification_parameters_, int32 length_)
  : device_verification_parameters_(std::move(device_verification_parameters_))
  , length_(length_)
{}

const std::int32_t authenticationCodeTypeFirebaseAndroid::ID;

void authenticationCodeTypeFirebaseAndroid::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authenticationCodeTypeFirebaseAndroid");
    s.store_object_field("device_verification_parameters", static_cast<const BaseObject *>(device_verification_parameters_.get()));
    s.store_field("length", length_);
    s.store_class_end();
  }
}

authenticationCodeTypeFirebaseIos::authenticationCodeTypeFirebaseIos()
  : receipt_()
  , push_timeout_()
  , length_()
{}

authenticationCodeTypeFirebaseIos::authenticationCodeTypeFirebaseIos(string const &receipt_, int32 push_timeout_, int32 length_)
  : receipt_(receipt_)
  , push_timeout_(push_timeout_)
  , length_(length_)
{}

const std::int32_t authenticationCodeTypeFirebaseIos::ID;

void authenticationCodeTypeFirebaseIos::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "authenticationCodeTypeFirebaseIos");
    s.store_field("receipt", receipt_);
    s.store_field("push_timeout", push_timeout_);
    s.store_field("length", length_);
    s.store_class_end();
  }
}

availableGift::availableGift()
  : gift_()
  , resale_count_()
  , min_resale_star_count_()
  , title_()
{}

availableGift::availableGift(object_ptr<gift> &&gift_, int32 resale_count_, int53 min_resale_star_count_, string const &title_)
  : gift_(std::move(gift_))
  , resale_count_(resale_count_)
  , min_resale_star_count_(min_resale_star_count_)
  , title_(title_)
{}

const std::int32_t availableGift::ID;

void availableGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "availableGift");
    s.store_object_field("gift", static_cast<const BaseObject *>(gift_.get()));
    s.store_field("resale_count", resale_count_);
    s.store_field("min_resale_star_count", min_resale_star_count_);
    s.store_field("title", title_);
    s.store_class_end();
  }
}

basicGroup::basicGroup()
  : id_()
  , member_count_()
  , status_()
  , is_active_()
  , upgraded_to_supergroup_id_()
{}

basicGroup::basicGroup(int53 id_, int32 member_count_, object_ptr<ChatMemberStatus> &&status_, bool is_active_, int53 upgraded_to_supergroup_id_)
  : id_(id_)
  , member_count_(member_count_)
  , status_(std::move(status_))
  , is_active_(is_active_)
  , upgraded_to_supergroup_id_(upgraded_to_supergroup_id_)
{}

const std::int32_t basicGroup::ID;

void basicGroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "basicGroup");
    s.store_field("id", id_);
    s.store_field("member_count", member_count_);
    s.store_object_field("status", static_cast<const BaseObject *>(status_.get()));
    s.store_field("is_active", is_active_);
    s.store_field("upgraded_to_supergroup_id", upgraded_to_supergroup_id_);
    s.store_class_end();
  }
}

businessBotManageBar::businessBotManageBar()
  : bot_user_id_()
  , manage_url_()
  , is_bot_paused_()
  , can_bot_reply_()
{}

businessBotManageBar::businessBotManageBar(int53 bot_user_id_, string const &manage_url_, bool is_bot_paused_, bool can_bot_reply_)
  : bot_user_id_(bot_user_id_)
  , manage_url_(manage_url_)
  , is_bot_paused_(is_bot_paused_)
  , can_bot_reply_(can_bot_reply_)
{}

const std::int32_t businessBotManageBar::ID;

void businessBotManageBar::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessBotManageBar");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("manage_url", manage_url_);
    s.store_field("is_bot_paused", is_bot_paused_);
    s.store_field("can_bot_reply", can_bot_reply_);
    s.store_class_end();
  }
}

businessLocation::businessLocation()
  : location_()
  , address_()
{}

businessLocation::businessLocation(object_ptr<location> &&location_, string const &address_)
  : location_(std::move(location_))
  , address_(address_)
{}

const std::int32_t businessLocation::ID;

void businessLocation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "businessLocation");
    s.store_object_field("location", static_cast<const BaseObject *>(location_.get()));
    s.store_field("address", address_);
    s.store_class_end();
  }
}

callId::callId()
  : id_()
{}

callId::callId(int32 id_)
  : id_(id_)
{}

const std::int32_t callId::ID;

void callId::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callId");
    s.store_field("id", id_);
    s.store_class_end();
  }
}

callStatePending::callStatePending()
  : is_created_()
  , is_received_()
{}

callStatePending::callStatePending(bool is_created_, bool is_received_)
  : is_created_(is_created_)
  , is_received_(is_received_)
{}

const std::int32_t callStatePending::ID;

void callStatePending::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callStatePending");
    s.store_field("is_created", is_created_);
    s.store_field("is_received", is_received_);
    s.store_class_end();
  }
}

callStateExchangingKeys::callStateExchangingKeys() {
}

const std::int32_t callStateExchangingKeys::ID;

void callStateExchangingKeys::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callStateExchangingKeys");
    s.store_class_end();
  }
}

callStateReady::callStateReady()
  : protocol_()
  , servers_()
  , config_()
  , encryption_key_()
  , emojis_()
  , allow_p2p_()
  , is_group_call_supported_()
  , custom_parameters_()
{}

callStateReady::callStateReady(object_ptr<callProtocol> &&protocol_, array<object_ptr<callServer>> &&servers_, string const &config_, bytes const &encryption_key_, array<string> &&emojis_, bool allow_p2p_, bool is_group_call_supported_, string const &custom_parameters_)
  : protocol_(std::move(protocol_))
  , servers_(std::move(servers_))
  , config_(config_)
  , encryption_key_(std::move(encryption_key_))
  , emojis_(std::move(emojis_))
  , allow_p2p_(allow_p2p_)
  , is_group_call_supported_(is_group_call_supported_)
  , custom_parameters_(custom_parameters_)
{}

const std::int32_t callStateReady::ID;

void callStateReady::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callStateReady");
    s.store_object_field("protocol", static_cast<const BaseObject *>(protocol_.get()));
    { s.store_vector_begin("servers", servers_.size()); for (const auto &_value : servers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("config", config_);
    s.store_bytes_field("encryption_key", encryption_key_);
    { s.store_vector_begin("emojis", emojis_.size()); for (const auto &_value : emojis_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("allow_p2p", allow_p2p_);
    s.store_field("is_group_call_supported", is_group_call_supported_);
    s.store_field("custom_parameters", custom_parameters_);
    s.store_class_end();
  }
}

callStateHangingUp::callStateHangingUp() {
}

const std::int32_t callStateHangingUp::ID;

void callStateHangingUp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callStateHangingUp");
    s.store_class_end();
  }
}

callStateDiscarded::callStateDiscarded()
  : reason_()
  , need_rating_()
  , need_debug_information_()
  , need_log_()
{}

callStateDiscarded::callStateDiscarded(object_ptr<CallDiscardReason> &&reason_, bool need_rating_, bool need_debug_information_, bool need_log_)
  : reason_(std::move(reason_))
  , need_rating_(need_rating_)
  , need_debug_information_(need_debug_information_)
  , need_log_(need_log_)
{}

const std::int32_t callStateDiscarded::ID;

void callStateDiscarded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callStateDiscarded");
    s.store_object_field("reason", static_cast<const BaseObject *>(reason_.get()));
    s.store_field("need_rating", need_rating_);
    s.store_field("need_debug_information", need_debug_information_);
    s.store_field("need_log", need_log_);
    s.store_class_end();
  }
}

callStateError::callStateError()
  : error_()
{}

callStateError::callStateError(object_ptr<error> &&error_)
  : error_(std::move(error_))
{}

const std::int32_t callStateError::ID;

void callStateError::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "callStateError");
    s.store_object_field("error", static_cast<const BaseObject *>(error_.get()));
    s.store_class_end();
  }
}

chatFolderIcon::chatFolderIcon()
  : name_()
{}

chatFolderIcon::chatFolderIcon(string const &name_)
  : name_(name_)
{}

const std::int32_t chatFolderIcon::ID;

void chatFolderIcon::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatFolderIcon");
    s.store_field("name", name_);
    s.store_class_end();
  }
}

chatFolderInviteLink::chatFolderInviteLink()
  : invite_link_()
  , name_()
  , chat_ids_()
{}

chatFolderInviteLink::chatFolderInviteLink(string const &invite_link_, string const &name_, array<int53> &&chat_ids_)
  : invite_link_(invite_link_)
  , name_(name_)
  , chat_ids_(std::move(chat_ids_))
{}

const std::int32_t chatFolderInviteLink::ID;

void chatFolderInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatFolderInviteLink");
    s.store_field("invite_link", invite_link_);
    s.store_field("name", name_);
    { s.store_vector_begin("chat_ids", chat_ids_.size()); for (const auto &_value : chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatInviteLinkInfo::chatInviteLinkInfo()
  : chat_id_()
  , accessible_for_()
  , type_()
  , title_()
  , photo_()
  , accent_color_id_()
  , description_()
  , member_count_()
  , member_user_ids_()
  , subscription_info_()
  , creates_join_request_()
  , is_public_()
  , verification_status_()
{}

chatInviteLinkInfo::chatInviteLinkInfo(int53 chat_id_, int32 accessible_for_, object_ptr<InviteLinkChatType> &&type_, string const &title_, object_ptr<chatPhotoInfo> &&photo_, int32 accent_color_id_, string const &description_, int32 member_count_, array<int53> &&member_user_ids_, object_ptr<chatInviteLinkSubscriptionInfo> &&subscription_info_, bool creates_join_request_, bool is_public_, object_ptr<verificationStatus> &&verification_status_)
  : chat_id_(chat_id_)
  , accessible_for_(accessible_for_)
  , type_(std::move(type_))
  , title_(title_)
  , photo_(std::move(photo_))
  , accent_color_id_(accent_color_id_)
  , description_(description_)
  , member_count_(member_count_)
  , member_user_ids_(std::move(member_user_ids_))
  , subscription_info_(std::move(subscription_info_))
  , creates_join_request_(creates_join_request_)
  , is_public_(is_public_)
  , verification_status_(std::move(verification_status_))
{}

const std::int32_t chatInviteLinkInfo::ID;

void chatInviteLinkInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatInviteLinkInfo");
    s.store_field("chat_id", chat_id_);
    s.store_field("accessible_for", accessible_for_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_field("title", title_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_field("accent_color_id", accent_color_id_);
    s.store_field("description", description_);
    s.store_field("member_count", member_count_);
    { s.store_vector_begin("member_user_ids", member_user_ids_.size()); for (const auto &_value : member_user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_object_field("subscription_info", static_cast<const BaseObject *>(subscription_info_.get()));
    s.store_field("creates_join_request", creates_join_request_);
    s.store_field("is_public", is_public_);
    s.store_object_field("verification_status", static_cast<const BaseObject *>(verification_status_.get()));
    s.store_class_end();
  }
}

chatInviteLinkMember::chatInviteLinkMember()
  : user_id_()
  , joined_chat_date_()
  , via_chat_folder_invite_link_()
  , approver_user_id_()
{}

chatInviteLinkMember::chatInviteLinkMember(int53 user_id_, int32 joined_chat_date_, bool via_chat_folder_invite_link_, int53 approver_user_id_)
  : user_id_(user_id_)
  , joined_chat_date_(joined_chat_date_)
  , via_chat_folder_invite_link_(via_chat_folder_invite_link_)
  , approver_user_id_(approver_user_id_)
{}

const std::int32_t chatInviteLinkMember::ID;

void chatInviteLinkMember::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatInviteLinkMember");
    s.store_field("user_id", user_id_);
    s.store_field("joined_chat_date", joined_chat_date_);
    s.store_field("via_chat_folder_invite_link", via_chat_folder_invite_link_);
    s.store_field("approver_user_id", approver_user_id_);
    s.store_class_end();
  }
}

chatMessageSenders::chatMessageSenders()
  : senders_()
{}

chatMessageSenders::chatMessageSenders(array<object_ptr<chatMessageSender>> &&senders_)
  : senders_(std::move(senders_))
{}

const std::int32_t chatMessageSenders::ID;

void chatMessageSenders::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatMessageSenders");
    { s.store_vector_begin("senders", senders_.size()); for (const auto &_value : senders_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

chatPhotoStickerTypeRegularOrMask::chatPhotoStickerTypeRegularOrMask()
  : sticker_set_id_()
  , sticker_id_()
{}

chatPhotoStickerTypeRegularOrMask::chatPhotoStickerTypeRegularOrMask(int64 sticker_set_id_, int64 sticker_id_)
  : sticker_set_id_(sticker_set_id_)
  , sticker_id_(sticker_id_)
{}

const std::int32_t chatPhotoStickerTypeRegularOrMask::ID;

void chatPhotoStickerTypeRegularOrMask::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatPhotoStickerTypeRegularOrMask");
    s.store_field("sticker_set_id", sticker_set_id_);
    s.store_field("sticker_id", sticker_id_);
    s.store_class_end();
  }
}

chatPhotoStickerTypeCustomEmoji::chatPhotoStickerTypeCustomEmoji()
  : custom_emoji_id_()
{}

chatPhotoStickerTypeCustomEmoji::chatPhotoStickerTypeCustomEmoji(int64 custom_emoji_id_)
  : custom_emoji_id_(custom_emoji_id_)
{}

const std::int32_t chatPhotoStickerTypeCustomEmoji::ID;

void chatPhotoStickerTypeCustomEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatPhotoStickerTypeCustomEmoji");
    s.store_field("custom_emoji_id", custom_emoji_id_);
    s.store_class_end();
  }
}

chatPosition::chatPosition()
  : list_()
  , order_()
  , is_pinned_()
  , source_()
{}

chatPosition::chatPosition(object_ptr<ChatList> &&list_, int64 order_, bool is_pinned_, object_ptr<ChatSource> &&source_)
  : list_(std::move(list_))
  , order_(order_)
  , is_pinned_(is_pinned_)
  , source_(std::move(source_))
{}

const std::int32_t chatPosition::ID;

void chatPosition::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatPosition");
    s.store_object_field("list", static_cast<const BaseObject *>(list_.get()));
    s.store_field("order", order_);
    s.store_field("is_pinned", is_pinned_);
    s.store_object_field("source", static_cast<const BaseObject *>(source_.get()));
    s.store_class_end();
  }
}

chatThemeEmoji::chatThemeEmoji()
  : name_()
{}

chatThemeEmoji::chatThemeEmoji(string const &name_)
  : name_(name_)
{}

const std::int32_t chatThemeEmoji::ID;

void chatThemeEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatThemeEmoji");
    s.store_field("name", name_);
    s.store_class_end();
  }
}

chatThemeGift::chatThemeGift()
  : gift_theme_()
{}

chatThemeGift::chatThemeGift(object_ptr<giftChatTheme> &&gift_theme_)
  : gift_theme_(std::move(gift_theme_))
{}

const std::int32_t chatThemeGift::ID;

void chatThemeGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "chatThemeGift");
    s.store_object_field("gift_theme", static_cast<const BaseObject *>(gift_theme_.get()));
    s.store_class_end();
  }
}

checkStickerSetNameResultOk::checkStickerSetNameResultOk() {
}

const std::int32_t checkStickerSetNameResultOk::ID;

void checkStickerSetNameResultOk::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkStickerSetNameResultOk");
    s.store_class_end();
  }
}

checkStickerSetNameResultNameInvalid::checkStickerSetNameResultNameInvalid() {
}

const std::int32_t checkStickerSetNameResultNameInvalid::ID;

void checkStickerSetNameResultNameInvalid::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkStickerSetNameResultNameInvalid");
    s.store_class_end();
  }
}

checkStickerSetNameResultNameOccupied::checkStickerSetNameResultNameOccupied() {
}

const std::int32_t checkStickerSetNameResultNameOccupied::ID;

void checkStickerSetNameResultNameOccupied::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "checkStickerSetNameResultNameOccupied");
    s.store_class_end();
  }
}

count::count()
  : count_()
{}

count::count(int32 count_)
  : count_(count_)
{}

const std::int32_t count::ID;

void count::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "count");
    s.store_field("count", count_);
    s.store_class_end();
  }
}

emojiReaction::emojiReaction()
  : emoji_()
  , title_()
  , is_active_()
  , static_icon_()
  , appear_animation_()
  , select_animation_()
  , activate_animation_()
  , effect_animation_()
  , around_animation_()
  , center_animation_()
{}

emojiReaction::emojiReaction(string const &emoji_, string const &title_, bool is_active_, object_ptr<sticker> &&static_icon_, object_ptr<sticker> &&appear_animation_, object_ptr<sticker> &&select_animation_, object_ptr<sticker> &&activate_animation_, object_ptr<sticker> &&effect_animation_, object_ptr<sticker> &&around_animation_, object_ptr<sticker> &&center_animation_)
  : emoji_(emoji_)
  , title_(title_)
  , is_active_(is_active_)
  , static_icon_(std::move(static_icon_))
  , appear_animation_(std::move(appear_animation_))
  , select_animation_(std::move(select_animation_))
  , activate_animation_(std::move(activate_animation_))
  , effect_animation_(std::move(effect_animation_))
  , around_animation_(std::move(around_animation_))
  , center_animation_(std::move(center_animation_))
{}

const std::int32_t emojiReaction::ID;

void emojiReaction::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "emojiReaction");
    s.store_field("emoji", emoji_);
    s.store_field("title", title_);
    s.store_field("is_active", is_active_);
    s.store_object_field("static_icon", static_cast<const BaseObject *>(static_icon_.get()));
    s.store_object_field("appear_animation", static_cast<const BaseObject *>(appear_animation_.get()));
    s.store_object_field("select_animation", static_cast<const BaseObject *>(select_animation_.get()));
    s.store_object_field("activate_animation", static_cast<const BaseObject *>(activate_animation_.get()));
    s.store_object_field("effect_animation", static_cast<const BaseObject *>(effect_animation_.get()));
    s.store_object_field("around_animation", static_cast<const BaseObject *>(around_animation_.get()));
    s.store_object_field("center_animation", static_cast<const BaseObject *>(center_animation_.get()));
    s.store_class_end();
  }
}

formattedText::formattedText()
  : text_()
  , entities_()
{}

formattedText::formattedText(string const &text_, array<object_ptr<textEntity>> &&entities_)
  : text_(text_)
  , entities_(std::move(entities_))
{}

const std::int32_t formattedText::ID;

void formattedText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "formattedText");
    s.store_field("text", text_);
    { s.store_vector_begin("entities", entities_.size()); for (const auto &_value : entities_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

foundStories::foundStories()
  : total_count_()
  , stories_()
  , next_offset_()
{}

foundStories::foundStories(int32 total_count_, array<object_ptr<story>> &&stories_, string const &next_offset_)
  : total_count_(total_count_)
  , stories_(std::move(stories_))
  , next_offset_(next_offset_)
{}

const std::int32_t foundStories::ID;

void foundStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "foundStories");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("stories", stories_.size()); for (const auto &_value : stories_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("next_offset", next_offset_);
    s.store_class_end();
  }
}

gameHighScores::gameHighScores()
  : scores_()
{}

gameHighScores::gameHighScores(array<object_ptr<gameHighScore>> &&scores_)
  : scores_(std::move(scores_))
{}

const std::int32_t gameHighScores::ID;

void gameHighScores::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "gameHighScores");
    { s.store_vector_begin("scores", scores_.size()); for (const auto &_value : scores_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

giveawayPrizePremium::giveawayPrizePremium()
  : month_count_()
{}

giveawayPrizePremium::giveawayPrizePremium(int32 month_count_)
  : month_count_(month_count_)
{}

const std::int32_t giveawayPrizePremium::ID;

void giveawayPrizePremium::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giveawayPrizePremium");
    s.store_field("month_count", month_count_);
    s.store_class_end();
  }
}

giveawayPrizeStars::giveawayPrizeStars()
  : star_count_()
{}

giveawayPrizeStars::giveawayPrizeStars(int53 star_count_)
  : star_count_(star_count_)
{}

const std::int32_t giveawayPrizeStars::ID;

void giveawayPrizeStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "giveawayPrizeStars");
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

groupCallRecentSpeaker::groupCallRecentSpeaker()
  : participant_id_()
  , is_speaking_()
{}

groupCallRecentSpeaker::groupCallRecentSpeaker(object_ptr<MessageSender> &&participant_id_, bool is_speaking_)
  : participant_id_(std::move(participant_id_))
  , is_speaking_(is_speaking_)
{}

const std::int32_t groupCallRecentSpeaker::ID;

void groupCallRecentSpeaker::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "groupCallRecentSpeaker");
    s.store_object_field("participant_id", static_cast<const BaseObject *>(participant_id_.get()));
    s.store_field("is_speaking", is_speaking_);
    s.store_class_end();
  }
}

httpUrl::httpUrl()
  : url_()
{}

httpUrl::httpUrl(string const &url_)
  : url_(url_)
{}

const std::int32_t httpUrl::ID;

void httpUrl::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "httpUrl");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

importedContacts::importedContacts()
  : user_ids_()
  , importer_count_()
{}

importedContacts::importedContacts(array<int53> &&user_ids_, array<int32> &&importer_count_)
  : user_ids_(std::move(user_ids_))
  , importer_count_(std::move(importer_count_))
{}

const std::int32_t importedContacts::ID;

void importedContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "importedContacts");
    { s.store_vector_begin("user_ids", user_ids_.size()); for (const auto &_value : user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    { s.store_vector_begin("importer_count", importer_count_.size()); for (const auto &_value : importer_count_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

inputChatThemeEmoji::inputChatThemeEmoji()
  : name_()
{}

inputChatThemeEmoji::inputChatThemeEmoji(string const &name_)
  : name_(name_)
{}

const std::int32_t inputChatThemeEmoji::ID;

void inputChatThemeEmoji::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputChatThemeEmoji");
    s.store_field("name", name_);
    s.store_class_end();
  }
}

inputChatThemeGift::inputChatThemeGift()
  : name_()
{}

inputChatThemeGift::inputChatThemeGift(string const &name_)
  : name_(name_)
{}

const std::int32_t inputChatThemeGift::ID;

void inputChatThemeGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputChatThemeGift");
    s.store_field("name", name_);
    s.store_class_end();
  }
}

inputSuggestedPostInfo::inputSuggestedPostInfo()
  : price_()
  , send_date_()
{}

inputSuggestedPostInfo::inputSuggestedPostInfo(object_ptr<SuggestedPostPrice> &&price_, int32 send_date_)
  : price_(std::move(price_))
  , send_date_(send_date_)
{}

const std::int32_t inputSuggestedPostInfo::ID;

void inputSuggestedPostInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "inputSuggestedPostInfo");
    s.store_object_field("price", static_cast<const BaseObject *>(price_.get()));
    s.store_field("send_date", send_date_);
    s.store_class_end();
  }
}

invoice::invoice()
  : currency_()
  , price_parts_()
  , subscription_period_()
  , max_tip_amount_()
  , suggested_tip_amounts_()
  , recurring_payment_terms_of_service_url_()
  , terms_of_service_url_()
  , is_test_()
  , need_name_()
  , need_phone_number_()
  , need_email_address_()
  , need_shipping_address_()
  , send_phone_number_to_provider_()
  , send_email_address_to_provider_()
  , is_flexible_()
{}

invoice::invoice(string const &currency_, array<object_ptr<labeledPricePart>> &&price_parts_, int32 subscription_period_, int53 max_tip_amount_, array<int53> &&suggested_tip_amounts_, string const &recurring_payment_terms_of_service_url_, string const &terms_of_service_url_, bool is_test_, bool need_name_, bool need_phone_number_, bool need_email_address_, bool need_shipping_address_, bool send_phone_number_to_provider_, bool send_email_address_to_provider_, bool is_flexible_)
  : currency_(currency_)
  , price_parts_(std::move(price_parts_))
  , subscription_period_(subscription_period_)
  , max_tip_amount_(max_tip_amount_)
  , suggested_tip_amounts_(std::move(suggested_tip_amounts_))
  , recurring_payment_terms_of_service_url_(recurring_payment_terms_of_service_url_)
  , terms_of_service_url_(terms_of_service_url_)
  , is_test_(is_test_)
  , need_name_(need_name_)
  , need_phone_number_(need_phone_number_)
  , need_email_address_(need_email_address_)
  , need_shipping_address_(need_shipping_address_)
  , send_phone_number_to_provider_(send_phone_number_to_provider_)
  , send_email_address_to_provider_(send_email_address_to_provider_)
  , is_flexible_(is_flexible_)
{}

const std::int32_t invoice::ID;

void invoice::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "invoice");
    s.store_field("currency", currency_);
    { s.store_vector_begin("price_parts", price_parts_.size()); for (const auto &_value : price_parts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("subscription_period", subscription_period_);
    s.store_field("max_tip_amount", max_tip_amount_);
    { s.store_vector_begin("suggested_tip_amounts", suggested_tip_amounts_.size()); for (const auto &_value : suggested_tip_amounts_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("recurring_payment_terms_of_service_url", recurring_payment_terms_of_service_url_);
    s.store_field("terms_of_service_url", terms_of_service_url_);
    s.store_field("is_test", is_test_);
    s.store_field("need_name", need_name_);
    s.store_field("need_phone_number", need_phone_number_);
    s.store_field("need_email_address", need_email_address_);
    s.store_field("need_shipping_address", need_shipping_address_);
    s.store_field("send_phone_number_to_provider", send_phone_number_to_provider_);
    s.store_field("send_email_address_to_provider", send_email_address_to_provider_);
    s.store_field("is_flexible", is_flexible_);
    s.store_class_end();
  }
}

languagePackInfo::languagePackInfo()
  : id_()
  , base_language_pack_id_()
  , name_()
  , native_name_()
  , plural_code_()
  , is_official_()
  , is_rtl_()
  , is_beta_()
  , is_installed_()
  , total_string_count_()
  , translated_string_count_()
  , local_string_count_()
  , translation_url_()
{}

languagePackInfo::languagePackInfo(string const &id_, string const &base_language_pack_id_, string const &name_, string const &native_name_, string const &plural_code_, bool is_official_, bool is_rtl_, bool is_beta_, bool is_installed_, int32 total_string_count_, int32 translated_string_count_, int32 local_string_count_, string const &translation_url_)
  : id_(id_)
  , base_language_pack_id_(base_language_pack_id_)
  , name_(name_)
  , native_name_(native_name_)
  , plural_code_(plural_code_)
  , is_official_(is_official_)
  , is_rtl_(is_rtl_)
  , is_beta_(is_beta_)
  , is_installed_(is_installed_)
  , total_string_count_(total_string_count_)
  , translated_string_count_(translated_string_count_)
  , local_string_count_(local_string_count_)
  , translation_url_(translation_url_)
{}

const std::int32_t languagePackInfo::ID;

void languagePackInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "languagePackInfo");
    s.store_field("id", id_);
    s.store_field("base_language_pack_id", base_language_pack_id_);
    s.store_field("name", name_);
    s.store_field("native_name", native_name_);
    s.store_field("plural_code", plural_code_);
    s.store_field("is_official", is_official_);
    s.store_field("is_rtl", is_rtl_);
    s.store_field("is_beta", is_beta_);
    s.store_field("is_installed", is_installed_);
    s.store_field("total_string_count", total_string_count_);
    s.store_field("translated_string_count", translated_string_count_);
    s.store_field("local_string_count", local_string_count_);
    s.store_field("translation_url", translation_url_);
    s.store_class_end();
  }
}

localFile::localFile()
  : path_()
  , can_be_downloaded_()
  , can_be_deleted_()
  , is_downloading_active_()
  , is_downloading_completed_()
  , download_offset_()
  , downloaded_prefix_size_()
  , downloaded_size_()
{}

localFile::localFile(string const &path_, bool can_be_downloaded_, bool can_be_deleted_, bool is_downloading_active_, bool is_downloading_completed_, int53 download_offset_, int53 downloaded_prefix_size_, int53 downloaded_size_)
  : path_(path_)
  , can_be_downloaded_(can_be_downloaded_)
  , can_be_deleted_(can_be_deleted_)
  , is_downloading_active_(is_downloading_active_)
  , is_downloading_completed_(is_downloading_completed_)
  , download_offset_(download_offset_)
  , downloaded_prefix_size_(downloaded_prefix_size_)
  , downloaded_size_(downloaded_size_)
{}

const std::int32_t localFile::ID;

void localFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "localFile");
    s.store_field("path", path_);
    s.store_field("can_be_downloaded", can_be_downloaded_);
    s.store_field("can_be_deleted", can_be_deleted_);
    s.store_field("is_downloading_active", is_downloading_active_);
    s.store_field("is_downloading_completed", is_downloading_completed_);
    s.store_field("download_offset", download_offset_);
    s.store_field("downloaded_prefix_size", downloaded_prefix_size_);
    s.store_field("downloaded_size", downloaded_size_);
    s.store_class_end();
  }
}

messageFileTypePrivate::messageFileTypePrivate()
  : name_()
{}

messageFileTypePrivate::messageFileTypePrivate(string const &name_)
  : name_(name_)
{}

const std::int32_t messageFileTypePrivate::ID;

void messageFileTypePrivate::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageFileTypePrivate");
    s.store_field("name", name_);
    s.store_class_end();
  }
}

messageFileTypeGroup::messageFileTypeGroup()
  : title_()
{}

messageFileTypeGroup::messageFileTypeGroup(string const &title_)
  : title_(title_)
{}

const std::int32_t messageFileTypeGroup::ID;

void messageFileTypeGroup::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageFileTypeGroup");
    s.store_field("title", title_);
    s.store_class_end();
  }
}

messageFileTypeUnknown::messageFileTypeUnknown() {
}

const std::int32_t messageFileTypeUnknown::ID;

void messageFileTypeUnknown::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageFileTypeUnknown");
    s.store_class_end();
  }
}

messageLink::messageLink()
  : link_()
  , is_public_()
{}

messageLink::messageLink(string const &link_, bool is_public_)
  : link_(link_)
  , is_public_(is_public_)
{}

const std::int32_t messageLink::ID;

void messageLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageLink");
    s.store_field("link", link_);
    s.store_field("is_public", is_public_);
    s.store_class_end();
  }
}

messagePositions::messagePositions()
  : total_count_()
  , positions_()
{}

messagePositions::messagePositions(int32 total_count_, array<object_ptr<messagePosition>> &&positions_)
  : total_count_(total_count_)
  , positions_(std::move(positions_))
{}

const std::int32_t messagePositions::ID;

void messagePositions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messagePositions");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("positions", positions_.size()); for (const auto &_value : positions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

messageReactions::messageReactions()
  : reactions_()
  , are_tags_()
  , paid_reactors_()
  , can_get_added_reactions_()
{}

messageReactions::messageReactions(array<object_ptr<messageReaction>> &&reactions_, bool are_tags_, array<object_ptr<paidReactor>> &&paid_reactors_, bool can_get_added_reactions_)
  : reactions_(std::move(reactions_))
  , are_tags_(are_tags_)
  , paid_reactors_(std::move(paid_reactors_))
  , can_get_added_reactions_(can_get_added_reactions_)
{}

const std::int32_t messageReactions::ID;

void messageReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageReactions");
    { s.store_vector_begin("reactions", reactions_.size()); for (const auto &_value : reactions_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("are_tags", are_tags_);
    { s.store_vector_begin("paid_reactors", paid_reactors_.size()); for (const auto &_value : paid_reactors_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("can_get_added_reactions", can_get_added_reactions_);
    s.store_class_end();
  }
}

messageViewer::messageViewer()
  : user_id_()
  , view_date_()
{}

messageViewer::messageViewer(int53 user_id_, int32 view_date_)
  : user_id_(user_id_)
  , view_date_(view_date_)
{}

const std::int32_t messageViewer::ID;

void messageViewer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "messageViewer");
    s.store_field("user_id", user_id_);
    s.store_field("view_date", view_date_);
    s.store_class_end();
  }
}

passportElementErrorSourceUnspecified::passportElementErrorSourceUnspecified() {
}

const std::int32_t passportElementErrorSourceUnspecified::ID;

void passportElementErrorSourceUnspecified::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementErrorSourceUnspecified");
    s.store_class_end();
  }
}

passportElementErrorSourceDataField::passportElementErrorSourceDataField()
  : field_name_()
{}

passportElementErrorSourceDataField::passportElementErrorSourceDataField(string const &field_name_)
  : field_name_(field_name_)
{}

const std::int32_t passportElementErrorSourceDataField::ID;

void passportElementErrorSourceDataField::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementErrorSourceDataField");
    s.store_field("field_name", field_name_);
    s.store_class_end();
  }
}

passportElementErrorSourceFrontSide::passportElementErrorSourceFrontSide() {
}

const std::int32_t passportElementErrorSourceFrontSide::ID;

void passportElementErrorSourceFrontSide::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementErrorSourceFrontSide");
    s.store_class_end();
  }
}

passportElementErrorSourceReverseSide::passportElementErrorSourceReverseSide() {
}

const std::int32_t passportElementErrorSourceReverseSide::ID;

void passportElementErrorSourceReverseSide::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementErrorSourceReverseSide");
    s.store_class_end();
  }
}

passportElementErrorSourceSelfie::passportElementErrorSourceSelfie() {
}

const std::int32_t passportElementErrorSourceSelfie::ID;

void passportElementErrorSourceSelfie::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementErrorSourceSelfie");
    s.store_class_end();
  }
}

passportElementErrorSourceTranslationFile::passportElementErrorSourceTranslationFile()
  : file_index_()
{}

passportElementErrorSourceTranslationFile::passportElementErrorSourceTranslationFile(int32 file_index_)
  : file_index_(file_index_)
{}

const std::int32_t passportElementErrorSourceTranslationFile::ID;

void passportElementErrorSourceTranslationFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementErrorSourceTranslationFile");
    s.store_field("file_index", file_index_);
    s.store_class_end();
  }
}

passportElementErrorSourceTranslationFiles::passportElementErrorSourceTranslationFiles() {
}

const std::int32_t passportElementErrorSourceTranslationFiles::ID;

void passportElementErrorSourceTranslationFiles::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementErrorSourceTranslationFiles");
    s.store_class_end();
  }
}

passportElementErrorSourceFile::passportElementErrorSourceFile()
  : file_index_()
{}

passportElementErrorSourceFile::passportElementErrorSourceFile(int32 file_index_)
  : file_index_(file_index_)
{}

const std::int32_t passportElementErrorSourceFile::ID;

void passportElementErrorSourceFile::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementErrorSourceFile");
    s.store_field("file_index", file_index_);
    s.store_class_end();
  }
}

passportElementErrorSourceFiles::passportElementErrorSourceFiles() {
}

const std::int32_t passportElementErrorSourceFiles::ID;

void passportElementErrorSourceFiles::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "passportElementErrorSourceFiles");
    s.store_class_end();
  }
}

paymentProviderSmartGlocal::paymentProviderSmartGlocal()
  : public_token_()
  , tokenize_url_()
{}

paymentProviderSmartGlocal::paymentProviderSmartGlocal(string const &public_token_, string const &tokenize_url_)
  : public_token_(public_token_)
  , tokenize_url_(tokenize_url_)
{}

const std::int32_t paymentProviderSmartGlocal::ID;

void paymentProviderSmartGlocal::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paymentProviderSmartGlocal");
    s.store_field("public_token", public_token_);
    s.store_field("tokenize_url", tokenize_url_);
    s.store_class_end();
  }
}

paymentProviderStripe::paymentProviderStripe()
  : publishable_key_()
  , need_country_()
  , need_postal_code_()
  , need_cardholder_name_()
{}

paymentProviderStripe::paymentProviderStripe(string const &publishable_key_, bool need_country_, bool need_postal_code_, bool need_cardholder_name_)
  : publishable_key_(publishable_key_)
  , need_country_(need_country_)
  , need_postal_code_(need_postal_code_)
  , need_cardholder_name_(need_cardholder_name_)
{}

const std::int32_t paymentProviderStripe::ID;

void paymentProviderStripe::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paymentProviderStripe");
    s.store_field("publishable_key", publishable_key_);
    s.store_field("need_country", need_country_);
    s.store_field("need_postal_code", need_postal_code_);
    s.store_field("need_cardholder_name", need_cardholder_name_);
    s.store_class_end();
  }
}

paymentProviderOther::paymentProviderOther()
  : url_()
{}

paymentProviderOther::paymentProviderOther(string const &url_)
  : url_(url_)
{}

const std::int32_t paymentProviderOther::ID;

void paymentProviderOther::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "paymentProviderOther");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

pollTypeRegular::pollTypeRegular()
  : allow_multiple_answers_()
{}

pollTypeRegular::pollTypeRegular(bool allow_multiple_answers_)
  : allow_multiple_answers_(allow_multiple_answers_)
{}

const std::int32_t pollTypeRegular::ID;

void pollTypeRegular::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pollTypeRegular");
    s.store_field("allow_multiple_answers", allow_multiple_answers_);
    s.store_class_end();
  }
}

pollTypeQuiz::pollTypeQuiz()
  : correct_option_id_()
  , explanation_()
{}

pollTypeQuiz::pollTypeQuiz(int32 correct_option_id_, object_ptr<formattedText> &&explanation_)
  : correct_option_id_(correct_option_id_)
  , explanation_(std::move(explanation_))
{}

const std::int32_t pollTypeQuiz::ID;

void pollTypeQuiz::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "pollTypeQuiz");
    s.store_field("correct_option_id", correct_option_id_);
    s.store_object_field("explanation", static_cast<const BaseObject *>(explanation_.get()));
    s.store_class_end();
  }
}

savedMessagesTag::savedMessagesTag()
  : tag_()
  , label_()
  , count_()
{}

savedMessagesTag::savedMessagesTag(object_ptr<ReactionType> &&tag_, string const &label_, int32 count_)
  : tag_(std::move(tag_))
  , label_(label_)
  , count_(count_)
{}

const std::int32_t savedMessagesTag::ID;

void savedMessagesTag::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "savedMessagesTag");
    s.store_object_field("tag", static_cast<const BaseObject *>(tag_.get()));
    s.store_field("label", label_);
    s.store_field("count", count_);
    s.store_class_end();
  }
}

seconds::seconds()
  : seconds_()
{}

seconds::seconds(double seconds_)
  : seconds_(seconds_)
{}

const std::int32_t seconds::ID;

void seconds::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "seconds");
    s.store_field("seconds", seconds_);
    s.store_class_end();
  }
}

sharedChat::sharedChat()
  : chat_id_()
  , title_()
  , username_()
  , photo_()
{}

sharedChat::sharedChat(int53 chat_id_, string const &title_, string const &username_, object_ptr<photo> &&photo_)
  : chat_id_(chat_id_)
  , title_(title_)
  , username_(username_)
  , photo_(std::move(photo_))
{}

const std::int32_t sharedChat::ID;

void sharedChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sharedChat");
    s.store_field("chat_id", chat_id_);
    s.store_field("title", title_);
    s.store_field("username", username_);
    s.store_object_field("photo", static_cast<const BaseObject *>(photo_.get()));
    s.store_class_end();
  }
}

sponsoredMessages::sponsoredMessages()
  : messages_()
  , messages_between_()
{}

sponsoredMessages::sponsoredMessages(array<object_ptr<sponsoredMessage>> &&messages_, int32 messages_between_)
  : messages_(std::move(messages_))
  , messages_between_(messages_between_)
{}

const std::int32_t sponsoredMessages::ID;

void sponsoredMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sponsoredMessages");
    { s.store_vector_begin("messages", messages_.size()); for (const auto &_value : messages_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("messages_between", messages_between_);
    s.store_class_end();
  }
}

stickerSet::stickerSet()
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
  , stickers_()
  , emojis_()
{}

stickerSet::stickerSet(int64 id_, string const &title_, string const &name_, object_ptr<thumbnail> &&thumbnail_, object_ptr<outline> &&thumbnail_outline_, bool is_owned_, bool is_installed_, bool is_archived_, bool is_official_, object_ptr<StickerType> &&sticker_type_, bool needs_repainting_, bool is_allowed_as_chat_emoji_status_, bool is_viewed_, array<object_ptr<sticker>> &&stickers_, array<object_ptr<emojis>> &&emojis_)
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
  , stickers_(std::move(stickers_))
  , emojis_(std::move(emojis_))
{}

const std::int32_t stickerSet::ID;

void stickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "stickerSet");
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
    { s.store_vector_begin("stickers", stickers_.size()); for (const auto &_value : stickers_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    { s.store_vector_begin("emojis", emojis_.size()); for (const auto &_value : emojis_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

storyInteractionTypeView::storyInteractionTypeView()
  : chosen_reaction_type_()
{}

storyInteractionTypeView::storyInteractionTypeView(object_ptr<ReactionType> &&chosen_reaction_type_)
  : chosen_reaction_type_(std::move(chosen_reaction_type_))
{}

const std::int32_t storyInteractionTypeView::ID;

void storyInteractionTypeView::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyInteractionTypeView");
    s.store_object_field("chosen_reaction_type", static_cast<const BaseObject *>(chosen_reaction_type_.get()));
    s.store_class_end();
  }
}

storyInteractionTypeForward::storyInteractionTypeForward()
  : message_()
{}

storyInteractionTypeForward::storyInteractionTypeForward(object_ptr<message> &&message_)
  : message_(std::move(message_))
{}

const std::int32_t storyInteractionTypeForward::ID;

void storyInteractionTypeForward::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyInteractionTypeForward");
    s.store_object_field("message", static_cast<const BaseObject *>(message_.get()));
    s.store_class_end();
  }
}

storyInteractionTypeRepost::storyInteractionTypeRepost()
  : story_()
{}

storyInteractionTypeRepost::storyInteractionTypeRepost(object_ptr<story> &&story_)
  : story_(std::move(story_))
{}

const std::int32_t storyInteractionTypeRepost::ID;

void storyInteractionTypeRepost::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "storyInteractionTypeRepost");
    s.store_object_field("story", static_cast<const BaseObject *>(story_.get()));
    s.store_class_end();
  }
}

suggestedPostInfo::suggestedPostInfo()
  : price_()
  , send_date_()
  , state_()
  , can_be_approved_()
  , can_be_declined_()
{}

suggestedPostInfo::suggestedPostInfo(object_ptr<SuggestedPostPrice> &&price_, int32 send_date_, object_ptr<SuggestedPostState> &&state_, bool can_be_approved_, bool can_be_declined_)
  : price_(std::move(price_))
  , send_date_(send_date_)
  , state_(std::move(state_))
  , can_be_approved_(can_be_approved_)
  , can_be_declined_(can_be_declined_)
{}

const std::int32_t suggestedPostInfo::ID;

void suggestedPostInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedPostInfo");
    s.store_object_field("price", static_cast<const BaseObject *>(price_.get()));
    s.store_field("send_date", send_date_);
    s.store_object_field("state", static_cast<const BaseObject *>(state_.get()));
    s.store_field("can_be_approved", can_be_approved_);
    s.store_field("can_be_declined", can_be_declined_);
    s.store_class_end();
  }
}

suggestedPostRefundReasonPostDeleted::suggestedPostRefundReasonPostDeleted() {
}

const std::int32_t suggestedPostRefundReasonPostDeleted::ID;

void suggestedPostRefundReasonPostDeleted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedPostRefundReasonPostDeleted");
    s.store_class_end();
  }
}

suggestedPostRefundReasonPaymentRefunded::suggestedPostRefundReasonPaymentRefunded() {
}

const std::int32_t suggestedPostRefundReasonPaymentRefunded::ID;

void suggestedPostRefundReasonPaymentRefunded::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "suggestedPostRefundReasonPaymentRefunded");
    s.store_class_end();
  }
}

targetChatCurrent::targetChatCurrent() {
}

const std::int32_t targetChatCurrent::ID;

void targetChatCurrent::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "targetChatCurrent");
    s.store_class_end();
  }
}

targetChatChosen::targetChatChosen()
  : types_()
{}

targetChatChosen::targetChatChosen(object_ptr<targetChatTypes> &&types_)
  : types_(std::move(types_))
{}

const std::int32_t targetChatChosen::ID;

void targetChatChosen::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "targetChatChosen");
    s.store_object_field("types", static_cast<const BaseObject *>(types_.get()));
    s.store_class_end();
  }
}

targetChatInternalLink::targetChatInternalLink()
  : link_()
{}

targetChatInternalLink::targetChatInternalLink(object_ptr<InternalLinkType> &&link_)
  : link_(std::move(link_))
{}

const std::int32_t targetChatInternalLink::ID;

void targetChatInternalLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "targetChatInternalLink");
    s.store_object_field("link", static_cast<const BaseObject *>(link_.get()));
    s.store_class_end();
  }
}

telegramPaymentPurposePremiumGift::telegramPaymentPurposePremiumGift()
  : currency_()
  , amount_()
  , user_id_()
  , month_count_()
  , text_()
{}

telegramPaymentPurposePremiumGift::telegramPaymentPurposePremiumGift(string const &currency_, int53 amount_, int53 user_id_, int32 month_count_, object_ptr<formattedText> &&text_)
  : currency_(currency_)
  , amount_(amount_)
  , user_id_(user_id_)
  , month_count_(month_count_)
  , text_(std::move(text_))
{}

const std::int32_t telegramPaymentPurposePremiumGift::ID;

void telegramPaymentPurposePremiumGift::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "telegramPaymentPurposePremiumGift");
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("user_id", user_id_);
    s.store_field("month_count", month_count_);
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

telegramPaymentPurposePremiumGiftCodes::telegramPaymentPurposePremiumGiftCodes()
  : boosted_chat_id_()
  , currency_()
  , amount_()
  , user_ids_()
  , month_count_()
  , text_()
{}

telegramPaymentPurposePremiumGiftCodes::telegramPaymentPurposePremiumGiftCodes(int53 boosted_chat_id_, string const &currency_, int53 amount_, array<int53> &&user_ids_, int32 month_count_, object_ptr<formattedText> &&text_)
  : boosted_chat_id_(boosted_chat_id_)
  , currency_(currency_)
  , amount_(amount_)
  , user_ids_(std::move(user_ids_))
  , month_count_(month_count_)
  , text_(std::move(text_))
{}

const std::int32_t telegramPaymentPurposePremiumGiftCodes::ID;

void telegramPaymentPurposePremiumGiftCodes::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "telegramPaymentPurposePremiumGiftCodes");
    s.store_field("boosted_chat_id", boosted_chat_id_);
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    { s.store_vector_begin("user_ids", user_ids_.size()); for (const auto &_value : user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_field("month_count", month_count_);
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_class_end();
  }
}

telegramPaymentPurposePremiumGiveaway::telegramPaymentPurposePremiumGiveaway()
  : parameters_()
  , currency_()
  , amount_()
  , winner_count_()
  , month_count_()
{}

telegramPaymentPurposePremiumGiveaway::telegramPaymentPurposePremiumGiveaway(object_ptr<giveawayParameters> &&parameters_, string const &currency_, int53 amount_, int32 winner_count_, int32 month_count_)
  : parameters_(std::move(parameters_))
  , currency_(currency_)
  , amount_(amount_)
  , winner_count_(winner_count_)
  , month_count_(month_count_)
{}

const std::int32_t telegramPaymentPurposePremiumGiveaway::ID;

void telegramPaymentPurposePremiumGiveaway::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "telegramPaymentPurposePremiumGiveaway");
    s.store_object_field("parameters", static_cast<const BaseObject *>(parameters_.get()));
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("winner_count", winner_count_);
    s.store_field("month_count", month_count_);
    s.store_class_end();
  }
}

telegramPaymentPurposeStars::telegramPaymentPurposeStars()
  : currency_()
  , amount_()
  , star_count_()
  , chat_id_()
{}

telegramPaymentPurposeStars::telegramPaymentPurposeStars(string const &currency_, int53 amount_, int53 star_count_, int53 chat_id_)
  : currency_(currency_)
  , amount_(amount_)
  , star_count_(star_count_)
  , chat_id_(chat_id_)
{}

const std::int32_t telegramPaymentPurposeStars::ID;

void telegramPaymentPurposeStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "telegramPaymentPurposeStars");
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("star_count", star_count_);
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

telegramPaymentPurposeGiftedStars::telegramPaymentPurposeGiftedStars()
  : user_id_()
  , currency_()
  , amount_()
  , star_count_()
{}

telegramPaymentPurposeGiftedStars::telegramPaymentPurposeGiftedStars(int53 user_id_, string const &currency_, int53 amount_, int53 star_count_)
  : user_id_(user_id_)
  , currency_(currency_)
  , amount_(amount_)
  , star_count_(star_count_)
{}

const std::int32_t telegramPaymentPurposeGiftedStars::ID;

void telegramPaymentPurposeGiftedStars::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "telegramPaymentPurposeGiftedStars");
    s.store_field("user_id", user_id_);
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

telegramPaymentPurposeStarGiveaway::telegramPaymentPurposeStarGiveaway()
  : parameters_()
  , currency_()
  , amount_()
  , winner_count_()
  , star_count_()
{}

telegramPaymentPurposeStarGiveaway::telegramPaymentPurposeStarGiveaway(object_ptr<giveawayParameters> &&parameters_, string const &currency_, int53 amount_, int32 winner_count_, int53 star_count_)
  : parameters_(std::move(parameters_))
  , currency_(currency_)
  , amount_(amount_)
  , winner_count_(winner_count_)
  , star_count_(star_count_)
{}

const std::int32_t telegramPaymentPurposeStarGiveaway::ID;

void telegramPaymentPurposeStarGiveaway::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "telegramPaymentPurposeStarGiveaway");
    s.store_object_field("parameters", static_cast<const BaseObject *>(parameters_.get()));
    s.store_field("currency", currency_);
    s.store_field("amount", amount_);
    s.store_field("winner_count", winner_count_);
    s.store_field("star_count", star_count_);
    s.store_class_end();
  }
}

telegramPaymentPurposeJoinChat::telegramPaymentPurposeJoinChat()
  : invite_link_()
{}

telegramPaymentPurposeJoinChat::telegramPaymentPurposeJoinChat(string const &invite_link_)
  : invite_link_(invite_link_)
{}

const std::int32_t telegramPaymentPurposeJoinChat::ID;

void telegramPaymentPurposeJoinChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "telegramPaymentPurposeJoinChat");
    s.store_field("invite_link", invite_link_);
    s.store_class_end();
  }
}

testString::testString()
  : value_()
{}

testString::testString(string const &value_)
  : value_(value_)
{}

const std::int32_t testString::ID;

void testString::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testString");
    s.store_field("value", value_);
    s.store_class_end();
  }
}

text::text()
  : text_()
{}

text::text(string const &text_)
  : text_(text_)
{}

const std::int32_t text::ID;

void text::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "text");
    s.store_field("text", text_);
    s.store_class_end();
  }
}

textQuote::textQuote()
  : text_()
  , position_()
  , is_manual_()
{}

textQuote::textQuote(object_ptr<formattedText> &&text_, int32 position_, bool is_manual_)
  : text_(std::move(text_))
  , position_(position_)
  , is_manual_(is_manual_)
{}

const std::int32_t textQuote::ID;

void textQuote::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "textQuote");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("position", position_);
    s.store_field("is_manual", is_manual_);
    s.store_class_end();
  }
}

transactionDirectionIncoming::transactionDirectionIncoming() {
}

const std::int32_t transactionDirectionIncoming::ID;

void transactionDirectionIncoming::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "transactionDirectionIncoming");
    s.store_class_end();
  }
}

transactionDirectionOutgoing::transactionDirectionOutgoing() {
}

const std::int32_t transactionDirectionOutgoing::ID;

void transactionDirectionOutgoing::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "transactionDirectionOutgoing");
    s.store_class_end();
  }
}

trendingStickerSets::trendingStickerSets()
  : total_count_()
  , sets_()
  , is_premium_()
{}

trendingStickerSets::trendingStickerSets(int32 total_count_, array<object_ptr<stickerSetInfo>> &&sets_, bool is_premium_)
  : total_count_(total_count_)
  , sets_(std::move(sets_))
  , is_premium_(is_premium_)
{}

const std::int32_t trendingStickerSets::ID;

void trendingStickerSets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "trendingStickerSets");
    s.store_field("total_count", total_count_);
    { s.store_vector_begin("sets", sets_.size()); for (const auto &_value : sets_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_field("is_premium", is_premium_);
    s.store_class_end();
  }
}

upgradedGiftSymbolCount::upgradedGiftSymbolCount()
  : symbol_()
  , total_count_()
{}

upgradedGiftSymbolCount::upgradedGiftSymbolCount(object_ptr<upgradedGiftSymbol> &&symbol_, int32 total_count_)
  : symbol_(std::move(symbol_))
  , total_count_(total_count_)
{}

const std::int32_t upgradedGiftSymbolCount::ID;

void upgradedGiftSymbolCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "upgradedGiftSymbolCount");
    s.store_object_field("symbol", static_cast<const BaseObject *>(symbol_.get()));
    s.store_field("total_count", total_count_);
    s.store_class_end();
  }
}

userTypeRegular::userTypeRegular() {
}

const std::int32_t userTypeRegular::ID;

void userTypeRegular::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userTypeRegular");
    s.store_class_end();
  }
}

userTypeDeleted::userTypeDeleted() {
}

const std::int32_t userTypeDeleted::ID;

void userTypeDeleted::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userTypeDeleted");
    s.store_class_end();
  }
}

userTypeBot::userTypeBot()
  : can_be_edited_()
  , can_join_groups_()
  , can_read_all_group_messages_()
  , has_main_web_app_()
  , has_topics_()
  , is_inline_()
  , inline_query_placeholder_()
  , need_location_()
  , can_connect_to_business_()
  , can_be_added_to_attachment_menu_()
  , active_user_count_()
{}

userTypeBot::userTypeBot(bool can_be_edited_, bool can_join_groups_, bool can_read_all_group_messages_, bool has_main_web_app_, bool has_topics_, bool is_inline_, string const &inline_query_placeholder_, bool need_location_, bool can_connect_to_business_, bool can_be_added_to_attachment_menu_, int32 active_user_count_)
  : can_be_edited_(can_be_edited_)
  , can_join_groups_(can_join_groups_)
  , can_read_all_group_messages_(can_read_all_group_messages_)
  , has_main_web_app_(has_main_web_app_)
  , has_topics_(has_topics_)
  , is_inline_(is_inline_)
  , inline_query_placeholder_(inline_query_placeholder_)
  , need_location_(need_location_)
  , can_connect_to_business_(can_connect_to_business_)
  , can_be_added_to_attachment_menu_(can_be_added_to_attachment_menu_)
  , active_user_count_(active_user_count_)
{}

const std::int32_t userTypeBot::ID;

void userTypeBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userTypeBot");
    s.store_field("can_be_edited", can_be_edited_);
    s.store_field("can_join_groups", can_join_groups_);
    s.store_field("can_read_all_group_messages", can_read_all_group_messages_);
    s.store_field("has_main_web_app", has_main_web_app_);
    s.store_field("has_topics", has_topics_);
    s.store_field("is_inline", is_inline_);
    s.store_field("inline_query_placeholder", inline_query_placeholder_);
    s.store_field("need_location", need_location_);
    s.store_field("can_connect_to_business", can_connect_to_business_);
    s.store_field("can_be_added_to_attachment_menu", can_be_added_to_attachment_menu_);
    s.store_field("active_user_count", active_user_count_);
    s.store_class_end();
  }
}

userTypeUnknown::userTypeUnknown() {
}

const std::int32_t userTypeUnknown::ID;

void userTypeUnknown::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "userTypeUnknown");
    s.store_class_end();
  }
}

videoChatStream::videoChatStream()
  : channel_id_()
  , scale_()
  , time_offset_()
{}

videoChatStream::videoChatStream(int32 channel_id_, int32 scale_, int53 time_offset_)
  : channel_id_(channel_id_)
  , scale_(scale_)
  , time_offset_(time_offset_)
{}

const std::int32_t videoChatStream::ID;

void videoChatStream::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "videoChatStream");
    s.store_field("channel_id", channel_id_);
    s.store_field("scale", scale_);
    s.store_field("time_offset", time_offset_);
    s.store_class_end();
  }
}

videoMessageAdvertisement::videoMessageAdvertisement()
  : unique_id_()
  , text_()
  , min_display_duration_()
  , max_display_duration_()
  , can_be_reported_()
  , sponsor_()
  , title_()
  , additional_info_()
{}

videoMessageAdvertisement::videoMessageAdvertisement(int53 unique_id_, string const &text_, int32 min_display_duration_, int32 max_display_duration_, bool can_be_reported_, object_ptr<advertisementSponsor> &&sponsor_, string const &title_, string const &additional_info_)
  : unique_id_(unique_id_)
  , text_(text_)
  , min_display_duration_(min_display_duration_)
  , max_display_duration_(max_display_duration_)
  , can_be_reported_(can_be_reported_)
  , sponsor_(std::move(sponsor_))
  , title_(title_)
  , additional_info_(additional_info_)
{}

const std::int32_t videoMessageAdvertisement::ID;

void videoMessageAdvertisement::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "videoMessageAdvertisement");
    s.store_field("unique_id", unique_id_);
    s.store_field("text", text_);
    s.store_field("min_display_duration", min_display_duration_);
    s.store_field("max_display_duration", max_display_duration_);
    s.store_field("can_be_reported", can_be_reported_);
    s.store_object_field("sponsor", static_cast<const BaseObject *>(sponsor_.get()));
    s.store_field("title", title_);
    s.store_field("additional_info", additional_info_);
    s.store_class_end();
  }
}

webAppInfo::webAppInfo()
  : launch_id_()
  , url_()
{}

webAppInfo::webAppInfo(int64 launch_id_, string const &url_)
  : launch_id_(launch_id_)
  , url_(url_)
{}

const std::int32_t webAppInfo::ID;

void webAppInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "webAppInfo");
    s.store_field("launch_id", launch_id_);
    s.store_field("url", url_);
    s.store_class_end();
  }
}

addFileToDownloads::addFileToDownloads()
  : file_id_()
  , chat_id_()
  , message_id_()
  , priority_()
{}

addFileToDownloads::addFileToDownloads(int32 file_id_, int53 chat_id_, int53 message_id_, int32 priority_)
  : file_id_(file_id_)
  , chat_id_(chat_id_)
  , message_id_(message_id_)
  , priority_(priority_)
{}

const std::int32_t addFileToDownloads::ID;

void addFileToDownloads::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addFileToDownloads");
    s.store_field("file_id", file_id_);
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_field("priority", priority_);
    s.store_class_end();
  }
}

addOffer::addOffer()
  : chat_id_()
  , message_id_()
  , options_()
{}

addOffer::addOffer(int53 chat_id_, int53 message_id_, object_ptr<messageSendOptions> &&options_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , options_(std::move(options_))
{}

const std::int32_t addOffer::ID;

void addOffer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addOffer");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("options", static_cast<const BaseObject *>(options_.get()));
    s.store_class_end();
  }
}

addProxy::addProxy()
  : server_()
  , port_()
  , enable_()
  , type_()
{}

addProxy::addProxy(string const &server_, int32 port_, bool enable_, object_ptr<ProxyType> &&type_)
  : server_(server_)
  , port_(port_)
  , enable_(enable_)
  , type_(std::move(type_))
{}

const std::int32_t addProxy::ID;

void addProxy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "addProxy");
    s.store_field("server", server_);
    s.store_field("port", port_);
    s.store_field("enable", enable_);
    s.store_object_field("type", static_cast<const BaseObject *>(type_.get()));
    s.store_class_end();
  }
}

answerCustomQuery::answerCustomQuery()
  : custom_query_id_()
  , data_()
{}

answerCustomQuery::answerCustomQuery(int64 custom_query_id_, string const &data_)
  : custom_query_id_(custom_query_id_)
  , data_(data_)
{}

const std::int32_t answerCustomQuery::ID;

void answerCustomQuery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "answerCustomQuery");
    s.store_field("custom_query_id", custom_query_id_);
    s.store_field("data", data_);
    s.store_class_end();
  }
}

changeImportedContacts::changeImportedContacts()
  : contacts_()
{}

changeImportedContacts::changeImportedContacts(array<object_ptr<importedContact>> &&contacts_)
  : contacts_(std::move(contacts_))
{}

const std::int32_t changeImportedContacts::ID;

void changeImportedContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "changeImportedContacts");
    { s.store_vector_begin("contacts", contacts_.size()); for (const auto &_value : contacts_) { s.store_object_field("", static_cast<const BaseObject *>(_value.get())); } s.store_class_end(); }
    s.store_class_end();
  }
}

changeStickerSet::changeStickerSet()
  : set_id_()
  , is_installed_()
  , is_archived_()
{}

changeStickerSet::changeStickerSet(int64 set_id_, bool is_installed_, bool is_archived_)
  : set_id_(set_id_)
  , is_installed_(is_installed_)
  , is_archived_(is_archived_)
{}

const std::int32_t changeStickerSet::ID;

void changeStickerSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "changeStickerSet");
    s.store_field("set_id", set_id_);
    s.store_field("is_installed", is_installed_);
    s.store_field("is_archived", is_archived_);
    s.store_class_end();
  }
}

closeWebApp::closeWebApp()
  : web_app_launch_id_()
{}

closeWebApp::closeWebApp(int64 web_app_launch_id_)
  : web_app_launch_id_(web_app_launch_id_)
{}

const std::int32_t closeWebApp::ID;

void closeWebApp::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "closeWebApp");
    s.store_field("web_app_launch_id", web_app_launch_id_);
    s.store_class_end();
  }
}

createBasicGroupChat::createBasicGroupChat()
  : basic_group_id_()
  , force_()
{}

createBasicGroupChat::createBasicGroupChat(int53 basic_group_id_, bool force_)
  : basic_group_id_(basic_group_id_)
  , force_(force_)
{}

const std::int32_t createBasicGroupChat::ID;

void createBasicGroupChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "createBasicGroupChat");
    s.store_field("basic_group_id", basic_group_id_);
    s.store_field("force", force_);
    s.store_class_end();
  }
}

deleteBusinessConnectedBot::deleteBusinessConnectedBot()
  : bot_user_id_()
{}

deleteBusinessConnectedBot::deleteBusinessConnectedBot(int53 bot_user_id_)
  : bot_user_id_(bot_user_id_)
{}

const std::int32_t deleteBusinessConnectedBot::ID;

void deleteBusinessConnectedBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteBusinessConnectedBot");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_class_end();
  }
}

deleteBusinessMessages::deleteBusinessMessages()
  : business_connection_id_()
  , message_ids_()
{}

deleteBusinessMessages::deleteBusinessMessages(string const &business_connection_id_, array<int53> &&message_ids_)
  : business_connection_id_(business_connection_id_)
  , message_ids_(std::move(message_ids_))
{}

const std::int32_t deleteBusinessMessages::ID;

void deleteBusinessMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "deleteBusinessMessages");
    s.store_field("business_connection_id", business_connection_id_);
    { s.store_vector_begin("message_ids", message_ids_.size()); for (const auto &_value : message_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

editBusinessMessageChecklist::editBusinessMessageChecklist()
  : business_connection_id_()
  , chat_id_()
  , message_id_()
  , reply_markup_()
  , checklist_()
{}

editBusinessMessageChecklist::editBusinessMessageChecklist(string const &business_connection_id_, int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<inputChecklist> &&checklist_)
  : business_connection_id_(business_connection_id_)
  , chat_id_(chat_id_)
  , message_id_(message_id_)
  , reply_markup_(std::move(reply_markup_))
  , checklist_(std::move(checklist_))
{}

const std::int32_t editBusinessMessageChecklist::ID;

void editBusinessMessageChecklist::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editBusinessMessageChecklist");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("checklist", static_cast<const BaseObject *>(checklist_.get()));
    s.store_class_end();
  }
}

editBusinessMessageMedia::editBusinessMessageMedia()
  : business_connection_id_()
  , chat_id_()
  , message_id_()
  , reply_markup_()
  , input_message_content_()
{}

editBusinessMessageMedia::editBusinessMessageMedia(string const &business_connection_id_, int53 chat_id_, int53 message_id_, object_ptr<ReplyMarkup> &&reply_markup_, object_ptr<InputMessageContent> &&input_message_content_)
  : business_connection_id_(business_connection_id_)
  , chat_id_(chat_id_)
  , message_id_(message_id_)
  , reply_markup_(std::move(reply_markup_))
  , input_message_content_(std::move(input_message_content_))
{}

const std::int32_t editBusinessMessageMedia::ID;

void editBusinessMessageMedia::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editBusinessMessageMedia");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("reply_markup", static_cast<const BaseObject *>(reply_markup_.get()));
    s.store_object_field("input_message_content", static_cast<const BaseObject *>(input_message_content_.get()));
    s.store_class_end();
  }
}

editChatFolderInviteLink::editChatFolderInviteLink()
  : chat_folder_id_()
  , invite_link_()
  , name_()
  , chat_ids_()
{}

editChatFolderInviteLink::editChatFolderInviteLink(int32 chat_folder_id_, string const &invite_link_, string const &name_, array<int53> &&chat_ids_)
  : chat_folder_id_(chat_folder_id_)
  , invite_link_(invite_link_)
  , name_(name_)
  , chat_ids_(std::move(chat_ids_))
{}

const std::int32_t editChatFolderInviteLink::ID;

void editChatFolderInviteLink::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "editChatFolderInviteLink");
    s.store_field("chat_folder_id", chat_folder_id_);
    s.store_field("invite_link", invite_link_);
    s.store_field("name", name_);
    { s.store_vector_begin("chat_ids", chat_ids_.size()); for (const auto &_value : chat_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

enableProxy::enableProxy()
  : proxy_id_()
{}

enableProxy::enableProxy(int32 proxy_id_)
  : proxy_id_(proxy_id_)
{}

const std::int32_t enableProxy::ID;

void enableProxy::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "enableProxy");
    s.store_field("proxy_id", proxy_id_);
    s.store_class_end();
  }
}

forwardMessages::forwardMessages()
  : chat_id_()
  , topic_id_()
  , from_chat_id_()
  , message_ids_()
  , options_()
  , send_copy_()
  , remove_caption_()
{}

forwardMessages::forwardMessages(int53 chat_id_, object_ptr<MessageTopic> &&topic_id_, int53 from_chat_id_, array<int53> &&message_ids_, object_ptr<messageSendOptions> &&options_, bool send_copy_, bool remove_caption_)
  : chat_id_(chat_id_)
  , topic_id_(std::move(topic_id_))
  , from_chat_id_(from_chat_id_)
  , message_ids_(std::move(message_ids_))
  , options_(std::move(options_))
  , send_copy_(send_copy_)
  , remove_caption_(remove_caption_)
{}

const std::int32_t forwardMessages::ID;

void forwardMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "forwardMessages");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("topic_id", static_cast<const BaseObject *>(topic_id_.get()));
    s.store_field("from_chat_id", from_chat_id_);
    { s.store_vector_begin("message_ids", message_ids_.size()); for (const auto &_value : message_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_object_field("options", static_cast<const BaseObject *>(options_.get()));
    s.store_field("send_copy", send_copy_);
    s.store_field("remove_caption", remove_caption_);
    s.store_class_end();
  }
}

getActiveSessions::getActiveSessions() {
}

const std::int32_t getActiveSessions::ID;

void getActiveSessions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getActiveSessions");
    s.store_class_end();
  }
}

getAutosaveSettings::getAutosaveSettings() {
}

const std::int32_t getAutosaveSettings::ID;

void getAutosaveSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getAutosaveSettings");
    s.store_class_end();
  }
}

getChatActiveStories::getChatActiveStories()
  : chat_id_()
{}

getChatActiveStories::getChatActiveStories(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t getChatActiveStories::ID;

void getChatActiveStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatActiveStories");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

getChatAvailablePaidMessageReactionSenders::getChatAvailablePaidMessageReactionSenders()
  : chat_id_()
{}

getChatAvailablePaidMessageReactionSenders::getChatAvailablePaidMessageReactionSenders(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t getChatAvailablePaidMessageReactionSenders::ID;

void getChatAvailablePaidMessageReactionSenders::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatAvailablePaidMessageReactionSenders");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

getChatFolderChatCount::getChatFolderChatCount()
  : folder_()
{}

getChatFolderChatCount::getChatFolderChatCount(object_ptr<chatFolder> &&folder_)
  : folder_(std::move(folder_))
{}

const std::int32_t getChatFolderChatCount::ID;

void getChatFolderChatCount::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatFolderChatCount");
    s.store_object_field("folder", static_cast<const BaseObject *>(folder_.get()));
    s.store_class_end();
  }
}

getChatScheduledMessages::getChatScheduledMessages()
  : chat_id_()
{}

getChatScheduledMessages::getChatScheduledMessages(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t getChatScheduledMessages::ID;

void getChatScheduledMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatScheduledMessages");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

getChatSponsoredMessages::getChatSponsoredMessages()
  : chat_id_()
{}

getChatSponsoredMessages::getChatSponsoredMessages(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t getChatSponsoredMessages::ID;

void getChatSponsoredMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getChatSponsoredMessages");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

getContacts::getContacts() {
}

const std::int32_t getContacts::ID;

void getContacts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getContacts");
    s.store_class_end();
  }
}

getCustomEmojiReactionAnimations::getCustomEmojiReactionAnimations() {
}

const std::int32_t getCustomEmojiReactionAnimations::ID;

void getCustomEmojiReactionAnimations::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getCustomEmojiReactionAnimations");
    s.store_class_end();
  }
}

getCustomEmojiStickers::getCustomEmojiStickers()
  : custom_emoji_ids_()
{}

getCustomEmojiStickers::getCustomEmojiStickers(array<int64> &&custom_emoji_ids_)
  : custom_emoji_ids_(std::move(custom_emoji_ids_))
{}

const std::int32_t getCustomEmojiStickers::ID;

void getCustomEmojiStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getCustomEmojiStickers");
    { s.store_vector_begin("custom_emoji_ids", custom_emoji_ids_.size()); for (const auto &_value : custom_emoji_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

getGiftCollections::getGiftCollections()
  : owner_id_()
{}

getGiftCollections::getGiftCollections(object_ptr<MessageSender> &&owner_id_)
  : owner_id_(std::move(owner_id_))
{}

const std::int32_t getGiftCollections::ID;

void getGiftCollections::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getGiftCollections");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_class_end();
  }
}

getGiveawayInfo::getGiveawayInfo()
  : chat_id_()
  , message_id_()
{}

getGiveawayInfo::getGiveawayInfo(int53 chat_id_, int53 message_id_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t getGiveawayInfo::ID;

void getGiveawayInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getGiveawayInfo");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

getGreetingStickers::getGreetingStickers() {
}

const std::int32_t getGreetingStickers::ID;

void getGreetingStickers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getGreetingStickers");
    s.store_class_end();
  }
}

getGrossingWebAppBots::getGrossingWebAppBots()
  : offset_()
  , limit_()
{}

getGrossingWebAppBots::getGrossingWebAppBots(string const &offset_, int32 limit_)
  : offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getGrossingWebAppBots::ID;

void getGrossingWebAppBots::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getGrossingWebAppBots");
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getLanguagePackStrings::getLanguagePackStrings()
  : language_pack_id_()
  , keys_()
{}

getLanguagePackStrings::getLanguagePackStrings(string const &language_pack_id_, array<string> &&keys_)
  : language_pack_id_(language_pack_id_)
  , keys_(std::move(keys_))
{}

const std::int32_t getLanguagePackStrings::ID;

void getLanguagePackStrings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getLanguagePackStrings");
    s.store_field("language_pack_id", language_pack_id_);
    { s.store_vector_begin("keys", keys_.size()); for (const auto &_value : keys_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

getMessageAddedReactions::getMessageAddedReactions()
  : chat_id_()
  , message_id_()
  , reaction_type_()
  , offset_()
  , limit_()
{}

getMessageAddedReactions::getMessageAddedReactions(int53 chat_id_, int53 message_id_, object_ptr<ReactionType> &&reaction_type_, string const &offset_, int32 limit_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , reaction_type_(std::move(reaction_type_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getMessageAddedReactions::ID;

void getMessageAddedReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMessageAddedReactions");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_object_field("reaction_type", static_cast<const BaseObject *>(reaction_type_.get()));
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getMessageLinkInfo::getMessageLinkInfo()
  : url_()
{}

getMessageLinkInfo::getMessageLinkInfo(string const &url_)
  : url_(url_)
{}

const std::int32_t getMessageLinkInfo::ID;

void getMessageLinkInfo::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getMessageLinkInfo");
    s.store_field("url", url_);
    s.store_class_end();
  }
}

getNewChatPrivacySettings::getNewChatPrivacySettings() {
}

const std::int32_t getNewChatPrivacySettings::ID;

void getNewChatPrivacySettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getNewChatPrivacySettings");
    s.store_class_end();
  }
}

getOwnedStickerSets::getOwnedStickerSets()
  : offset_sticker_set_id_()
  , limit_()
{}

getOwnedStickerSets::getOwnedStickerSets(int64 offset_sticker_set_id_, int32 limit_)
  : offset_sticker_set_id_(offset_sticker_set_id_)
  , limit_(limit_)
{}

const std::int32_t getOwnedStickerSets::ID;

void getOwnedStickerSets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getOwnedStickerSets");
    s.store_field("offset_sticker_set_id", offset_sticker_set_id_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getPassportAuthorizationFormAvailableElements::getPassportAuthorizationFormAvailableElements()
  : authorization_form_id_()
  , password_()
{}

getPassportAuthorizationFormAvailableElements::getPassportAuthorizationFormAvailableElements(int32 authorization_form_id_, string const &password_)
  : authorization_form_id_(authorization_form_id_)
  , password_(password_)
{}

const std::int32_t getPassportAuthorizationFormAvailableElements::ID;

void getPassportAuthorizationFormAvailableElements::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPassportAuthorizationFormAvailableElements");
    s.store_field("authorization_form_id", authorization_form_id_);
    s.store_field("password", password_);
    s.store_class_end();
  }
}

getPreferredCountryLanguage::getPreferredCountryLanguage()
  : country_code_()
{}

getPreferredCountryLanguage::getPreferredCountryLanguage(string const &country_code_)
  : country_code_(country_code_)
{}

const std::int32_t getPreferredCountryLanguage::ID;

void getPreferredCountryLanguage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPreferredCountryLanguage");
    s.store_field("country_code", country_code_);
    s.store_class_end();
  }
}

getPreparedInlineMessage::getPreparedInlineMessage()
  : bot_user_id_()
  , prepared_message_id_()
{}

getPreparedInlineMessage::getPreparedInlineMessage(int53 bot_user_id_, string const &prepared_message_id_)
  : bot_user_id_(bot_user_id_)
  , prepared_message_id_(prepared_message_id_)
{}

const std::int32_t getPreparedInlineMessage::ID;

void getPreparedInlineMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getPreparedInlineMessage");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("prepared_message_id", prepared_message_id_);
    s.store_class_end();
  }
}

getSavedNotificationSound::getSavedNotificationSound()
  : notification_sound_id_()
{}

getSavedNotificationSound::getSavedNotificationSound(int64 notification_sound_id_)
  : notification_sound_id_(notification_sound_id_)
{}

const std::int32_t getSavedNotificationSound::ID;

void getSavedNotificationSound::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getSavedNotificationSound");
    s.store_field("notification_sound_id", notification_sound_id_);
    s.store_class_end();
  }
}

getSearchedForTags::getSearchedForTags()
  : tag_prefix_()
  , limit_()
{}

getSearchedForTags::getSearchedForTags(string const &tag_prefix_, int32 limit_)
  : tag_prefix_(tag_prefix_)
  , limit_(limit_)
{}

const std::int32_t getSearchedForTags::ID;

void getSearchedForTags::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getSearchedForTags");
    s.store_field("tag_prefix", tag_prefix_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getStarTransactions::getStarTransactions()
  : owner_id_()
  , subscription_id_()
  , direction_()
  , offset_()
  , limit_()
{}

getStarTransactions::getStarTransactions(object_ptr<MessageSender> &&owner_id_, string const &subscription_id_, object_ptr<TransactionDirection> &&direction_, string const &offset_, int32 limit_)
  : owner_id_(std::move(owner_id_))
  , subscription_id_(subscription_id_)
  , direction_(std::move(direction_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getStarTransactions::ID;

void getStarTransactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStarTransactions");
    s.store_object_field("owner_id", static_cast<const BaseObject *>(owner_id_.get()));
    s.store_field("subscription_id", subscription_id_);
    s.store_object_field("direction", static_cast<const BaseObject *>(direction_.get()));
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getStickerOutline::getStickerOutline()
  : sticker_file_id_()
  , for_animated_emoji_()
  , for_clicked_animated_emoji_message_()
{}

getStickerOutline::getStickerOutline(int32 sticker_file_id_, bool for_animated_emoji_, bool for_clicked_animated_emoji_message_)
  : sticker_file_id_(sticker_file_id_)
  , for_animated_emoji_(for_animated_emoji_)
  , for_clicked_animated_emoji_message_(for_clicked_animated_emoji_message_)
{}

const std::int32_t getStickerOutline::ID;

void getStickerOutline::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getStickerOutline");
    s.store_field("sticker_file_id", sticker_file_id_);
    s.store_field("for_animated_emoji", for_animated_emoji_);
    s.store_field("for_clicked_animated_emoji_message", for_clicked_animated_emoji_message_);
    s.store_class_end();
  }
}

getSupergroupMembers::getSupergroupMembers()
  : supergroup_id_()
  , filter_()
  , offset_()
  , limit_()
{}

getSupergroupMembers::getSupergroupMembers(int53 supergroup_id_, object_ptr<SupergroupMembersFilter> &&filter_, int32 offset_, int32 limit_)
  : supergroup_id_(supergroup_id_)
  , filter_(std::move(filter_))
  , offset_(offset_)
  , limit_(limit_)
{}

const std::int32_t getSupergroupMembers::ID;

void getSupergroupMembers::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getSupergroupMembers");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_object_field("filter", static_cast<const BaseObject *>(filter_.get()));
    s.store_field("offset", offset_);
    s.store_field("limit", limit_);
    s.store_class_end();
  }
}

getUser::getUser()
  : user_id_()
{}

getUser::getUser(int53 user_id_)
  : user_id_(user_id_)
{}

const std::int32_t getUser::ID;

void getUser::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getUser");
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

getUserChatBoosts::getUserChatBoosts()
  : chat_id_()
  , user_id_()
{}

getUserChatBoosts::getUserChatBoosts(int53 chat_id_, int53 user_id_)
  : chat_id_(chat_id_)
  , user_id_(user_id_)
{}

const std::int32_t getUserChatBoosts::ID;

void getUserChatBoosts::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "getUserChatBoosts");
    s.store_field("chat_id", chat_id_);
    s.store_field("user_id", user_id_);
    s.store_class_end();
  }
}

isProfileAudio::isProfileAudio()
  : file_id_()
{}

isProfileAudio::isProfileAudio(int32 file_id_)
  : file_id_(file_id_)
{}

const std::int32_t isProfileAudio::ID;

void isProfileAudio::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "isProfileAudio");
    s.store_field("file_id", file_id_);
    s.store_class_end();
  }
}

loadActiveStories::loadActiveStories()
  : story_list_()
{}

loadActiveStories::loadActiveStories(object_ptr<StoryList> &&story_list_)
  : story_list_(std::move(story_list_))
{}

const std::int32_t loadActiveStories::ID;

void loadActiveStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "loadActiveStories");
    s.store_object_field("story_list", static_cast<const BaseObject *>(story_list_.get()));
    s.store_class_end();
  }
}

openBotSimilarBot::openBotSimilarBot()
  : bot_user_id_()
  , opened_bot_user_id_()
{}

openBotSimilarBot::openBotSimilarBot(int53 bot_user_id_, int53 opened_bot_user_id_)
  : bot_user_id_(bot_user_id_)
  , opened_bot_user_id_(opened_bot_user_id_)
{}

const std::int32_t openBotSimilarBot::ID;

void openBotSimilarBot::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "openBotSimilarBot");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("opened_bot_user_id", opened_bot_user_id_);
    s.store_class_end();
  }
}

openSponsoredChat::openSponsoredChat()
  : sponsored_chat_unique_id_()
{}

openSponsoredChat::openSponsoredChat(int53 sponsored_chat_unique_id_)
  : sponsored_chat_unique_id_(sponsored_chat_unique_id_)
{}

const std::int32_t openSponsoredChat::ID;

void openSponsoredChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "openSponsoredChat");
    s.store_field("sponsored_chat_unique_id", sponsored_chat_unique_id_);
    s.store_class_end();
  }
}

readBusinessMessage::readBusinessMessage()
  : business_connection_id_()
  , chat_id_()
  , message_id_()
{}

readBusinessMessage::readBusinessMessage(string const &business_connection_id_, int53 chat_id_, int53 message_id_)
  : business_connection_id_(business_connection_id_)
  , chat_id_(chat_id_)
  , message_id_(message_id_)
{}

const std::int32_t readBusinessMessage::ID;

void readBusinessMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "readBusinessMessage");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    s.store_class_end();
  }
}

recoverAuthenticationPassword::recoverAuthenticationPassword()
  : recovery_code_()
  , new_password_()
  , new_hint_()
{}

recoverAuthenticationPassword::recoverAuthenticationPassword(string const &recovery_code_, string const &new_password_, string const &new_hint_)
  : recovery_code_(recovery_code_)
  , new_password_(new_password_)
  , new_hint_(new_hint_)
{}

const std::int32_t recoverAuthenticationPassword::ID;

void recoverAuthenticationPassword::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "recoverAuthenticationPassword");
    s.store_field("recovery_code", recovery_code_);
    s.store_field("new_password", new_password_);
    s.store_field("new_hint", new_hint_);
    s.store_class_end();
  }
}

removeChatActionBar::removeChatActionBar()
  : chat_id_()
{}

removeChatActionBar::removeChatActionBar(int53 chat_id_)
  : chat_id_(chat_id_)
{}

const std::int32_t removeChatActionBar::ID;

void removeChatActionBar::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeChatActionBar");
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

removeSavedAnimation::removeSavedAnimation()
  : animation_()
{}

removeSavedAnimation::removeSavedAnimation(object_ptr<InputFile> &&animation_)
  : animation_(std::move(animation_))
{}

const std::int32_t removeSavedAnimation::ID;

void removeSavedAnimation::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeSavedAnimation");
    s.store_object_field("animation", static_cast<const BaseObject *>(animation_.get()));
    s.store_class_end();
  }
}

removeStoryAlbumStories::removeStoryAlbumStories()
  : chat_id_()
  , story_album_id_()
  , story_ids_()
{}

removeStoryAlbumStories::removeStoryAlbumStories(int53 chat_id_, int32 story_album_id_, array<int32> &&story_ids_)
  : chat_id_(chat_id_)
  , story_album_id_(story_album_id_)
  , story_ids_(std::move(story_ids_))
{}

const std::int32_t removeStoryAlbumStories::ID;

void removeStoryAlbumStories::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeStoryAlbumStories");
    s.store_field("chat_id", chat_id_);
    s.store_field("story_album_id", story_album_id_);
    { s.store_vector_begin("story_ids", story_ids_.size()); for (const auto &_value : story_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

removeTopChat::removeTopChat()
  : category_()
  , chat_id_()
{}

removeTopChat::removeTopChat(object_ptr<TopChatCategory> &&category_, int53 chat_id_)
  : category_(std::move(category_))
  , chat_id_(chat_id_)
{}

const std::int32_t removeTopChat::ID;

void removeTopChat::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "removeTopChat");
    s.store_object_field("category", static_cast<const BaseObject *>(category_.get()));
    s.store_field("chat_id", chat_id_);
    s.store_class_end();
  }
}

reorderInstalledStickerSets::reorderInstalledStickerSets()
  : sticker_type_()
  , sticker_set_ids_()
{}

reorderInstalledStickerSets::reorderInstalledStickerSets(object_ptr<StickerType> &&sticker_type_, array<int64> &&sticker_set_ids_)
  : sticker_type_(std::move(sticker_type_))
  , sticker_set_ids_(std::move(sticker_set_ids_))
{}

const std::int32_t reorderInstalledStickerSets::ID;

void reorderInstalledStickerSets::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "reorderInstalledStickerSets");
    s.store_object_field("sticker_type", static_cast<const BaseObject *>(sticker_type_.get()));
    { s.store_vector_begin("sticker_set_ids", sticker_set_ids_.size()); for (const auto &_value : sticker_set_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

requestPasswordRecovery::requestPasswordRecovery() {
}

const std::int32_t requestPasswordRecovery::ID;

void requestPasswordRecovery::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "requestPasswordRecovery");
    s.store_class_end();
  }
}

requestQrCodeAuthentication::requestQrCodeAuthentication()
  : other_user_ids_()
{}

requestQrCodeAuthentication::requestQrCodeAuthentication(array<int53> &&other_user_ids_)
  : other_user_ids_(std::move(other_user_ids_))
{}

const std::int32_t requestQrCodeAuthentication::ID;

void requestQrCodeAuthentication::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "requestQrCodeAuthentication");
    { s.store_vector_begin("other_user_ids", other_user_ids_.size()); for (const auto &_value : other_user_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

resendRecoveryEmailAddressCode::resendRecoveryEmailAddressCode() {
}

const std::int32_t resendRecoveryEmailAddressCode::ID;

void resendRecoveryEmailAddressCode::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "resendRecoveryEmailAddressCode");
    s.store_class_end();
  }
}

sendInlineQueryResultMessage::sendInlineQueryResultMessage()
  : chat_id_()
  , topic_id_()
  , reply_to_()
  , options_()
  , query_id_()
  , result_id_()
  , hide_via_bot_()
{}

sendInlineQueryResultMessage::sendInlineQueryResultMessage(int53 chat_id_, object_ptr<MessageTopic> &&topic_id_, object_ptr<InputMessageReplyTo> &&reply_to_, object_ptr<messageSendOptions> &&options_, int64 query_id_, string const &result_id_, bool hide_via_bot_)
  : chat_id_(chat_id_)
  , topic_id_(std::move(topic_id_))
  , reply_to_(std::move(reply_to_))
  , options_(std::move(options_))
  , query_id_(query_id_)
  , result_id_(result_id_)
  , hide_via_bot_(hide_via_bot_)
{}

const std::int32_t sendInlineQueryResultMessage::ID;

void sendInlineQueryResultMessage::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendInlineQueryResultMessage");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("topic_id", static_cast<const BaseObject *>(topic_id_.get()));
    s.store_object_field("reply_to", static_cast<const BaseObject *>(reply_to_.get()));
    s.store_object_field("options", static_cast<const BaseObject *>(options_.get()));
    s.store_field("query_id", query_id_);
    s.store_field("result_id", result_id_);
    s.store_field("hide_via_bot", hide_via_bot_);
    s.store_class_end();
  }
}

sendQuickReplyShortcutMessages::sendQuickReplyShortcutMessages()
  : chat_id_()
  , shortcut_id_()
  , sending_id_()
{}

sendQuickReplyShortcutMessages::sendQuickReplyShortcutMessages(int53 chat_id_, int32 shortcut_id_, int32 sending_id_)
  : chat_id_(chat_id_)
  , shortcut_id_(shortcut_id_)
  , sending_id_(sending_id_)
{}

const std::int32_t sendQuickReplyShortcutMessages::ID;

void sendQuickReplyShortcutMessages::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "sendQuickReplyShortcutMessages");
    s.store_field("chat_id", chat_id_);
    s.store_field("shortcut_id", shortcut_id_);
    s.store_field("sending_id", sending_id_);
    s.store_class_end();
  }
}

setAutosaveSettings::setAutosaveSettings()
  : scope_()
  , settings_()
{}

setAutosaveSettings::setAutosaveSettings(object_ptr<AutosaveSettingsScope> &&scope_, object_ptr<scopeAutosaveSettings> &&settings_)
  : scope_(std::move(scope_))
  , settings_(std::move(settings_))
{}

const std::int32_t setAutosaveSettings::ID;

void setAutosaveSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setAutosaveSettings");
    s.store_object_field("scope", static_cast<const BaseObject *>(scope_.get()));
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

setBotInfoDescription::setBotInfoDescription()
  : bot_user_id_()
  , language_code_()
  , description_()
{}

setBotInfoDescription::setBotInfoDescription(int53 bot_user_id_, string const &language_code_, string const &description_)
  : bot_user_id_(bot_user_id_)
  , language_code_(language_code_)
  , description_(description_)
{}

const std::int32_t setBotInfoDescription::ID;

void setBotInfoDescription::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setBotInfoDescription");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("language_code", language_code_);
    s.store_field("description", description_);
    s.store_class_end();
  }
}

setBotInfoShortDescription::setBotInfoShortDescription()
  : bot_user_id_()
  , language_code_()
  , short_description_()
{}

setBotInfoShortDescription::setBotInfoShortDescription(int53 bot_user_id_, string const &language_code_, string const &short_description_)
  : bot_user_id_(bot_user_id_)
  , language_code_(language_code_)
  , short_description_(short_description_)
{}

const std::int32_t setBotInfoShortDescription::ID;

void setBotInfoShortDescription::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setBotInfoShortDescription");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("language_code", language_code_);
    s.store_field("short_description", short_description_);
    s.store_class_end();
  }
}

setBotUpdatesStatus::setBotUpdatesStatus()
  : pending_update_count_()
  , error_message_()
{}

setBotUpdatesStatus::setBotUpdatesStatus(int32 pending_update_count_, string const &error_message_)
  : pending_update_count_(pending_update_count_)
  , error_message_(error_message_)
{}

const std::int32_t setBotUpdatesStatus::ID;

void setBotUpdatesStatus::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setBotUpdatesStatus");
    s.store_field("pending_update_count", pending_update_count_);
    s.store_field("error_message", error_message_);
    s.store_class_end();
  }
}

setBusinessAccountUsername::setBusinessAccountUsername()
  : business_connection_id_()
  , username_()
{}

setBusinessAccountUsername::setBusinessAccountUsername(string const &business_connection_id_, string const &username_)
  : business_connection_id_(business_connection_id_)
  , username_(username_)
{}

const std::int32_t setBusinessAccountUsername::ID;

void setBusinessAccountUsername::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setBusinessAccountUsername");
    s.store_field("business_connection_id", business_connection_id_);
    s.store_field("username", username_);
    s.store_class_end();
  }
}

setChatAvailableReactions::setChatAvailableReactions()
  : chat_id_()
  , available_reactions_()
{}

setChatAvailableReactions::setChatAvailableReactions(int53 chat_id_, object_ptr<ChatAvailableReactions> &&available_reactions_)
  : chat_id_(chat_id_)
  , available_reactions_(std::move(available_reactions_))
{}

const std::int32_t setChatAvailableReactions::ID;

void setChatAvailableReactions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatAvailableReactions");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("available_reactions", static_cast<const BaseObject *>(available_reactions_.get()));
    s.store_class_end();
  }
}

setChatSlowModeDelay::setChatSlowModeDelay()
  : chat_id_()
  , slow_mode_delay_()
{}

setChatSlowModeDelay::setChatSlowModeDelay(int53 chat_id_, int32 slow_mode_delay_)
  : chat_id_(chat_id_)
  , slow_mode_delay_(slow_mode_delay_)
{}

const std::int32_t setChatSlowModeDelay::ID;

void setChatSlowModeDelay::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setChatSlowModeDelay");
    s.store_field("chat_id", chat_id_);
    s.store_field("slow_mode_delay", slow_mode_delay_);
    s.store_class_end();
  }
}

setFileGenerationProgress::setFileGenerationProgress()
  : generation_id_()
  , expected_size_()
  , local_prefix_size_()
{}

setFileGenerationProgress::setFileGenerationProgress(int64 generation_id_, int53 expected_size_, int53 local_prefix_size_)
  : generation_id_(generation_id_)
  , expected_size_(expected_size_)
  , local_prefix_size_(local_prefix_size_)
{}

const std::int32_t setFileGenerationProgress::ID;

void setFileGenerationProgress::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setFileGenerationProgress");
    s.store_field("generation_id", generation_id_);
    s.store_field("expected_size", expected_size_);
    s.store_field("local_prefix_size", local_prefix_size_);
    s.store_class_end();
  }
}

setGiftSettings::setGiftSettings()
  : settings_()
{}

setGiftSettings::setGiftSettings(object_ptr<giftSettings> &&settings_)
  : settings_(std::move(settings_))
{}

const std::int32_t setGiftSettings::ID;

void setGiftSettings::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setGiftSettings");
    s.store_object_field("settings", static_cast<const BaseObject *>(settings_.get()));
    s.store_class_end();
  }
}

setMessageSenderBlockList::setMessageSenderBlockList()
  : sender_id_()
  , block_list_()
{}

setMessageSenderBlockList::setMessageSenderBlockList(object_ptr<MessageSender> &&sender_id_, object_ptr<BlockList> &&block_list_)
  : sender_id_(std::move(sender_id_))
  , block_list_(std::move(block_list_))
{}

const std::int32_t setMessageSenderBlockList::ID;

void setMessageSenderBlockList::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setMessageSenderBlockList");
    s.store_object_field("sender_id", static_cast<const BaseObject *>(sender_id_.get()));
    s.store_object_field("block_list", static_cast<const BaseObject *>(block_list_.get()));
    s.store_class_end();
  }
}

setPinnedSavedMessagesTopics::setPinnedSavedMessagesTopics()
  : saved_messages_topic_ids_()
{}

setPinnedSavedMessagesTopics::setPinnedSavedMessagesTopics(array<int53> &&saved_messages_topic_ids_)
  : saved_messages_topic_ids_(std::move(saved_messages_topic_ids_))
{}

const std::int32_t setPinnedSavedMessagesTopics::ID;

void setPinnedSavedMessagesTopics::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setPinnedSavedMessagesTopics");
    { s.store_vector_begin("saved_messages_topic_ids", saved_messages_topic_ids_.size()); for (const auto &_value : saved_messages_topic_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

setPollAnswer::setPollAnswer()
  : chat_id_()
  , message_id_()
  , option_ids_()
{}

setPollAnswer::setPollAnswer(int53 chat_id_, int53 message_id_, array<int32> &&option_ids_)
  : chat_id_(chat_id_)
  , message_id_(message_id_)
  , option_ids_(std::move(option_ids_))
{}

const std::int32_t setPollAnswer::ID;

void setPollAnswer::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setPollAnswer");
    s.store_field("chat_id", chat_id_);
    s.store_field("message_id", message_id_);
    { s.store_vector_begin("option_ids", option_ids_.size()); for (const auto &_value : option_ids_) { s.store_field("", _value); } s.store_class_end(); }
    s.store_class_end();
  }
}

setStickerPositionInSet::setStickerPositionInSet()
  : sticker_()
  , position_()
{}

setStickerPositionInSet::setStickerPositionInSet(object_ptr<InputFile> &&sticker_, int32 position_)
  : sticker_(std::move(sticker_))
  , position_(position_)
{}

const std::int32_t setStickerPositionInSet::ID;

void setStickerPositionInSet::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setStickerPositionInSet");
    s.store_object_field("sticker", static_cast<const BaseObject *>(sticker_.get()));
    s.store_field("position", position_);
    s.store_class_end();
  }
}

setStickerSetTitle::setStickerSetTitle()
  : name_()
  , title_()
{}

setStickerSetTitle::setStickerSetTitle(string const &name_, string const &title_)
  : name_(name_)
  , title_(title_)
{}

const std::int32_t setStickerSetTitle::ID;

void setStickerSetTitle::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setStickerSetTitle");
    s.store_field("name", name_);
    s.store_field("title", title_);
    s.store_class_end();
  }
}

setVideoChatDefaultParticipant::setVideoChatDefaultParticipant()
  : chat_id_()
  , default_participant_id_()
{}

setVideoChatDefaultParticipant::setVideoChatDefaultParticipant(int53 chat_id_, object_ptr<MessageSender> &&default_participant_id_)
  : chat_id_(chat_id_)
  , default_participant_id_(std::move(default_participant_id_))
{}

const std::int32_t setVideoChatDefaultParticipant::ID;

void setVideoChatDefaultParticipant::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "setVideoChatDefaultParticipant");
    s.store_field("chat_id", chat_id_);
    s.store_object_field("default_participant_id", static_cast<const BaseObject *>(default_participant_id_.get()));
    s.store_class_end();
  }
}

terminateAllOtherSessions::terminateAllOtherSessions() {
}

const std::int32_t terminateAllOtherSessions::ID;

void terminateAllOtherSessions::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "terminateAllOtherSessions");
    s.store_class_end();
  }
}

terminateSession::terminateSession()
  : session_id_()
{}

terminateSession::terminateSession(int64 session_id_)
  : session_id_(session_id_)
{}

const std::int32_t terminateSession::ID;

void terminateSession::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "terminateSession");
    s.store_field("session_id", session_id_);
    s.store_class_end();
  }
}

testGetDifference::testGetDifference() {
}

const std::int32_t testGetDifference::ID;

void testGetDifference::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "testGetDifference");
    s.store_class_end();
  }
}

toggleBotUsernameIsActive::toggleBotUsernameIsActive()
  : bot_user_id_()
  , username_()
  , is_active_()
{}

toggleBotUsernameIsActive::toggleBotUsernameIsActive(int53 bot_user_id_, string const &username_, bool is_active_)
  : bot_user_id_(bot_user_id_)
  , username_(username_)
  , is_active_(is_active_)
{}

const std::int32_t toggleBotUsernameIsActive::ID;

void toggleBotUsernameIsActive::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleBotUsernameIsActive");
    s.store_field("bot_user_id", bot_user_id_);
    s.store_field("username", username_);
    s.store_field("is_active", is_active_);
    s.store_class_end();
  }
}

toggleChatFolderTags::toggleChatFolderTags()
  : are_tags_enabled_()
{}

toggleChatFolderTags::toggleChatFolderTags(bool are_tags_enabled_)
  : are_tags_enabled_(are_tags_enabled_)
{}

const std::int32_t toggleChatFolderTags::ID;

void toggleChatFolderTags::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleChatFolderTags");
    s.store_field("are_tags_enabled", are_tags_enabled_);
    s.store_class_end();
  }
}

toggleChatIsTranslatable::toggleChatIsTranslatable()
  : chat_id_()
  , is_translatable_()
{}

toggleChatIsTranslatable::toggleChatIsTranslatable(int53 chat_id_, bool is_translatable_)
  : chat_id_(chat_id_)
  , is_translatable_(is_translatable_)
{}

const std::int32_t toggleChatIsTranslatable::ID;

void toggleChatIsTranslatable::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleChatIsTranslatable");
    s.store_field("chat_id", chat_id_);
    s.store_field("is_translatable", is_translatable_);
    s.store_class_end();
  }
}

toggleSessionCanAcceptCalls::toggleSessionCanAcceptCalls()
  : session_id_()
  , can_accept_calls_()
{}

toggleSessionCanAcceptCalls::toggleSessionCanAcceptCalls(int64 session_id_, bool can_accept_calls_)
  : session_id_(session_id_)
  , can_accept_calls_(can_accept_calls_)
{}

const std::int32_t toggleSessionCanAcceptCalls::ID;

void toggleSessionCanAcceptCalls::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleSessionCanAcceptCalls");
    s.store_field("session_id", session_id_);
    s.store_field("can_accept_calls", can_accept_calls_);
    s.store_class_end();
  }
}

toggleSessionCanAcceptSecretChats::toggleSessionCanAcceptSecretChats()
  : session_id_()
  , can_accept_secret_chats_()
{}

toggleSessionCanAcceptSecretChats::toggleSessionCanAcceptSecretChats(int64 session_id_, bool can_accept_secret_chats_)
  : session_id_(session_id_)
  , can_accept_secret_chats_(can_accept_secret_chats_)
{}

const std::int32_t toggleSessionCanAcceptSecretChats::ID;

void toggleSessionCanAcceptSecretChats::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleSessionCanAcceptSecretChats");
    s.store_field("session_id", session_id_);
    s.store_field("can_accept_secret_chats", can_accept_secret_chats_);
    s.store_class_end();
  }
}

toggleSupergroupIsForum::toggleSupergroupIsForum()
  : supergroup_id_()
  , is_forum_()
  , has_forum_tabs_()
{}

toggleSupergroupIsForum::toggleSupergroupIsForum(int53 supergroup_id_, bool is_forum_, bool has_forum_tabs_)
  : supergroup_id_(supergroup_id_)
  , is_forum_(is_forum_)
  , has_forum_tabs_(has_forum_tabs_)
{}

const std::int32_t toggleSupergroupIsForum::ID;

void toggleSupergroupIsForum::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "toggleSupergroupIsForum");
    s.store_field("supergroup_id", supergroup_id_);
    s.store_field("is_forum", is_forum_);
    s.store_field("has_forum_tabs", has_forum_tabs_);
    s.store_class_end();
  }
}

translateText::translateText()
  : text_()
  , to_language_code_()
{}

translateText::translateText(object_ptr<formattedText> &&text_, string const &to_language_code_)
  : text_(std::move(text_))
  , to_language_code_(to_language_code_)
{}

const std::int32_t translateText::ID;

void translateText::store(TlStorerToString &s, const char *field_name) const {
  if (!LOG_IS_STRIPPED(ERROR)) {
    s.store_class_begin(field_name, "translateText");
    s.store_object_field("text", static_cast<const BaseObject *>(text_.get()));
    s.store_field("to_language_code", to_language_code_);
    s.store_class_end();
  }
}
}  // namespace td_api
}  // namespace td
