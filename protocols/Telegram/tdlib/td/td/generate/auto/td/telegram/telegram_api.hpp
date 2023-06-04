#pragma once

/**
 * \file
 * Contains downcast_call methods for calling a function object on downcasted to
 * the most derived class TDLib API object.
 */
#include "telegram_api.h"

namespace td {
namespace telegram_api {

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(Object &obj, const T &func) {
  switch (obj.get_id()) {
    case accessPointRule::ID:
      func(static_cast<accessPointRule &>(obj));
      return true;
    case accountDaysTTL::ID:
      func(static_cast<accountDaysTTL &>(obj));
      return true;
    case appWebViewResultUrl::ID:
      func(static_cast<appWebViewResultUrl &>(obj));
      return true;
    case attachMenuBot::ID:
      func(static_cast<attachMenuBot &>(obj));
      return true;
    case attachMenuBotIcon::ID:
      func(static_cast<attachMenuBotIcon &>(obj));
      return true;
    case attachMenuBotIconColor::ID:
      func(static_cast<attachMenuBotIconColor &>(obj));
      return true;
    case attachMenuBotsNotModified::ID:
      func(static_cast<attachMenuBotsNotModified &>(obj));
      return true;
    case attachMenuBots::ID:
      func(static_cast<attachMenuBots &>(obj));
      return true;
    case attachMenuBotsBot::ID:
      func(static_cast<attachMenuBotsBot &>(obj));
      return true;
    case attachMenuPeerTypeSameBotPM::ID:
      func(static_cast<attachMenuPeerTypeSameBotPM &>(obj));
      return true;
    case attachMenuPeerTypeBotPM::ID:
      func(static_cast<attachMenuPeerTypeBotPM &>(obj));
      return true;
    case attachMenuPeerTypePM::ID:
      func(static_cast<attachMenuPeerTypePM &>(obj));
      return true;
    case attachMenuPeerTypeChat::ID:
      func(static_cast<attachMenuPeerTypeChat &>(obj));
      return true;
    case attachMenuPeerTypeBroadcast::ID:
      func(static_cast<attachMenuPeerTypeBroadcast &>(obj));
      return true;
    case authorization::ID:
      func(static_cast<authorization &>(obj));
      return true;
    case autoDownloadSettings::ID:
      func(static_cast<autoDownloadSettings &>(obj));
      return true;
    case autoSaveException::ID:
      func(static_cast<autoSaveException &>(obj));
      return true;
    case autoSaveSettings::ID:
      func(static_cast<autoSaveSettings &>(obj));
      return true;
    case availableReaction::ID:
      func(static_cast<availableReaction &>(obj));
      return true;
    case bankCardOpenUrl::ID:
      func(static_cast<bankCardOpenUrl &>(obj));
      return true;
    case baseThemeClassic::ID:
      func(static_cast<baseThemeClassic &>(obj));
      return true;
    case baseThemeDay::ID:
      func(static_cast<baseThemeDay &>(obj));
      return true;
    case baseThemeNight::ID:
      func(static_cast<baseThemeNight &>(obj));
      return true;
    case baseThemeTinted::ID:
      func(static_cast<baseThemeTinted &>(obj));
      return true;
    case baseThemeArctic::ID:
      func(static_cast<baseThemeArctic &>(obj));
      return true;
    case botAppNotModified::ID:
      func(static_cast<botAppNotModified &>(obj));
      return true;
    case botApp::ID:
      func(static_cast<botApp &>(obj));
      return true;
    case botCommand::ID:
      func(static_cast<botCommand &>(obj));
      return true;
    case botCommandScopeDefault::ID:
      func(static_cast<botCommandScopeDefault &>(obj));
      return true;
    case botCommandScopeUsers::ID:
      func(static_cast<botCommandScopeUsers &>(obj));
      return true;
    case botCommandScopeChats::ID:
      func(static_cast<botCommandScopeChats &>(obj));
      return true;
    case botCommandScopeChatAdmins::ID:
      func(static_cast<botCommandScopeChatAdmins &>(obj));
      return true;
    case botCommandScopePeer::ID:
      func(static_cast<botCommandScopePeer &>(obj));
      return true;
    case botCommandScopePeerAdmins::ID:
      func(static_cast<botCommandScopePeerAdmins &>(obj));
      return true;
    case botCommandScopePeerUser::ID:
      func(static_cast<botCommandScopePeerUser &>(obj));
      return true;
    case botInfo::ID:
      func(static_cast<botInfo &>(obj));
      return true;
    case botInlineMessageMediaAuto::ID:
      func(static_cast<botInlineMessageMediaAuto &>(obj));
      return true;
    case botInlineMessageText::ID:
      func(static_cast<botInlineMessageText &>(obj));
      return true;
    case botInlineMessageMediaGeo::ID:
      func(static_cast<botInlineMessageMediaGeo &>(obj));
      return true;
    case botInlineMessageMediaVenue::ID:
      func(static_cast<botInlineMessageMediaVenue &>(obj));
      return true;
    case botInlineMessageMediaContact::ID:
      func(static_cast<botInlineMessageMediaContact &>(obj));
      return true;
    case botInlineMessageMediaInvoice::ID:
      func(static_cast<botInlineMessageMediaInvoice &>(obj));
      return true;
    case botInlineResult::ID:
      func(static_cast<botInlineResult &>(obj));
      return true;
    case botInlineMediaResult::ID:
      func(static_cast<botInlineMediaResult &>(obj));
      return true;
    case botMenuButtonDefault::ID:
      func(static_cast<botMenuButtonDefault &>(obj));
      return true;
    case botMenuButtonCommands::ID:
      func(static_cast<botMenuButtonCommands &>(obj));
      return true;
    case botMenuButton::ID:
      func(static_cast<botMenuButton &>(obj));
      return true;
    case cdnConfig::ID:
      func(static_cast<cdnConfig &>(obj));
      return true;
    case cdnPublicKey::ID:
      func(static_cast<cdnPublicKey &>(obj));
      return true;
    case channelAdminLogEvent::ID:
      func(static_cast<channelAdminLogEvent &>(obj));
      return true;
    case channelAdminLogEventActionChangeTitle::ID:
      func(static_cast<channelAdminLogEventActionChangeTitle &>(obj));
      return true;
    case channelAdminLogEventActionChangeAbout::ID:
      func(static_cast<channelAdminLogEventActionChangeAbout &>(obj));
      return true;
    case channelAdminLogEventActionChangeUsername::ID:
      func(static_cast<channelAdminLogEventActionChangeUsername &>(obj));
      return true;
    case channelAdminLogEventActionChangePhoto::ID:
      func(static_cast<channelAdminLogEventActionChangePhoto &>(obj));
      return true;
    case channelAdminLogEventActionToggleInvites::ID:
      func(static_cast<channelAdminLogEventActionToggleInvites &>(obj));
      return true;
    case channelAdminLogEventActionToggleSignatures::ID:
      func(static_cast<channelAdminLogEventActionToggleSignatures &>(obj));
      return true;
    case channelAdminLogEventActionUpdatePinned::ID:
      func(static_cast<channelAdminLogEventActionUpdatePinned &>(obj));
      return true;
    case channelAdminLogEventActionEditMessage::ID:
      func(static_cast<channelAdminLogEventActionEditMessage &>(obj));
      return true;
    case channelAdminLogEventActionDeleteMessage::ID:
      func(static_cast<channelAdminLogEventActionDeleteMessage &>(obj));
      return true;
    case channelAdminLogEventActionParticipantJoin::ID:
      func(static_cast<channelAdminLogEventActionParticipantJoin &>(obj));
      return true;
    case channelAdminLogEventActionParticipantLeave::ID:
      func(static_cast<channelAdminLogEventActionParticipantLeave &>(obj));
      return true;
    case channelAdminLogEventActionParticipantInvite::ID:
      func(static_cast<channelAdminLogEventActionParticipantInvite &>(obj));
      return true;
    case channelAdminLogEventActionParticipantToggleBan::ID:
      func(static_cast<channelAdminLogEventActionParticipantToggleBan &>(obj));
      return true;
    case channelAdminLogEventActionParticipantToggleAdmin::ID:
      func(static_cast<channelAdminLogEventActionParticipantToggleAdmin &>(obj));
      return true;
    case channelAdminLogEventActionChangeStickerSet::ID:
      func(static_cast<channelAdminLogEventActionChangeStickerSet &>(obj));
      return true;
    case channelAdminLogEventActionTogglePreHistoryHidden::ID:
      func(static_cast<channelAdminLogEventActionTogglePreHistoryHidden &>(obj));
      return true;
    case channelAdminLogEventActionDefaultBannedRights::ID:
      func(static_cast<channelAdminLogEventActionDefaultBannedRights &>(obj));
      return true;
    case channelAdminLogEventActionStopPoll::ID:
      func(static_cast<channelAdminLogEventActionStopPoll &>(obj));
      return true;
    case channelAdminLogEventActionChangeLinkedChat::ID:
      func(static_cast<channelAdminLogEventActionChangeLinkedChat &>(obj));
      return true;
    case channelAdminLogEventActionChangeLocation::ID:
      func(static_cast<channelAdminLogEventActionChangeLocation &>(obj));
      return true;
    case channelAdminLogEventActionToggleSlowMode::ID:
      func(static_cast<channelAdminLogEventActionToggleSlowMode &>(obj));
      return true;
    case channelAdminLogEventActionStartGroupCall::ID:
      func(static_cast<channelAdminLogEventActionStartGroupCall &>(obj));
      return true;
    case channelAdminLogEventActionDiscardGroupCall::ID:
      func(static_cast<channelAdminLogEventActionDiscardGroupCall &>(obj));
      return true;
    case channelAdminLogEventActionParticipantMute::ID:
      func(static_cast<channelAdminLogEventActionParticipantMute &>(obj));
      return true;
    case channelAdminLogEventActionParticipantUnmute::ID:
      func(static_cast<channelAdminLogEventActionParticipantUnmute &>(obj));
      return true;
    case channelAdminLogEventActionToggleGroupCallSetting::ID:
      func(static_cast<channelAdminLogEventActionToggleGroupCallSetting &>(obj));
      return true;
    case channelAdminLogEventActionParticipantJoinByInvite::ID:
      func(static_cast<channelAdminLogEventActionParticipantJoinByInvite &>(obj));
      return true;
    case channelAdminLogEventActionExportedInviteDelete::ID:
      func(static_cast<channelAdminLogEventActionExportedInviteDelete &>(obj));
      return true;
    case channelAdminLogEventActionExportedInviteRevoke::ID:
      func(static_cast<channelAdminLogEventActionExportedInviteRevoke &>(obj));
      return true;
    case channelAdminLogEventActionExportedInviteEdit::ID:
      func(static_cast<channelAdminLogEventActionExportedInviteEdit &>(obj));
      return true;
    case channelAdminLogEventActionParticipantVolume::ID:
      func(static_cast<channelAdminLogEventActionParticipantVolume &>(obj));
      return true;
    case channelAdminLogEventActionChangeHistoryTTL::ID:
      func(static_cast<channelAdminLogEventActionChangeHistoryTTL &>(obj));
      return true;
    case channelAdminLogEventActionParticipantJoinByRequest::ID:
      func(static_cast<channelAdminLogEventActionParticipantJoinByRequest &>(obj));
      return true;
    case channelAdminLogEventActionToggleNoForwards::ID:
      func(static_cast<channelAdminLogEventActionToggleNoForwards &>(obj));
      return true;
    case channelAdminLogEventActionSendMessage::ID:
      func(static_cast<channelAdminLogEventActionSendMessage &>(obj));
      return true;
    case channelAdminLogEventActionChangeAvailableReactions::ID:
      func(static_cast<channelAdminLogEventActionChangeAvailableReactions &>(obj));
      return true;
    case channelAdminLogEventActionChangeUsernames::ID:
      func(static_cast<channelAdminLogEventActionChangeUsernames &>(obj));
      return true;
    case channelAdminLogEventActionToggleForum::ID:
      func(static_cast<channelAdminLogEventActionToggleForum &>(obj));
      return true;
    case channelAdminLogEventActionCreateTopic::ID:
      func(static_cast<channelAdminLogEventActionCreateTopic &>(obj));
      return true;
    case channelAdminLogEventActionEditTopic::ID:
      func(static_cast<channelAdminLogEventActionEditTopic &>(obj));
      return true;
    case channelAdminLogEventActionDeleteTopic::ID:
      func(static_cast<channelAdminLogEventActionDeleteTopic &>(obj));
      return true;
    case channelAdminLogEventActionPinTopic::ID:
      func(static_cast<channelAdminLogEventActionPinTopic &>(obj));
      return true;
    case channelAdminLogEventActionToggleAntiSpam::ID:
      func(static_cast<channelAdminLogEventActionToggleAntiSpam &>(obj));
      return true;
    case channelAdminLogEventsFilter::ID:
      func(static_cast<channelAdminLogEventsFilter &>(obj));
      return true;
    case channelLocationEmpty::ID:
      func(static_cast<channelLocationEmpty &>(obj));
      return true;
    case channelLocation::ID:
      func(static_cast<channelLocation &>(obj));
      return true;
    case channelMessagesFilterEmpty::ID:
      func(static_cast<channelMessagesFilterEmpty &>(obj));
      return true;
    case channelMessagesFilter::ID:
      func(static_cast<channelMessagesFilter &>(obj));
      return true;
    case channelParticipant::ID:
      func(static_cast<channelParticipant &>(obj));
      return true;
    case channelParticipantSelf::ID:
      func(static_cast<channelParticipantSelf &>(obj));
      return true;
    case channelParticipantCreator::ID:
      func(static_cast<channelParticipantCreator &>(obj));
      return true;
    case channelParticipantAdmin::ID:
      func(static_cast<channelParticipantAdmin &>(obj));
      return true;
    case channelParticipantBanned::ID:
      func(static_cast<channelParticipantBanned &>(obj));
      return true;
    case channelParticipantLeft::ID:
      func(static_cast<channelParticipantLeft &>(obj));
      return true;
    case channelParticipantsRecent::ID:
      func(static_cast<channelParticipantsRecent &>(obj));
      return true;
    case channelParticipantsAdmins::ID:
      func(static_cast<channelParticipantsAdmins &>(obj));
      return true;
    case channelParticipantsKicked::ID:
      func(static_cast<channelParticipantsKicked &>(obj));
      return true;
    case channelParticipantsBots::ID:
      func(static_cast<channelParticipantsBots &>(obj));
      return true;
    case channelParticipantsBanned::ID:
      func(static_cast<channelParticipantsBanned &>(obj));
      return true;
    case channelParticipantsSearch::ID:
      func(static_cast<channelParticipantsSearch &>(obj));
      return true;
    case channelParticipantsContacts::ID:
      func(static_cast<channelParticipantsContacts &>(obj));
      return true;
    case channelParticipantsMentions::ID:
      func(static_cast<channelParticipantsMentions &>(obj));
      return true;
    case chatEmpty::ID:
      func(static_cast<chatEmpty &>(obj));
      return true;
    case chat::ID:
      func(static_cast<chat &>(obj));
      return true;
    case chatForbidden::ID:
      func(static_cast<chatForbidden &>(obj));
      return true;
    case channel::ID:
      func(static_cast<channel &>(obj));
      return true;
    case channelForbidden::ID:
      func(static_cast<channelForbidden &>(obj));
      return true;
    case chatAdminRights::ID:
      func(static_cast<chatAdminRights &>(obj));
      return true;
    case chatAdminWithInvites::ID:
      func(static_cast<chatAdminWithInvites &>(obj));
      return true;
    case chatBannedRights::ID:
      func(static_cast<chatBannedRights &>(obj));
      return true;
    case chatFull::ID:
      func(static_cast<chatFull &>(obj));
      return true;
    case channelFull::ID:
      func(static_cast<channelFull &>(obj));
      return true;
    case chatInviteAlready::ID:
      func(static_cast<chatInviteAlready &>(obj));
      return true;
    case chatInvite::ID:
      func(static_cast<chatInvite &>(obj));
      return true;
    case chatInvitePeek::ID:
      func(static_cast<chatInvitePeek &>(obj));
      return true;
    case chatInviteImporter::ID:
      func(static_cast<chatInviteImporter &>(obj));
      return true;
    case chatOnlines::ID:
      func(static_cast<chatOnlines &>(obj));
      return true;
    case chatParticipant::ID:
      func(static_cast<chatParticipant &>(obj));
      return true;
    case chatParticipantCreator::ID:
      func(static_cast<chatParticipantCreator &>(obj));
      return true;
    case chatParticipantAdmin::ID:
      func(static_cast<chatParticipantAdmin &>(obj));
      return true;
    case chatParticipantsForbidden::ID:
      func(static_cast<chatParticipantsForbidden &>(obj));
      return true;
    case chatParticipants::ID:
      func(static_cast<chatParticipants &>(obj));
      return true;
    case chatPhotoEmpty::ID:
      func(static_cast<chatPhotoEmpty &>(obj));
      return true;
    case chatPhoto::ID:
      func(static_cast<chatPhoto &>(obj));
      return true;
    case chatReactionsNone::ID:
      func(static_cast<chatReactionsNone &>(obj));
      return true;
    case chatReactionsAll::ID:
      func(static_cast<chatReactionsAll &>(obj));
      return true;
    case chatReactionsSome::ID:
      func(static_cast<chatReactionsSome &>(obj));
      return true;
    case codeSettings::ID:
      func(static_cast<codeSettings &>(obj));
      return true;
    case config::ID:
      func(static_cast<config &>(obj));
      return true;
    case contact::ID:
      func(static_cast<contact &>(obj));
      return true;
    case contactStatus::ID:
      func(static_cast<contactStatus &>(obj));
      return true;
    case dataJSON::ID:
      func(static_cast<dataJSON &>(obj));
      return true;
    case dcOption::ID:
      func(static_cast<dcOption &>(obj));
      return true;
    case defaultHistoryTTL::ID:
      func(static_cast<defaultHistoryTTL &>(obj));
      return true;
    case dialog::ID:
      func(static_cast<dialog &>(obj));
      return true;
    case dialogFolder::ID:
      func(static_cast<dialogFolder &>(obj));
      return true;
    case dialogFilter::ID:
      func(static_cast<dialogFilter &>(obj));
      return true;
    case dialogFilterDefault::ID:
      func(static_cast<dialogFilterDefault &>(obj));
      return true;
    case dialogFilterChatlist::ID:
      func(static_cast<dialogFilterChatlist &>(obj));
      return true;
    case dialogFilterSuggested::ID:
      func(static_cast<dialogFilterSuggested &>(obj));
      return true;
    case dialogPeer::ID:
      func(static_cast<dialogPeer &>(obj));
      return true;
    case dialogPeerFolder::ID:
      func(static_cast<dialogPeerFolder &>(obj));
      return true;
    case documentEmpty::ID:
      func(static_cast<documentEmpty &>(obj));
      return true;
    case document::ID:
      func(static_cast<document &>(obj));
      return true;
    case documentAttributeImageSize::ID:
      func(static_cast<documentAttributeImageSize &>(obj));
      return true;
    case documentAttributeAnimated::ID:
      func(static_cast<documentAttributeAnimated &>(obj));
      return true;
    case documentAttributeSticker::ID:
      func(static_cast<documentAttributeSticker &>(obj));
      return true;
    case documentAttributeVideo::ID:
      func(static_cast<documentAttributeVideo &>(obj));
      return true;
    case documentAttributeAudio::ID:
      func(static_cast<documentAttributeAudio &>(obj));
      return true;
    case documentAttributeFilename::ID:
      func(static_cast<documentAttributeFilename &>(obj));
      return true;
    case documentAttributeHasStickers::ID:
      func(static_cast<documentAttributeHasStickers &>(obj));
      return true;
    case documentAttributeCustomEmoji::ID:
      func(static_cast<documentAttributeCustomEmoji &>(obj));
      return true;
    case draftMessageEmpty::ID:
      func(static_cast<draftMessageEmpty &>(obj));
      return true;
    case draftMessage::ID:
      func(static_cast<draftMessage &>(obj));
      return true;
    case emailVerificationCode::ID:
      func(static_cast<emailVerificationCode &>(obj));
      return true;
    case emailVerificationGoogle::ID:
      func(static_cast<emailVerificationGoogle &>(obj));
      return true;
    case emailVerificationApple::ID:
      func(static_cast<emailVerificationApple &>(obj));
      return true;
    case emailVerifyPurposeLoginSetup::ID:
      func(static_cast<emailVerifyPurposeLoginSetup &>(obj));
      return true;
    case emailVerifyPurposeLoginChange::ID:
      func(static_cast<emailVerifyPurposeLoginChange &>(obj));
      return true;
    case emailVerifyPurposePassport::ID:
      func(static_cast<emailVerifyPurposePassport &>(obj));
      return true;
    case emojiGroup::ID:
      func(static_cast<emojiGroup &>(obj));
      return true;
    case emojiKeyword::ID:
      func(static_cast<emojiKeyword &>(obj));
      return true;
    case emojiKeywordDeleted::ID:
      func(static_cast<emojiKeywordDeleted &>(obj));
      return true;
    case emojiKeywordsDifference::ID:
      func(static_cast<emojiKeywordsDifference &>(obj));
      return true;
    case emojiLanguage::ID:
      func(static_cast<emojiLanguage &>(obj));
      return true;
    case emojiListNotModified::ID:
      func(static_cast<emojiListNotModified &>(obj));
      return true;
    case emojiList::ID:
      func(static_cast<emojiList &>(obj));
      return true;
    case emojiStatusEmpty::ID:
      func(static_cast<emojiStatusEmpty &>(obj));
      return true;
    case emojiStatus::ID:
      func(static_cast<emojiStatus &>(obj));
      return true;
    case emojiStatusUntil::ID:
      func(static_cast<emojiStatusUntil &>(obj));
      return true;
    case emojiURL::ID:
      func(static_cast<emojiURL &>(obj));
      return true;
    case encryptedChatEmpty::ID:
      func(static_cast<encryptedChatEmpty &>(obj));
      return true;
    case encryptedChatWaiting::ID:
      func(static_cast<encryptedChatWaiting &>(obj));
      return true;
    case encryptedChatRequested::ID:
      func(static_cast<encryptedChatRequested &>(obj));
      return true;
    case encryptedChat::ID:
      func(static_cast<encryptedChat &>(obj));
      return true;
    case encryptedChatDiscarded::ID:
      func(static_cast<encryptedChatDiscarded &>(obj));
      return true;
    case encryptedFileEmpty::ID:
      func(static_cast<encryptedFileEmpty &>(obj));
      return true;
    case encryptedFile::ID:
      func(static_cast<encryptedFile &>(obj));
      return true;
    case encryptedMessage::ID:
      func(static_cast<encryptedMessage &>(obj));
      return true;
    case encryptedMessageService::ID:
      func(static_cast<encryptedMessageService &>(obj));
      return true;
    case error::ID:
      func(static_cast<error &>(obj));
      return true;
    case chatInviteExported::ID:
      func(static_cast<chatInviteExported &>(obj));
      return true;
    case chatInvitePublicJoinRequests::ID:
      func(static_cast<chatInvitePublicJoinRequests &>(obj));
      return true;
    case exportedChatlistInvite::ID:
      func(static_cast<exportedChatlistInvite &>(obj));
      return true;
    case exportedContactToken::ID:
      func(static_cast<exportedContactToken &>(obj));
      return true;
    case exportedMessageLink::ID:
      func(static_cast<exportedMessageLink &>(obj));
      return true;
    case fileHash::ID:
      func(static_cast<fileHash &>(obj));
      return true;
    case folder::ID:
      func(static_cast<folder &>(obj));
      return true;
    case folderPeer::ID:
      func(static_cast<folderPeer &>(obj));
      return true;
    case forumTopicDeleted::ID:
      func(static_cast<forumTopicDeleted &>(obj));
      return true;
    case forumTopic::ID:
      func(static_cast<forumTopic &>(obj));
      return true;
    case game::ID:
      func(static_cast<game &>(obj));
      return true;
    case geoPointEmpty::ID:
      func(static_cast<geoPointEmpty &>(obj));
      return true;
    case geoPoint::ID:
      func(static_cast<geoPoint &>(obj));
      return true;
    case globalPrivacySettings::ID:
      func(static_cast<globalPrivacySettings &>(obj));
      return true;
    case groupCallDiscarded::ID:
      func(static_cast<groupCallDiscarded &>(obj));
      return true;
    case groupCall::ID:
      func(static_cast<groupCall &>(obj));
      return true;
    case groupCallParticipant::ID:
      func(static_cast<groupCallParticipant &>(obj));
      return true;
    case groupCallParticipantVideo::ID:
      func(static_cast<groupCallParticipantVideo &>(obj));
      return true;
    case groupCallParticipantVideoSourceGroup::ID:
      func(static_cast<groupCallParticipantVideoSourceGroup &>(obj));
      return true;
    case groupCallStreamChannel::ID:
      func(static_cast<groupCallStreamChannel &>(obj));
      return true;
    case highScore::ID:
      func(static_cast<highScore &>(obj));
      return true;
    case importedContact::ID:
      func(static_cast<importedContact &>(obj));
      return true;
    case inlineBotSwitchPM::ID:
      func(static_cast<inlineBotSwitchPM &>(obj));
      return true;
    case inlineBotWebView::ID:
      func(static_cast<inlineBotWebView &>(obj));
      return true;
    case inlineQueryPeerTypeSameBotPM::ID:
      func(static_cast<inlineQueryPeerTypeSameBotPM &>(obj));
      return true;
    case inlineQueryPeerTypePM::ID:
      func(static_cast<inlineQueryPeerTypePM &>(obj));
      return true;
    case inlineQueryPeerTypeChat::ID:
      func(static_cast<inlineQueryPeerTypeChat &>(obj));
      return true;
    case inlineQueryPeerTypeMegagroup::ID:
      func(static_cast<inlineQueryPeerTypeMegagroup &>(obj));
      return true;
    case inlineQueryPeerTypeBroadcast::ID:
      func(static_cast<inlineQueryPeerTypeBroadcast &>(obj));
      return true;
    case inlineQueryPeerTypeBotPM::ID:
      func(static_cast<inlineQueryPeerTypeBotPM &>(obj));
      return true;
    case inputAppEvent::ID:
      func(static_cast<inputAppEvent &>(obj));
      return true;
    case inputBotAppID::ID:
      func(static_cast<inputBotAppID &>(obj));
      return true;
    case inputBotAppShortName::ID:
      func(static_cast<inputBotAppShortName &>(obj));
      return true;
    case inputBotInlineMessageMediaAuto::ID:
      func(static_cast<inputBotInlineMessageMediaAuto &>(obj));
      return true;
    case inputBotInlineMessageText::ID:
      func(static_cast<inputBotInlineMessageText &>(obj));
      return true;
    case inputBotInlineMessageMediaGeo::ID:
      func(static_cast<inputBotInlineMessageMediaGeo &>(obj));
      return true;
    case inputBotInlineMessageMediaVenue::ID:
      func(static_cast<inputBotInlineMessageMediaVenue &>(obj));
      return true;
    case inputBotInlineMessageMediaContact::ID:
      func(static_cast<inputBotInlineMessageMediaContact &>(obj));
      return true;
    case inputBotInlineMessageGame::ID:
      func(static_cast<inputBotInlineMessageGame &>(obj));
      return true;
    case inputBotInlineMessageMediaInvoice::ID:
      func(static_cast<inputBotInlineMessageMediaInvoice &>(obj));
      return true;
    case inputBotInlineMessageID::ID:
      func(static_cast<inputBotInlineMessageID &>(obj));
      return true;
    case inputBotInlineMessageID64::ID:
      func(static_cast<inputBotInlineMessageID64 &>(obj));
      return true;
    case inputBotInlineResult::ID:
      func(static_cast<inputBotInlineResult &>(obj));
      return true;
    case inputBotInlineResultPhoto::ID:
      func(static_cast<inputBotInlineResultPhoto &>(obj));
      return true;
    case inputBotInlineResultDocument::ID:
      func(static_cast<inputBotInlineResultDocument &>(obj));
      return true;
    case inputBotInlineResultGame::ID:
      func(static_cast<inputBotInlineResultGame &>(obj));
      return true;
    case inputChannelEmpty::ID:
      func(static_cast<inputChannelEmpty &>(obj));
      return true;
    case inputChannel::ID:
      func(static_cast<inputChannel &>(obj));
      return true;
    case inputChannelFromMessage::ID:
      func(static_cast<inputChannelFromMessage &>(obj));
      return true;
    case inputChatPhotoEmpty::ID:
      func(static_cast<inputChatPhotoEmpty &>(obj));
      return true;
    case inputChatUploadedPhoto::ID:
      func(static_cast<inputChatUploadedPhoto &>(obj));
      return true;
    case inputChatPhoto::ID:
      func(static_cast<inputChatPhoto &>(obj));
      return true;
    case inputChatlistDialogFilter::ID:
      func(static_cast<inputChatlistDialogFilter &>(obj));
      return true;
    case inputCheckPasswordEmpty::ID:
      func(static_cast<inputCheckPasswordEmpty &>(obj));
      return true;
    case inputCheckPasswordSRP::ID:
      func(static_cast<inputCheckPasswordSRP &>(obj));
      return true;
    case inputClientProxy::ID:
      func(static_cast<inputClientProxy &>(obj));
      return true;
    case inputPhoneContact::ID:
      func(static_cast<inputPhoneContact &>(obj));
      return true;
    case inputDialogPeer::ID:
      func(static_cast<inputDialogPeer &>(obj));
      return true;
    case inputDialogPeerFolder::ID:
      func(static_cast<inputDialogPeerFolder &>(obj));
      return true;
    case inputDocumentEmpty::ID:
      func(static_cast<inputDocumentEmpty &>(obj));
      return true;
    case inputDocument::ID:
      func(static_cast<inputDocument &>(obj));
      return true;
    case inputEncryptedChat::ID:
      func(static_cast<inputEncryptedChat &>(obj));
      return true;
    case inputEncryptedFileEmpty::ID:
      func(static_cast<inputEncryptedFileEmpty &>(obj));
      return true;
    case inputEncryptedFileUploaded::ID:
      func(static_cast<inputEncryptedFileUploaded &>(obj));
      return true;
    case inputEncryptedFile::ID:
      func(static_cast<inputEncryptedFile &>(obj));
      return true;
    case inputEncryptedFileBigUploaded::ID:
      func(static_cast<inputEncryptedFileBigUploaded &>(obj));
      return true;
    case inputFile::ID:
      func(static_cast<inputFile &>(obj));
      return true;
    case inputFileBig::ID:
      func(static_cast<inputFileBig &>(obj));
      return true;
    case inputPeerPhotoFileLocationLegacy::ID:
      func(static_cast<inputPeerPhotoFileLocationLegacy &>(obj));
      return true;
    case inputStickerSetThumbLegacy::ID:
      func(static_cast<inputStickerSetThumbLegacy &>(obj));
      return true;
    case inputFileLocation::ID:
      func(static_cast<inputFileLocation &>(obj));
      return true;
    case inputEncryptedFileLocation::ID:
      func(static_cast<inputEncryptedFileLocation &>(obj));
      return true;
    case inputDocumentFileLocation::ID:
      func(static_cast<inputDocumentFileLocation &>(obj));
      return true;
    case inputSecureFileLocation::ID:
      func(static_cast<inputSecureFileLocation &>(obj));
      return true;
    case inputTakeoutFileLocation::ID:
      func(static_cast<inputTakeoutFileLocation &>(obj));
      return true;
    case inputPhotoFileLocation::ID:
      func(static_cast<inputPhotoFileLocation &>(obj));
      return true;
    case inputPhotoLegacyFileLocation::ID:
      func(static_cast<inputPhotoLegacyFileLocation &>(obj));
      return true;
    case inputPeerPhotoFileLocation::ID:
      func(static_cast<inputPeerPhotoFileLocation &>(obj));
      return true;
    case inputStickerSetThumb::ID:
      func(static_cast<inputStickerSetThumb &>(obj));
      return true;
    case inputGroupCallStream::ID:
      func(static_cast<inputGroupCallStream &>(obj));
      return true;
    case inputFolderPeer::ID:
      func(static_cast<inputFolderPeer &>(obj));
      return true;
    case inputGameID::ID:
      func(static_cast<inputGameID &>(obj));
      return true;
    case inputGameShortName::ID:
      func(static_cast<inputGameShortName &>(obj));
      return true;
    case inputGeoPointEmpty::ID:
      func(static_cast<inputGeoPointEmpty &>(obj));
      return true;
    case inputGeoPoint::ID:
      func(static_cast<inputGeoPoint &>(obj));
      return true;
    case inputGroupCall::ID:
      func(static_cast<inputGroupCall &>(obj));
      return true;
    case inputInvoiceMessage::ID:
      func(static_cast<inputInvoiceMessage &>(obj));
      return true;
    case inputInvoiceSlug::ID:
      func(static_cast<inputInvoiceSlug &>(obj));
      return true;
    case inputMediaEmpty::ID:
      func(static_cast<inputMediaEmpty &>(obj));
      return true;
    case inputMediaUploadedPhoto::ID:
      func(static_cast<inputMediaUploadedPhoto &>(obj));
      return true;
    case inputMediaPhoto::ID:
      func(static_cast<inputMediaPhoto &>(obj));
      return true;
    case inputMediaGeoPoint::ID:
      func(static_cast<inputMediaGeoPoint &>(obj));
      return true;
    case inputMediaContact::ID:
      func(static_cast<inputMediaContact &>(obj));
      return true;
    case inputMediaUploadedDocument::ID:
      func(static_cast<inputMediaUploadedDocument &>(obj));
      return true;
    case inputMediaDocument::ID:
      func(static_cast<inputMediaDocument &>(obj));
      return true;
    case inputMediaVenue::ID:
      func(static_cast<inputMediaVenue &>(obj));
      return true;
    case inputMediaPhotoExternal::ID:
      func(static_cast<inputMediaPhotoExternal &>(obj));
      return true;
    case inputMediaDocumentExternal::ID:
      func(static_cast<inputMediaDocumentExternal &>(obj));
      return true;
    case inputMediaGame::ID:
      func(static_cast<inputMediaGame &>(obj));
      return true;
    case inputMediaInvoice::ID:
      func(static_cast<inputMediaInvoice &>(obj));
      return true;
    case inputMediaGeoLive::ID:
      func(static_cast<inputMediaGeoLive &>(obj));
      return true;
    case inputMediaPoll::ID:
      func(static_cast<inputMediaPoll &>(obj));
      return true;
    case inputMediaDice::ID:
      func(static_cast<inputMediaDice &>(obj));
      return true;
    case inputMessageID::ID:
      func(static_cast<inputMessageID &>(obj));
      return true;
    case inputMessageReplyTo::ID:
      func(static_cast<inputMessageReplyTo &>(obj));
      return true;
    case inputMessagePinned::ID:
      func(static_cast<inputMessagePinned &>(obj));
      return true;
    case inputMessageCallbackQuery::ID:
      func(static_cast<inputMessageCallbackQuery &>(obj));
      return true;
    case inputNotifyPeer::ID:
      func(static_cast<inputNotifyPeer &>(obj));
      return true;
    case inputNotifyUsers::ID:
      func(static_cast<inputNotifyUsers &>(obj));
      return true;
    case inputNotifyChats::ID:
      func(static_cast<inputNotifyChats &>(obj));
      return true;
    case inputNotifyBroadcasts::ID:
      func(static_cast<inputNotifyBroadcasts &>(obj));
      return true;
    case inputNotifyForumTopic::ID:
      func(static_cast<inputNotifyForumTopic &>(obj));
      return true;
    case inputPaymentCredentialsSaved::ID:
      func(static_cast<inputPaymentCredentialsSaved &>(obj));
      return true;
    case inputPaymentCredentials::ID:
      func(static_cast<inputPaymentCredentials &>(obj));
      return true;
    case inputPaymentCredentialsApplePay::ID:
      func(static_cast<inputPaymentCredentialsApplePay &>(obj));
      return true;
    case inputPaymentCredentialsGooglePay::ID:
      func(static_cast<inputPaymentCredentialsGooglePay &>(obj));
      return true;
    case inputPeerEmpty::ID:
      func(static_cast<inputPeerEmpty &>(obj));
      return true;
    case inputPeerSelf::ID:
      func(static_cast<inputPeerSelf &>(obj));
      return true;
    case inputPeerChat::ID:
      func(static_cast<inputPeerChat &>(obj));
      return true;
    case inputPeerUser::ID:
      func(static_cast<inputPeerUser &>(obj));
      return true;
    case inputPeerChannel::ID:
      func(static_cast<inputPeerChannel &>(obj));
      return true;
    case inputPeerUserFromMessage::ID:
      func(static_cast<inputPeerUserFromMessage &>(obj));
      return true;
    case inputPeerChannelFromMessage::ID:
      func(static_cast<inputPeerChannelFromMessage &>(obj));
      return true;
    case inputPeerNotifySettings::ID:
      func(static_cast<inputPeerNotifySettings &>(obj));
      return true;
    case inputPhoneCall::ID:
      func(static_cast<inputPhoneCall &>(obj));
      return true;
    case inputPhotoEmpty::ID:
      func(static_cast<inputPhotoEmpty &>(obj));
      return true;
    case inputPhoto::ID:
      func(static_cast<inputPhoto &>(obj));
      return true;
    case inputPrivacyKeyStatusTimestamp::ID:
      func(static_cast<inputPrivacyKeyStatusTimestamp &>(obj));
      return true;
    case inputPrivacyKeyChatInvite::ID:
      func(static_cast<inputPrivacyKeyChatInvite &>(obj));
      return true;
    case inputPrivacyKeyPhoneCall::ID:
      func(static_cast<inputPrivacyKeyPhoneCall &>(obj));
      return true;
    case inputPrivacyKeyPhoneP2P::ID:
      func(static_cast<inputPrivacyKeyPhoneP2P &>(obj));
      return true;
    case inputPrivacyKeyForwards::ID:
      func(static_cast<inputPrivacyKeyForwards &>(obj));
      return true;
    case inputPrivacyKeyProfilePhoto::ID:
      func(static_cast<inputPrivacyKeyProfilePhoto &>(obj));
      return true;
    case inputPrivacyKeyPhoneNumber::ID:
      func(static_cast<inputPrivacyKeyPhoneNumber &>(obj));
      return true;
    case inputPrivacyKeyAddedByPhone::ID:
      func(static_cast<inputPrivacyKeyAddedByPhone &>(obj));
      return true;
    case inputPrivacyKeyVoiceMessages::ID:
      func(static_cast<inputPrivacyKeyVoiceMessages &>(obj));
      return true;
    case inputPrivacyValueAllowContacts::ID:
      func(static_cast<inputPrivacyValueAllowContacts &>(obj));
      return true;
    case inputPrivacyValueAllowAll::ID:
      func(static_cast<inputPrivacyValueAllowAll &>(obj));
      return true;
    case inputPrivacyValueAllowUsers::ID:
      func(static_cast<inputPrivacyValueAllowUsers &>(obj));
      return true;
    case inputPrivacyValueDisallowContacts::ID:
      func(static_cast<inputPrivacyValueDisallowContacts &>(obj));
      return true;
    case inputPrivacyValueDisallowAll::ID:
      func(static_cast<inputPrivacyValueDisallowAll &>(obj));
      return true;
    case inputPrivacyValueDisallowUsers::ID:
      func(static_cast<inputPrivacyValueDisallowUsers &>(obj));
      return true;
    case inputPrivacyValueAllowChatParticipants::ID:
      func(static_cast<inputPrivacyValueAllowChatParticipants &>(obj));
      return true;
    case inputPrivacyValueDisallowChatParticipants::ID:
      func(static_cast<inputPrivacyValueDisallowChatParticipants &>(obj));
      return true;
    case inputSecureFileUploaded::ID:
      func(static_cast<inputSecureFileUploaded &>(obj));
      return true;
    case inputSecureFile::ID:
      func(static_cast<inputSecureFile &>(obj));
      return true;
    case inputSecureValue::ID:
      func(static_cast<inputSecureValue &>(obj));
      return true;
    case inputSingleMedia::ID:
      func(static_cast<inputSingleMedia &>(obj));
      return true;
    case inputStickerSetEmpty::ID:
      func(static_cast<inputStickerSetEmpty &>(obj));
      return true;
    case inputStickerSetID::ID:
      func(static_cast<inputStickerSetID &>(obj));
      return true;
    case inputStickerSetShortName::ID:
      func(static_cast<inputStickerSetShortName &>(obj));
      return true;
    case inputStickerSetAnimatedEmoji::ID:
      func(static_cast<inputStickerSetAnimatedEmoji &>(obj));
      return true;
    case inputStickerSetDice::ID:
      func(static_cast<inputStickerSetDice &>(obj));
      return true;
    case inputStickerSetAnimatedEmojiAnimations::ID:
      func(static_cast<inputStickerSetAnimatedEmojiAnimations &>(obj));
      return true;
    case inputStickerSetPremiumGifts::ID:
      func(static_cast<inputStickerSetPremiumGifts &>(obj));
      return true;
    case inputStickerSetEmojiGenericAnimations::ID:
      func(static_cast<inputStickerSetEmojiGenericAnimations &>(obj));
      return true;
    case inputStickerSetEmojiDefaultStatuses::ID:
      func(static_cast<inputStickerSetEmojiDefaultStatuses &>(obj));
      return true;
    case inputStickerSetEmojiDefaultTopicIcons::ID:
      func(static_cast<inputStickerSetEmojiDefaultTopicIcons &>(obj));
      return true;
    case inputStickerSetItem::ID:
      func(static_cast<inputStickerSetItem &>(obj));
      return true;
    case inputStickeredMediaPhoto::ID:
      func(static_cast<inputStickeredMediaPhoto &>(obj));
      return true;
    case inputStickeredMediaDocument::ID:
      func(static_cast<inputStickeredMediaDocument &>(obj));
      return true;
    case inputStorePaymentPremiumSubscription::ID:
      func(static_cast<inputStorePaymentPremiumSubscription &>(obj));
      return true;
    case inputStorePaymentGiftPremium::ID:
      func(static_cast<inputStorePaymentGiftPremium &>(obj));
      return true;
    case inputTheme::ID:
      func(static_cast<inputTheme &>(obj));
      return true;
    case inputThemeSlug::ID:
      func(static_cast<inputThemeSlug &>(obj));
      return true;
    case inputThemeSettings::ID:
      func(static_cast<inputThemeSettings &>(obj));
      return true;
    case inputUserEmpty::ID:
      func(static_cast<inputUserEmpty &>(obj));
      return true;
    case inputUserSelf::ID:
      func(static_cast<inputUserSelf &>(obj));
      return true;
    case inputUser::ID:
      func(static_cast<inputUser &>(obj));
      return true;
    case inputUserFromMessage::ID:
      func(static_cast<inputUserFromMessage &>(obj));
      return true;
    case inputWallPaper::ID:
      func(static_cast<inputWallPaper &>(obj));
      return true;
    case inputWallPaperSlug::ID:
      func(static_cast<inputWallPaperSlug &>(obj));
      return true;
    case inputWallPaperNoFile::ID:
      func(static_cast<inputWallPaperNoFile &>(obj));
      return true;
    case inputWebDocument::ID:
      func(static_cast<inputWebDocument &>(obj));
      return true;
    case inputWebFileLocation::ID:
      func(static_cast<inputWebFileLocation &>(obj));
      return true;
    case inputWebFileGeoPointLocation::ID:
      func(static_cast<inputWebFileGeoPointLocation &>(obj));
      return true;
    case inputWebFileAudioAlbumThumbLocation::ID:
      func(static_cast<inputWebFileAudioAlbumThumbLocation &>(obj));
      return true;
    case invoice::ID:
      func(static_cast<invoice &>(obj));
      return true;
    case ipPort::ID:
      func(static_cast<ipPort &>(obj));
      return true;
    case ipPortSecret::ID:
      func(static_cast<ipPortSecret &>(obj));
      return true;
    case jsonObjectValue::ID:
      func(static_cast<jsonObjectValue &>(obj));
      return true;
    case jsonNull::ID:
      func(static_cast<jsonNull &>(obj));
      return true;
    case jsonBool::ID:
      func(static_cast<jsonBool &>(obj));
      return true;
    case jsonNumber::ID:
      func(static_cast<jsonNumber &>(obj));
      return true;
    case jsonString::ID:
      func(static_cast<jsonString &>(obj));
      return true;
    case jsonArray::ID:
      func(static_cast<jsonArray &>(obj));
      return true;
    case jsonObject::ID:
      func(static_cast<jsonObject &>(obj));
      return true;
    case keyboardButton::ID:
      func(static_cast<keyboardButton &>(obj));
      return true;
    case keyboardButtonUrl::ID:
      func(static_cast<keyboardButtonUrl &>(obj));
      return true;
    case keyboardButtonCallback::ID:
      func(static_cast<keyboardButtonCallback &>(obj));
      return true;
    case keyboardButtonRequestPhone::ID:
      func(static_cast<keyboardButtonRequestPhone &>(obj));
      return true;
    case keyboardButtonRequestGeoLocation::ID:
      func(static_cast<keyboardButtonRequestGeoLocation &>(obj));
      return true;
    case keyboardButtonSwitchInline::ID:
      func(static_cast<keyboardButtonSwitchInline &>(obj));
      return true;
    case keyboardButtonGame::ID:
      func(static_cast<keyboardButtonGame &>(obj));
      return true;
    case keyboardButtonBuy::ID:
      func(static_cast<keyboardButtonBuy &>(obj));
      return true;
    case keyboardButtonUrlAuth::ID:
      func(static_cast<keyboardButtonUrlAuth &>(obj));
      return true;
    case inputKeyboardButtonUrlAuth::ID:
      func(static_cast<inputKeyboardButtonUrlAuth &>(obj));
      return true;
    case keyboardButtonRequestPoll::ID:
      func(static_cast<keyboardButtonRequestPoll &>(obj));
      return true;
    case inputKeyboardButtonUserProfile::ID:
      func(static_cast<inputKeyboardButtonUserProfile &>(obj));
      return true;
    case keyboardButtonUserProfile::ID:
      func(static_cast<keyboardButtonUserProfile &>(obj));
      return true;
    case keyboardButtonWebView::ID:
      func(static_cast<keyboardButtonWebView &>(obj));
      return true;
    case keyboardButtonSimpleWebView::ID:
      func(static_cast<keyboardButtonSimpleWebView &>(obj));
      return true;
    case keyboardButtonRequestPeer::ID:
      func(static_cast<keyboardButtonRequestPeer &>(obj));
      return true;
    case keyboardButtonRow::ID:
      func(static_cast<keyboardButtonRow &>(obj));
      return true;
    case labeledPrice::ID:
      func(static_cast<labeledPrice &>(obj));
      return true;
    case langPackDifference::ID:
      func(static_cast<langPackDifference &>(obj));
      return true;
    case langPackLanguage::ID:
      func(static_cast<langPackLanguage &>(obj));
      return true;
    case langPackString::ID:
      func(static_cast<langPackString &>(obj));
      return true;
    case langPackStringPluralized::ID:
      func(static_cast<langPackStringPluralized &>(obj));
      return true;
    case langPackStringDeleted::ID:
      func(static_cast<langPackStringDeleted &>(obj));
      return true;
    case maskCoords::ID:
      func(static_cast<maskCoords &>(obj));
      return true;
    case messageEmpty::ID:
      func(static_cast<messageEmpty &>(obj));
      return true;
    case message::ID:
      func(static_cast<message &>(obj));
      return true;
    case messageService::ID:
      func(static_cast<messageService &>(obj));
      return true;
    case messageActionEmpty::ID:
      func(static_cast<messageActionEmpty &>(obj));
      return true;
    case messageActionChatCreate::ID:
      func(static_cast<messageActionChatCreate &>(obj));
      return true;
    case messageActionChatEditTitle::ID:
      func(static_cast<messageActionChatEditTitle &>(obj));
      return true;
    case messageActionChatEditPhoto::ID:
      func(static_cast<messageActionChatEditPhoto &>(obj));
      return true;
    case messageActionChatDeletePhoto::ID:
      func(static_cast<messageActionChatDeletePhoto &>(obj));
      return true;
    case messageActionChatAddUser::ID:
      func(static_cast<messageActionChatAddUser &>(obj));
      return true;
    case messageActionChatDeleteUser::ID:
      func(static_cast<messageActionChatDeleteUser &>(obj));
      return true;
    case messageActionChatJoinedByLink::ID:
      func(static_cast<messageActionChatJoinedByLink &>(obj));
      return true;
    case messageActionChannelCreate::ID:
      func(static_cast<messageActionChannelCreate &>(obj));
      return true;
    case messageActionChatMigrateTo::ID:
      func(static_cast<messageActionChatMigrateTo &>(obj));
      return true;
    case messageActionChannelMigrateFrom::ID:
      func(static_cast<messageActionChannelMigrateFrom &>(obj));
      return true;
    case messageActionPinMessage::ID:
      func(static_cast<messageActionPinMessage &>(obj));
      return true;
    case messageActionHistoryClear::ID:
      func(static_cast<messageActionHistoryClear &>(obj));
      return true;
    case messageActionGameScore::ID:
      func(static_cast<messageActionGameScore &>(obj));
      return true;
    case messageActionPaymentSentMe::ID:
      func(static_cast<messageActionPaymentSentMe &>(obj));
      return true;
    case messageActionPaymentSent::ID:
      func(static_cast<messageActionPaymentSent &>(obj));
      return true;
    case messageActionPhoneCall::ID:
      func(static_cast<messageActionPhoneCall &>(obj));
      return true;
    case messageActionScreenshotTaken::ID:
      func(static_cast<messageActionScreenshotTaken &>(obj));
      return true;
    case messageActionCustomAction::ID:
      func(static_cast<messageActionCustomAction &>(obj));
      return true;
    case messageActionBotAllowed::ID:
      func(static_cast<messageActionBotAllowed &>(obj));
      return true;
    case messageActionSecureValuesSentMe::ID:
      func(static_cast<messageActionSecureValuesSentMe &>(obj));
      return true;
    case messageActionSecureValuesSent::ID:
      func(static_cast<messageActionSecureValuesSent &>(obj));
      return true;
    case messageActionContactSignUp::ID:
      func(static_cast<messageActionContactSignUp &>(obj));
      return true;
    case messageActionGeoProximityReached::ID:
      func(static_cast<messageActionGeoProximityReached &>(obj));
      return true;
    case messageActionGroupCall::ID:
      func(static_cast<messageActionGroupCall &>(obj));
      return true;
    case messageActionInviteToGroupCall::ID:
      func(static_cast<messageActionInviteToGroupCall &>(obj));
      return true;
    case messageActionSetMessagesTTL::ID:
      func(static_cast<messageActionSetMessagesTTL &>(obj));
      return true;
    case messageActionGroupCallScheduled::ID:
      func(static_cast<messageActionGroupCallScheduled &>(obj));
      return true;
    case messageActionSetChatTheme::ID:
      func(static_cast<messageActionSetChatTheme &>(obj));
      return true;
    case messageActionChatJoinedByRequest::ID:
      func(static_cast<messageActionChatJoinedByRequest &>(obj));
      return true;
    case messageActionWebViewDataSentMe::ID:
      func(static_cast<messageActionWebViewDataSentMe &>(obj));
      return true;
    case messageActionWebViewDataSent::ID:
      func(static_cast<messageActionWebViewDataSent &>(obj));
      return true;
    case messageActionGiftPremium::ID:
      func(static_cast<messageActionGiftPremium &>(obj));
      return true;
    case messageActionTopicCreate::ID:
      func(static_cast<messageActionTopicCreate &>(obj));
      return true;
    case messageActionTopicEdit::ID:
      func(static_cast<messageActionTopicEdit &>(obj));
      return true;
    case messageActionSuggestProfilePhoto::ID:
      func(static_cast<messageActionSuggestProfilePhoto &>(obj));
      return true;
    case messageActionRequestedPeer::ID:
      func(static_cast<messageActionRequestedPeer &>(obj));
      return true;
    case messageActionSetChatWallPaper::ID:
      func(static_cast<messageActionSetChatWallPaper &>(obj));
      return true;
    case messageActionSetSameChatWallPaper::ID:
      func(static_cast<messageActionSetSameChatWallPaper &>(obj));
      return true;
    case messageEntityUnknown::ID:
      func(static_cast<messageEntityUnknown &>(obj));
      return true;
    case messageEntityMention::ID:
      func(static_cast<messageEntityMention &>(obj));
      return true;
    case messageEntityHashtag::ID:
      func(static_cast<messageEntityHashtag &>(obj));
      return true;
    case messageEntityBotCommand::ID:
      func(static_cast<messageEntityBotCommand &>(obj));
      return true;
    case messageEntityUrl::ID:
      func(static_cast<messageEntityUrl &>(obj));
      return true;
    case messageEntityEmail::ID:
      func(static_cast<messageEntityEmail &>(obj));
      return true;
    case messageEntityBold::ID:
      func(static_cast<messageEntityBold &>(obj));
      return true;
    case messageEntityItalic::ID:
      func(static_cast<messageEntityItalic &>(obj));
      return true;
    case messageEntityCode::ID:
      func(static_cast<messageEntityCode &>(obj));
      return true;
    case messageEntityPre::ID:
      func(static_cast<messageEntityPre &>(obj));
      return true;
    case messageEntityTextUrl::ID:
      func(static_cast<messageEntityTextUrl &>(obj));
      return true;
    case messageEntityMentionName::ID:
      func(static_cast<messageEntityMentionName &>(obj));
      return true;
    case inputMessageEntityMentionName::ID:
      func(static_cast<inputMessageEntityMentionName &>(obj));
      return true;
    case messageEntityPhone::ID:
      func(static_cast<messageEntityPhone &>(obj));
      return true;
    case messageEntityCashtag::ID:
      func(static_cast<messageEntityCashtag &>(obj));
      return true;
    case messageEntityUnderline::ID:
      func(static_cast<messageEntityUnderline &>(obj));
      return true;
    case messageEntityStrike::ID:
      func(static_cast<messageEntityStrike &>(obj));
      return true;
    case messageEntityBlockquote::ID:
      func(static_cast<messageEntityBlockquote &>(obj));
      return true;
    case messageEntityBankCard::ID:
      func(static_cast<messageEntityBankCard &>(obj));
      return true;
    case messageEntitySpoiler::ID:
      func(static_cast<messageEntitySpoiler &>(obj));
      return true;
    case messageEntityCustomEmoji::ID:
      func(static_cast<messageEntityCustomEmoji &>(obj));
      return true;
    case messageExtendedMediaPreview::ID:
      func(static_cast<messageExtendedMediaPreview &>(obj));
      return true;
    case messageExtendedMedia::ID:
      func(static_cast<messageExtendedMedia &>(obj));
      return true;
    case messageFwdHeader::ID:
      func(static_cast<messageFwdHeader &>(obj));
      return true;
    case messageInteractionCounters::ID:
      func(static_cast<messageInteractionCounters &>(obj));
      return true;
    case messageMediaEmpty::ID:
      func(static_cast<messageMediaEmpty &>(obj));
      return true;
    case messageMediaPhoto::ID:
      func(static_cast<messageMediaPhoto &>(obj));
      return true;
    case messageMediaGeo::ID:
      func(static_cast<messageMediaGeo &>(obj));
      return true;
    case messageMediaContact::ID:
      func(static_cast<messageMediaContact &>(obj));
      return true;
    case messageMediaUnsupported::ID:
      func(static_cast<messageMediaUnsupported &>(obj));
      return true;
    case messageMediaDocument::ID:
      func(static_cast<messageMediaDocument &>(obj));
      return true;
    case messageMediaWebPage::ID:
      func(static_cast<messageMediaWebPage &>(obj));
      return true;
    case messageMediaVenue::ID:
      func(static_cast<messageMediaVenue &>(obj));
      return true;
    case messageMediaGame::ID:
      func(static_cast<messageMediaGame &>(obj));
      return true;
    case messageMediaInvoice::ID:
      func(static_cast<messageMediaInvoice &>(obj));
      return true;
    case messageMediaGeoLive::ID:
      func(static_cast<messageMediaGeoLive &>(obj));
      return true;
    case messageMediaPoll::ID:
      func(static_cast<messageMediaPoll &>(obj));
      return true;
    case messageMediaDice::ID:
      func(static_cast<messageMediaDice &>(obj));
      return true;
    case messagePeerReaction::ID:
      func(static_cast<messagePeerReaction &>(obj));
      return true;
    case messageRange::ID:
      func(static_cast<messageRange &>(obj));
      return true;
    case messageReactions::ID:
      func(static_cast<messageReactions &>(obj));
      return true;
    case messageReplies::ID:
      func(static_cast<messageReplies &>(obj));
      return true;
    case messageReplyHeader::ID:
      func(static_cast<messageReplyHeader &>(obj));
      return true;
    case messageUserVote::ID:
      func(static_cast<messageUserVote &>(obj));
      return true;
    case messageUserVoteInputOption::ID:
      func(static_cast<messageUserVoteInputOption &>(obj));
      return true;
    case messageUserVoteMultiple::ID:
      func(static_cast<messageUserVoteMultiple &>(obj));
      return true;
    case messageViews::ID:
      func(static_cast<messageViews &>(obj));
      return true;
    case inputMessagesFilterEmpty::ID:
      func(static_cast<inputMessagesFilterEmpty &>(obj));
      return true;
    case inputMessagesFilterPhotos::ID:
      func(static_cast<inputMessagesFilterPhotos &>(obj));
      return true;
    case inputMessagesFilterVideo::ID:
      func(static_cast<inputMessagesFilterVideo &>(obj));
      return true;
    case inputMessagesFilterPhotoVideo::ID:
      func(static_cast<inputMessagesFilterPhotoVideo &>(obj));
      return true;
    case inputMessagesFilterDocument::ID:
      func(static_cast<inputMessagesFilterDocument &>(obj));
      return true;
    case inputMessagesFilterUrl::ID:
      func(static_cast<inputMessagesFilterUrl &>(obj));
      return true;
    case inputMessagesFilterGif::ID:
      func(static_cast<inputMessagesFilterGif &>(obj));
      return true;
    case inputMessagesFilterVoice::ID:
      func(static_cast<inputMessagesFilterVoice &>(obj));
      return true;
    case inputMessagesFilterMusic::ID:
      func(static_cast<inputMessagesFilterMusic &>(obj));
      return true;
    case inputMessagesFilterChatPhotos::ID:
      func(static_cast<inputMessagesFilterChatPhotos &>(obj));
      return true;
    case inputMessagesFilterPhoneCalls::ID:
      func(static_cast<inputMessagesFilterPhoneCalls &>(obj));
      return true;
    case inputMessagesFilterRoundVoice::ID:
      func(static_cast<inputMessagesFilterRoundVoice &>(obj));
      return true;
    case inputMessagesFilterRoundVideo::ID:
      func(static_cast<inputMessagesFilterRoundVideo &>(obj));
      return true;
    case inputMessagesFilterMyMentions::ID:
      func(static_cast<inputMessagesFilterMyMentions &>(obj));
      return true;
    case inputMessagesFilterGeo::ID:
      func(static_cast<inputMessagesFilterGeo &>(obj));
      return true;
    case inputMessagesFilterContacts::ID:
      func(static_cast<inputMessagesFilterContacts &>(obj));
      return true;
    case inputMessagesFilterPinned::ID:
      func(static_cast<inputMessagesFilterPinned &>(obj));
      return true;
    case nearestDc::ID:
      func(static_cast<nearestDc &>(obj));
      return true;
    case notificationSoundDefault::ID:
      func(static_cast<notificationSoundDefault &>(obj));
      return true;
    case notificationSoundNone::ID:
      func(static_cast<notificationSoundNone &>(obj));
      return true;
    case notificationSoundLocal::ID:
      func(static_cast<notificationSoundLocal &>(obj));
      return true;
    case notificationSoundRingtone::ID:
      func(static_cast<notificationSoundRingtone &>(obj));
      return true;
    case notifyPeer::ID:
      func(static_cast<notifyPeer &>(obj));
      return true;
    case notifyUsers::ID:
      func(static_cast<notifyUsers &>(obj));
      return true;
    case notifyChats::ID:
      func(static_cast<notifyChats &>(obj));
      return true;
    case notifyBroadcasts::ID:
      func(static_cast<notifyBroadcasts &>(obj));
      return true;
    case notifyForumTopic::ID:
      func(static_cast<notifyForumTopic &>(obj));
      return true;
    case page::ID:
      func(static_cast<page &>(obj));
      return true;
    case pageBlockUnsupported::ID:
      func(static_cast<pageBlockUnsupported &>(obj));
      return true;
    case pageBlockTitle::ID:
      func(static_cast<pageBlockTitle &>(obj));
      return true;
    case pageBlockSubtitle::ID:
      func(static_cast<pageBlockSubtitle &>(obj));
      return true;
    case pageBlockAuthorDate::ID:
      func(static_cast<pageBlockAuthorDate &>(obj));
      return true;
    case pageBlockHeader::ID:
      func(static_cast<pageBlockHeader &>(obj));
      return true;
    case pageBlockSubheader::ID:
      func(static_cast<pageBlockSubheader &>(obj));
      return true;
    case pageBlockParagraph::ID:
      func(static_cast<pageBlockParagraph &>(obj));
      return true;
    case pageBlockPreformatted::ID:
      func(static_cast<pageBlockPreformatted &>(obj));
      return true;
    case pageBlockFooter::ID:
      func(static_cast<pageBlockFooter &>(obj));
      return true;
    case pageBlockDivider::ID:
      func(static_cast<pageBlockDivider &>(obj));
      return true;
    case pageBlockAnchor::ID:
      func(static_cast<pageBlockAnchor &>(obj));
      return true;
    case pageBlockList::ID:
      func(static_cast<pageBlockList &>(obj));
      return true;
    case pageBlockBlockquote::ID:
      func(static_cast<pageBlockBlockquote &>(obj));
      return true;
    case pageBlockPullquote::ID:
      func(static_cast<pageBlockPullquote &>(obj));
      return true;
    case pageBlockPhoto::ID:
      func(static_cast<pageBlockPhoto &>(obj));
      return true;
    case pageBlockVideo::ID:
      func(static_cast<pageBlockVideo &>(obj));
      return true;
    case pageBlockCover::ID:
      func(static_cast<pageBlockCover &>(obj));
      return true;
    case pageBlockEmbed::ID:
      func(static_cast<pageBlockEmbed &>(obj));
      return true;
    case pageBlockEmbedPost::ID:
      func(static_cast<pageBlockEmbedPost &>(obj));
      return true;
    case pageBlockCollage::ID:
      func(static_cast<pageBlockCollage &>(obj));
      return true;
    case pageBlockSlideshow::ID:
      func(static_cast<pageBlockSlideshow &>(obj));
      return true;
    case pageBlockChannel::ID:
      func(static_cast<pageBlockChannel &>(obj));
      return true;
    case pageBlockAudio::ID:
      func(static_cast<pageBlockAudio &>(obj));
      return true;
    case pageBlockKicker::ID:
      func(static_cast<pageBlockKicker &>(obj));
      return true;
    case pageBlockTable::ID:
      func(static_cast<pageBlockTable &>(obj));
      return true;
    case pageBlockOrderedList::ID:
      func(static_cast<pageBlockOrderedList &>(obj));
      return true;
    case pageBlockDetails::ID:
      func(static_cast<pageBlockDetails &>(obj));
      return true;
    case pageBlockRelatedArticles::ID:
      func(static_cast<pageBlockRelatedArticles &>(obj));
      return true;
    case pageBlockMap::ID:
      func(static_cast<pageBlockMap &>(obj));
      return true;
    case pageCaption::ID:
      func(static_cast<pageCaption &>(obj));
      return true;
    case pageListItemText::ID:
      func(static_cast<pageListItemText &>(obj));
      return true;
    case pageListItemBlocks::ID:
      func(static_cast<pageListItemBlocks &>(obj));
      return true;
    case pageListOrderedItemText::ID:
      func(static_cast<pageListOrderedItemText &>(obj));
      return true;
    case pageListOrderedItemBlocks::ID:
      func(static_cast<pageListOrderedItemBlocks &>(obj));
      return true;
    case pageRelatedArticle::ID:
      func(static_cast<pageRelatedArticle &>(obj));
      return true;
    case pageTableCell::ID:
      func(static_cast<pageTableCell &>(obj));
      return true;
    case pageTableRow::ID:
      func(static_cast<pageTableRow &>(obj));
      return true;
    case passwordKdfAlgoUnknown::ID:
      func(static_cast<passwordKdfAlgoUnknown &>(obj));
      return true;
    case passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow::ID:
      func(static_cast<passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow &>(obj));
      return true;
    case paymentCharge::ID:
      func(static_cast<paymentCharge &>(obj));
      return true;
    case paymentFormMethod::ID:
      func(static_cast<paymentFormMethod &>(obj));
      return true;
    case paymentRequestedInfo::ID:
      func(static_cast<paymentRequestedInfo &>(obj));
      return true;
    case paymentSavedCredentialsCard::ID:
      func(static_cast<paymentSavedCredentialsCard &>(obj));
      return true;
    case peerUser::ID:
      func(static_cast<peerUser &>(obj));
      return true;
    case peerChat::ID:
      func(static_cast<peerChat &>(obj));
      return true;
    case peerChannel::ID:
      func(static_cast<peerChannel &>(obj));
      return true;
    case peerBlocked::ID:
      func(static_cast<peerBlocked &>(obj));
      return true;
    case peerLocated::ID:
      func(static_cast<peerLocated &>(obj));
      return true;
    case peerSelfLocated::ID:
      func(static_cast<peerSelfLocated &>(obj));
      return true;
    case peerNotifySettings::ID:
      func(static_cast<peerNotifySettings &>(obj));
      return true;
    case peerSettings::ID:
      func(static_cast<peerSettings &>(obj));
      return true;
    case phoneCallEmpty::ID:
      func(static_cast<phoneCallEmpty &>(obj));
      return true;
    case phoneCallWaiting::ID:
      func(static_cast<phoneCallWaiting &>(obj));
      return true;
    case phoneCallRequested::ID:
      func(static_cast<phoneCallRequested &>(obj));
      return true;
    case phoneCallAccepted::ID:
      func(static_cast<phoneCallAccepted &>(obj));
      return true;
    case phoneCall::ID:
      func(static_cast<phoneCall &>(obj));
      return true;
    case phoneCallDiscarded::ID:
      func(static_cast<phoneCallDiscarded &>(obj));
      return true;
    case phoneCallDiscardReasonMissed::ID:
      func(static_cast<phoneCallDiscardReasonMissed &>(obj));
      return true;
    case phoneCallDiscardReasonDisconnect::ID:
      func(static_cast<phoneCallDiscardReasonDisconnect &>(obj));
      return true;
    case phoneCallDiscardReasonHangup::ID:
      func(static_cast<phoneCallDiscardReasonHangup &>(obj));
      return true;
    case phoneCallDiscardReasonBusy::ID:
      func(static_cast<phoneCallDiscardReasonBusy &>(obj));
      return true;
    case phoneCallProtocol::ID:
      func(static_cast<phoneCallProtocol &>(obj));
      return true;
    case phoneConnection::ID:
      func(static_cast<phoneConnection &>(obj));
      return true;
    case phoneConnectionWebrtc::ID:
      func(static_cast<phoneConnectionWebrtc &>(obj));
      return true;
    case photoEmpty::ID:
      func(static_cast<photoEmpty &>(obj));
      return true;
    case photo::ID:
      func(static_cast<photo &>(obj));
      return true;
    case photoSizeEmpty::ID:
      func(static_cast<photoSizeEmpty &>(obj));
      return true;
    case photoSize::ID:
      func(static_cast<photoSize &>(obj));
      return true;
    case photoCachedSize::ID:
      func(static_cast<photoCachedSize &>(obj));
      return true;
    case photoStrippedSize::ID:
      func(static_cast<photoStrippedSize &>(obj));
      return true;
    case photoSizeProgressive::ID:
      func(static_cast<photoSizeProgressive &>(obj));
      return true;
    case photoPathSize::ID:
      func(static_cast<photoPathSize &>(obj));
      return true;
    case poll::ID:
      func(static_cast<poll &>(obj));
      return true;
    case pollAnswer::ID:
      func(static_cast<pollAnswer &>(obj));
      return true;
    case pollAnswerVoters::ID:
      func(static_cast<pollAnswerVoters &>(obj));
      return true;
    case pollResults::ID:
      func(static_cast<pollResults &>(obj));
      return true;
    case popularContact::ID:
      func(static_cast<popularContact &>(obj));
      return true;
    case postAddress::ID:
      func(static_cast<postAddress &>(obj));
      return true;
    case premiumGiftOption::ID:
      func(static_cast<premiumGiftOption &>(obj));
      return true;
    case premiumSubscriptionOption::ID:
      func(static_cast<premiumSubscriptionOption &>(obj));
      return true;
    case privacyKeyStatusTimestamp::ID:
      func(static_cast<privacyKeyStatusTimestamp &>(obj));
      return true;
    case privacyKeyChatInvite::ID:
      func(static_cast<privacyKeyChatInvite &>(obj));
      return true;
    case privacyKeyPhoneCall::ID:
      func(static_cast<privacyKeyPhoneCall &>(obj));
      return true;
    case privacyKeyPhoneP2P::ID:
      func(static_cast<privacyKeyPhoneP2P &>(obj));
      return true;
    case privacyKeyForwards::ID:
      func(static_cast<privacyKeyForwards &>(obj));
      return true;
    case privacyKeyProfilePhoto::ID:
      func(static_cast<privacyKeyProfilePhoto &>(obj));
      return true;
    case privacyKeyPhoneNumber::ID:
      func(static_cast<privacyKeyPhoneNumber &>(obj));
      return true;
    case privacyKeyAddedByPhone::ID:
      func(static_cast<privacyKeyAddedByPhone &>(obj));
      return true;
    case privacyKeyVoiceMessages::ID:
      func(static_cast<privacyKeyVoiceMessages &>(obj));
      return true;
    case privacyValueAllowContacts::ID:
      func(static_cast<privacyValueAllowContacts &>(obj));
      return true;
    case privacyValueAllowAll::ID:
      func(static_cast<privacyValueAllowAll &>(obj));
      return true;
    case privacyValueAllowUsers::ID:
      func(static_cast<privacyValueAllowUsers &>(obj));
      return true;
    case privacyValueDisallowContacts::ID:
      func(static_cast<privacyValueDisallowContacts &>(obj));
      return true;
    case privacyValueDisallowAll::ID:
      func(static_cast<privacyValueDisallowAll &>(obj));
      return true;
    case privacyValueDisallowUsers::ID:
      func(static_cast<privacyValueDisallowUsers &>(obj));
      return true;
    case privacyValueAllowChatParticipants::ID:
      func(static_cast<privacyValueAllowChatParticipants &>(obj));
      return true;
    case privacyValueDisallowChatParticipants::ID:
      func(static_cast<privacyValueDisallowChatParticipants &>(obj));
      return true;
    case reactionEmpty::ID:
      func(static_cast<reactionEmpty &>(obj));
      return true;
    case reactionEmoji::ID:
      func(static_cast<reactionEmoji &>(obj));
      return true;
    case reactionCustomEmoji::ID:
      func(static_cast<reactionCustomEmoji &>(obj));
      return true;
    case reactionCount::ID:
      func(static_cast<reactionCount &>(obj));
      return true;
    case readParticipantDate::ID:
      func(static_cast<readParticipantDate &>(obj));
      return true;
    case receivedNotifyMessage::ID:
      func(static_cast<receivedNotifyMessage &>(obj));
      return true;
    case recentMeUrlUnknown::ID:
      func(static_cast<recentMeUrlUnknown &>(obj));
      return true;
    case recentMeUrlUser::ID:
      func(static_cast<recentMeUrlUser &>(obj));
      return true;
    case recentMeUrlChat::ID:
      func(static_cast<recentMeUrlChat &>(obj));
      return true;
    case recentMeUrlChatInvite::ID:
      func(static_cast<recentMeUrlChatInvite &>(obj));
      return true;
    case recentMeUrlStickerSet::ID:
      func(static_cast<recentMeUrlStickerSet &>(obj));
      return true;
    case replyKeyboardHide::ID:
      func(static_cast<replyKeyboardHide &>(obj));
      return true;
    case replyKeyboardForceReply::ID:
      func(static_cast<replyKeyboardForceReply &>(obj));
      return true;
    case replyKeyboardMarkup::ID:
      func(static_cast<replyKeyboardMarkup &>(obj));
      return true;
    case replyInlineMarkup::ID:
      func(static_cast<replyInlineMarkup &>(obj));
      return true;
    case inputReportReasonSpam::ID:
      func(static_cast<inputReportReasonSpam &>(obj));
      return true;
    case inputReportReasonViolence::ID:
      func(static_cast<inputReportReasonViolence &>(obj));
      return true;
    case inputReportReasonPornography::ID:
      func(static_cast<inputReportReasonPornography &>(obj));
      return true;
    case inputReportReasonChildAbuse::ID:
      func(static_cast<inputReportReasonChildAbuse &>(obj));
      return true;
    case inputReportReasonOther::ID:
      func(static_cast<inputReportReasonOther &>(obj));
      return true;
    case inputReportReasonCopyright::ID:
      func(static_cast<inputReportReasonCopyright &>(obj));
      return true;
    case inputReportReasonGeoIrrelevant::ID:
      func(static_cast<inputReportReasonGeoIrrelevant &>(obj));
      return true;
    case inputReportReasonFake::ID:
      func(static_cast<inputReportReasonFake &>(obj));
      return true;
    case inputReportReasonIllegalDrugs::ID:
      func(static_cast<inputReportReasonIllegalDrugs &>(obj));
      return true;
    case inputReportReasonPersonalDetails::ID:
      func(static_cast<inputReportReasonPersonalDetails &>(obj));
      return true;
    case requestPeerTypeUser::ID:
      func(static_cast<requestPeerTypeUser &>(obj));
      return true;
    case requestPeerTypeChat::ID:
      func(static_cast<requestPeerTypeChat &>(obj));
      return true;
    case requestPeerTypeBroadcast::ID:
      func(static_cast<requestPeerTypeBroadcast &>(obj));
      return true;
    case restrictionReason::ID:
      func(static_cast<restrictionReason &>(obj));
      return true;
    case textEmpty::ID:
      func(static_cast<textEmpty &>(obj));
      return true;
    case textPlain::ID:
      func(static_cast<textPlain &>(obj));
      return true;
    case textBold::ID:
      func(static_cast<textBold &>(obj));
      return true;
    case textItalic::ID:
      func(static_cast<textItalic &>(obj));
      return true;
    case textUnderline::ID:
      func(static_cast<textUnderline &>(obj));
      return true;
    case textStrike::ID:
      func(static_cast<textStrike &>(obj));
      return true;
    case textFixed::ID:
      func(static_cast<textFixed &>(obj));
      return true;
    case textUrl::ID:
      func(static_cast<textUrl &>(obj));
      return true;
    case textEmail::ID:
      func(static_cast<textEmail &>(obj));
      return true;
    case textConcat::ID:
      func(static_cast<textConcat &>(obj));
      return true;
    case textSubscript::ID:
      func(static_cast<textSubscript &>(obj));
      return true;
    case textSuperscript::ID:
      func(static_cast<textSuperscript &>(obj));
      return true;
    case textMarked::ID:
      func(static_cast<textMarked &>(obj));
      return true;
    case textPhone::ID:
      func(static_cast<textPhone &>(obj));
      return true;
    case textImage::ID:
      func(static_cast<textImage &>(obj));
      return true;
    case textAnchor::ID:
      func(static_cast<textAnchor &>(obj));
      return true;
    case savedPhoneContact::ID:
      func(static_cast<savedPhoneContact &>(obj));
      return true;
    case searchResultsCalendarPeriod::ID:
      func(static_cast<searchResultsCalendarPeriod &>(obj));
      return true;
    case searchResultPosition::ID:
      func(static_cast<searchResultPosition &>(obj));
      return true;
    case secureCredentialsEncrypted::ID:
      func(static_cast<secureCredentialsEncrypted &>(obj));
      return true;
    case secureData::ID:
      func(static_cast<secureData &>(obj));
      return true;
    case secureFileEmpty::ID:
      func(static_cast<secureFileEmpty &>(obj));
      return true;
    case secureFile::ID:
      func(static_cast<secureFile &>(obj));
      return true;
    case securePasswordKdfAlgoUnknown::ID:
      func(static_cast<securePasswordKdfAlgoUnknown &>(obj));
      return true;
    case securePasswordKdfAlgoPBKDF2HMACSHA512iter100000::ID:
      func(static_cast<securePasswordKdfAlgoPBKDF2HMACSHA512iter100000 &>(obj));
      return true;
    case securePasswordKdfAlgoSHA512::ID:
      func(static_cast<securePasswordKdfAlgoSHA512 &>(obj));
      return true;
    case securePlainPhone::ID:
      func(static_cast<securePlainPhone &>(obj));
      return true;
    case securePlainEmail::ID:
      func(static_cast<securePlainEmail &>(obj));
      return true;
    case secureRequiredType::ID:
      func(static_cast<secureRequiredType &>(obj));
      return true;
    case secureRequiredTypeOneOf::ID:
      func(static_cast<secureRequiredTypeOneOf &>(obj));
      return true;
    case secureSecretSettings::ID:
      func(static_cast<secureSecretSettings &>(obj));
      return true;
    case secureValue::ID:
      func(static_cast<secureValue &>(obj));
      return true;
    case secureValueErrorData::ID:
      func(static_cast<secureValueErrorData &>(obj));
      return true;
    case secureValueErrorFrontSide::ID:
      func(static_cast<secureValueErrorFrontSide &>(obj));
      return true;
    case secureValueErrorReverseSide::ID:
      func(static_cast<secureValueErrorReverseSide &>(obj));
      return true;
    case secureValueErrorSelfie::ID:
      func(static_cast<secureValueErrorSelfie &>(obj));
      return true;
    case secureValueErrorFile::ID:
      func(static_cast<secureValueErrorFile &>(obj));
      return true;
    case secureValueErrorFiles::ID:
      func(static_cast<secureValueErrorFiles &>(obj));
      return true;
    case secureValueError::ID:
      func(static_cast<secureValueError &>(obj));
      return true;
    case secureValueErrorTranslationFile::ID:
      func(static_cast<secureValueErrorTranslationFile &>(obj));
      return true;
    case secureValueErrorTranslationFiles::ID:
      func(static_cast<secureValueErrorTranslationFiles &>(obj));
      return true;
    case secureValueHash::ID:
      func(static_cast<secureValueHash &>(obj));
      return true;
    case secureValueTypePersonalDetails::ID:
      func(static_cast<secureValueTypePersonalDetails &>(obj));
      return true;
    case secureValueTypePassport::ID:
      func(static_cast<secureValueTypePassport &>(obj));
      return true;
    case secureValueTypeDriverLicense::ID:
      func(static_cast<secureValueTypeDriverLicense &>(obj));
      return true;
    case secureValueTypeIdentityCard::ID:
      func(static_cast<secureValueTypeIdentityCard &>(obj));
      return true;
    case secureValueTypeInternalPassport::ID:
      func(static_cast<secureValueTypeInternalPassport &>(obj));
      return true;
    case secureValueTypeAddress::ID:
      func(static_cast<secureValueTypeAddress &>(obj));
      return true;
    case secureValueTypeUtilityBill::ID:
      func(static_cast<secureValueTypeUtilityBill &>(obj));
      return true;
    case secureValueTypeBankStatement::ID:
      func(static_cast<secureValueTypeBankStatement &>(obj));
      return true;
    case secureValueTypeRentalAgreement::ID:
      func(static_cast<secureValueTypeRentalAgreement &>(obj));
      return true;
    case secureValueTypePassportRegistration::ID:
      func(static_cast<secureValueTypePassportRegistration &>(obj));
      return true;
    case secureValueTypeTemporaryRegistration::ID:
      func(static_cast<secureValueTypeTemporaryRegistration &>(obj));
      return true;
    case secureValueTypePhone::ID:
      func(static_cast<secureValueTypePhone &>(obj));
      return true;
    case secureValueTypeEmail::ID:
      func(static_cast<secureValueTypeEmail &>(obj));
      return true;
    case sendAsPeer::ID:
      func(static_cast<sendAsPeer &>(obj));
      return true;
    case sendMessageTypingAction::ID:
      func(static_cast<sendMessageTypingAction &>(obj));
      return true;
    case sendMessageCancelAction::ID:
      func(static_cast<sendMessageCancelAction &>(obj));
      return true;
    case sendMessageRecordVideoAction::ID:
      func(static_cast<sendMessageRecordVideoAction &>(obj));
      return true;
    case sendMessageUploadVideoAction::ID:
      func(static_cast<sendMessageUploadVideoAction &>(obj));
      return true;
    case sendMessageRecordAudioAction::ID:
      func(static_cast<sendMessageRecordAudioAction &>(obj));
      return true;
    case sendMessageUploadAudioAction::ID:
      func(static_cast<sendMessageUploadAudioAction &>(obj));
      return true;
    case sendMessageUploadPhotoAction::ID:
      func(static_cast<sendMessageUploadPhotoAction &>(obj));
      return true;
    case sendMessageUploadDocumentAction::ID:
      func(static_cast<sendMessageUploadDocumentAction &>(obj));
      return true;
    case sendMessageGeoLocationAction::ID:
      func(static_cast<sendMessageGeoLocationAction &>(obj));
      return true;
    case sendMessageChooseContactAction::ID:
      func(static_cast<sendMessageChooseContactAction &>(obj));
      return true;
    case sendMessageGamePlayAction::ID:
      func(static_cast<sendMessageGamePlayAction &>(obj));
      return true;
    case sendMessageRecordRoundAction::ID:
      func(static_cast<sendMessageRecordRoundAction &>(obj));
      return true;
    case sendMessageUploadRoundAction::ID:
      func(static_cast<sendMessageUploadRoundAction &>(obj));
      return true;
    case speakingInGroupCallAction::ID:
      func(static_cast<speakingInGroupCallAction &>(obj));
      return true;
    case sendMessageHistoryImportAction::ID:
      func(static_cast<sendMessageHistoryImportAction &>(obj));
      return true;
    case sendMessageChooseStickerAction::ID:
      func(static_cast<sendMessageChooseStickerAction &>(obj));
      return true;
    case sendMessageEmojiInteraction::ID:
      func(static_cast<sendMessageEmojiInteraction &>(obj));
      return true;
    case sendMessageEmojiInteractionSeen::ID:
      func(static_cast<sendMessageEmojiInteractionSeen &>(obj));
      return true;
    case shippingOption::ID:
      func(static_cast<shippingOption &>(obj));
      return true;
    case simpleWebViewResultUrl::ID:
      func(static_cast<simpleWebViewResultUrl &>(obj));
      return true;
    case sponsoredMessage::ID:
      func(static_cast<sponsoredMessage &>(obj));
      return true;
    case statsAbsValueAndPrev::ID:
      func(static_cast<statsAbsValueAndPrev &>(obj));
      return true;
    case statsDateRangeDays::ID:
      func(static_cast<statsDateRangeDays &>(obj));
      return true;
    case statsGraphAsync::ID:
      func(static_cast<statsGraphAsync &>(obj));
      return true;
    case statsGraphError::ID:
      func(static_cast<statsGraphError &>(obj));
      return true;
    case statsGraph::ID:
      func(static_cast<statsGraph &>(obj));
      return true;
    case statsGroupTopAdmin::ID:
      func(static_cast<statsGroupTopAdmin &>(obj));
      return true;
    case statsGroupTopInviter::ID:
      func(static_cast<statsGroupTopInviter &>(obj));
      return true;
    case statsGroupTopPoster::ID:
      func(static_cast<statsGroupTopPoster &>(obj));
      return true;
    case statsPercentValue::ID:
      func(static_cast<statsPercentValue &>(obj));
      return true;
    case statsURL::ID:
      func(static_cast<statsURL &>(obj));
      return true;
    case stickerKeyword::ID:
      func(static_cast<stickerKeyword &>(obj));
      return true;
    case stickerPack::ID:
      func(static_cast<stickerPack &>(obj));
      return true;
    case stickerSet::ID:
      func(static_cast<stickerSet &>(obj));
      return true;
    case stickerSetCovered::ID:
      func(static_cast<stickerSetCovered &>(obj));
      return true;
    case stickerSetMultiCovered::ID:
      func(static_cast<stickerSetMultiCovered &>(obj));
      return true;
    case stickerSetFullCovered::ID:
      func(static_cast<stickerSetFullCovered &>(obj));
      return true;
    case stickerSetNoCovered::ID:
      func(static_cast<stickerSetNoCovered &>(obj));
      return true;
    case textWithEntities::ID:
      func(static_cast<textWithEntities &>(obj));
      return true;
    case theme::ID:
      func(static_cast<theme &>(obj));
      return true;
    case themeSettings::ID:
      func(static_cast<themeSettings &>(obj));
      return true;
    case topPeer::ID:
      func(static_cast<topPeer &>(obj));
      return true;
    case topPeerCategoryBotsPM::ID:
      func(static_cast<topPeerCategoryBotsPM &>(obj));
      return true;
    case topPeerCategoryBotsInline::ID:
      func(static_cast<topPeerCategoryBotsInline &>(obj));
      return true;
    case topPeerCategoryCorrespondents::ID:
      func(static_cast<topPeerCategoryCorrespondents &>(obj));
      return true;
    case topPeerCategoryGroups::ID:
      func(static_cast<topPeerCategoryGroups &>(obj));
      return true;
    case topPeerCategoryChannels::ID:
      func(static_cast<topPeerCategoryChannels &>(obj));
      return true;
    case topPeerCategoryPhoneCalls::ID:
      func(static_cast<topPeerCategoryPhoneCalls &>(obj));
      return true;
    case topPeerCategoryForwardUsers::ID:
      func(static_cast<topPeerCategoryForwardUsers &>(obj));
      return true;
    case topPeerCategoryForwardChats::ID:
      func(static_cast<topPeerCategoryForwardChats &>(obj));
      return true;
    case topPeerCategoryPeers::ID:
      func(static_cast<topPeerCategoryPeers &>(obj));
      return true;
    case updateNewMessage::ID:
      func(static_cast<updateNewMessage &>(obj));
      return true;
    case updateMessageID::ID:
      func(static_cast<updateMessageID &>(obj));
      return true;
    case updateDeleteMessages::ID:
      func(static_cast<updateDeleteMessages &>(obj));
      return true;
    case updateUserTyping::ID:
      func(static_cast<updateUserTyping &>(obj));
      return true;
    case updateChatUserTyping::ID:
      func(static_cast<updateChatUserTyping &>(obj));
      return true;
    case updateChatParticipants::ID:
      func(static_cast<updateChatParticipants &>(obj));
      return true;
    case updateUserStatus::ID:
      func(static_cast<updateUserStatus &>(obj));
      return true;
    case updateUserName::ID:
      func(static_cast<updateUserName &>(obj));
      return true;
    case updateNewEncryptedMessage::ID:
      func(static_cast<updateNewEncryptedMessage &>(obj));
      return true;
    case updateEncryptedChatTyping::ID:
      func(static_cast<updateEncryptedChatTyping &>(obj));
      return true;
    case updateEncryption::ID:
      func(static_cast<updateEncryption &>(obj));
      return true;
    case updateEncryptedMessagesRead::ID:
      func(static_cast<updateEncryptedMessagesRead &>(obj));
      return true;
    case updateChatParticipantAdd::ID:
      func(static_cast<updateChatParticipantAdd &>(obj));
      return true;
    case updateChatParticipantDelete::ID:
      func(static_cast<updateChatParticipantDelete &>(obj));
      return true;
    case updateDcOptions::ID:
      func(static_cast<updateDcOptions &>(obj));
      return true;
    case updateNotifySettings::ID:
      func(static_cast<updateNotifySettings &>(obj));
      return true;
    case updateServiceNotification::ID:
      func(static_cast<updateServiceNotification &>(obj));
      return true;
    case updatePrivacy::ID:
      func(static_cast<updatePrivacy &>(obj));
      return true;
    case updateUserPhone::ID:
      func(static_cast<updateUserPhone &>(obj));
      return true;
    case updateReadHistoryInbox::ID:
      func(static_cast<updateReadHistoryInbox &>(obj));
      return true;
    case updateReadHistoryOutbox::ID:
      func(static_cast<updateReadHistoryOutbox &>(obj));
      return true;
    case updateWebPage::ID:
      func(static_cast<updateWebPage &>(obj));
      return true;
    case updateReadMessagesContents::ID:
      func(static_cast<updateReadMessagesContents &>(obj));
      return true;
    case updateChannelTooLong::ID:
      func(static_cast<updateChannelTooLong &>(obj));
      return true;
    case updateChannel::ID:
      func(static_cast<updateChannel &>(obj));
      return true;
    case updateNewChannelMessage::ID:
      func(static_cast<updateNewChannelMessage &>(obj));
      return true;
    case updateReadChannelInbox::ID:
      func(static_cast<updateReadChannelInbox &>(obj));
      return true;
    case updateDeleteChannelMessages::ID:
      func(static_cast<updateDeleteChannelMessages &>(obj));
      return true;
    case updateChannelMessageViews::ID:
      func(static_cast<updateChannelMessageViews &>(obj));
      return true;
    case updateChatParticipantAdmin::ID:
      func(static_cast<updateChatParticipantAdmin &>(obj));
      return true;
    case updateNewStickerSet::ID:
      func(static_cast<updateNewStickerSet &>(obj));
      return true;
    case updateStickerSetsOrder::ID:
      func(static_cast<updateStickerSetsOrder &>(obj));
      return true;
    case updateStickerSets::ID:
      func(static_cast<updateStickerSets &>(obj));
      return true;
    case updateSavedGifs::ID:
      func(static_cast<updateSavedGifs &>(obj));
      return true;
    case updateBotInlineQuery::ID:
      func(static_cast<updateBotInlineQuery &>(obj));
      return true;
    case updateBotInlineSend::ID:
      func(static_cast<updateBotInlineSend &>(obj));
      return true;
    case updateEditChannelMessage::ID:
      func(static_cast<updateEditChannelMessage &>(obj));
      return true;
    case updateBotCallbackQuery::ID:
      func(static_cast<updateBotCallbackQuery &>(obj));
      return true;
    case updateEditMessage::ID:
      func(static_cast<updateEditMessage &>(obj));
      return true;
    case updateInlineBotCallbackQuery::ID:
      func(static_cast<updateInlineBotCallbackQuery &>(obj));
      return true;
    case updateReadChannelOutbox::ID:
      func(static_cast<updateReadChannelOutbox &>(obj));
      return true;
    case updateDraftMessage::ID:
      func(static_cast<updateDraftMessage &>(obj));
      return true;
    case updateReadFeaturedStickers::ID:
      func(static_cast<updateReadFeaturedStickers &>(obj));
      return true;
    case updateRecentStickers::ID:
      func(static_cast<updateRecentStickers &>(obj));
      return true;
    case updateConfig::ID:
      func(static_cast<updateConfig &>(obj));
      return true;
    case updatePtsChanged::ID:
      func(static_cast<updatePtsChanged &>(obj));
      return true;
    case updateChannelWebPage::ID:
      func(static_cast<updateChannelWebPage &>(obj));
      return true;
    case updateDialogPinned::ID:
      func(static_cast<updateDialogPinned &>(obj));
      return true;
    case updatePinnedDialogs::ID:
      func(static_cast<updatePinnedDialogs &>(obj));
      return true;
    case updateBotWebhookJSON::ID:
      func(static_cast<updateBotWebhookJSON &>(obj));
      return true;
    case updateBotWebhookJSONQuery::ID:
      func(static_cast<updateBotWebhookJSONQuery &>(obj));
      return true;
    case updateBotShippingQuery::ID:
      func(static_cast<updateBotShippingQuery &>(obj));
      return true;
    case updateBotPrecheckoutQuery::ID:
      func(static_cast<updateBotPrecheckoutQuery &>(obj));
      return true;
    case updatePhoneCall::ID:
      func(static_cast<updatePhoneCall &>(obj));
      return true;
    case updateLangPackTooLong::ID:
      func(static_cast<updateLangPackTooLong &>(obj));
      return true;
    case updateLangPack::ID:
      func(static_cast<updateLangPack &>(obj));
      return true;
    case updateFavedStickers::ID:
      func(static_cast<updateFavedStickers &>(obj));
      return true;
    case updateChannelReadMessagesContents::ID:
      func(static_cast<updateChannelReadMessagesContents &>(obj));
      return true;
    case updateContactsReset::ID:
      func(static_cast<updateContactsReset &>(obj));
      return true;
    case updateChannelAvailableMessages::ID:
      func(static_cast<updateChannelAvailableMessages &>(obj));
      return true;
    case updateDialogUnreadMark::ID:
      func(static_cast<updateDialogUnreadMark &>(obj));
      return true;
    case updateMessagePoll::ID:
      func(static_cast<updateMessagePoll &>(obj));
      return true;
    case updateChatDefaultBannedRights::ID:
      func(static_cast<updateChatDefaultBannedRights &>(obj));
      return true;
    case updateFolderPeers::ID:
      func(static_cast<updateFolderPeers &>(obj));
      return true;
    case updatePeerSettings::ID:
      func(static_cast<updatePeerSettings &>(obj));
      return true;
    case updatePeerLocated::ID:
      func(static_cast<updatePeerLocated &>(obj));
      return true;
    case updateNewScheduledMessage::ID:
      func(static_cast<updateNewScheduledMessage &>(obj));
      return true;
    case updateDeleteScheduledMessages::ID:
      func(static_cast<updateDeleteScheduledMessages &>(obj));
      return true;
    case updateTheme::ID:
      func(static_cast<updateTheme &>(obj));
      return true;
    case updateGeoLiveViewed::ID:
      func(static_cast<updateGeoLiveViewed &>(obj));
      return true;
    case updateLoginToken::ID:
      func(static_cast<updateLoginToken &>(obj));
      return true;
    case updateMessagePollVote::ID:
      func(static_cast<updateMessagePollVote &>(obj));
      return true;
    case updateDialogFilter::ID:
      func(static_cast<updateDialogFilter &>(obj));
      return true;
    case updateDialogFilterOrder::ID:
      func(static_cast<updateDialogFilterOrder &>(obj));
      return true;
    case updateDialogFilters::ID:
      func(static_cast<updateDialogFilters &>(obj));
      return true;
    case updatePhoneCallSignalingData::ID:
      func(static_cast<updatePhoneCallSignalingData &>(obj));
      return true;
    case updateChannelMessageForwards::ID:
      func(static_cast<updateChannelMessageForwards &>(obj));
      return true;
    case updateReadChannelDiscussionInbox::ID:
      func(static_cast<updateReadChannelDiscussionInbox &>(obj));
      return true;
    case updateReadChannelDiscussionOutbox::ID:
      func(static_cast<updateReadChannelDiscussionOutbox &>(obj));
      return true;
    case updatePeerBlocked::ID:
      func(static_cast<updatePeerBlocked &>(obj));
      return true;
    case updateChannelUserTyping::ID:
      func(static_cast<updateChannelUserTyping &>(obj));
      return true;
    case updatePinnedMessages::ID:
      func(static_cast<updatePinnedMessages &>(obj));
      return true;
    case updatePinnedChannelMessages::ID:
      func(static_cast<updatePinnedChannelMessages &>(obj));
      return true;
    case updateChat::ID:
      func(static_cast<updateChat &>(obj));
      return true;
    case updateGroupCallParticipants::ID:
      func(static_cast<updateGroupCallParticipants &>(obj));
      return true;
    case updateGroupCall::ID:
      func(static_cast<updateGroupCall &>(obj));
      return true;
    case updatePeerHistoryTTL::ID:
      func(static_cast<updatePeerHistoryTTL &>(obj));
      return true;
    case updateChatParticipant::ID:
      func(static_cast<updateChatParticipant &>(obj));
      return true;
    case updateChannelParticipant::ID:
      func(static_cast<updateChannelParticipant &>(obj));
      return true;
    case updateBotStopped::ID:
      func(static_cast<updateBotStopped &>(obj));
      return true;
    case updateGroupCallConnection::ID:
      func(static_cast<updateGroupCallConnection &>(obj));
      return true;
    case updateBotCommands::ID:
      func(static_cast<updateBotCommands &>(obj));
      return true;
    case updatePendingJoinRequests::ID:
      func(static_cast<updatePendingJoinRequests &>(obj));
      return true;
    case updateBotChatInviteRequester::ID:
      func(static_cast<updateBotChatInviteRequester &>(obj));
      return true;
    case updateMessageReactions::ID:
      func(static_cast<updateMessageReactions &>(obj));
      return true;
    case updateAttachMenuBots::ID:
      func(static_cast<updateAttachMenuBots &>(obj));
      return true;
    case updateWebViewResultSent::ID:
      func(static_cast<updateWebViewResultSent &>(obj));
      return true;
    case updateBotMenuButton::ID:
      func(static_cast<updateBotMenuButton &>(obj));
      return true;
    case updateSavedRingtones::ID:
      func(static_cast<updateSavedRingtones &>(obj));
      return true;
    case updateTranscribedAudio::ID:
      func(static_cast<updateTranscribedAudio &>(obj));
      return true;
    case updateReadFeaturedEmojiStickers::ID:
      func(static_cast<updateReadFeaturedEmojiStickers &>(obj));
      return true;
    case updateUserEmojiStatus::ID:
      func(static_cast<updateUserEmojiStatus &>(obj));
      return true;
    case updateRecentEmojiStatuses::ID:
      func(static_cast<updateRecentEmojiStatuses &>(obj));
      return true;
    case updateRecentReactions::ID:
      func(static_cast<updateRecentReactions &>(obj));
      return true;
    case updateMoveStickerSetToTop::ID:
      func(static_cast<updateMoveStickerSetToTop &>(obj));
      return true;
    case updateMessageExtendedMedia::ID:
      func(static_cast<updateMessageExtendedMedia &>(obj));
      return true;
    case updateChannelPinnedTopic::ID:
      func(static_cast<updateChannelPinnedTopic &>(obj));
      return true;
    case updateChannelPinnedTopics::ID:
      func(static_cast<updateChannelPinnedTopics &>(obj));
      return true;
    case updateUser::ID:
      func(static_cast<updateUser &>(obj));
      return true;
    case updateAutoSaveSettings::ID:
      func(static_cast<updateAutoSaveSettings &>(obj));
      return true;
    case updateGroupInvitePrivacyForbidden::ID:
      func(static_cast<updateGroupInvitePrivacyForbidden &>(obj));
      return true;
    case updatesTooLong::ID:
      func(static_cast<updatesTooLong &>(obj));
      return true;
    case updateShortMessage::ID:
      func(static_cast<updateShortMessage &>(obj));
      return true;
    case updateShortChatMessage::ID:
      func(static_cast<updateShortChatMessage &>(obj));
      return true;
    case updateShort::ID:
      func(static_cast<updateShort &>(obj));
      return true;
    case updatesCombined::ID:
      func(static_cast<updatesCombined &>(obj));
      return true;
    case updates::ID:
      func(static_cast<updates &>(obj));
      return true;
    case updateShortSentMessage::ID:
      func(static_cast<updateShortSentMessage &>(obj));
      return true;
    case urlAuthResultRequest::ID:
      func(static_cast<urlAuthResultRequest &>(obj));
      return true;
    case urlAuthResultAccepted::ID:
      func(static_cast<urlAuthResultAccepted &>(obj));
      return true;
    case urlAuthResultDefault::ID:
      func(static_cast<urlAuthResultDefault &>(obj));
      return true;
    case userEmpty::ID:
      func(static_cast<userEmpty &>(obj));
      return true;
    case user::ID:
      func(static_cast<user &>(obj));
      return true;
    case userFull::ID:
      func(static_cast<userFull &>(obj));
      return true;
    case userProfilePhotoEmpty::ID:
      func(static_cast<userProfilePhotoEmpty &>(obj));
      return true;
    case userProfilePhoto::ID:
      func(static_cast<userProfilePhoto &>(obj));
      return true;
    case userStatusEmpty::ID:
      func(static_cast<userStatusEmpty &>(obj));
      return true;
    case userStatusOnline::ID:
      func(static_cast<userStatusOnline &>(obj));
      return true;
    case userStatusOffline::ID:
      func(static_cast<userStatusOffline &>(obj));
      return true;
    case userStatusRecently::ID:
      func(static_cast<userStatusRecently &>(obj));
      return true;
    case userStatusLastWeek::ID:
      func(static_cast<userStatusLastWeek &>(obj));
      return true;
    case userStatusLastMonth::ID:
      func(static_cast<userStatusLastMonth &>(obj));
      return true;
    case username::ID:
      func(static_cast<username &>(obj));
      return true;
    case videoSize::ID:
      func(static_cast<videoSize &>(obj));
      return true;
    case videoSizeEmojiMarkup::ID:
      func(static_cast<videoSizeEmojiMarkup &>(obj));
      return true;
    case videoSizeStickerMarkup::ID:
      func(static_cast<videoSizeStickerMarkup &>(obj));
      return true;
    case wallPaper::ID:
      func(static_cast<wallPaper &>(obj));
      return true;
    case wallPaperNoFile::ID:
      func(static_cast<wallPaperNoFile &>(obj));
      return true;
    case wallPaperSettings::ID:
      func(static_cast<wallPaperSettings &>(obj));
      return true;
    case webAuthorization::ID:
      func(static_cast<webAuthorization &>(obj));
      return true;
    case webDocument::ID:
      func(static_cast<webDocument &>(obj));
      return true;
    case webDocumentNoProxy::ID:
      func(static_cast<webDocumentNoProxy &>(obj));
      return true;
    case webPageEmpty::ID:
      func(static_cast<webPageEmpty &>(obj));
      return true;
    case webPagePending::ID:
      func(static_cast<webPagePending &>(obj));
      return true;
    case webPage::ID:
      func(static_cast<webPage &>(obj));
      return true;
    case webPageNotModified::ID:
      func(static_cast<webPageNotModified &>(obj));
      return true;
    case webPageAttributeTheme::ID:
      func(static_cast<webPageAttributeTheme &>(obj));
      return true;
    case webViewMessageSent::ID:
      func(static_cast<webViewMessageSent &>(obj));
      return true;
    case webViewResultUrl::ID:
      func(static_cast<webViewResultUrl &>(obj));
      return true;
    case account_authorizationForm::ID:
      func(static_cast<account_authorizationForm &>(obj));
      return true;
    case account_authorizations::ID:
      func(static_cast<account_authorizations &>(obj));
      return true;
    case account_autoDownloadSettings::ID:
      func(static_cast<account_autoDownloadSettings &>(obj));
      return true;
    case account_autoSaveSettings::ID:
      func(static_cast<account_autoSaveSettings &>(obj));
      return true;
    case account_contentSettings::ID:
      func(static_cast<account_contentSettings &>(obj));
      return true;
    case account_emailVerified::ID:
      func(static_cast<account_emailVerified &>(obj));
      return true;
    case account_emailVerifiedLogin::ID:
      func(static_cast<account_emailVerifiedLogin &>(obj));
      return true;
    case account_emojiStatusesNotModified::ID:
      func(static_cast<account_emojiStatusesNotModified &>(obj));
      return true;
    case account_emojiStatuses::ID:
      func(static_cast<account_emojiStatuses &>(obj));
      return true;
    case account_password::ID:
      func(static_cast<account_password &>(obj));
      return true;
    case account_passwordInputSettings::ID:
      func(static_cast<account_passwordInputSettings &>(obj));
      return true;
    case account_passwordSettings::ID:
      func(static_cast<account_passwordSettings &>(obj));
      return true;
    case account_privacyRules::ID:
      func(static_cast<account_privacyRules &>(obj));
      return true;
    case account_resetPasswordFailedWait::ID:
      func(static_cast<account_resetPasswordFailedWait &>(obj));
      return true;
    case account_resetPasswordRequestedWait::ID:
      func(static_cast<account_resetPasswordRequestedWait &>(obj));
      return true;
    case account_resetPasswordOk::ID:
      func(static_cast<account_resetPasswordOk &>(obj));
      return true;
    case account_savedRingtone::ID:
      func(static_cast<account_savedRingtone &>(obj));
      return true;
    case account_savedRingtoneConverted::ID:
      func(static_cast<account_savedRingtoneConverted &>(obj));
      return true;
    case account_savedRingtonesNotModified::ID:
      func(static_cast<account_savedRingtonesNotModified &>(obj));
      return true;
    case account_savedRingtones::ID:
      func(static_cast<account_savedRingtones &>(obj));
      return true;
    case account_sentEmailCode::ID:
      func(static_cast<account_sentEmailCode &>(obj));
      return true;
    case account_takeout::ID:
      func(static_cast<account_takeout &>(obj));
      return true;
    case account_themesNotModified::ID:
      func(static_cast<account_themesNotModified &>(obj));
      return true;
    case account_themes::ID:
      func(static_cast<account_themes &>(obj));
      return true;
    case account_tmpPassword::ID:
      func(static_cast<account_tmpPassword &>(obj));
      return true;
    case account_wallPapersNotModified::ID:
      func(static_cast<account_wallPapersNotModified &>(obj));
      return true;
    case account_wallPapers::ID:
      func(static_cast<account_wallPapers &>(obj));
      return true;
    case account_webAuthorizations::ID:
      func(static_cast<account_webAuthorizations &>(obj));
      return true;
    case auth_authorization::ID:
      func(static_cast<auth_authorization &>(obj));
      return true;
    case auth_authorizationSignUpRequired::ID:
      func(static_cast<auth_authorizationSignUpRequired &>(obj));
      return true;
    case auth_codeTypeSms::ID:
      func(static_cast<auth_codeTypeSms &>(obj));
      return true;
    case auth_codeTypeCall::ID:
      func(static_cast<auth_codeTypeCall &>(obj));
      return true;
    case auth_codeTypeFlashCall::ID:
      func(static_cast<auth_codeTypeFlashCall &>(obj));
      return true;
    case auth_codeTypeMissedCall::ID:
      func(static_cast<auth_codeTypeMissedCall &>(obj));
      return true;
    case auth_codeTypeFragmentSms::ID:
      func(static_cast<auth_codeTypeFragmentSms &>(obj));
      return true;
    case auth_exportedAuthorization::ID:
      func(static_cast<auth_exportedAuthorization &>(obj));
      return true;
    case auth_loggedOut::ID:
      func(static_cast<auth_loggedOut &>(obj));
      return true;
    case auth_loginToken::ID:
      func(static_cast<auth_loginToken &>(obj));
      return true;
    case auth_loginTokenMigrateTo::ID:
      func(static_cast<auth_loginTokenMigrateTo &>(obj));
      return true;
    case auth_loginTokenSuccess::ID:
      func(static_cast<auth_loginTokenSuccess &>(obj));
      return true;
    case auth_passwordRecovery::ID:
      func(static_cast<auth_passwordRecovery &>(obj));
      return true;
    case auth_sentCode::ID:
      func(static_cast<auth_sentCode &>(obj));
      return true;
    case auth_sentCodeSuccess::ID:
      func(static_cast<auth_sentCodeSuccess &>(obj));
      return true;
    case auth_sentCodeTypeApp::ID:
      func(static_cast<auth_sentCodeTypeApp &>(obj));
      return true;
    case auth_sentCodeTypeSms::ID:
      func(static_cast<auth_sentCodeTypeSms &>(obj));
      return true;
    case auth_sentCodeTypeCall::ID:
      func(static_cast<auth_sentCodeTypeCall &>(obj));
      return true;
    case auth_sentCodeTypeFlashCall::ID:
      func(static_cast<auth_sentCodeTypeFlashCall &>(obj));
      return true;
    case auth_sentCodeTypeMissedCall::ID:
      func(static_cast<auth_sentCodeTypeMissedCall &>(obj));
      return true;
    case auth_sentCodeTypeEmailCode::ID:
      func(static_cast<auth_sentCodeTypeEmailCode &>(obj));
      return true;
    case auth_sentCodeTypeSetUpEmailRequired::ID:
      func(static_cast<auth_sentCodeTypeSetUpEmailRequired &>(obj));
      return true;
    case auth_sentCodeTypeFragmentSms::ID:
      func(static_cast<auth_sentCodeTypeFragmentSms &>(obj));
      return true;
    case auth_sentCodeTypeFirebaseSms::ID:
      func(static_cast<auth_sentCodeTypeFirebaseSms &>(obj));
      return true;
    case bots_botInfo::ID:
      func(static_cast<bots_botInfo &>(obj));
      return true;
    case channels_adminLogResults::ID:
      func(static_cast<channels_adminLogResults &>(obj));
      return true;
    case channels_channelParticipant::ID:
      func(static_cast<channels_channelParticipant &>(obj));
      return true;
    case channels_channelParticipants::ID:
      func(static_cast<channels_channelParticipants &>(obj));
      return true;
    case channels_channelParticipantsNotModified::ID:
      func(static_cast<channels_channelParticipantsNotModified &>(obj));
      return true;
    case channels_sendAsPeers::ID:
      func(static_cast<channels_sendAsPeers &>(obj));
      return true;
    case chatlists_chatlistInviteAlready::ID:
      func(static_cast<chatlists_chatlistInviteAlready &>(obj));
      return true;
    case chatlists_chatlistInvite::ID:
      func(static_cast<chatlists_chatlistInvite &>(obj));
      return true;
    case chatlists_chatlistUpdates::ID:
      func(static_cast<chatlists_chatlistUpdates &>(obj));
      return true;
    case chatlists_exportedChatlistInvite::ID:
      func(static_cast<chatlists_exportedChatlistInvite &>(obj));
      return true;
    case chatlists_exportedInvites::ID:
      func(static_cast<chatlists_exportedInvites &>(obj));
      return true;
    case contacts_blocked::ID:
      func(static_cast<contacts_blocked &>(obj));
      return true;
    case contacts_blockedSlice::ID:
      func(static_cast<contacts_blockedSlice &>(obj));
      return true;
    case contacts_contactsNotModified::ID:
      func(static_cast<contacts_contactsNotModified &>(obj));
      return true;
    case contacts_contacts::ID:
      func(static_cast<contacts_contacts &>(obj));
      return true;
    case contacts_found::ID:
      func(static_cast<contacts_found &>(obj));
      return true;
    case contacts_importedContacts::ID:
      func(static_cast<contacts_importedContacts &>(obj));
      return true;
    case contacts_resolvedPeer::ID:
      func(static_cast<contacts_resolvedPeer &>(obj));
      return true;
    case contacts_topPeersNotModified::ID:
      func(static_cast<contacts_topPeersNotModified &>(obj));
      return true;
    case contacts_topPeers::ID:
      func(static_cast<contacts_topPeers &>(obj));
      return true;
    case contacts_topPeersDisabled::ID:
      func(static_cast<contacts_topPeersDisabled &>(obj));
      return true;
    case help_appConfigNotModified::ID:
      func(static_cast<help_appConfigNotModified &>(obj));
      return true;
    case help_appConfig::ID:
      func(static_cast<help_appConfig &>(obj));
      return true;
    case help_appUpdate::ID:
      func(static_cast<help_appUpdate &>(obj));
      return true;
    case help_noAppUpdate::ID:
      func(static_cast<help_noAppUpdate &>(obj));
      return true;
    case help_configSimple::ID:
      func(static_cast<help_configSimple &>(obj));
      return true;
    case help_countriesListNotModified::ID:
      func(static_cast<help_countriesListNotModified &>(obj));
      return true;
    case help_countriesList::ID:
      func(static_cast<help_countriesList &>(obj));
      return true;
    case help_country::ID:
      func(static_cast<help_country &>(obj));
      return true;
    case help_countryCode::ID:
      func(static_cast<help_countryCode &>(obj));
      return true;
    case help_deepLinkInfoEmpty::ID:
      func(static_cast<help_deepLinkInfoEmpty &>(obj));
      return true;
    case help_deepLinkInfo::ID:
      func(static_cast<help_deepLinkInfo &>(obj));
      return true;
    case help_inviteText::ID:
      func(static_cast<help_inviteText &>(obj));
      return true;
    case help_passportConfigNotModified::ID:
      func(static_cast<help_passportConfigNotModified &>(obj));
      return true;
    case help_passportConfig::ID:
      func(static_cast<help_passportConfig &>(obj));
      return true;
    case help_premiumPromo::ID:
      func(static_cast<help_premiumPromo &>(obj));
      return true;
    case help_promoDataEmpty::ID:
      func(static_cast<help_promoDataEmpty &>(obj));
      return true;
    case help_promoData::ID:
      func(static_cast<help_promoData &>(obj));
      return true;
    case help_recentMeUrls::ID:
      func(static_cast<help_recentMeUrls &>(obj));
      return true;
    case help_support::ID:
      func(static_cast<help_support &>(obj));
      return true;
    case help_supportName::ID:
      func(static_cast<help_supportName &>(obj));
      return true;
    case help_termsOfService::ID:
      func(static_cast<help_termsOfService &>(obj));
      return true;
    case help_termsOfServiceUpdateEmpty::ID:
      func(static_cast<help_termsOfServiceUpdateEmpty &>(obj));
      return true;
    case help_termsOfServiceUpdate::ID:
      func(static_cast<help_termsOfServiceUpdate &>(obj));
      return true;
    case help_userInfoEmpty::ID:
      func(static_cast<help_userInfoEmpty &>(obj));
      return true;
    case help_userInfo::ID:
      func(static_cast<help_userInfo &>(obj));
      return true;
    case messages_affectedFoundMessages::ID:
      func(static_cast<messages_affectedFoundMessages &>(obj));
      return true;
    case messages_affectedHistory::ID:
      func(static_cast<messages_affectedHistory &>(obj));
      return true;
    case messages_affectedMessages::ID:
      func(static_cast<messages_affectedMessages &>(obj));
      return true;
    case messages_allStickersNotModified::ID:
      func(static_cast<messages_allStickersNotModified &>(obj));
      return true;
    case messages_allStickers::ID:
      func(static_cast<messages_allStickers &>(obj));
      return true;
    case messages_archivedStickers::ID:
      func(static_cast<messages_archivedStickers &>(obj));
      return true;
    case messages_availableReactionsNotModified::ID:
      func(static_cast<messages_availableReactionsNotModified &>(obj));
      return true;
    case messages_availableReactions::ID:
      func(static_cast<messages_availableReactions &>(obj));
      return true;
    case messages_botApp::ID:
      func(static_cast<messages_botApp &>(obj));
      return true;
    case messages_botCallbackAnswer::ID:
      func(static_cast<messages_botCallbackAnswer &>(obj));
      return true;
    case messages_botResults::ID:
      func(static_cast<messages_botResults &>(obj));
      return true;
    case messages_chatAdminsWithInvites::ID:
      func(static_cast<messages_chatAdminsWithInvites &>(obj));
      return true;
    case messages_chatFull::ID:
      func(static_cast<messages_chatFull &>(obj));
      return true;
    case messages_chatInviteImporters::ID:
      func(static_cast<messages_chatInviteImporters &>(obj));
      return true;
    case messages_chats::ID:
      func(static_cast<messages_chats &>(obj));
      return true;
    case messages_chatsSlice::ID:
      func(static_cast<messages_chatsSlice &>(obj));
      return true;
    case messages_checkedHistoryImportPeer::ID:
      func(static_cast<messages_checkedHistoryImportPeer &>(obj));
      return true;
    case messages_dhConfigNotModified::ID:
      func(static_cast<messages_dhConfigNotModified &>(obj));
      return true;
    case messages_dhConfig::ID:
      func(static_cast<messages_dhConfig &>(obj));
      return true;
    case messages_dialogs::ID:
      func(static_cast<messages_dialogs &>(obj));
      return true;
    case messages_dialogsSlice::ID:
      func(static_cast<messages_dialogsSlice &>(obj));
      return true;
    case messages_dialogsNotModified::ID:
      func(static_cast<messages_dialogsNotModified &>(obj));
      return true;
    case messages_discussionMessage::ID:
      func(static_cast<messages_discussionMessage &>(obj));
      return true;
    case messages_emojiGroupsNotModified::ID:
      func(static_cast<messages_emojiGroupsNotModified &>(obj));
      return true;
    case messages_emojiGroups::ID:
      func(static_cast<messages_emojiGroups &>(obj));
      return true;
    case messages_exportedChatInvite::ID:
      func(static_cast<messages_exportedChatInvite &>(obj));
      return true;
    case messages_exportedChatInviteReplaced::ID:
      func(static_cast<messages_exportedChatInviteReplaced &>(obj));
      return true;
    case messages_exportedChatInvites::ID:
      func(static_cast<messages_exportedChatInvites &>(obj));
      return true;
    case messages_favedStickersNotModified::ID:
      func(static_cast<messages_favedStickersNotModified &>(obj));
      return true;
    case messages_favedStickers::ID:
      func(static_cast<messages_favedStickers &>(obj));
      return true;
    case messages_featuredStickersNotModified::ID:
      func(static_cast<messages_featuredStickersNotModified &>(obj));
      return true;
    case messages_featuredStickers::ID:
      func(static_cast<messages_featuredStickers &>(obj));
      return true;
    case messages_forumTopics::ID:
      func(static_cast<messages_forumTopics &>(obj));
      return true;
    case messages_foundStickerSetsNotModified::ID:
      func(static_cast<messages_foundStickerSetsNotModified &>(obj));
      return true;
    case messages_foundStickerSets::ID:
      func(static_cast<messages_foundStickerSets &>(obj));
      return true;
    case messages_highScores::ID:
      func(static_cast<messages_highScores &>(obj));
      return true;
    case messages_historyImport::ID:
      func(static_cast<messages_historyImport &>(obj));
      return true;
    case messages_historyImportParsed::ID:
      func(static_cast<messages_historyImportParsed &>(obj));
      return true;
    case messages_inactiveChats::ID:
      func(static_cast<messages_inactiveChats &>(obj));
      return true;
    case messages_messageEditData::ID:
      func(static_cast<messages_messageEditData &>(obj));
      return true;
    case messages_messageReactionsList::ID:
      func(static_cast<messages_messageReactionsList &>(obj));
      return true;
    case messages_messageViews::ID:
      func(static_cast<messages_messageViews &>(obj));
      return true;
    case messages_messages::ID:
      func(static_cast<messages_messages &>(obj));
      return true;
    case messages_messagesSlice::ID:
      func(static_cast<messages_messagesSlice &>(obj));
      return true;
    case messages_channelMessages::ID:
      func(static_cast<messages_channelMessages &>(obj));
      return true;
    case messages_messagesNotModified::ID:
      func(static_cast<messages_messagesNotModified &>(obj));
      return true;
    case messages_peerDialogs::ID:
      func(static_cast<messages_peerDialogs &>(obj));
      return true;
    case messages_peerSettings::ID:
      func(static_cast<messages_peerSettings &>(obj));
      return true;
    case messages_reactionsNotModified::ID:
      func(static_cast<messages_reactionsNotModified &>(obj));
      return true;
    case messages_reactions::ID:
      func(static_cast<messages_reactions &>(obj));
      return true;
    case messages_recentStickersNotModified::ID:
      func(static_cast<messages_recentStickersNotModified &>(obj));
      return true;
    case messages_recentStickers::ID:
      func(static_cast<messages_recentStickers &>(obj));
      return true;
    case messages_savedGifsNotModified::ID:
      func(static_cast<messages_savedGifsNotModified &>(obj));
      return true;
    case messages_savedGifs::ID:
      func(static_cast<messages_savedGifs &>(obj));
      return true;
    case messages_searchCounter::ID:
      func(static_cast<messages_searchCounter &>(obj));
      return true;
    case messages_searchResultsCalendar::ID:
      func(static_cast<messages_searchResultsCalendar &>(obj));
      return true;
    case messages_searchResultsPositions::ID:
      func(static_cast<messages_searchResultsPositions &>(obj));
      return true;
    case messages_sentEncryptedMessage::ID:
      func(static_cast<messages_sentEncryptedMessage &>(obj));
      return true;
    case messages_sentEncryptedFile::ID:
      func(static_cast<messages_sentEncryptedFile &>(obj));
      return true;
    case messages_sponsoredMessages::ID:
      func(static_cast<messages_sponsoredMessages &>(obj));
      return true;
    case messages_sponsoredMessagesEmpty::ID:
      func(static_cast<messages_sponsoredMessagesEmpty &>(obj));
      return true;
    case messages_stickerSet::ID:
      func(static_cast<messages_stickerSet &>(obj));
      return true;
    case messages_stickerSetNotModified::ID:
      func(static_cast<messages_stickerSetNotModified &>(obj));
      return true;
    case messages_stickerSetInstallResultSuccess::ID:
      func(static_cast<messages_stickerSetInstallResultSuccess &>(obj));
      return true;
    case messages_stickerSetInstallResultArchive::ID:
      func(static_cast<messages_stickerSetInstallResultArchive &>(obj));
      return true;
    case messages_stickersNotModified::ID:
      func(static_cast<messages_stickersNotModified &>(obj));
      return true;
    case messages_stickers::ID:
      func(static_cast<messages_stickers &>(obj));
      return true;
    case messages_transcribedAudio::ID:
      func(static_cast<messages_transcribedAudio &>(obj));
      return true;
    case messages_translateResult::ID:
      func(static_cast<messages_translateResult &>(obj));
      return true;
    case messages_votesList::ID:
      func(static_cast<messages_votesList &>(obj));
      return true;
    case payments_bankCardData::ID:
      func(static_cast<payments_bankCardData &>(obj));
      return true;
    case payments_exportedInvoice::ID:
      func(static_cast<payments_exportedInvoice &>(obj));
      return true;
    case payments_paymentForm::ID:
      func(static_cast<payments_paymentForm &>(obj));
      return true;
    case payments_paymentReceipt::ID:
      func(static_cast<payments_paymentReceipt &>(obj));
      return true;
    case payments_paymentResult::ID:
      func(static_cast<payments_paymentResult &>(obj));
      return true;
    case payments_paymentVerificationNeeded::ID:
      func(static_cast<payments_paymentVerificationNeeded &>(obj));
      return true;
    case payments_savedInfo::ID:
      func(static_cast<payments_savedInfo &>(obj));
      return true;
    case payments_validatedRequestedInfo::ID:
      func(static_cast<payments_validatedRequestedInfo &>(obj));
      return true;
    case phone_exportedGroupCallInvite::ID:
      func(static_cast<phone_exportedGroupCallInvite &>(obj));
      return true;
    case phone_groupCall::ID:
      func(static_cast<phone_groupCall &>(obj));
      return true;
    case phone_groupCallStreamChannels::ID:
      func(static_cast<phone_groupCallStreamChannels &>(obj));
      return true;
    case phone_groupCallStreamRtmpUrl::ID:
      func(static_cast<phone_groupCallStreamRtmpUrl &>(obj));
      return true;
    case phone_groupParticipants::ID:
      func(static_cast<phone_groupParticipants &>(obj));
      return true;
    case phone_joinAsPeers::ID:
      func(static_cast<phone_joinAsPeers &>(obj));
      return true;
    case phone_phoneCall::ID:
      func(static_cast<phone_phoneCall &>(obj));
      return true;
    case photos_photo::ID:
      func(static_cast<photos_photo &>(obj));
      return true;
    case photos_photos::ID:
      func(static_cast<photos_photos &>(obj));
      return true;
    case photos_photosSlice::ID:
      func(static_cast<photos_photosSlice &>(obj));
      return true;
    case stats_broadcastStats::ID:
      func(static_cast<stats_broadcastStats &>(obj));
      return true;
    case stats_megagroupStats::ID:
      func(static_cast<stats_megagroupStats &>(obj));
      return true;
    case stats_messageStats::ID:
      func(static_cast<stats_messageStats &>(obj));
      return true;
    case stickers_suggestedShortName::ID:
      func(static_cast<stickers_suggestedShortName &>(obj));
      return true;
    case storage_fileUnknown::ID:
      func(static_cast<storage_fileUnknown &>(obj));
      return true;
    case storage_filePartial::ID:
      func(static_cast<storage_filePartial &>(obj));
      return true;
    case storage_fileJpeg::ID:
      func(static_cast<storage_fileJpeg &>(obj));
      return true;
    case storage_fileGif::ID:
      func(static_cast<storage_fileGif &>(obj));
      return true;
    case storage_filePng::ID:
      func(static_cast<storage_filePng &>(obj));
      return true;
    case storage_filePdf::ID:
      func(static_cast<storage_filePdf &>(obj));
      return true;
    case storage_fileMp3::ID:
      func(static_cast<storage_fileMp3 &>(obj));
      return true;
    case storage_fileMov::ID:
      func(static_cast<storage_fileMov &>(obj));
      return true;
    case storage_fileMp4::ID:
      func(static_cast<storage_fileMp4 &>(obj));
      return true;
    case storage_fileWebp::ID:
      func(static_cast<storage_fileWebp &>(obj));
      return true;
    case updates_channelDifferenceEmpty::ID:
      func(static_cast<updates_channelDifferenceEmpty &>(obj));
      return true;
    case updates_channelDifferenceTooLong::ID:
      func(static_cast<updates_channelDifferenceTooLong &>(obj));
      return true;
    case updates_channelDifference::ID:
      func(static_cast<updates_channelDifference &>(obj));
      return true;
    case updates_differenceEmpty::ID:
      func(static_cast<updates_differenceEmpty &>(obj));
      return true;
    case updates_difference::ID:
      func(static_cast<updates_difference &>(obj));
      return true;
    case updates_differenceSlice::ID:
      func(static_cast<updates_differenceSlice &>(obj));
      return true;
    case updates_differenceTooLong::ID:
      func(static_cast<updates_differenceTooLong &>(obj));
      return true;
    case updates_state::ID:
      func(static_cast<updates_state &>(obj));
      return true;
    case upload_cdnFileReuploadNeeded::ID:
      func(static_cast<upload_cdnFileReuploadNeeded &>(obj));
      return true;
    case upload_cdnFile::ID:
      func(static_cast<upload_cdnFile &>(obj));
      return true;
    case upload_file::ID:
      func(static_cast<upload_file &>(obj));
      return true;
    case upload_fileCdnRedirect::ID:
      func(static_cast<upload_fileCdnRedirect &>(obj));
      return true;
    case upload_webFile::ID:
      func(static_cast<upload_webFile &>(obj));
      return true;
    case users_userFull::ID:
      func(static_cast<users_userFull &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(Function &obj, const T &func) {
  switch (obj.get_id()) {
    case account_acceptAuthorization::ID:
      func(static_cast<account_acceptAuthorization &>(obj));
      return true;
    case account_cancelPasswordEmail::ID:
      func(static_cast<account_cancelPasswordEmail &>(obj));
      return true;
    case account_changeAuthorizationSettings::ID:
      func(static_cast<account_changeAuthorizationSettings &>(obj));
      return true;
    case account_changePhone::ID:
      func(static_cast<account_changePhone &>(obj));
      return true;
    case account_checkUsername::ID:
      func(static_cast<account_checkUsername &>(obj));
      return true;
    case account_clearRecentEmojiStatuses::ID:
      func(static_cast<account_clearRecentEmojiStatuses &>(obj));
      return true;
    case account_confirmPasswordEmail::ID:
      func(static_cast<account_confirmPasswordEmail &>(obj));
      return true;
    case account_confirmPhone::ID:
      func(static_cast<account_confirmPhone &>(obj));
      return true;
    case account_createTheme::ID:
      func(static_cast<account_createTheme &>(obj));
      return true;
    case account_declinePasswordReset::ID:
      func(static_cast<account_declinePasswordReset &>(obj));
      return true;
    case account_deleteAccount::ID:
      func(static_cast<account_deleteAccount &>(obj));
      return true;
    case account_deleteAutoSaveExceptions::ID:
      func(static_cast<account_deleteAutoSaveExceptions &>(obj));
      return true;
    case account_deleteSecureValue::ID:
      func(static_cast<account_deleteSecureValue &>(obj));
      return true;
    case account_finishTakeoutSession::ID:
      func(static_cast<account_finishTakeoutSession &>(obj));
      return true;
    case account_getAccountTTL::ID:
      func(static_cast<account_getAccountTTL &>(obj));
      return true;
    case account_getAllSecureValues::ID:
      func(static_cast<account_getAllSecureValues &>(obj));
      return true;
    case account_getAuthorizationForm::ID:
      func(static_cast<account_getAuthorizationForm &>(obj));
      return true;
    case account_getAuthorizations::ID:
      func(static_cast<account_getAuthorizations &>(obj));
      return true;
    case account_getAutoDownloadSettings::ID:
      func(static_cast<account_getAutoDownloadSettings &>(obj));
      return true;
    case account_getAutoSaveSettings::ID:
      func(static_cast<account_getAutoSaveSettings &>(obj));
      return true;
    case account_getChatThemes::ID:
      func(static_cast<account_getChatThemes &>(obj));
      return true;
    case account_getContactSignUpNotification::ID:
      func(static_cast<account_getContactSignUpNotification &>(obj));
      return true;
    case account_getContentSettings::ID:
      func(static_cast<account_getContentSettings &>(obj));
      return true;
    case account_getDefaultEmojiStatuses::ID:
      func(static_cast<account_getDefaultEmojiStatuses &>(obj));
      return true;
    case account_getDefaultGroupPhotoEmojis::ID:
      func(static_cast<account_getDefaultGroupPhotoEmojis &>(obj));
      return true;
    case account_getDefaultProfilePhotoEmojis::ID:
      func(static_cast<account_getDefaultProfilePhotoEmojis &>(obj));
      return true;
    case account_getGlobalPrivacySettings::ID:
      func(static_cast<account_getGlobalPrivacySettings &>(obj));
      return true;
    case account_getMultiWallPapers::ID:
      func(static_cast<account_getMultiWallPapers &>(obj));
      return true;
    case account_getNotifyExceptions::ID:
      func(static_cast<account_getNotifyExceptions &>(obj));
      return true;
    case account_getNotifySettings::ID:
      func(static_cast<account_getNotifySettings &>(obj));
      return true;
    case account_getPassword::ID:
      func(static_cast<account_getPassword &>(obj));
      return true;
    case account_getPasswordSettings::ID:
      func(static_cast<account_getPasswordSettings &>(obj));
      return true;
    case account_getPrivacy::ID:
      func(static_cast<account_getPrivacy &>(obj));
      return true;
    case account_getRecentEmojiStatuses::ID:
      func(static_cast<account_getRecentEmojiStatuses &>(obj));
      return true;
    case account_getSavedRingtones::ID:
      func(static_cast<account_getSavedRingtones &>(obj));
      return true;
    case account_getSecureValue::ID:
      func(static_cast<account_getSecureValue &>(obj));
      return true;
    case account_getTheme::ID:
      func(static_cast<account_getTheme &>(obj));
      return true;
    case account_getThemes::ID:
      func(static_cast<account_getThemes &>(obj));
      return true;
    case account_getTmpPassword::ID:
      func(static_cast<account_getTmpPassword &>(obj));
      return true;
    case account_getWallPaper::ID:
      func(static_cast<account_getWallPaper &>(obj));
      return true;
    case account_getWallPapers::ID:
      func(static_cast<account_getWallPapers &>(obj));
      return true;
    case account_getWebAuthorizations::ID:
      func(static_cast<account_getWebAuthorizations &>(obj));
      return true;
    case account_initTakeoutSession::ID:
      func(static_cast<account_initTakeoutSession &>(obj));
      return true;
    case account_installTheme::ID:
      func(static_cast<account_installTheme &>(obj));
      return true;
    case account_installWallPaper::ID:
      func(static_cast<account_installWallPaper &>(obj));
      return true;
    case account_registerDevice::ID:
      func(static_cast<account_registerDevice &>(obj));
      return true;
    case account_reorderUsernames::ID:
      func(static_cast<account_reorderUsernames &>(obj));
      return true;
    case account_reportPeer::ID:
      func(static_cast<account_reportPeer &>(obj));
      return true;
    case account_reportProfilePhoto::ID:
      func(static_cast<account_reportProfilePhoto &>(obj));
      return true;
    case account_resendPasswordEmail::ID:
      func(static_cast<account_resendPasswordEmail &>(obj));
      return true;
    case account_resetAuthorization::ID:
      func(static_cast<account_resetAuthorization &>(obj));
      return true;
    case account_resetNotifySettings::ID:
      func(static_cast<account_resetNotifySettings &>(obj));
      return true;
    case account_resetPassword::ID:
      func(static_cast<account_resetPassword &>(obj));
      return true;
    case account_resetWallPapers::ID:
      func(static_cast<account_resetWallPapers &>(obj));
      return true;
    case account_resetWebAuthorization::ID:
      func(static_cast<account_resetWebAuthorization &>(obj));
      return true;
    case account_resetWebAuthorizations::ID:
      func(static_cast<account_resetWebAuthorizations &>(obj));
      return true;
    case account_saveAutoDownloadSettings::ID:
      func(static_cast<account_saveAutoDownloadSettings &>(obj));
      return true;
    case account_saveAutoSaveSettings::ID:
      func(static_cast<account_saveAutoSaveSettings &>(obj));
      return true;
    case account_saveRingtone::ID:
      func(static_cast<account_saveRingtone &>(obj));
      return true;
    case account_saveSecureValue::ID:
      func(static_cast<account_saveSecureValue &>(obj));
      return true;
    case account_saveTheme::ID:
      func(static_cast<account_saveTheme &>(obj));
      return true;
    case account_saveWallPaper::ID:
      func(static_cast<account_saveWallPaper &>(obj));
      return true;
    case account_sendChangePhoneCode::ID:
      func(static_cast<account_sendChangePhoneCode &>(obj));
      return true;
    case account_sendConfirmPhoneCode::ID:
      func(static_cast<account_sendConfirmPhoneCode &>(obj));
      return true;
    case account_sendVerifyEmailCode::ID:
      func(static_cast<account_sendVerifyEmailCode &>(obj));
      return true;
    case account_sendVerifyPhoneCode::ID:
      func(static_cast<account_sendVerifyPhoneCode &>(obj));
      return true;
    case account_setAccountTTL::ID:
      func(static_cast<account_setAccountTTL &>(obj));
      return true;
    case account_setAuthorizationTTL::ID:
      func(static_cast<account_setAuthorizationTTL &>(obj));
      return true;
    case account_setContactSignUpNotification::ID:
      func(static_cast<account_setContactSignUpNotification &>(obj));
      return true;
    case account_setContentSettings::ID:
      func(static_cast<account_setContentSettings &>(obj));
      return true;
    case account_setGlobalPrivacySettings::ID:
      func(static_cast<account_setGlobalPrivacySettings &>(obj));
      return true;
    case account_setPrivacy::ID:
      func(static_cast<account_setPrivacy &>(obj));
      return true;
    case account_toggleUsername::ID:
      func(static_cast<account_toggleUsername &>(obj));
      return true;
    case account_unregisterDevice::ID:
      func(static_cast<account_unregisterDevice &>(obj));
      return true;
    case account_updateDeviceLocked::ID:
      func(static_cast<account_updateDeviceLocked &>(obj));
      return true;
    case account_updateEmojiStatus::ID:
      func(static_cast<account_updateEmojiStatus &>(obj));
      return true;
    case account_updateNotifySettings::ID:
      func(static_cast<account_updateNotifySettings &>(obj));
      return true;
    case account_updatePasswordSettings::ID:
      func(static_cast<account_updatePasswordSettings &>(obj));
      return true;
    case account_updateProfile::ID:
      func(static_cast<account_updateProfile &>(obj));
      return true;
    case account_updateStatus::ID:
      func(static_cast<account_updateStatus &>(obj));
      return true;
    case account_updateTheme::ID:
      func(static_cast<account_updateTheme &>(obj));
      return true;
    case account_updateUsername::ID:
      func(static_cast<account_updateUsername &>(obj));
      return true;
    case account_uploadRingtone::ID:
      func(static_cast<account_uploadRingtone &>(obj));
      return true;
    case account_uploadTheme::ID:
      func(static_cast<account_uploadTheme &>(obj));
      return true;
    case account_uploadWallPaper::ID:
      func(static_cast<account_uploadWallPaper &>(obj));
      return true;
    case account_verifyEmail::ID:
      func(static_cast<account_verifyEmail &>(obj));
      return true;
    case account_verifyPhone::ID:
      func(static_cast<account_verifyPhone &>(obj));
      return true;
    case auth_acceptLoginToken::ID:
      func(static_cast<auth_acceptLoginToken &>(obj));
      return true;
    case auth_bindTempAuthKey::ID:
      func(static_cast<auth_bindTempAuthKey &>(obj));
      return true;
    case auth_cancelCode::ID:
      func(static_cast<auth_cancelCode &>(obj));
      return true;
    case auth_checkPassword::ID:
      func(static_cast<auth_checkPassword &>(obj));
      return true;
    case auth_checkRecoveryPassword::ID:
      func(static_cast<auth_checkRecoveryPassword &>(obj));
      return true;
    case auth_dropTempAuthKeys::ID:
      func(static_cast<auth_dropTempAuthKeys &>(obj));
      return true;
    case auth_exportAuthorization::ID:
      func(static_cast<auth_exportAuthorization &>(obj));
      return true;
    case auth_exportLoginToken::ID:
      func(static_cast<auth_exportLoginToken &>(obj));
      return true;
    case auth_importAuthorization::ID:
      func(static_cast<auth_importAuthorization &>(obj));
      return true;
    case auth_importBotAuthorization::ID:
      func(static_cast<auth_importBotAuthorization &>(obj));
      return true;
    case auth_importLoginToken::ID:
      func(static_cast<auth_importLoginToken &>(obj));
      return true;
    case auth_importWebTokenAuthorization::ID:
      func(static_cast<auth_importWebTokenAuthorization &>(obj));
      return true;
    case auth_logOut::ID:
      func(static_cast<auth_logOut &>(obj));
      return true;
    case auth_recoverPassword::ID:
      func(static_cast<auth_recoverPassword &>(obj));
      return true;
    case auth_requestFirebaseSms::ID:
      func(static_cast<auth_requestFirebaseSms &>(obj));
      return true;
    case auth_requestPasswordRecovery::ID:
      func(static_cast<auth_requestPasswordRecovery &>(obj));
      return true;
    case auth_resendCode::ID:
      func(static_cast<auth_resendCode &>(obj));
      return true;
    case auth_resetAuthorizations::ID:
      func(static_cast<auth_resetAuthorizations &>(obj));
      return true;
    case auth_resetLoginEmail::ID:
      func(static_cast<auth_resetLoginEmail &>(obj));
      return true;
    case auth_sendCode::ID:
      func(static_cast<auth_sendCode &>(obj));
      return true;
    case auth_signIn::ID:
      func(static_cast<auth_signIn &>(obj));
      return true;
    case auth_signUp::ID:
      func(static_cast<auth_signUp &>(obj));
      return true;
    case bots_answerWebhookJSONQuery::ID:
      func(static_cast<bots_answerWebhookJSONQuery &>(obj));
      return true;
    case bots_getBotCommands::ID:
      func(static_cast<bots_getBotCommands &>(obj));
      return true;
    case bots_getBotInfo::ID:
      func(static_cast<bots_getBotInfo &>(obj));
      return true;
    case bots_getBotMenuButton::ID:
      func(static_cast<bots_getBotMenuButton &>(obj));
      return true;
    case bots_reorderUsernames::ID:
      func(static_cast<bots_reorderUsernames &>(obj));
      return true;
    case bots_resetBotCommands::ID:
      func(static_cast<bots_resetBotCommands &>(obj));
      return true;
    case bots_sendCustomRequest::ID:
      func(static_cast<bots_sendCustomRequest &>(obj));
      return true;
    case bots_setBotBroadcastDefaultAdminRights::ID:
      func(static_cast<bots_setBotBroadcastDefaultAdminRights &>(obj));
      return true;
    case bots_setBotCommands::ID:
      func(static_cast<bots_setBotCommands &>(obj));
      return true;
    case bots_setBotGroupDefaultAdminRights::ID:
      func(static_cast<bots_setBotGroupDefaultAdminRights &>(obj));
      return true;
    case bots_setBotInfo::ID:
      func(static_cast<bots_setBotInfo &>(obj));
      return true;
    case bots_setBotMenuButton::ID:
      func(static_cast<bots_setBotMenuButton &>(obj));
      return true;
    case bots_toggleUsername::ID:
      func(static_cast<bots_toggleUsername &>(obj));
      return true;
    case channels_checkUsername::ID:
      func(static_cast<channels_checkUsername &>(obj));
      return true;
    case channels_convertToGigagroup::ID:
      func(static_cast<channels_convertToGigagroup &>(obj));
      return true;
    case channels_createChannel::ID:
      func(static_cast<channels_createChannel &>(obj));
      return true;
    case channels_createForumTopic::ID:
      func(static_cast<channels_createForumTopic &>(obj));
      return true;
    case channels_deactivateAllUsernames::ID:
      func(static_cast<channels_deactivateAllUsernames &>(obj));
      return true;
    case channels_deleteChannel::ID:
      func(static_cast<channels_deleteChannel &>(obj));
      return true;
    case channels_deleteHistory::ID:
      func(static_cast<channels_deleteHistory &>(obj));
      return true;
    case channels_deleteMessages::ID:
      func(static_cast<channels_deleteMessages &>(obj));
      return true;
    case channels_deleteParticipantHistory::ID:
      func(static_cast<channels_deleteParticipantHistory &>(obj));
      return true;
    case channels_deleteTopicHistory::ID:
      func(static_cast<channels_deleteTopicHistory &>(obj));
      return true;
    case channels_editAdmin::ID:
      func(static_cast<channels_editAdmin &>(obj));
      return true;
    case channels_editBanned::ID:
      func(static_cast<channels_editBanned &>(obj));
      return true;
    case channels_editCreator::ID:
      func(static_cast<channels_editCreator &>(obj));
      return true;
    case channels_editForumTopic::ID:
      func(static_cast<channels_editForumTopic &>(obj));
      return true;
    case channels_editLocation::ID:
      func(static_cast<channels_editLocation &>(obj));
      return true;
    case channels_editPhoto::ID:
      func(static_cast<channels_editPhoto &>(obj));
      return true;
    case channels_editTitle::ID:
      func(static_cast<channels_editTitle &>(obj));
      return true;
    case channels_exportMessageLink::ID:
      func(static_cast<channels_exportMessageLink &>(obj));
      return true;
    case channels_getAdminLog::ID:
      func(static_cast<channels_getAdminLog &>(obj));
      return true;
    case channels_getAdminedPublicChannels::ID:
      func(static_cast<channels_getAdminedPublicChannels &>(obj));
      return true;
    case channels_getChannels::ID:
      func(static_cast<channels_getChannels &>(obj));
      return true;
    case channels_getForumTopics::ID:
      func(static_cast<channels_getForumTopics &>(obj));
      return true;
    case channels_getForumTopicsByID::ID:
      func(static_cast<channels_getForumTopicsByID &>(obj));
      return true;
    case channels_getFullChannel::ID:
      func(static_cast<channels_getFullChannel &>(obj));
      return true;
    case channels_getGroupsForDiscussion::ID:
      func(static_cast<channels_getGroupsForDiscussion &>(obj));
      return true;
    case channels_getInactiveChannels::ID:
      func(static_cast<channels_getInactiveChannels &>(obj));
      return true;
    case channels_getLeftChannels::ID:
      func(static_cast<channels_getLeftChannels &>(obj));
      return true;
    case channels_getMessages::ID:
      func(static_cast<channels_getMessages &>(obj));
      return true;
    case channels_getParticipant::ID:
      func(static_cast<channels_getParticipant &>(obj));
      return true;
    case channels_getParticipants::ID:
      func(static_cast<channels_getParticipants &>(obj));
      return true;
    case channels_getSendAs::ID:
      func(static_cast<channels_getSendAs &>(obj));
      return true;
    case channels_getSponsoredMessages::ID:
      func(static_cast<channels_getSponsoredMessages &>(obj));
      return true;
    case channels_inviteToChannel::ID:
      func(static_cast<channels_inviteToChannel &>(obj));
      return true;
    case channels_joinChannel::ID:
      func(static_cast<channels_joinChannel &>(obj));
      return true;
    case channels_leaveChannel::ID:
      func(static_cast<channels_leaveChannel &>(obj));
      return true;
    case channels_readHistory::ID:
      func(static_cast<channels_readHistory &>(obj));
      return true;
    case channels_readMessageContents::ID:
      func(static_cast<channels_readMessageContents &>(obj));
      return true;
    case channels_reorderPinnedForumTopics::ID:
      func(static_cast<channels_reorderPinnedForumTopics &>(obj));
      return true;
    case channels_reorderUsernames::ID:
      func(static_cast<channels_reorderUsernames &>(obj));
      return true;
    case channels_reportAntiSpamFalsePositive::ID:
      func(static_cast<channels_reportAntiSpamFalsePositive &>(obj));
      return true;
    case channels_reportSpam::ID:
      func(static_cast<channels_reportSpam &>(obj));
      return true;
    case channels_setDiscussionGroup::ID:
      func(static_cast<channels_setDiscussionGroup &>(obj));
      return true;
    case channels_setStickers::ID:
      func(static_cast<channels_setStickers &>(obj));
      return true;
    case channels_toggleAntiSpam::ID:
      func(static_cast<channels_toggleAntiSpam &>(obj));
      return true;
    case channels_toggleForum::ID:
      func(static_cast<channels_toggleForum &>(obj));
      return true;
    case channels_toggleJoinRequest::ID:
      func(static_cast<channels_toggleJoinRequest &>(obj));
      return true;
    case channels_toggleJoinToSend::ID:
      func(static_cast<channels_toggleJoinToSend &>(obj));
      return true;
    case channels_toggleParticipantsHidden::ID:
      func(static_cast<channels_toggleParticipantsHidden &>(obj));
      return true;
    case channels_togglePreHistoryHidden::ID:
      func(static_cast<channels_togglePreHistoryHidden &>(obj));
      return true;
    case channels_toggleSignatures::ID:
      func(static_cast<channels_toggleSignatures &>(obj));
      return true;
    case channels_toggleSlowMode::ID:
      func(static_cast<channels_toggleSlowMode &>(obj));
      return true;
    case channels_toggleUsername::ID:
      func(static_cast<channels_toggleUsername &>(obj));
      return true;
    case channels_updatePinnedForumTopic::ID:
      func(static_cast<channels_updatePinnedForumTopic &>(obj));
      return true;
    case channels_updateUsername::ID:
      func(static_cast<channels_updateUsername &>(obj));
      return true;
    case channels_viewSponsoredMessage::ID:
      func(static_cast<channels_viewSponsoredMessage &>(obj));
      return true;
    case chatlists_checkChatlistInvite::ID:
      func(static_cast<chatlists_checkChatlistInvite &>(obj));
      return true;
    case chatlists_deleteExportedInvite::ID:
      func(static_cast<chatlists_deleteExportedInvite &>(obj));
      return true;
    case chatlists_editExportedInvite::ID:
      func(static_cast<chatlists_editExportedInvite &>(obj));
      return true;
    case chatlists_exportChatlistInvite::ID:
      func(static_cast<chatlists_exportChatlistInvite &>(obj));
      return true;
    case chatlists_getChatlistUpdates::ID:
      func(static_cast<chatlists_getChatlistUpdates &>(obj));
      return true;
    case chatlists_getExportedInvites::ID:
      func(static_cast<chatlists_getExportedInvites &>(obj));
      return true;
    case chatlists_getLeaveChatlistSuggestions::ID:
      func(static_cast<chatlists_getLeaveChatlistSuggestions &>(obj));
      return true;
    case chatlists_hideChatlistUpdates::ID:
      func(static_cast<chatlists_hideChatlistUpdates &>(obj));
      return true;
    case chatlists_joinChatlistInvite::ID:
      func(static_cast<chatlists_joinChatlistInvite &>(obj));
      return true;
    case chatlists_joinChatlistUpdates::ID:
      func(static_cast<chatlists_joinChatlistUpdates &>(obj));
      return true;
    case chatlists_leaveChatlist::ID:
      func(static_cast<chatlists_leaveChatlist &>(obj));
      return true;
    case contacts_acceptContact::ID:
      func(static_cast<contacts_acceptContact &>(obj));
      return true;
    case contacts_addContact::ID:
      func(static_cast<contacts_addContact &>(obj));
      return true;
    case contacts_block::ID:
      func(static_cast<contacts_block &>(obj));
      return true;
    case contacts_blockFromReplies::ID:
      func(static_cast<contacts_blockFromReplies &>(obj));
      return true;
    case contacts_deleteByPhones::ID:
      func(static_cast<contacts_deleteByPhones &>(obj));
      return true;
    case contacts_deleteContacts::ID:
      func(static_cast<contacts_deleteContacts &>(obj));
      return true;
    case contacts_exportContactToken::ID:
      func(static_cast<contacts_exportContactToken &>(obj));
      return true;
    case contacts_getBlocked::ID:
      func(static_cast<contacts_getBlocked &>(obj));
      return true;
    case contacts_getContactIDs::ID:
      func(static_cast<contacts_getContactIDs &>(obj));
      return true;
    case contacts_getContacts::ID:
      func(static_cast<contacts_getContacts &>(obj));
      return true;
    case contacts_getLocated::ID:
      func(static_cast<contacts_getLocated &>(obj));
      return true;
    case contacts_getSaved::ID:
      func(static_cast<contacts_getSaved &>(obj));
      return true;
    case contacts_getStatuses::ID:
      func(static_cast<contacts_getStatuses &>(obj));
      return true;
    case contacts_getTopPeers::ID:
      func(static_cast<contacts_getTopPeers &>(obj));
      return true;
    case contacts_importContactToken::ID:
      func(static_cast<contacts_importContactToken &>(obj));
      return true;
    case contacts_importContacts::ID:
      func(static_cast<contacts_importContacts &>(obj));
      return true;
    case contacts_resetSaved::ID:
      func(static_cast<contacts_resetSaved &>(obj));
      return true;
    case contacts_resetTopPeerRating::ID:
      func(static_cast<contacts_resetTopPeerRating &>(obj));
      return true;
    case contacts_resolvePhone::ID:
      func(static_cast<contacts_resolvePhone &>(obj));
      return true;
    case contacts_resolveUsername::ID:
      func(static_cast<contacts_resolveUsername &>(obj));
      return true;
    case contacts_search::ID:
      func(static_cast<contacts_search &>(obj));
      return true;
    case contacts_toggleTopPeers::ID:
      func(static_cast<contacts_toggleTopPeers &>(obj));
      return true;
    case contacts_unblock::ID:
      func(static_cast<contacts_unblock &>(obj));
      return true;
    case folders_editPeerFolders::ID:
      func(static_cast<folders_editPeerFolders &>(obj));
      return true;
    case help_acceptTermsOfService::ID:
      func(static_cast<help_acceptTermsOfService &>(obj));
      return true;
    case help_dismissSuggestion::ID:
      func(static_cast<help_dismissSuggestion &>(obj));
      return true;
    case help_editUserInfo::ID:
      func(static_cast<help_editUserInfo &>(obj));
      return true;
    case help_getAppChangelog::ID:
      func(static_cast<help_getAppChangelog &>(obj));
      return true;
    case help_getAppConfig::ID:
      func(static_cast<help_getAppConfig &>(obj));
      return true;
    case help_getAppUpdate::ID:
      func(static_cast<help_getAppUpdate &>(obj));
      return true;
    case help_getCdnConfig::ID:
      func(static_cast<help_getCdnConfig &>(obj));
      return true;
    case help_getConfig::ID:
      func(static_cast<help_getConfig &>(obj));
      return true;
    case help_getCountriesList::ID:
      func(static_cast<help_getCountriesList &>(obj));
      return true;
    case help_getDeepLinkInfo::ID:
      func(static_cast<help_getDeepLinkInfo &>(obj));
      return true;
    case help_getInviteText::ID:
      func(static_cast<help_getInviteText &>(obj));
      return true;
    case help_getNearestDc::ID:
      func(static_cast<help_getNearestDc &>(obj));
      return true;
    case help_getPassportConfig::ID:
      func(static_cast<help_getPassportConfig &>(obj));
      return true;
    case help_getPremiumPromo::ID:
      func(static_cast<help_getPremiumPromo &>(obj));
      return true;
    case help_getPromoData::ID:
      func(static_cast<help_getPromoData &>(obj));
      return true;
    case help_getRecentMeUrls::ID:
      func(static_cast<help_getRecentMeUrls &>(obj));
      return true;
    case help_getSupport::ID:
      func(static_cast<help_getSupport &>(obj));
      return true;
    case help_getSupportName::ID:
      func(static_cast<help_getSupportName &>(obj));
      return true;
    case help_getTermsOfServiceUpdate::ID:
      func(static_cast<help_getTermsOfServiceUpdate &>(obj));
      return true;
    case help_getUserInfo::ID:
      func(static_cast<help_getUserInfo &>(obj));
      return true;
    case help_hidePromoData::ID:
      func(static_cast<help_hidePromoData &>(obj));
      return true;
    case help_saveAppLog::ID:
      func(static_cast<help_saveAppLog &>(obj));
      return true;
    case help_setBotUpdatesStatus::ID:
      func(static_cast<help_setBotUpdatesStatus &>(obj));
      return true;
    case langpack_getDifference::ID:
      func(static_cast<langpack_getDifference &>(obj));
      return true;
    case langpack_getLangPack::ID:
      func(static_cast<langpack_getLangPack &>(obj));
      return true;
    case langpack_getLanguage::ID:
      func(static_cast<langpack_getLanguage &>(obj));
      return true;
    case langpack_getLanguages::ID:
      func(static_cast<langpack_getLanguages &>(obj));
      return true;
    case langpack_getStrings::ID:
      func(static_cast<langpack_getStrings &>(obj));
      return true;
    case messages_acceptEncryption::ID:
      func(static_cast<messages_acceptEncryption &>(obj));
      return true;
    case messages_acceptUrlAuth::ID:
      func(static_cast<messages_acceptUrlAuth &>(obj));
      return true;
    case messages_addChatUser::ID:
      func(static_cast<messages_addChatUser &>(obj));
      return true;
    case messages_checkChatInvite::ID:
      func(static_cast<messages_checkChatInvite &>(obj));
      return true;
    case messages_checkHistoryImport::ID:
      func(static_cast<messages_checkHistoryImport &>(obj));
      return true;
    case messages_checkHistoryImportPeer::ID:
      func(static_cast<messages_checkHistoryImportPeer &>(obj));
      return true;
    case messages_clearAllDrafts::ID:
      func(static_cast<messages_clearAllDrafts &>(obj));
      return true;
    case messages_clearRecentReactions::ID:
      func(static_cast<messages_clearRecentReactions &>(obj));
      return true;
    case messages_clearRecentStickers::ID:
      func(static_cast<messages_clearRecentStickers &>(obj));
      return true;
    case messages_createChat::ID:
      func(static_cast<messages_createChat &>(obj));
      return true;
    case messages_deleteChat::ID:
      func(static_cast<messages_deleteChat &>(obj));
      return true;
    case messages_deleteChatUser::ID:
      func(static_cast<messages_deleteChatUser &>(obj));
      return true;
    case messages_deleteExportedChatInvite::ID:
      func(static_cast<messages_deleteExportedChatInvite &>(obj));
      return true;
    case messages_deleteHistory::ID:
      func(static_cast<messages_deleteHistory &>(obj));
      return true;
    case messages_deleteMessages::ID:
      func(static_cast<messages_deleteMessages &>(obj));
      return true;
    case messages_deletePhoneCallHistory::ID:
      func(static_cast<messages_deletePhoneCallHistory &>(obj));
      return true;
    case messages_deleteRevokedExportedChatInvites::ID:
      func(static_cast<messages_deleteRevokedExportedChatInvites &>(obj));
      return true;
    case messages_deleteScheduledMessages::ID:
      func(static_cast<messages_deleteScheduledMessages &>(obj));
      return true;
    case messages_discardEncryption::ID:
      func(static_cast<messages_discardEncryption &>(obj));
      return true;
    case messages_editChatAbout::ID:
      func(static_cast<messages_editChatAbout &>(obj));
      return true;
    case messages_editChatAdmin::ID:
      func(static_cast<messages_editChatAdmin &>(obj));
      return true;
    case messages_editChatDefaultBannedRights::ID:
      func(static_cast<messages_editChatDefaultBannedRights &>(obj));
      return true;
    case messages_editChatPhoto::ID:
      func(static_cast<messages_editChatPhoto &>(obj));
      return true;
    case messages_editChatTitle::ID:
      func(static_cast<messages_editChatTitle &>(obj));
      return true;
    case messages_editExportedChatInvite::ID:
      func(static_cast<messages_editExportedChatInvite &>(obj));
      return true;
    case messages_editInlineBotMessage::ID:
      func(static_cast<messages_editInlineBotMessage &>(obj));
      return true;
    case messages_editMessage::ID:
      func(static_cast<messages_editMessage &>(obj));
      return true;
    case messages_exportChatInvite::ID:
      func(static_cast<messages_exportChatInvite &>(obj));
      return true;
    case messages_faveSticker::ID:
      func(static_cast<messages_faveSticker &>(obj));
      return true;
    case messages_forwardMessages::ID:
      func(static_cast<messages_forwardMessages &>(obj));
      return true;
    case messages_getAdminsWithInvites::ID:
      func(static_cast<messages_getAdminsWithInvites &>(obj));
      return true;
    case messages_getAllChats::ID:
      func(static_cast<messages_getAllChats &>(obj));
      return true;
    case messages_getAllDrafts::ID:
      func(static_cast<messages_getAllDrafts &>(obj));
      return true;
    case messages_getAllStickers::ID:
      func(static_cast<messages_getAllStickers &>(obj));
      return true;
    case messages_getArchivedStickers::ID:
      func(static_cast<messages_getArchivedStickers &>(obj));
      return true;
    case messages_getAttachMenuBot::ID:
      func(static_cast<messages_getAttachMenuBot &>(obj));
      return true;
    case messages_getAttachMenuBots::ID:
      func(static_cast<messages_getAttachMenuBots &>(obj));
      return true;
    case messages_getAttachedStickers::ID:
      func(static_cast<messages_getAttachedStickers &>(obj));
      return true;
    case messages_getAvailableReactions::ID:
      func(static_cast<messages_getAvailableReactions &>(obj));
      return true;
    case messages_getBotApp::ID:
      func(static_cast<messages_getBotApp &>(obj));
      return true;
    case messages_getBotCallbackAnswer::ID:
      func(static_cast<messages_getBotCallbackAnswer &>(obj));
      return true;
    case messages_getChatInviteImporters::ID:
      func(static_cast<messages_getChatInviteImporters &>(obj));
      return true;
    case messages_getChats::ID:
      func(static_cast<messages_getChats &>(obj));
      return true;
    case messages_getCommonChats::ID:
      func(static_cast<messages_getCommonChats &>(obj));
      return true;
    case messages_getCustomEmojiDocuments::ID:
      func(static_cast<messages_getCustomEmojiDocuments &>(obj));
      return true;
    case messages_getDefaultHistoryTTL::ID:
      func(static_cast<messages_getDefaultHistoryTTL &>(obj));
      return true;
    case messages_getDhConfig::ID:
      func(static_cast<messages_getDhConfig &>(obj));
      return true;
    case messages_getDialogFilters::ID:
      func(static_cast<messages_getDialogFilters &>(obj));
      return true;
    case messages_getDialogUnreadMarks::ID:
      func(static_cast<messages_getDialogUnreadMarks &>(obj));
      return true;
    case messages_getDialogs::ID:
      func(static_cast<messages_getDialogs &>(obj));
      return true;
    case messages_getDiscussionMessage::ID:
      func(static_cast<messages_getDiscussionMessage &>(obj));
      return true;
    case messages_getDocumentByHash::ID:
      func(static_cast<messages_getDocumentByHash &>(obj));
      return true;
    case messages_getEmojiGroups::ID:
      func(static_cast<messages_getEmojiGroups &>(obj));
      return true;
    case messages_getEmojiKeywords::ID:
      func(static_cast<messages_getEmojiKeywords &>(obj));
      return true;
    case messages_getEmojiKeywordsDifference::ID:
      func(static_cast<messages_getEmojiKeywordsDifference &>(obj));
      return true;
    case messages_getEmojiKeywordsLanguages::ID:
      func(static_cast<messages_getEmojiKeywordsLanguages &>(obj));
      return true;
    case messages_getEmojiProfilePhotoGroups::ID:
      func(static_cast<messages_getEmojiProfilePhotoGroups &>(obj));
      return true;
    case messages_getEmojiStatusGroups::ID:
      func(static_cast<messages_getEmojiStatusGroups &>(obj));
      return true;
    case messages_getEmojiStickers::ID:
      func(static_cast<messages_getEmojiStickers &>(obj));
      return true;
    case messages_getEmojiURL::ID:
      func(static_cast<messages_getEmojiURL &>(obj));
      return true;
    case messages_getExportedChatInvite::ID:
      func(static_cast<messages_getExportedChatInvite &>(obj));
      return true;
    case messages_getExportedChatInvites::ID:
      func(static_cast<messages_getExportedChatInvites &>(obj));
      return true;
    case messages_getExtendedMedia::ID:
      func(static_cast<messages_getExtendedMedia &>(obj));
      return true;
    case messages_getFavedStickers::ID:
      func(static_cast<messages_getFavedStickers &>(obj));
      return true;
    case messages_getFeaturedEmojiStickers::ID:
      func(static_cast<messages_getFeaturedEmojiStickers &>(obj));
      return true;
    case messages_getFeaturedStickers::ID:
      func(static_cast<messages_getFeaturedStickers &>(obj));
      return true;
    case messages_getFullChat::ID:
      func(static_cast<messages_getFullChat &>(obj));
      return true;
    case messages_getGameHighScores::ID:
      func(static_cast<messages_getGameHighScores &>(obj));
      return true;
    case messages_getHistory::ID:
      func(static_cast<messages_getHistory &>(obj));
      return true;
    case messages_getInlineBotResults::ID:
      func(static_cast<messages_getInlineBotResults &>(obj));
      return true;
    case messages_getInlineGameHighScores::ID:
      func(static_cast<messages_getInlineGameHighScores &>(obj));
      return true;
    case messages_getMaskStickers::ID:
      func(static_cast<messages_getMaskStickers &>(obj));
      return true;
    case messages_getMessageEditData::ID:
      func(static_cast<messages_getMessageEditData &>(obj));
      return true;
    case messages_getMessageReactionsList::ID:
      func(static_cast<messages_getMessageReactionsList &>(obj));
      return true;
    case messages_getMessageReadParticipants::ID:
      func(static_cast<messages_getMessageReadParticipants &>(obj));
      return true;
    case messages_getMessages::ID:
      func(static_cast<messages_getMessages &>(obj));
      return true;
    case messages_getMessagesReactions::ID:
      func(static_cast<messages_getMessagesReactions &>(obj));
      return true;
    case messages_getMessagesViews::ID:
      func(static_cast<messages_getMessagesViews &>(obj));
      return true;
    case messages_getOldFeaturedStickers::ID:
      func(static_cast<messages_getOldFeaturedStickers &>(obj));
      return true;
    case messages_getOnlines::ID:
      func(static_cast<messages_getOnlines &>(obj));
      return true;
    case messages_getPeerDialogs::ID:
      func(static_cast<messages_getPeerDialogs &>(obj));
      return true;
    case messages_getPeerSettings::ID:
      func(static_cast<messages_getPeerSettings &>(obj));
      return true;
    case messages_getPinnedDialogs::ID:
      func(static_cast<messages_getPinnedDialogs &>(obj));
      return true;
    case messages_getPollResults::ID:
      func(static_cast<messages_getPollResults &>(obj));
      return true;
    case messages_getPollVotes::ID:
      func(static_cast<messages_getPollVotes &>(obj));
      return true;
    case messages_getRecentLocations::ID:
      func(static_cast<messages_getRecentLocations &>(obj));
      return true;
    case messages_getRecentReactions::ID:
      func(static_cast<messages_getRecentReactions &>(obj));
      return true;
    case messages_getRecentStickers::ID:
      func(static_cast<messages_getRecentStickers &>(obj));
      return true;
    case messages_getReplies::ID:
      func(static_cast<messages_getReplies &>(obj));
      return true;
    case messages_getSavedGifs::ID:
      func(static_cast<messages_getSavedGifs &>(obj));
      return true;
    case messages_getScheduledHistory::ID:
      func(static_cast<messages_getScheduledHistory &>(obj));
      return true;
    case messages_getScheduledMessages::ID:
      func(static_cast<messages_getScheduledMessages &>(obj));
      return true;
    case messages_getSearchCounters::ID:
      func(static_cast<messages_getSearchCounters &>(obj));
      return true;
    case messages_getSearchResultsCalendar::ID:
      func(static_cast<messages_getSearchResultsCalendar &>(obj));
      return true;
    case messages_getSearchResultsPositions::ID:
      func(static_cast<messages_getSearchResultsPositions &>(obj));
      return true;
    case messages_getSplitRanges::ID:
      func(static_cast<messages_getSplitRanges &>(obj));
      return true;
    case messages_getStickerSet::ID:
      func(static_cast<messages_getStickerSet &>(obj));
      return true;
    case messages_getStickers::ID:
      func(static_cast<messages_getStickers &>(obj));
      return true;
    case messages_getSuggestedDialogFilters::ID:
      func(static_cast<messages_getSuggestedDialogFilters &>(obj));
      return true;
    case messages_getTopReactions::ID:
      func(static_cast<messages_getTopReactions &>(obj));
      return true;
    case messages_getUnreadMentions::ID:
      func(static_cast<messages_getUnreadMentions &>(obj));
      return true;
    case messages_getUnreadReactions::ID:
      func(static_cast<messages_getUnreadReactions &>(obj));
      return true;
    case messages_getWebPage::ID:
      func(static_cast<messages_getWebPage &>(obj));
      return true;
    case messages_getWebPagePreview::ID:
      func(static_cast<messages_getWebPagePreview &>(obj));
      return true;
    case messages_hideAllChatJoinRequests::ID:
      func(static_cast<messages_hideAllChatJoinRequests &>(obj));
      return true;
    case messages_hideChatJoinRequest::ID:
      func(static_cast<messages_hideChatJoinRequest &>(obj));
      return true;
    case messages_hidePeerSettingsBar::ID:
      func(static_cast<messages_hidePeerSettingsBar &>(obj));
      return true;
    case messages_importChatInvite::ID:
      func(static_cast<messages_importChatInvite &>(obj));
      return true;
    case messages_initHistoryImport::ID:
      func(static_cast<messages_initHistoryImport &>(obj));
      return true;
    case messages_installStickerSet::ID:
      func(static_cast<messages_installStickerSet &>(obj));
      return true;
    case messages_markDialogUnread::ID:
      func(static_cast<messages_markDialogUnread &>(obj));
      return true;
    case messages_migrateChat::ID:
      func(static_cast<messages_migrateChat &>(obj));
      return true;
    case messages_prolongWebView::ID:
      func(static_cast<messages_prolongWebView &>(obj));
      return true;
    case messages_rateTranscribedAudio::ID:
      func(static_cast<messages_rateTranscribedAudio &>(obj));
      return true;
    case messages_readDiscussion::ID:
      func(static_cast<messages_readDiscussion &>(obj));
      return true;
    case messages_readEncryptedHistory::ID:
      func(static_cast<messages_readEncryptedHistory &>(obj));
      return true;
    case messages_readFeaturedStickers::ID:
      func(static_cast<messages_readFeaturedStickers &>(obj));
      return true;
    case messages_readHistory::ID:
      func(static_cast<messages_readHistory &>(obj));
      return true;
    case messages_readMentions::ID:
      func(static_cast<messages_readMentions &>(obj));
      return true;
    case messages_readMessageContents::ID:
      func(static_cast<messages_readMessageContents &>(obj));
      return true;
    case messages_readReactions::ID:
      func(static_cast<messages_readReactions &>(obj));
      return true;
    case messages_receivedMessages::ID:
      func(static_cast<messages_receivedMessages &>(obj));
      return true;
    case messages_receivedQueue::ID:
      func(static_cast<messages_receivedQueue &>(obj));
      return true;
    case messages_reorderPinnedDialogs::ID:
      func(static_cast<messages_reorderPinnedDialogs &>(obj));
      return true;
    case messages_reorderStickerSets::ID:
      func(static_cast<messages_reorderStickerSets &>(obj));
      return true;
    case messages_report::ID:
      func(static_cast<messages_report &>(obj));
      return true;
    case messages_reportEncryptedSpam::ID:
      func(static_cast<messages_reportEncryptedSpam &>(obj));
      return true;
    case messages_reportReaction::ID:
      func(static_cast<messages_reportReaction &>(obj));
      return true;
    case messages_reportSpam::ID:
      func(static_cast<messages_reportSpam &>(obj));
      return true;
    case messages_requestAppWebView::ID:
      func(static_cast<messages_requestAppWebView &>(obj));
      return true;
    case messages_requestEncryption::ID:
      func(static_cast<messages_requestEncryption &>(obj));
      return true;
    case messages_requestSimpleWebView::ID:
      func(static_cast<messages_requestSimpleWebView &>(obj));
      return true;
    case messages_requestUrlAuth::ID:
      func(static_cast<messages_requestUrlAuth &>(obj));
      return true;
    case messages_requestWebView::ID:
      func(static_cast<messages_requestWebView &>(obj));
      return true;
    case messages_saveDefaultSendAs::ID:
      func(static_cast<messages_saveDefaultSendAs &>(obj));
      return true;
    case messages_saveDraft::ID:
      func(static_cast<messages_saveDraft &>(obj));
      return true;
    case messages_saveGif::ID:
      func(static_cast<messages_saveGif &>(obj));
      return true;
    case messages_saveRecentSticker::ID:
      func(static_cast<messages_saveRecentSticker &>(obj));
      return true;
    case messages_search::ID:
      func(static_cast<messages_search &>(obj));
      return true;
    case messages_searchCustomEmoji::ID:
      func(static_cast<messages_searchCustomEmoji &>(obj));
      return true;
    case messages_searchGlobal::ID:
      func(static_cast<messages_searchGlobal &>(obj));
      return true;
    case messages_searchSentMedia::ID:
      func(static_cast<messages_searchSentMedia &>(obj));
      return true;
    case messages_searchStickerSets::ID:
      func(static_cast<messages_searchStickerSets &>(obj));
      return true;
    case messages_sendBotRequestedPeer::ID:
      func(static_cast<messages_sendBotRequestedPeer &>(obj));
      return true;
    case messages_sendEncrypted::ID:
      func(static_cast<messages_sendEncrypted &>(obj));
      return true;
    case messages_sendEncryptedFile::ID:
      func(static_cast<messages_sendEncryptedFile &>(obj));
      return true;
    case messages_sendEncryptedService::ID:
      func(static_cast<messages_sendEncryptedService &>(obj));
      return true;
    case messages_sendInlineBotResult::ID:
      func(static_cast<messages_sendInlineBotResult &>(obj));
      return true;
    case messages_sendMedia::ID:
      func(static_cast<messages_sendMedia &>(obj));
      return true;
    case messages_sendMessage::ID:
      func(static_cast<messages_sendMessage &>(obj));
      return true;
    case messages_sendMultiMedia::ID:
      func(static_cast<messages_sendMultiMedia &>(obj));
      return true;
    case messages_sendReaction::ID:
      func(static_cast<messages_sendReaction &>(obj));
      return true;
    case messages_sendScheduledMessages::ID:
      func(static_cast<messages_sendScheduledMessages &>(obj));
      return true;
    case messages_sendScreenshotNotification::ID:
      func(static_cast<messages_sendScreenshotNotification &>(obj));
      return true;
    case messages_sendVote::ID:
      func(static_cast<messages_sendVote &>(obj));
      return true;
    case messages_sendWebViewData::ID:
      func(static_cast<messages_sendWebViewData &>(obj));
      return true;
    case messages_sendWebViewResultMessage::ID:
      func(static_cast<messages_sendWebViewResultMessage &>(obj));
      return true;
    case messages_setBotCallbackAnswer::ID:
      func(static_cast<messages_setBotCallbackAnswer &>(obj));
      return true;
    case messages_setBotPrecheckoutResults::ID:
      func(static_cast<messages_setBotPrecheckoutResults &>(obj));
      return true;
    case messages_setBotShippingResults::ID:
      func(static_cast<messages_setBotShippingResults &>(obj));
      return true;
    case messages_setChatAvailableReactions::ID:
      func(static_cast<messages_setChatAvailableReactions &>(obj));
      return true;
    case messages_setChatTheme::ID:
      func(static_cast<messages_setChatTheme &>(obj));
      return true;
    case messages_setChatWallPaper::ID:
      func(static_cast<messages_setChatWallPaper &>(obj));
      return true;
    case messages_setDefaultHistoryTTL::ID:
      func(static_cast<messages_setDefaultHistoryTTL &>(obj));
      return true;
    case messages_setDefaultReaction::ID:
      func(static_cast<messages_setDefaultReaction &>(obj));
      return true;
    case messages_setEncryptedTyping::ID:
      func(static_cast<messages_setEncryptedTyping &>(obj));
      return true;
    case messages_setGameScore::ID:
      func(static_cast<messages_setGameScore &>(obj));
      return true;
    case messages_setHistoryTTL::ID:
      func(static_cast<messages_setHistoryTTL &>(obj));
      return true;
    case messages_setInlineBotResults::ID:
      func(static_cast<messages_setInlineBotResults &>(obj));
      return true;
    case messages_setInlineGameScore::ID:
      func(static_cast<messages_setInlineGameScore &>(obj));
      return true;
    case messages_setTyping::ID:
      func(static_cast<messages_setTyping &>(obj));
      return true;
    case messages_startBot::ID:
      func(static_cast<messages_startBot &>(obj));
      return true;
    case messages_startHistoryImport::ID:
      func(static_cast<messages_startHistoryImport &>(obj));
      return true;
    case messages_toggleBotInAttachMenu::ID:
      func(static_cast<messages_toggleBotInAttachMenu &>(obj));
      return true;
    case messages_toggleDialogPin::ID:
      func(static_cast<messages_toggleDialogPin &>(obj));
      return true;
    case messages_toggleNoForwards::ID:
      func(static_cast<messages_toggleNoForwards &>(obj));
      return true;
    case messages_togglePeerTranslations::ID:
      func(static_cast<messages_togglePeerTranslations &>(obj));
      return true;
    case messages_toggleStickerSets::ID:
      func(static_cast<messages_toggleStickerSets &>(obj));
      return true;
    case messages_transcribeAudio::ID:
      func(static_cast<messages_transcribeAudio &>(obj));
      return true;
    case messages_translateText::ID:
      func(static_cast<messages_translateText &>(obj));
      return true;
    case messages_uninstallStickerSet::ID:
      func(static_cast<messages_uninstallStickerSet &>(obj));
      return true;
    case messages_unpinAllMessages::ID:
      func(static_cast<messages_unpinAllMessages &>(obj));
      return true;
    case messages_updateDialogFilter::ID:
      func(static_cast<messages_updateDialogFilter &>(obj));
      return true;
    case messages_updateDialogFiltersOrder::ID:
      func(static_cast<messages_updateDialogFiltersOrder &>(obj));
      return true;
    case messages_updatePinnedMessage::ID:
      func(static_cast<messages_updatePinnedMessage &>(obj));
      return true;
    case messages_uploadEncryptedFile::ID:
      func(static_cast<messages_uploadEncryptedFile &>(obj));
      return true;
    case messages_uploadImportedMedia::ID:
      func(static_cast<messages_uploadImportedMedia &>(obj));
      return true;
    case messages_uploadMedia::ID:
      func(static_cast<messages_uploadMedia &>(obj));
      return true;
    case payments_assignAppStoreTransaction::ID:
      func(static_cast<payments_assignAppStoreTransaction &>(obj));
      return true;
    case payments_assignPlayMarketTransaction::ID:
      func(static_cast<payments_assignPlayMarketTransaction &>(obj));
      return true;
    case payments_canPurchasePremium::ID:
      func(static_cast<payments_canPurchasePremium &>(obj));
      return true;
    case payments_clearSavedInfo::ID:
      func(static_cast<payments_clearSavedInfo &>(obj));
      return true;
    case payments_exportInvoice::ID:
      func(static_cast<payments_exportInvoice &>(obj));
      return true;
    case payments_getBankCardData::ID:
      func(static_cast<payments_getBankCardData &>(obj));
      return true;
    case payments_getPaymentForm::ID:
      func(static_cast<payments_getPaymentForm &>(obj));
      return true;
    case payments_getPaymentReceipt::ID:
      func(static_cast<payments_getPaymentReceipt &>(obj));
      return true;
    case payments_getSavedInfo::ID:
      func(static_cast<payments_getSavedInfo &>(obj));
      return true;
    case payments_sendPaymentForm::ID:
      func(static_cast<payments_sendPaymentForm &>(obj));
      return true;
    case payments_validateRequestedInfo::ID:
      func(static_cast<payments_validateRequestedInfo &>(obj));
      return true;
    case phone_acceptCall::ID:
      func(static_cast<phone_acceptCall &>(obj));
      return true;
    case phone_checkGroupCall::ID:
      func(static_cast<phone_checkGroupCall &>(obj));
      return true;
    case phone_confirmCall::ID:
      func(static_cast<phone_confirmCall &>(obj));
      return true;
    case phone_createGroupCall::ID:
      func(static_cast<phone_createGroupCall &>(obj));
      return true;
    case phone_discardCall::ID:
      func(static_cast<phone_discardCall &>(obj));
      return true;
    case phone_discardGroupCall::ID:
      func(static_cast<phone_discardGroupCall &>(obj));
      return true;
    case phone_editGroupCallParticipant::ID:
      func(static_cast<phone_editGroupCallParticipant &>(obj));
      return true;
    case phone_editGroupCallTitle::ID:
      func(static_cast<phone_editGroupCallTitle &>(obj));
      return true;
    case phone_exportGroupCallInvite::ID:
      func(static_cast<phone_exportGroupCallInvite &>(obj));
      return true;
    case phone_getCallConfig::ID:
      func(static_cast<phone_getCallConfig &>(obj));
      return true;
    case phone_getGroupCall::ID:
      func(static_cast<phone_getGroupCall &>(obj));
      return true;
    case phone_getGroupCallJoinAs::ID:
      func(static_cast<phone_getGroupCallJoinAs &>(obj));
      return true;
    case phone_getGroupCallStreamChannels::ID:
      func(static_cast<phone_getGroupCallStreamChannels &>(obj));
      return true;
    case phone_getGroupCallStreamRtmpUrl::ID:
      func(static_cast<phone_getGroupCallStreamRtmpUrl &>(obj));
      return true;
    case phone_getGroupParticipants::ID:
      func(static_cast<phone_getGroupParticipants &>(obj));
      return true;
    case phone_inviteToGroupCall::ID:
      func(static_cast<phone_inviteToGroupCall &>(obj));
      return true;
    case phone_joinGroupCall::ID:
      func(static_cast<phone_joinGroupCall &>(obj));
      return true;
    case phone_joinGroupCallPresentation::ID:
      func(static_cast<phone_joinGroupCallPresentation &>(obj));
      return true;
    case phone_leaveGroupCall::ID:
      func(static_cast<phone_leaveGroupCall &>(obj));
      return true;
    case phone_leaveGroupCallPresentation::ID:
      func(static_cast<phone_leaveGroupCallPresentation &>(obj));
      return true;
    case phone_receivedCall::ID:
      func(static_cast<phone_receivedCall &>(obj));
      return true;
    case phone_requestCall::ID:
      func(static_cast<phone_requestCall &>(obj));
      return true;
    case phone_saveCallDebug::ID:
      func(static_cast<phone_saveCallDebug &>(obj));
      return true;
    case phone_saveCallLog::ID:
      func(static_cast<phone_saveCallLog &>(obj));
      return true;
    case phone_saveDefaultGroupCallJoinAs::ID:
      func(static_cast<phone_saveDefaultGroupCallJoinAs &>(obj));
      return true;
    case phone_sendSignalingData::ID:
      func(static_cast<phone_sendSignalingData &>(obj));
      return true;
    case phone_setCallRating::ID:
      func(static_cast<phone_setCallRating &>(obj));
      return true;
    case phone_startScheduledGroupCall::ID:
      func(static_cast<phone_startScheduledGroupCall &>(obj));
      return true;
    case phone_toggleGroupCallRecord::ID:
      func(static_cast<phone_toggleGroupCallRecord &>(obj));
      return true;
    case phone_toggleGroupCallSettings::ID:
      func(static_cast<phone_toggleGroupCallSettings &>(obj));
      return true;
    case phone_toggleGroupCallStartSubscription::ID:
      func(static_cast<phone_toggleGroupCallStartSubscription &>(obj));
      return true;
    case photos_deletePhotos::ID:
      func(static_cast<photos_deletePhotos &>(obj));
      return true;
    case photos_getUserPhotos::ID:
      func(static_cast<photos_getUserPhotos &>(obj));
      return true;
    case photos_updateProfilePhoto::ID:
      func(static_cast<photos_updateProfilePhoto &>(obj));
      return true;
    case photos_uploadContactProfilePhoto::ID:
      func(static_cast<photos_uploadContactProfilePhoto &>(obj));
      return true;
    case photos_uploadProfilePhoto::ID:
      func(static_cast<photos_uploadProfilePhoto &>(obj));
      return true;
    case stats_getBroadcastStats::ID:
      func(static_cast<stats_getBroadcastStats &>(obj));
      return true;
    case stats_getMegagroupStats::ID:
      func(static_cast<stats_getMegagroupStats &>(obj));
      return true;
    case stats_getMessagePublicForwards::ID:
      func(static_cast<stats_getMessagePublicForwards &>(obj));
      return true;
    case stats_getMessageStats::ID:
      func(static_cast<stats_getMessageStats &>(obj));
      return true;
    case stats_loadAsyncGraph::ID:
      func(static_cast<stats_loadAsyncGraph &>(obj));
      return true;
    case stickers_addStickerToSet::ID:
      func(static_cast<stickers_addStickerToSet &>(obj));
      return true;
    case stickers_changeSticker::ID:
      func(static_cast<stickers_changeSticker &>(obj));
      return true;
    case stickers_changeStickerPosition::ID:
      func(static_cast<stickers_changeStickerPosition &>(obj));
      return true;
    case stickers_checkShortName::ID:
      func(static_cast<stickers_checkShortName &>(obj));
      return true;
    case stickers_createStickerSet::ID:
      func(static_cast<stickers_createStickerSet &>(obj));
      return true;
    case stickers_deleteStickerSet::ID:
      func(static_cast<stickers_deleteStickerSet &>(obj));
      return true;
    case stickers_removeStickerFromSet::ID:
      func(static_cast<stickers_removeStickerFromSet &>(obj));
      return true;
    case stickers_renameStickerSet::ID:
      func(static_cast<stickers_renameStickerSet &>(obj));
      return true;
    case stickers_setStickerSetThumb::ID:
      func(static_cast<stickers_setStickerSetThumb &>(obj));
      return true;
    case stickers_suggestShortName::ID:
      func(static_cast<stickers_suggestShortName &>(obj));
      return true;
    case test_parseInputAppEvent::ID:
      func(static_cast<test_parseInputAppEvent &>(obj));
      return true;
    case test_useConfigSimple::ID:
      func(static_cast<test_useConfigSimple &>(obj));
      return true;
    case test_useError::ID:
      func(static_cast<test_useError &>(obj));
      return true;
    case updates_getChannelDifference::ID:
      func(static_cast<updates_getChannelDifference &>(obj));
      return true;
    case updates_getDifference::ID:
      func(static_cast<updates_getDifference &>(obj));
      return true;
    case updates_getState::ID:
      func(static_cast<updates_getState &>(obj));
      return true;
    case upload_getCdnFile::ID:
      func(static_cast<upload_getCdnFile &>(obj));
      return true;
    case upload_getCdnFileHashes::ID:
      func(static_cast<upload_getCdnFileHashes &>(obj));
      return true;
    case upload_getFile::ID:
      func(static_cast<upload_getFile &>(obj));
      return true;
    case upload_getFileHashes::ID:
      func(static_cast<upload_getFileHashes &>(obj));
      return true;
    case upload_getWebFile::ID:
      func(static_cast<upload_getWebFile &>(obj));
      return true;
    case upload_reuploadCdnFile::ID:
      func(static_cast<upload_reuploadCdnFile &>(obj));
      return true;
    case upload_saveBigFilePart::ID:
      func(static_cast<upload_saveBigFilePart &>(obj));
      return true;
    case upload_saveFilePart::ID:
      func(static_cast<upload_saveFilePart &>(obj));
      return true;
    case users_getFullUser::ID:
      func(static_cast<users_getFullUser &>(obj));
      return true;
    case users_getUsers::ID:
      func(static_cast<users_getUsers &>(obj));
      return true;
    case users_setSecureValueErrors::ID:
      func(static_cast<users_setSecureValueErrors &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(AttachMenuBots &obj, const T &func) {
  switch (obj.get_id()) {
    case attachMenuBotsNotModified::ID:
      func(static_cast<attachMenuBotsNotModified &>(obj));
      return true;
    case attachMenuBots::ID:
      func(static_cast<attachMenuBots &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(AttachMenuPeerType &obj, const T &func) {
  switch (obj.get_id()) {
    case attachMenuPeerTypeSameBotPM::ID:
      func(static_cast<attachMenuPeerTypeSameBotPM &>(obj));
      return true;
    case attachMenuPeerTypeBotPM::ID:
      func(static_cast<attachMenuPeerTypeBotPM &>(obj));
      return true;
    case attachMenuPeerTypePM::ID:
      func(static_cast<attachMenuPeerTypePM &>(obj));
      return true;
    case attachMenuPeerTypeChat::ID:
      func(static_cast<attachMenuPeerTypeChat &>(obj));
      return true;
    case attachMenuPeerTypeBroadcast::ID:
      func(static_cast<attachMenuPeerTypeBroadcast &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(BaseTheme &obj, const T &func) {
  switch (obj.get_id()) {
    case baseThemeClassic::ID:
      func(static_cast<baseThemeClassic &>(obj));
      return true;
    case baseThemeDay::ID:
      func(static_cast<baseThemeDay &>(obj));
      return true;
    case baseThemeNight::ID:
      func(static_cast<baseThemeNight &>(obj));
      return true;
    case baseThemeTinted::ID:
      func(static_cast<baseThemeTinted &>(obj));
      return true;
    case baseThemeArctic::ID:
      func(static_cast<baseThemeArctic &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(BotApp &obj, const T &func) {
  switch (obj.get_id()) {
    case botAppNotModified::ID:
      func(static_cast<botAppNotModified &>(obj));
      return true;
    case botApp::ID:
      func(static_cast<botApp &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(BotCommandScope &obj, const T &func) {
  switch (obj.get_id()) {
    case botCommandScopeDefault::ID:
      func(static_cast<botCommandScopeDefault &>(obj));
      return true;
    case botCommandScopeUsers::ID:
      func(static_cast<botCommandScopeUsers &>(obj));
      return true;
    case botCommandScopeChats::ID:
      func(static_cast<botCommandScopeChats &>(obj));
      return true;
    case botCommandScopeChatAdmins::ID:
      func(static_cast<botCommandScopeChatAdmins &>(obj));
      return true;
    case botCommandScopePeer::ID:
      func(static_cast<botCommandScopePeer &>(obj));
      return true;
    case botCommandScopePeerAdmins::ID:
      func(static_cast<botCommandScopePeerAdmins &>(obj));
      return true;
    case botCommandScopePeerUser::ID:
      func(static_cast<botCommandScopePeerUser &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(BotInlineMessage &obj, const T &func) {
  switch (obj.get_id()) {
    case botInlineMessageMediaAuto::ID:
      func(static_cast<botInlineMessageMediaAuto &>(obj));
      return true;
    case botInlineMessageText::ID:
      func(static_cast<botInlineMessageText &>(obj));
      return true;
    case botInlineMessageMediaGeo::ID:
      func(static_cast<botInlineMessageMediaGeo &>(obj));
      return true;
    case botInlineMessageMediaVenue::ID:
      func(static_cast<botInlineMessageMediaVenue &>(obj));
      return true;
    case botInlineMessageMediaContact::ID:
      func(static_cast<botInlineMessageMediaContact &>(obj));
      return true;
    case botInlineMessageMediaInvoice::ID:
      func(static_cast<botInlineMessageMediaInvoice &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(BotInlineResult &obj, const T &func) {
  switch (obj.get_id()) {
    case botInlineResult::ID:
      func(static_cast<botInlineResult &>(obj));
      return true;
    case botInlineMediaResult::ID:
      func(static_cast<botInlineMediaResult &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(BotMenuButton &obj, const T &func) {
  switch (obj.get_id()) {
    case botMenuButtonDefault::ID:
      func(static_cast<botMenuButtonDefault &>(obj));
      return true;
    case botMenuButtonCommands::ID:
      func(static_cast<botMenuButtonCommands &>(obj));
      return true;
    case botMenuButton::ID:
      func(static_cast<botMenuButton &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(ChannelAdminLogEventAction &obj, const T &func) {
  switch (obj.get_id()) {
    case channelAdminLogEventActionChangeTitle::ID:
      func(static_cast<channelAdminLogEventActionChangeTitle &>(obj));
      return true;
    case channelAdminLogEventActionChangeAbout::ID:
      func(static_cast<channelAdminLogEventActionChangeAbout &>(obj));
      return true;
    case channelAdminLogEventActionChangeUsername::ID:
      func(static_cast<channelAdminLogEventActionChangeUsername &>(obj));
      return true;
    case channelAdminLogEventActionChangePhoto::ID:
      func(static_cast<channelAdminLogEventActionChangePhoto &>(obj));
      return true;
    case channelAdminLogEventActionToggleInvites::ID:
      func(static_cast<channelAdminLogEventActionToggleInvites &>(obj));
      return true;
    case channelAdminLogEventActionToggleSignatures::ID:
      func(static_cast<channelAdminLogEventActionToggleSignatures &>(obj));
      return true;
    case channelAdminLogEventActionUpdatePinned::ID:
      func(static_cast<channelAdminLogEventActionUpdatePinned &>(obj));
      return true;
    case channelAdminLogEventActionEditMessage::ID:
      func(static_cast<channelAdminLogEventActionEditMessage &>(obj));
      return true;
    case channelAdminLogEventActionDeleteMessage::ID:
      func(static_cast<channelAdminLogEventActionDeleteMessage &>(obj));
      return true;
    case channelAdminLogEventActionParticipantJoin::ID:
      func(static_cast<channelAdminLogEventActionParticipantJoin &>(obj));
      return true;
    case channelAdminLogEventActionParticipantLeave::ID:
      func(static_cast<channelAdminLogEventActionParticipantLeave &>(obj));
      return true;
    case channelAdminLogEventActionParticipantInvite::ID:
      func(static_cast<channelAdminLogEventActionParticipantInvite &>(obj));
      return true;
    case channelAdminLogEventActionParticipantToggleBan::ID:
      func(static_cast<channelAdminLogEventActionParticipantToggleBan &>(obj));
      return true;
    case channelAdminLogEventActionParticipantToggleAdmin::ID:
      func(static_cast<channelAdminLogEventActionParticipantToggleAdmin &>(obj));
      return true;
    case channelAdminLogEventActionChangeStickerSet::ID:
      func(static_cast<channelAdminLogEventActionChangeStickerSet &>(obj));
      return true;
    case channelAdminLogEventActionTogglePreHistoryHidden::ID:
      func(static_cast<channelAdminLogEventActionTogglePreHistoryHidden &>(obj));
      return true;
    case channelAdminLogEventActionDefaultBannedRights::ID:
      func(static_cast<channelAdminLogEventActionDefaultBannedRights &>(obj));
      return true;
    case channelAdminLogEventActionStopPoll::ID:
      func(static_cast<channelAdminLogEventActionStopPoll &>(obj));
      return true;
    case channelAdminLogEventActionChangeLinkedChat::ID:
      func(static_cast<channelAdminLogEventActionChangeLinkedChat &>(obj));
      return true;
    case channelAdminLogEventActionChangeLocation::ID:
      func(static_cast<channelAdminLogEventActionChangeLocation &>(obj));
      return true;
    case channelAdminLogEventActionToggleSlowMode::ID:
      func(static_cast<channelAdminLogEventActionToggleSlowMode &>(obj));
      return true;
    case channelAdminLogEventActionStartGroupCall::ID:
      func(static_cast<channelAdminLogEventActionStartGroupCall &>(obj));
      return true;
    case channelAdminLogEventActionDiscardGroupCall::ID:
      func(static_cast<channelAdminLogEventActionDiscardGroupCall &>(obj));
      return true;
    case channelAdminLogEventActionParticipantMute::ID:
      func(static_cast<channelAdminLogEventActionParticipantMute &>(obj));
      return true;
    case channelAdminLogEventActionParticipantUnmute::ID:
      func(static_cast<channelAdminLogEventActionParticipantUnmute &>(obj));
      return true;
    case channelAdminLogEventActionToggleGroupCallSetting::ID:
      func(static_cast<channelAdminLogEventActionToggleGroupCallSetting &>(obj));
      return true;
    case channelAdminLogEventActionParticipantJoinByInvite::ID:
      func(static_cast<channelAdminLogEventActionParticipantJoinByInvite &>(obj));
      return true;
    case channelAdminLogEventActionExportedInviteDelete::ID:
      func(static_cast<channelAdminLogEventActionExportedInviteDelete &>(obj));
      return true;
    case channelAdminLogEventActionExportedInviteRevoke::ID:
      func(static_cast<channelAdminLogEventActionExportedInviteRevoke &>(obj));
      return true;
    case channelAdminLogEventActionExportedInviteEdit::ID:
      func(static_cast<channelAdminLogEventActionExportedInviteEdit &>(obj));
      return true;
    case channelAdminLogEventActionParticipantVolume::ID:
      func(static_cast<channelAdminLogEventActionParticipantVolume &>(obj));
      return true;
    case channelAdminLogEventActionChangeHistoryTTL::ID:
      func(static_cast<channelAdminLogEventActionChangeHistoryTTL &>(obj));
      return true;
    case channelAdminLogEventActionParticipantJoinByRequest::ID:
      func(static_cast<channelAdminLogEventActionParticipantJoinByRequest &>(obj));
      return true;
    case channelAdminLogEventActionToggleNoForwards::ID:
      func(static_cast<channelAdminLogEventActionToggleNoForwards &>(obj));
      return true;
    case channelAdminLogEventActionSendMessage::ID:
      func(static_cast<channelAdminLogEventActionSendMessage &>(obj));
      return true;
    case channelAdminLogEventActionChangeAvailableReactions::ID:
      func(static_cast<channelAdminLogEventActionChangeAvailableReactions &>(obj));
      return true;
    case channelAdminLogEventActionChangeUsernames::ID:
      func(static_cast<channelAdminLogEventActionChangeUsernames &>(obj));
      return true;
    case channelAdminLogEventActionToggleForum::ID:
      func(static_cast<channelAdminLogEventActionToggleForum &>(obj));
      return true;
    case channelAdminLogEventActionCreateTopic::ID:
      func(static_cast<channelAdminLogEventActionCreateTopic &>(obj));
      return true;
    case channelAdminLogEventActionEditTopic::ID:
      func(static_cast<channelAdminLogEventActionEditTopic &>(obj));
      return true;
    case channelAdminLogEventActionDeleteTopic::ID:
      func(static_cast<channelAdminLogEventActionDeleteTopic &>(obj));
      return true;
    case channelAdminLogEventActionPinTopic::ID:
      func(static_cast<channelAdminLogEventActionPinTopic &>(obj));
      return true;
    case channelAdminLogEventActionToggleAntiSpam::ID:
      func(static_cast<channelAdminLogEventActionToggleAntiSpam &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(ChannelLocation &obj, const T &func) {
  switch (obj.get_id()) {
    case channelLocationEmpty::ID:
      func(static_cast<channelLocationEmpty &>(obj));
      return true;
    case channelLocation::ID:
      func(static_cast<channelLocation &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(ChannelMessagesFilter &obj, const T &func) {
  switch (obj.get_id()) {
    case channelMessagesFilterEmpty::ID:
      func(static_cast<channelMessagesFilterEmpty &>(obj));
      return true;
    case channelMessagesFilter::ID:
      func(static_cast<channelMessagesFilter &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(ChannelParticipant &obj, const T &func) {
  switch (obj.get_id()) {
    case channelParticipant::ID:
      func(static_cast<channelParticipant &>(obj));
      return true;
    case channelParticipantSelf::ID:
      func(static_cast<channelParticipantSelf &>(obj));
      return true;
    case channelParticipantCreator::ID:
      func(static_cast<channelParticipantCreator &>(obj));
      return true;
    case channelParticipantAdmin::ID:
      func(static_cast<channelParticipantAdmin &>(obj));
      return true;
    case channelParticipantBanned::ID:
      func(static_cast<channelParticipantBanned &>(obj));
      return true;
    case channelParticipantLeft::ID:
      func(static_cast<channelParticipantLeft &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(ChannelParticipantsFilter &obj, const T &func) {
  switch (obj.get_id()) {
    case channelParticipantsRecent::ID:
      func(static_cast<channelParticipantsRecent &>(obj));
      return true;
    case channelParticipantsAdmins::ID:
      func(static_cast<channelParticipantsAdmins &>(obj));
      return true;
    case channelParticipantsKicked::ID:
      func(static_cast<channelParticipantsKicked &>(obj));
      return true;
    case channelParticipantsBots::ID:
      func(static_cast<channelParticipantsBots &>(obj));
      return true;
    case channelParticipantsBanned::ID:
      func(static_cast<channelParticipantsBanned &>(obj));
      return true;
    case channelParticipantsSearch::ID:
      func(static_cast<channelParticipantsSearch &>(obj));
      return true;
    case channelParticipantsContacts::ID:
      func(static_cast<channelParticipantsContacts &>(obj));
      return true;
    case channelParticipantsMentions::ID:
      func(static_cast<channelParticipantsMentions &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(Chat &obj, const T &func) {
  switch (obj.get_id()) {
    case chatEmpty::ID:
      func(static_cast<chatEmpty &>(obj));
      return true;
    case chat::ID:
      func(static_cast<chat &>(obj));
      return true;
    case chatForbidden::ID:
      func(static_cast<chatForbidden &>(obj));
      return true;
    case channel::ID:
      func(static_cast<channel &>(obj));
      return true;
    case channelForbidden::ID:
      func(static_cast<channelForbidden &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(ChatFull &obj, const T &func) {
  switch (obj.get_id()) {
    case chatFull::ID:
      func(static_cast<chatFull &>(obj));
      return true;
    case channelFull::ID:
      func(static_cast<channelFull &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(ChatInvite &obj, const T &func) {
  switch (obj.get_id()) {
    case chatInviteAlready::ID:
      func(static_cast<chatInviteAlready &>(obj));
      return true;
    case chatInvite::ID:
      func(static_cast<chatInvite &>(obj));
      return true;
    case chatInvitePeek::ID:
      func(static_cast<chatInvitePeek &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(ChatParticipant &obj, const T &func) {
  switch (obj.get_id()) {
    case chatParticipant::ID:
      func(static_cast<chatParticipant &>(obj));
      return true;
    case chatParticipantCreator::ID:
      func(static_cast<chatParticipantCreator &>(obj));
      return true;
    case chatParticipantAdmin::ID:
      func(static_cast<chatParticipantAdmin &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(ChatParticipants &obj, const T &func) {
  switch (obj.get_id()) {
    case chatParticipantsForbidden::ID:
      func(static_cast<chatParticipantsForbidden &>(obj));
      return true;
    case chatParticipants::ID:
      func(static_cast<chatParticipants &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(ChatPhoto &obj, const T &func) {
  switch (obj.get_id()) {
    case chatPhotoEmpty::ID:
      func(static_cast<chatPhotoEmpty &>(obj));
      return true;
    case chatPhoto::ID:
      func(static_cast<chatPhoto &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(ChatReactions &obj, const T &func) {
  switch (obj.get_id()) {
    case chatReactionsNone::ID:
      func(static_cast<chatReactionsNone &>(obj));
      return true;
    case chatReactionsAll::ID:
      func(static_cast<chatReactionsAll &>(obj));
      return true;
    case chatReactionsSome::ID:
      func(static_cast<chatReactionsSome &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(Dialog &obj, const T &func) {
  switch (obj.get_id()) {
    case dialog::ID:
      func(static_cast<dialog &>(obj));
      return true;
    case dialogFolder::ID:
      func(static_cast<dialogFolder &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(DialogFilter &obj, const T &func) {
  switch (obj.get_id()) {
    case dialogFilter::ID:
      func(static_cast<dialogFilter &>(obj));
      return true;
    case dialogFilterDefault::ID:
      func(static_cast<dialogFilterDefault &>(obj));
      return true;
    case dialogFilterChatlist::ID:
      func(static_cast<dialogFilterChatlist &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(DialogPeer &obj, const T &func) {
  switch (obj.get_id()) {
    case dialogPeer::ID:
      func(static_cast<dialogPeer &>(obj));
      return true;
    case dialogPeerFolder::ID:
      func(static_cast<dialogPeerFolder &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(Document &obj, const T &func) {
  switch (obj.get_id()) {
    case documentEmpty::ID:
      func(static_cast<documentEmpty &>(obj));
      return true;
    case document::ID:
      func(static_cast<document &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(DocumentAttribute &obj, const T &func) {
  switch (obj.get_id()) {
    case documentAttributeImageSize::ID:
      func(static_cast<documentAttributeImageSize &>(obj));
      return true;
    case documentAttributeAnimated::ID:
      func(static_cast<documentAttributeAnimated &>(obj));
      return true;
    case documentAttributeSticker::ID:
      func(static_cast<documentAttributeSticker &>(obj));
      return true;
    case documentAttributeVideo::ID:
      func(static_cast<documentAttributeVideo &>(obj));
      return true;
    case documentAttributeAudio::ID:
      func(static_cast<documentAttributeAudio &>(obj));
      return true;
    case documentAttributeFilename::ID:
      func(static_cast<documentAttributeFilename &>(obj));
      return true;
    case documentAttributeHasStickers::ID:
      func(static_cast<documentAttributeHasStickers &>(obj));
      return true;
    case documentAttributeCustomEmoji::ID:
      func(static_cast<documentAttributeCustomEmoji &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(DraftMessage &obj, const T &func) {
  switch (obj.get_id()) {
    case draftMessageEmpty::ID:
      func(static_cast<draftMessageEmpty &>(obj));
      return true;
    case draftMessage::ID:
      func(static_cast<draftMessage &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(EmailVerification &obj, const T &func) {
  switch (obj.get_id()) {
    case emailVerificationCode::ID:
      func(static_cast<emailVerificationCode &>(obj));
      return true;
    case emailVerificationGoogle::ID:
      func(static_cast<emailVerificationGoogle &>(obj));
      return true;
    case emailVerificationApple::ID:
      func(static_cast<emailVerificationApple &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(EmailVerifyPurpose &obj, const T &func) {
  switch (obj.get_id()) {
    case emailVerifyPurposeLoginSetup::ID:
      func(static_cast<emailVerifyPurposeLoginSetup &>(obj));
      return true;
    case emailVerifyPurposeLoginChange::ID:
      func(static_cast<emailVerifyPurposeLoginChange &>(obj));
      return true;
    case emailVerifyPurposePassport::ID:
      func(static_cast<emailVerifyPurposePassport &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(EmojiKeyword &obj, const T &func) {
  switch (obj.get_id()) {
    case emojiKeyword::ID:
      func(static_cast<emojiKeyword &>(obj));
      return true;
    case emojiKeywordDeleted::ID:
      func(static_cast<emojiKeywordDeleted &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(EmojiList &obj, const T &func) {
  switch (obj.get_id()) {
    case emojiListNotModified::ID:
      func(static_cast<emojiListNotModified &>(obj));
      return true;
    case emojiList::ID:
      func(static_cast<emojiList &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(EmojiStatus &obj, const T &func) {
  switch (obj.get_id()) {
    case emojiStatusEmpty::ID:
      func(static_cast<emojiStatusEmpty &>(obj));
      return true;
    case emojiStatus::ID:
      func(static_cast<emojiStatus &>(obj));
      return true;
    case emojiStatusUntil::ID:
      func(static_cast<emojiStatusUntil &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(EncryptedChat &obj, const T &func) {
  switch (obj.get_id()) {
    case encryptedChatEmpty::ID:
      func(static_cast<encryptedChatEmpty &>(obj));
      return true;
    case encryptedChatWaiting::ID:
      func(static_cast<encryptedChatWaiting &>(obj));
      return true;
    case encryptedChatRequested::ID:
      func(static_cast<encryptedChatRequested &>(obj));
      return true;
    case encryptedChat::ID:
      func(static_cast<encryptedChat &>(obj));
      return true;
    case encryptedChatDiscarded::ID:
      func(static_cast<encryptedChatDiscarded &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(EncryptedFile &obj, const T &func) {
  switch (obj.get_id()) {
    case encryptedFileEmpty::ID:
      func(static_cast<encryptedFileEmpty &>(obj));
      return true;
    case encryptedFile::ID:
      func(static_cast<encryptedFile &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(EncryptedMessage &obj, const T &func) {
  switch (obj.get_id()) {
    case encryptedMessage::ID:
      func(static_cast<encryptedMessage &>(obj));
      return true;
    case encryptedMessageService::ID:
      func(static_cast<encryptedMessageService &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(ExportedChatInvite &obj, const T &func) {
  switch (obj.get_id()) {
    case chatInviteExported::ID:
      func(static_cast<chatInviteExported &>(obj));
      return true;
    case chatInvitePublicJoinRequests::ID:
      func(static_cast<chatInvitePublicJoinRequests &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(ForumTopic &obj, const T &func) {
  switch (obj.get_id()) {
    case forumTopicDeleted::ID:
      func(static_cast<forumTopicDeleted &>(obj));
      return true;
    case forumTopic::ID:
      func(static_cast<forumTopic &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(GeoPoint &obj, const T &func) {
  switch (obj.get_id()) {
    case geoPointEmpty::ID:
      func(static_cast<geoPointEmpty &>(obj));
      return true;
    case geoPoint::ID:
      func(static_cast<geoPoint &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(GroupCall &obj, const T &func) {
  switch (obj.get_id()) {
    case groupCallDiscarded::ID:
      func(static_cast<groupCallDiscarded &>(obj));
      return true;
    case groupCall::ID:
      func(static_cast<groupCall &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InlineQueryPeerType &obj, const T &func) {
  switch (obj.get_id()) {
    case inlineQueryPeerTypeSameBotPM::ID:
      func(static_cast<inlineQueryPeerTypeSameBotPM &>(obj));
      return true;
    case inlineQueryPeerTypePM::ID:
      func(static_cast<inlineQueryPeerTypePM &>(obj));
      return true;
    case inlineQueryPeerTypeChat::ID:
      func(static_cast<inlineQueryPeerTypeChat &>(obj));
      return true;
    case inlineQueryPeerTypeMegagroup::ID:
      func(static_cast<inlineQueryPeerTypeMegagroup &>(obj));
      return true;
    case inlineQueryPeerTypeBroadcast::ID:
      func(static_cast<inlineQueryPeerTypeBroadcast &>(obj));
      return true;
    case inlineQueryPeerTypeBotPM::ID:
      func(static_cast<inlineQueryPeerTypeBotPM &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputBotApp &obj, const T &func) {
  switch (obj.get_id()) {
    case inputBotAppID::ID:
      func(static_cast<inputBotAppID &>(obj));
      return true;
    case inputBotAppShortName::ID:
      func(static_cast<inputBotAppShortName &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputBotInlineMessage &obj, const T &func) {
  switch (obj.get_id()) {
    case inputBotInlineMessageMediaAuto::ID:
      func(static_cast<inputBotInlineMessageMediaAuto &>(obj));
      return true;
    case inputBotInlineMessageText::ID:
      func(static_cast<inputBotInlineMessageText &>(obj));
      return true;
    case inputBotInlineMessageMediaGeo::ID:
      func(static_cast<inputBotInlineMessageMediaGeo &>(obj));
      return true;
    case inputBotInlineMessageMediaVenue::ID:
      func(static_cast<inputBotInlineMessageMediaVenue &>(obj));
      return true;
    case inputBotInlineMessageMediaContact::ID:
      func(static_cast<inputBotInlineMessageMediaContact &>(obj));
      return true;
    case inputBotInlineMessageGame::ID:
      func(static_cast<inputBotInlineMessageGame &>(obj));
      return true;
    case inputBotInlineMessageMediaInvoice::ID:
      func(static_cast<inputBotInlineMessageMediaInvoice &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputBotInlineMessageID &obj, const T &func) {
  switch (obj.get_id()) {
    case inputBotInlineMessageID::ID:
      func(static_cast<inputBotInlineMessageID &>(obj));
      return true;
    case inputBotInlineMessageID64::ID:
      func(static_cast<inputBotInlineMessageID64 &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputBotInlineResult &obj, const T &func) {
  switch (obj.get_id()) {
    case inputBotInlineResult::ID:
      func(static_cast<inputBotInlineResult &>(obj));
      return true;
    case inputBotInlineResultPhoto::ID:
      func(static_cast<inputBotInlineResultPhoto &>(obj));
      return true;
    case inputBotInlineResultDocument::ID:
      func(static_cast<inputBotInlineResultDocument &>(obj));
      return true;
    case inputBotInlineResultGame::ID:
      func(static_cast<inputBotInlineResultGame &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputChannel &obj, const T &func) {
  switch (obj.get_id()) {
    case inputChannelEmpty::ID:
      func(static_cast<inputChannelEmpty &>(obj));
      return true;
    case inputChannel::ID:
      func(static_cast<inputChannel &>(obj));
      return true;
    case inputChannelFromMessage::ID:
      func(static_cast<inputChannelFromMessage &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputChatPhoto &obj, const T &func) {
  switch (obj.get_id()) {
    case inputChatPhotoEmpty::ID:
      func(static_cast<inputChatPhotoEmpty &>(obj));
      return true;
    case inputChatUploadedPhoto::ID:
      func(static_cast<inputChatUploadedPhoto &>(obj));
      return true;
    case inputChatPhoto::ID:
      func(static_cast<inputChatPhoto &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputCheckPasswordSRP &obj, const T &func) {
  switch (obj.get_id()) {
    case inputCheckPasswordEmpty::ID:
      func(static_cast<inputCheckPasswordEmpty &>(obj));
      return true;
    case inputCheckPasswordSRP::ID:
      func(static_cast<inputCheckPasswordSRP &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputDialogPeer &obj, const T &func) {
  switch (obj.get_id()) {
    case inputDialogPeer::ID:
      func(static_cast<inputDialogPeer &>(obj));
      return true;
    case inputDialogPeerFolder::ID:
      func(static_cast<inputDialogPeerFolder &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputDocument &obj, const T &func) {
  switch (obj.get_id()) {
    case inputDocumentEmpty::ID:
      func(static_cast<inputDocumentEmpty &>(obj));
      return true;
    case inputDocument::ID:
      func(static_cast<inputDocument &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputEncryptedFile &obj, const T &func) {
  switch (obj.get_id()) {
    case inputEncryptedFileEmpty::ID:
      func(static_cast<inputEncryptedFileEmpty &>(obj));
      return true;
    case inputEncryptedFileUploaded::ID:
      func(static_cast<inputEncryptedFileUploaded &>(obj));
      return true;
    case inputEncryptedFile::ID:
      func(static_cast<inputEncryptedFile &>(obj));
      return true;
    case inputEncryptedFileBigUploaded::ID:
      func(static_cast<inputEncryptedFileBigUploaded &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputFile &obj, const T &func) {
  switch (obj.get_id()) {
    case inputFile::ID:
      func(static_cast<inputFile &>(obj));
      return true;
    case inputFileBig::ID:
      func(static_cast<inputFileBig &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputFileLocation &obj, const T &func) {
  switch (obj.get_id()) {
    case inputPeerPhotoFileLocationLegacy::ID:
      func(static_cast<inputPeerPhotoFileLocationLegacy &>(obj));
      return true;
    case inputStickerSetThumbLegacy::ID:
      func(static_cast<inputStickerSetThumbLegacy &>(obj));
      return true;
    case inputFileLocation::ID:
      func(static_cast<inputFileLocation &>(obj));
      return true;
    case inputEncryptedFileLocation::ID:
      func(static_cast<inputEncryptedFileLocation &>(obj));
      return true;
    case inputDocumentFileLocation::ID:
      func(static_cast<inputDocumentFileLocation &>(obj));
      return true;
    case inputSecureFileLocation::ID:
      func(static_cast<inputSecureFileLocation &>(obj));
      return true;
    case inputTakeoutFileLocation::ID:
      func(static_cast<inputTakeoutFileLocation &>(obj));
      return true;
    case inputPhotoFileLocation::ID:
      func(static_cast<inputPhotoFileLocation &>(obj));
      return true;
    case inputPhotoLegacyFileLocation::ID:
      func(static_cast<inputPhotoLegacyFileLocation &>(obj));
      return true;
    case inputPeerPhotoFileLocation::ID:
      func(static_cast<inputPeerPhotoFileLocation &>(obj));
      return true;
    case inputStickerSetThumb::ID:
      func(static_cast<inputStickerSetThumb &>(obj));
      return true;
    case inputGroupCallStream::ID:
      func(static_cast<inputGroupCallStream &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputGame &obj, const T &func) {
  switch (obj.get_id()) {
    case inputGameID::ID:
      func(static_cast<inputGameID &>(obj));
      return true;
    case inputGameShortName::ID:
      func(static_cast<inputGameShortName &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputGeoPoint &obj, const T &func) {
  switch (obj.get_id()) {
    case inputGeoPointEmpty::ID:
      func(static_cast<inputGeoPointEmpty &>(obj));
      return true;
    case inputGeoPoint::ID:
      func(static_cast<inputGeoPoint &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputInvoice &obj, const T &func) {
  switch (obj.get_id()) {
    case inputInvoiceMessage::ID:
      func(static_cast<inputInvoiceMessage &>(obj));
      return true;
    case inputInvoiceSlug::ID:
      func(static_cast<inputInvoiceSlug &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputMedia &obj, const T &func) {
  switch (obj.get_id()) {
    case inputMediaEmpty::ID:
      func(static_cast<inputMediaEmpty &>(obj));
      return true;
    case inputMediaUploadedPhoto::ID:
      func(static_cast<inputMediaUploadedPhoto &>(obj));
      return true;
    case inputMediaPhoto::ID:
      func(static_cast<inputMediaPhoto &>(obj));
      return true;
    case inputMediaGeoPoint::ID:
      func(static_cast<inputMediaGeoPoint &>(obj));
      return true;
    case inputMediaContact::ID:
      func(static_cast<inputMediaContact &>(obj));
      return true;
    case inputMediaUploadedDocument::ID:
      func(static_cast<inputMediaUploadedDocument &>(obj));
      return true;
    case inputMediaDocument::ID:
      func(static_cast<inputMediaDocument &>(obj));
      return true;
    case inputMediaVenue::ID:
      func(static_cast<inputMediaVenue &>(obj));
      return true;
    case inputMediaPhotoExternal::ID:
      func(static_cast<inputMediaPhotoExternal &>(obj));
      return true;
    case inputMediaDocumentExternal::ID:
      func(static_cast<inputMediaDocumentExternal &>(obj));
      return true;
    case inputMediaGame::ID:
      func(static_cast<inputMediaGame &>(obj));
      return true;
    case inputMediaInvoice::ID:
      func(static_cast<inputMediaInvoice &>(obj));
      return true;
    case inputMediaGeoLive::ID:
      func(static_cast<inputMediaGeoLive &>(obj));
      return true;
    case inputMediaPoll::ID:
      func(static_cast<inputMediaPoll &>(obj));
      return true;
    case inputMediaDice::ID:
      func(static_cast<inputMediaDice &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputMessage &obj, const T &func) {
  switch (obj.get_id()) {
    case inputMessageID::ID:
      func(static_cast<inputMessageID &>(obj));
      return true;
    case inputMessageReplyTo::ID:
      func(static_cast<inputMessageReplyTo &>(obj));
      return true;
    case inputMessagePinned::ID:
      func(static_cast<inputMessagePinned &>(obj));
      return true;
    case inputMessageCallbackQuery::ID:
      func(static_cast<inputMessageCallbackQuery &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputNotifyPeer &obj, const T &func) {
  switch (obj.get_id()) {
    case inputNotifyPeer::ID:
      func(static_cast<inputNotifyPeer &>(obj));
      return true;
    case inputNotifyUsers::ID:
      func(static_cast<inputNotifyUsers &>(obj));
      return true;
    case inputNotifyChats::ID:
      func(static_cast<inputNotifyChats &>(obj));
      return true;
    case inputNotifyBroadcasts::ID:
      func(static_cast<inputNotifyBroadcasts &>(obj));
      return true;
    case inputNotifyForumTopic::ID:
      func(static_cast<inputNotifyForumTopic &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputPaymentCredentials &obj, const T &func) {
  switch (obj.get_id()) {
    case inputPaymentCredentialsSaved::ID:
      func(static_cast<inputPaymentCredentialsSaved &>(obj));
      return true;
    case inputPaymentCredentials::ID:
      func(static_cast<inputPaymentCredentials &>(obj));
      return true;
    case inputPaymentCredentialsApplePay::ID:
      func(static_cast<inputPaymentCredentialsApplePay &>(obj));
      return true;
    case inputPaymentCredentialsGooglePay::ID:
      func(static_cast<inputPaymentCredentialsGooglePay &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputPeer &obj, const T &func) {
  switch (obj.get_id()) {
    case inputPeerEmpty::ID:
      func(static_cast<inputPeerEmpty &>(obj));
      return true;
    case inputPeerSelf::ID:
      func(static_cast<inputPeerSelf &>(obj));
      return true;
    case inputPeerChat::ID:
      func(static_cast<inputPeerChat &>(obj));
      return true;
    case inputPeerUser::ID:
      func(static_cast<inputPeerUser &>(obj));
      return true;
    case inputPeerChannel::ID:
      func(static_cast<inputPeerChannel &>(obj));
      return true;
    case inputPeerUserFromMessage::ID:
      func(static_cast<inputPeerUserFromMessage &>(obj));
      return true;
    case inputPeerChannelFromMessage::ID:
      func(static_cast<inputPeerChannelFromMessage &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputPhoto &obj, const T &func) {
  switch (obj.get_id()) {
    case inputPhotoEmpty::ID:
      func(static_cast<inputPhotoEmpty &>(obj));
      return true;
    case inputPhoto::ID:
      func(static_cast<inputPhoto &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputPrivacyKey &obj, const T &func) {
  switch (obj.get_id()) {
    case inputPrivacyKeyStatusTimestamp::ID:
      func(static_cast<inputPrivacyKeyStatusTimestamp &>(obj));
      return true;
    case inputPrivacyKeyChatInvite::ID:
      func(static_cast<inputPrivacyKeyChatInvite &>(obj));
      return true;
    case inputPrivacyKeyPhoneCall::ID:
      func(static_cast<inputPrivacyKeyPhoneCall &>(obj));
      return true;
    case inputPrivacyKeyPhoneP2P::ID:
      func(static_cast<inputPrivacyKeyPhoneP2P &>(obj));
      return true;
    case inputPrivacyKeyForwards::ID:
      func(static_cast<inputPrivacyKeyForwards &>(obj));
      return true;
    case inputPrivacyKeyProfilePhoto::ID:
      func(static_cast<inputPrivacyKeyProfilePhoto &>(obj));
      return true;
    case inputPrivacyKeyPhoneNumber::ID:
      func(static_cast<inputPrivacyKeyPhoneNumber &>(obj));
      return true;
    case inputPrivacyKeyAddedByPhone::ID:
      func(static_cast<inputPrivacyKeyAddedByPhone &>(obj));
      return true;
    case inputPrivacyKeyVoiceMessages::ID:
      func(static_cast<inputPrivacyKeyVoiceMessages &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputPrivacyRule &obj, const T &func) {
  switch (obj.get_id()) {
    case inputPrivacyValueAllowContacts::ID:
      func(static_cast<inputPrivacyValueAllowContacts &>(obj));
      return true;
    case inputPrivacyValueAllowAll::ID:
      func(static_cast<inputPrivacyValueAllowAll &>(obj));
      return true;
    case inputPrivacyValueAllowUsers::ID:
      func(static_cast<inputPrivacyValueAllowUsers &>(obj));
      return true;
    case inputPrivacyValueDisallowContacts::ID:
      func(static_cast<inputPrivacyValueDisallowContacts &>(obj));
      return true;
    case inputPrivacyValueDisallowAll::ID:
      func(static_cast<inputPrivacyValueDisallowAll &>(obj));
      return true;
    case inputPrivacyValueDisallowUsers::ID:
      func(static_cast<inputPrivacyValueDisallowUsers &>(obj));
      return true;
    case inputPrivacyValueAllowChatParticipants::ID:
      func(static_cast<inputPrivacyValueAllowChatParticipants &>(obj));
      return true;
    case inputPrivacyValueDisallowChatParticipants::ID:
      func(static_cast<inputPrivacyValueDisallowChatParticipants &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputSecureFile &obj, const T &func) {
  switch (obj.get_id()) {
    case inputSecureFileUploaded::ID:
      func(static_cast<inputSecureFileUploaded &>(obj));
      return true;
    case inputSecureFile::ID:
      func(static_cast<inputSecureFile &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputStickerSet &obj, const T &func) {
  switch (obj.get_id()) {
    case inputStickerSetEmpty::ID:
      func(static_cast<inputStickerSetEmpty &>(obj));
      return true;
    case inputStickerSetID::ID:
      func(static_cast<inputStickerSetID &>(obj));
      return true;
    case inputStickerSetShortName::ID:
      func(static_cast<inputStickerSetShortName &>(obj));
      return true;
    case inputStickerSetAnimatedEmoji::ID:
      func(static_cast<inputStickerSetAnimatedEmoji &>(obj));
      return true;
    case inputStickerSetDice::ID:
      func(static_cast<inputStickerSetDice &>(obj));
      return true;
    case inputStickerSetAnimatedEmojiAnimations::ID:
      func(static_cast<inputStickerSetAnimatedEmojiAnimations &>(obj));
      return true;
    case inputStickerSetPremiumGifts::ID:
      func(static_cast<inputStickerSetPremiumGifts &>(obj));
      return true;
    case inputStickerSetEmojiGenericAnimations::ID:
      func(static_cast<inputStickerSetEmojiGenericAnimations &>(obj));
      return true;
    case inputStickerSetEmojiDefaultStatuses::ID:
      func(static_cast<inputStickerSetEmojiDefaultStatuses &>(obj));
      return true;
    case inputStickerSetEmojiDefaultTopicIcons::ID:
      func(static_cast<inputStickerSetEmojiDefaultTopicIcons &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputStickeredMedia &obj, const T &func) {
  switch (obj.get_id()) {
    case inputStickeredMediaPhoto::ID:
      func(static_cast<inputStickeredMediaPhoto &>(obj));
      return true;
    case inputStickeredMediaDocument::ID:
      func(static_cast<inputStickeredMediaDocument &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputStorePaymentPurpose &obj, const T &func) {
  switch (obj.get_id()) {
    case inputStorePaymentPremiumSubscription::ID:
      func(static_cast<inputStorePaymentPremiumSubscription &>(obj));
      return true;
    case inputStorePaymentGiftPremium::ID:
      func(static_cast<inputStorePaymentGiftPremium &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputTheme &obj, const T &func) {
  switch (obj.get_id()) {
    case inputTheme::ID:
      func(static_cast<inputTheme &>(obj));
      return true;
    case inputThemeSlug::ID:
      func(static_cast<inputThemeSlug &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputUser &obj, const T &func) {
  switch (obj.get_id()) {
    case inputUserEmpty::ID:
      func(static_cast<inputUserEmpty &>(obj));
      return true;
    case inputUserSelf::ID:
      func(static_cast<inputUserSelf &>(obj));
      return true;
    case inputUser::ID:
      func(static_cast<inputUser &>(obj));
      return true;
    case inputUserFromMessage::ID:
      func(static_cast<inputUserFromMessage &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputWallPaper &obj, const T &func) {
  switch (obj.get_id()) {
    case inputWallPaper::ID:
      func(static_cast<inputWallPaper &>(obj));
      return true;
    case inputWallPaperSlug::ID:
      func(static_cast<inputWallPaperSlug &>(obj));
      return true;
    case inputWallPaperNoFile::ID:
      func(static_cast<inputWallPaperNoFile &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(InputWebFileLocation &obj, const T &func) {
  switch (obj.get_id()) {
    case inputWebFileLocation::ID:
      func(static_cast<inputWebFileLocation &>(obj));
      return true;
    case inputWebFileGeoPointLocation::ID:
      func(static_cast<inputWebFileGeoPointLocation &>(obj));
      return true;
    case inputWebFileAudioAlbumThumbLocation::ID:
      func(static_cast<inputWebFileAudioAlbumThumbLocation &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(IpPort &obj, const T &func) {
  switch (obj.get_id()) {
    case ipPort::ID:
      func(static_cast<ipPort &>(obj));
      return true;
    case ipPortSecret::ID:
      func(static_cast<ipPortSecret &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(JSONValue &obj, const T &func) {
  switch (obj.get_id()) {
    case jsonNull::ID:
      func(static_cast<jsonNull &>(obj));
      return true;
    case jsonBool::ID:
      func(static_cast<jsonBool &>(obj));
      return true;
    case jsonNumber::ID:
      func(static_cast<jsonNumber &>(obj));
      return true;
    case jsonString::ID:
      func(static_cast<jsonString &>(obj));
      return true;
    case jsonArray::ID:
      func(static_cast<jsonArray &>(obj));
      return true;
    case jsonObject::ID:
      func(static_cast<jsonObject &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(KeyboardButton &obj, const T &func) {
  switch (obj.get_id()) {
    case keyboardButton::ID:
      func(static_cast<keyboardButton &>(obj));
      return true;
    case keyboardButtonUrl::ID:
      func(static_cast<keyboardButtonUrl &>(obj));
      return true;
    case keyboardButtonCallback::ID:
      func(static_cast<keyboardButtonCallback &>(obj));
      return true;
    case keyboardButtonRequestPhone::ID:
      func(static_cast<keyboardButtonRequestPhone &>(obj));
      return true;
    case keyboardButtonRequestGeoLocation::ID:
      func(static_cast<keyboardButtonRequestGeoLocation &>(obj));
      return true;
    case keyboardButtonSwitchInline::ID:
      func(static_cast<keyboardButtonSwitchInline &>(obj));
      return true;
    case keyboardButtonGame::ID:
      func(static_cast<keyboardButtonGame &>(obj));
      return true;
    case keyboardButtonBuy::ID:
      func(static_cast<keyboardButtonBuy &>(obj));
      return true;
    case keyboardButtonUrlAuth::ID:
      func(static_cast<keyboardButtonUrlAuth &>(obj));
      return true;
    case inputKeyboardButtonUrlAuth::ID:
      func(static_cast<inputKeyboardButtonUrlAuth &>(obj));
      return true;
    case keyboardButtonRequestPoll::ID:
      func(static_cast<keyboardButtonRequestPoll &>(obj));
      return true;
    case inputKeyboardButtonUserProfile::ID:
      func(static_cast<inputKeyboardButtonUserProfile &>(obj));
      return true;
    case keyboardButtonUserProfile::ID:
      func(static_cast<keyboardButtonUserProfile &>(obj));
      return true;
    case keyboardButtonWebView::ID:
      func(static_cast<keyboardButtonWebView &>(obj));
      return true;
    case keyboardButtonSimpleWebView::ID:
      func(static_cast<keyboardButtonSimpleWebView &>(obj));
      return true;
    case keyboardButtonRequestPeer::ID:
      func(static_cast<keyboardButtonRequestPeer &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(LangPackString &obj, const T &func) {
  switch (obj.get_id()) {
    case langPackString::ID:
      func(static_cast<langPackString &>(obj));
      return true;
    case langPackStringPluralized::ID:
      func(static_cast<langPackStringPluralized &>(obj));
      return true;
    case langPackStringDeleted::ID:
      func(static_cast<langPackStringDeleted &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(Message &obj, const T &func) {
  switch (obj.get_id()) {
    case messageEmpty::ID:
      func(static_cast<messageEmpty &>(obj));
      return true;
    case message::ID:
      func(static_cast<message &>(obj));
      return true;
    case messageService::ID:
      func(static_cast<messageService &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(MessageAction &obj, const T &func) {
  switch (obj.get_id()) {
    case messageActionEmpty::ID:
      func(static_cast<messageActionEmpty &>(obj));
      return true;
    case messageActionChatCreate::ID:
      func(static_cast<messageActionChatCreate &>(obj));
      return true;
    case messageActionChatEditTitle::ID:
      func(static_cast<messageActionChatEditTitle &>(obj));
      return true;
    case messageActionChatEditPhoto::ID:
      func(static_cast<messageActionChatEditPhoto &>(obj));
      return true;
    case messageActionChatDeletePhoto::ID:
      func(static_cast<messageActionChatDeletePhoto &>(obj));
      return true;
    case messageActionChatAddUser::ID:
      func(static_cast<messageActionChatAddUser &>(obj));
      return true;
    case messageActionChatDeleteUser::ID:
      func(static_cast<messageActionChatDeleteUser &>(obj));
      return true;
    case messageActionChatJoinedByLink::ID:
      func(static_cast<messageActionChatJoinedByLink &>(obj));
      return true;
    case messageActionChannelCreate::ID:
      func(static_cast<messageActionChannelCreate &>(obj));
      return true;
    case messageActionChatMigrateTo::ID:
      func(static_cast<messageActionChatMigrateTo &>(obj));
      return true;
    case messageActionChannelMigrateFrom::ID:
      func(static_cast<messageActionChannelMigrateFrom &>(obj));
      return true;
    case messageActionPinMessage::ID:
      func(static_cast<messageActionPinMessage &>(obj));
      return true;
    case messageActionHistoryClear::ID:
      func(static_cast<messageActionHistoryClear &>(obj));
      return true;
    case messageActionGameScore::ID:
      func(static_cast<messageActionGameScore &>(obj));
      return true;
    case messageActionPaymentSentMe::ID:
      func(static_cast<messageActionPaymentSentMe &>(obj));
      return true;
    case messageActionPaymentSent::ID:
      func(static_cast<messageActionPaymentSent &>(obj));
      return true;
    case messageActionPhoneCall::ID:
      func(static_cast<messageActionPhoneCall &>(obj));
      return true;
    case messageActionScreenshotTaken::ID:
      func(static_cast<messageActionScreenshotTaken &>(obj));
      return true;
    case messageActionCustomAction::ID:
      func(static_cast<messageActionCustomAction &>(obj));
      return true;
    case messageActionBotAllowed::ID:
      func(static_cast<messageActionBotAllowed &>(obj));
      return true;
    case messageActionSecureValuesSentMe::ID:
      func(static_cast<messageActionSecureValuesSentMe &>(obj));
      return true;
    case messageActionSecureValuesSent::ID:
      func(static_cast<messageActionSecureValuesSent &>(obj));
      return true;
    case messageActionContactSignUp::ID:
      func(static_cast<messageActionContactSignUp &>(obj));
      return true;
    case messageActionGeoProximityReached::ID:
      func(static_cast<messageActionGeoProximityReached &>(obj));
      return true;
    case messageActionGroupCall::ID:
      func(static_cast<messageActionGroupCall &>(obj));
      return true;
    case messageActionInviteToGroupCall::ID:
      func(static_cast<messageActionInviteToGroupCall &>(obj));
      return true;
    case messageActionSetMessagesTTL::ID:
      func(static_cast<messageActionSetMessagesTTL &>(obj));
      return true;
    case messageActionGroupCallScheduled::ID:
      func(static_cast<messageActionGroupCallScheduled &>(obj));
      return true;
    case messageActionSetChatTheme::ID:
      func(static_cast<messageActionSetChatTheme &>(obj));
      return true;
    case messageActionChatJoinedByRequest::ID:
      func(static_cast<messageActionChatJoinedByRequest &>(obj));
      return true;
    case messageActionWebViewDataSentMe::ID:
      func(static_cast<messageActionWebViewDataSentMe &>(obj));
      return true;
    case messageActionWebViewDataSent::ID:
      func(static_cast<messageActionWebViewDataSent &>(obj));
      return true;
    case messageActionGiftPremium::ID:
      func(static_cast<messageActionGiftPremium &>(obj));
      return true;
    case messageActionTopicCreate::ID:
      func(static_cast<messageActionTopicCreate &>(obj));
      return true;
    case messageActionTopicEdit::ID:
      func(static_cast<messageActionTopicEdit &>(obj));
      return true;
    case messageActionSuggestProfilePhoto::ID:
      func(static_cast<messageActionSuggestProfilePhoto &>(obj));
      return true;
    case messageActionRequestedPeer::ID:
      func(static_cast<messageActionRequestedPeer &>(obj));
      return true;
    case messageActionSetChatWallPaper::ID:
      func(static_cast<messageActionSetChatWallPaper &>(obj));
      return true;
    case messageActionSetSameChatWallPaper::ID:
      func(static_cast<messageActionSetSameChatWallPaper &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(MessageEntity &obj, const T &func) {
  switch (obj.get_id()) {
    case messageEntityUnknown::ID:
      func(static_cast<messageEntityUnknown &>(obj));
      return true;
    case messageEntityMention::ID:
      func(static_cast<messageEntityMention &>(obj));
      return true;
    case messageEntityHashtag::ID:
      func(static_cast<messageEntityHashtag &>(obj));
      return true;
    case messageEntityBotCommand::ID:
      func(static_cast<messageEntityBotCommand &>(obj));
      return true;
    case messageEntityUrl::ID:
      func(static_cast<messageEntityUrl &>(obj));
      return true;
    case messageEntityEmail::ID:
      func(static_cast<messageEntityEmail &>(obj));
      return true;
    case messageEntityBold::ID:
      func(static_cast<messageEntityBold &>(obj));
      return true;
    case messageEntityItalic::ID:
      func(static_cast<messageEntityItalic &>(obj));
      return true;
    case messageEntityCode::ID:
      func(static_cast<messageEntityCode &>(obj));
      return true;
    case messageEntityPre::ID:
      func(static_cast<messageEntityPre &>(obj));
      return true;
    case messageEntityTextUrl::ID:
      func(static_cast<messageEntityTextUrl &>(obj));
      return true;
    case messageEntityMentionName::ID:
      func(static_cast<messageEntityMentionName &>(obj));
      return true;
    case inputMessageEntityMentionName::ID:
      func(static_cast<inputMessageEntityMentionName &>(obj));
      return true;
    case messageEntityPhone::ID:
      func(static_cast<messageEntityPhone &>(obj));
      return true;
    case messageEntityCashtag::ID:
      func(static_cast<messageEntityCashtag &>(obj));
      return true;
    case messageEntityUnderline::ID:
      func(static_cast<messageEntityUnderline &>(obj));
      return true;
    case messageEntityStrike::ID:
      func(static_cast<messageEntityStrike &>(obj));
      return true;
    case messageEntityBlockquote::ID:
      func(static_cast<messageEntityBlockquote &>(obj));
      return true;
    case messageEntityBankCard::ID:
      func(static_cast<messageEntityBankCard &>(obj));
      return true;
    case messageEntitySpoiler::ID:
      func(static_cast<messageEntitySpoiler &>(obj));
      return true;
    case messageEntityCustomEmoji::ID:
      func(static_cast<messageEntityCustomEmoji &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(MessageExtendedMedia &obj, const T &func) {
  switch (obj.get_id()) {
    case messageExtendedMediaPreview::ID:
      func(static_cast<messageExtendedMediaPreview &>(obj));
      return true;
    case messageExtendedMedia::ID:
      func(static_cast<messageExtendedMedia &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(MessageMedia &obj, const T &func) {
  switch (obj.get_id()) {
    case messageMediaEmpty::ID:
      func(static_cast<messageMediaEmpty &>(obj));
      return true;
    case messageMediaPhoto::ID:
      func(static_cast<messageMediaPhoto &>(obj));
      return true;
    case messageMediaGeo::ID:
      func(static_cast<messageMediaGeo &>(obj));
      return true;
    case messageMediaContact::ID:
      func(static_cast<messageMediaContact &>(obj));
      return true;
    case messageMediaUnsupported::ID:
      func(static_cast<messageMediaUnsupported &>(obj));
      return true;
    case messageMediaDocument::ID:
      func(static_cast<messageMediaDocument &>(obj));
      return true;
    case messageMediaWebPage::ID:
      func(static_cast<messageMediaWebPage &>(obj));
      return true;
    case messageMediaVenue::ID:
      func(static_cast<messageMediaVenue &>(obj));
      return true;
    case messageMediaGame::ID:
      func(static_cast<messageMediaGame &>(obj));
      return true;
    case messageMediaInvoice::ID:
      func(static_cast<messageMediaInvoice &>(obj));
      return true;
    case messageMediaGeoLive::ID:
      func(static_cast<messageMediaGeoLive &>(obj));
      return true;
    case messageMediaPoll::ID:
      func(static_cast<messageMediaPoll &>(obj));
      return true;
    case messageMediaDice::ID:
      func(static_cast<messageMediaDice &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(MessageUserVote &obj, const T &func) {
  switch (obj.get_id()) {
    case messageUserVote::ID:
      func(static_cast<messageUserVote &>(obj));
      return true;
    case messageUserVoteInputOption::ID:
      func(static_cast<messageUserVoteInputOption &>(obj));
      return true;
    case messageUserVoteMultiple::ID:
      func(static_cast<messageUserVoteMultiple &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(MessagesFilter &obj, const T &func) {
  switch (obj.get_id()) {
    case inputMessagesFilterEmpty::ID:
      func(static_cast<inputMessagesFilterEmpty &>(obj));
      return true;
    case inputMessagesFilterPhotos::ID:
      func(static_cast<inputMessagesFilterPhotos &>(obj));
      return true;
    case inputMessagesFilterVideo::ID:
      func(static_cast<inputMessagesFilterVideo &>(obj));
      return true;
    case inputMessagesFilterPhotoVideo::ID:
      func(static_cast<inputMessagesFilterPhotoVideo &>(obj));
      return true;
    case inputMessagesFilterDocument::ID:
      func(static_cast<inputMessagesFilterDocument &>(obj));
      return true;
    case inputMessagesFilterUrl::ID:
      func(static_cast<inputMessagesFilterUrl &>(obj));
      return true;
    case inputMessagesFilterGif::ID:
      func(static_cast<inputMessagesFilterGif &>(obj));
      return true;
    case inputMessagesFilterVoice::ID:
      func(static_cast<inputMessagesFilterVoice &>(obj));
      return true;
    case inputMessagesFilterMusic::ID:
      func(static_cast<inputMessagesFilterMusic &>(obj));
      return true;
    case inputMessagesFilterChatPhotos::ID:
      func(static_cast<inputMessagesFilterChatPhotos &>(obj));
      return true;
    case inputMessagesFilterPhoneCalls::ID:
      func(static_cast<inputMessagesFilterPhoneCalls &>(obj));
      return true;
    case inputMessagesFilterRoundVoice::ID:
      func(static_cast<inputMessagesFilterRoundVoice &>(obj));
      return true;
    case inputMessagesFilterRoundVideo::ID:
      func(static_cast<inputMessagesFilterRoundVideo &>(obj));
      return true;
    case inputMessagesFilterMyMentions::ID:
      func(static_cast<inputMessagesFilterMyMentions &>(obj));
      return true;
    case inputMessagesFilterGeo::ID:
      func(static_cast<inputMessagesFilterGeo &>(obj));
      return true;
    case inputMessagesFilterContacts::ID:
      func(static_cast<inputMessagesFilterContacts &>(obj));
      return true;
    case inputMessagesFilterPinned::ID:
      func(static_cast<inputMessagesFilterPinned &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(NotificationSound &obj, const T &func) {
  switch (obj.get_id()) {
    case notificationSoundDefault::ID:
      func(static_cast<notificationSoundDefault &>(obj));
      return true;
    case notificationSoundNone::ID:
      func(static_cast<notificationSoundNone &>(obj));
      return true;
    case notificationSoundLocal::ID:
      func(static_cast<notificationSoundLocal &>(obj));
      return true;
    case notificationSoundRingtone::ID:
      func(static_cast<notificationSoundRingtone &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(NotifyPeer &obj, const T &func) {
  switch (obj.get_id()) {
    case notifyPeer::ID:
      func(static_cast<notifyPeer &>(obj));
      return true;
    case notifyUsers::ID:
      func(static_cast<notifyUsers &>(obj));
      return true;
    case notifyChats::ID:
      func(static_cast<notifyChats &>(obj));
      return true;
    case notifyBroadcasts::ID:
      func(static_cast<notifyBroadcasts &>(obj));
      return true;
    case notifyForumTopic::ID:
      func(static_cast<notifyForumTopic &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(PageBlock &obj, const T &func) {
  switch (obj.get_id()) {
    case pageBlockUnsupported::ID:
      func(static_cast<pageBlockUnsupported &>(obj));
      return true;
    case pageBlockTitle::ID:
      func(static_cast<pageBlockTitle &>(obj));
      return true;
    case pageBlockSubtitle::ID:
      func(static_cast<pageBlockSubtitle &>(obj));
      return true;
    case pageBlockAuthorDate::ID:
      func(static_cast<pageBlockAuthorDate &>(obj));
      return true;
    case pageBlockHeader::ID:
      func(static_cast<pageBlockHeader &>(obj));
      return true;
    case pageBlockSubheader::ID:
      func(static_cast<pageBlockSubheader &>(obj));
      return true;
    case pageBlockParagraph::ID:
      func(static_cast<pageBlockParagraph &>(obj));
      return true;
    case pageBlockPreformatted::ID:
      func(static_cast<pageBlockPreformatted &>(obj));
      return true;
    case pageBlockFooter::ID:
      func(static_cast<pageBlockFooter &>(obj));
      return true;
    case pageBlockDivider::ID:
      func(static_cast<pageBlockDivider &>(obj));
      return true;
    case pageBlockAnchor::ID:
      func(static_cast<pageBlockAnchor &>(obj));
      return true;
    case pageBlockList::ID:
      func(static_cast<pageBlockList &>(obj));
      return true;
    case pageBlockBlockquote::ID:
      func(static_cast<pageBlockBlockquote &>(obj));
      return true;
    case pageBlockPullquote::ID:
      func(static_cast<pageBlockPullquote &>(obj));
      return true;
    case pageBlockPhoto::ID:
      func(static_cast<pageBlockPhoto &>(obj));
      return true;
    case pageBlockVideo::ID:
      func(static_cast<pageBlockVideo &>(obj));
      return true;
    case pageBlockCover::ID:
      func(static_cast<pageBlockCover &>(obj));
      return true;
    case pageBlockEmbed::ID:
      func(static_cast<pageBlockEmbed &>(obj));
      return true;
    case pageBlockEmbedPost::ID:
      func(static_cast<pageBlockEmbedPost &>(obj));
      return true;
    case pageBlockCollage::ID:
      func(static_cast<pageBlockCollage &>(obj));
      return true;
    case pageBlockSlideshow::ID:
      func(static_cast<pageBlockSlideshow &>(obj));
      return true;
    case pageBlockChannel::ID:
      func(static_cast<pageBlockChannel &>(obj));
      return true;
    case pageBlockAudio::ID:
      func(static_cast<pageBlockAudio &>(obj));
      return true;
    case pageBlockKicker::ID:
      func(static_cast<pageBlockKicker &>(obj));
      return true;
    case pageBlockTable::ID:
      func(static_cast<pageBlockTable &>(obj));
      return true;
    case pageBlockOrderedList::ID:
      func(static_cast<pageBlockOrderedList &>(obj));
      return true;
    case pageBlockDetails::ID:
      func(static_cast<pageBlockDetails &>(obj));
      return true;
    case pageBlockRelatedArticles::ID:
      func(static_cast<pageBlockRelatedArticles &>(obj));
      return true;
    case pageBlockMap::ID:
      func(static_cast<pageBlockMap &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(PageListItem &obj, const T &func) {
  switch (obj.get_id()) {
    case pageListItemText::ID:
      func(static_cast<pageListItemText &>(obj));
      return true;
    case pageListItemBlocks::ID:
      func(static_cast<pageListItemBlocks &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(PageListOrderedItem &obj, const T &func) {
  switch (obj.get_id()) {
    case pageListOrderedItemText::ID:
      func(static_cast<pageListOrderedItemText &>(obj));
      return true;
    case pageListOrderedItemBlocks::ID:
      func(static_cast<pageListOrderedItemBlocks &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(PasswordKdfAlgo &obj, const T &func) {
  switch (obj.get_id()) {
    case passwordKdfAlgoUnknown::ID:
      func(static_cast<passwordKdfAlgoUnknown &>(obj));
      return true;
    case passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow::ID:
      func(static_cast<passwordKdfAlgoSHA256SHA256PBKDF2HMACSHA512iter100000SHA256ModPow &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(Peer &obj, const T &func) {
  switch (obj.get_id()) {
    case peerUser::ID:
      func(static_cast<peerUser &>(obj));
      return true;
    case peerChat::ID:
      func(static_cast<peerChat &>(obj));
      return true;
    case peerChannel::ID:
      func(static_cast<peerChannel &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(PeerLocated &obj, const T &func) {
  switch (obj.get_id()) {
    case peerLocated::ID:
      func(static_cast<peerLocated &>(obj));
      return true;
    case peerSelfLocated::ID:
      func(static_cast<peerSelfLocated &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(PhoneCall &obj, const T &func) {
  switch (obj.get_id()) {
    case phoneCallEmpty::ID:
      func(static_cast<phoneCallEmpty &>(obj));
      return true;
    case phoneCallWaiting::ID:
      func(static_cast<phoneCallWaiting &>(obj));
      return true;
    case phoneCallRequested::ID:
      func(static_cast<phoneCallRequested &>(obj));
      return true;
    case phoneCallAccepted::ID:
      func(static_cast<phoneCallAccepted &>(obj));
      return true;
    case phoneCall::ID:
      func(static_cast<phoneCall &>(obj));
      return true;
    case phoneCallDiscarded::ID:
      func(static_cast<phoneCallDiscarded &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(PhoneCallDiscardReason &obj, const T &func) {
  switch (obj.get_id()) {
    case phoneCallDiscardReasonMissed::ID:
      func(static_cast<phoneCallDiscardReasonMissed &>(obj));
      return true;
    case phoneCallDiscardReasonDisconnect::ID:
      func(static_cast<phoneCallDiscardReasonDisconnect &>(obj));
      return true;
    case phoneCallDiscardReasonHangup::ID:
      func(static_cast<phoneCallDiscardReasonHangup &>(obj));
      return true;
    case phoneCallDiscardReasonBusy::ID:
      func(static_cast<phoneCallDiscardReasonBusy &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(PhoneConnection &obj, const T &func) {
  switch (obj.get_id()) {
    case phoneConnection::ID:
      func(static_cast<phoneConnection &>(obj));
      return true;
    case phoneConnectionWebrtc::ID:
      func(static_cast<phoneConnectionWebrtc &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(Photo &obj, const T &func) {
  switch (obj.get_id()) {
    case photoEmpty::ID:
      func(static_cast<photoEmpty &>(obj));
      return true;
    case photo::ID:
      func(static_cast<photo &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(PhotoSize &obj, const T &func) {
  switch (obj.get_id()) {
    case photoSizeEmpty::ID:
      func(static_cast<photoSizeEmpty &>(obj));
      return true;
    case photoSize::ID:
      func(static_cast<photoSize &>(obj));
      return true;
    case photoCachedSize::ID:
      func(static_cast<photoCachedSize &>(obj));
      return true;
    case photoStrippedSize::ID:
      func(static_cast<photoStrippedSize &>(obj));
      return true;
    case photoSizeProgressive::ID:
      func(static_cast<photoSizeProgressive &>(obj));
      return true;
    case photoPathSize::ID:
      func(static_cast<photoPathSize &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(PrivacyKey &obj, const T &func) {
  switch (obj.get_id()) {
    case privacyKeyStatusTimestamp::ID:
      func(static_cast<privacyKeyStatusTimestamp &>(obj));
      return true;
    case privacyKeyChatInvite::ID:
      func(static_cast<privacyKeyChatInvite &>(obj));
      return true;
    case privacyKeyPhoneCall::ID:
      func(static_cast<privacyKeyPhoneCall &>(obj));
      return true;
    case privacyKeyPhoneP2P::ID:
      func(static_cast<privacyKeyPhoneP2P &>(obj));
      return true;
    case privacyKeyForwards::ID:
      func(static_cast<privacyKeyForwards &>(obj));
      return true;
    case privacyKeyProfilePhoto::ID:
      func(static_cast<privacyKeyProfilePhoto &>(obj));
      return true;
    case privacyKeyPhoneNumber::ID:
      func(static_cast<privacyKeyPhoneNumber &>(obj));
      return true;
    case privacyKeyAddedByPhone::ID:
      func(static_cast<privacyKeyAddedByPhone &>(obj));
      return true;
    case privacyKeyVoiceMessages::ID:
      func(static_cast<privacyKeyVoiceMessages &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(PrivacyRule &obj, const T &func) {
  switch (obj.get_id()) {
    case privacyValueAllowContacts::ID:
      func(static_cast<privacyValueAllowContacts &>(obj));
      return true;
    case privacyValueAllowAll::ID:
      func(static_cast<privacyValueAllowAll &>(obj));
      return true;
    case privacyValueAllowUsers::ID:
      func(static_cast<privacyValueAllowUsers &>(obj));
      return true;
    case privacyValueDisallowContacts::ID:
      func(static_cast<privacyValueDisallowContacts &>(obj));
      return true;
    case privacyValueDisallowAll::ID:
      func(static_cast<privacyValueDisallowAll &>(obj));
      return true;
    case privacyValueDisallowUsers::ID:
      func(static_cast<privacyValueDisallowUsers &>(obj));
      return true;
    case privacyValueAllowChatParticipants::ID:
      func(static_cast<privacyValueAllowChatParticipants &>(obj));
      return true;
    case privacyValueDisallowChatParticipants::ID:
      func(static_cast<privacyValueDisallowChatParticipants &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(Reaction &obj, const T &func) {
  switch (obj.get_id()) {
    case reactionEmpty::ID:
      func(static_cast<reactionEmpty &>(obj));
      return true;
    case reactionEmoji::ID:
      func(static_cast<reactionEmoji &>(obj));
      return true;
    case reactionCustomEmoji::ID:
      func(static_cast<reactionCustomEmoji &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(RecentMeUrl &obj, const T &func) {
  switch (obj.get_id()) {
    case recentMeUrlUnknown::ID:
      func(static_cast<recentMeUrlUnknown &>(obj));
      return true;
    case recentMeUrlUser::ID:
      func(static_cast<recentMeUrlUser &>(obj));
      return true;
    case recentMeUrlChat::ID:
      func(static_cast<recentMeUrlChat &>(obj));
      return true;
    case recentMeUrlChatInvite::ID:
      func(static_cast<recentMeUrlChatInvite &>(obj));
      return true;
    case recentMeUrlStickerSet::ID:
      func(static_cast<recentMeUrlStickerSet &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(ReplyMarkup &obj, const T &func) {
  switch (obj.get_id()) {
    case replyKeyboardHide::ID:
      func(static_cast<replyKeyboardHide &>(obj));
      return true;
    case replyKeyboardForceReply::ID:
      func(static_cast<replyKeyboardForceReply &>(obj));
      return true;
    case replyKeyboardMarkup::ID:
      func(static_cast<replyKeyboardMarkup &>(obj));
      return true;
    case replyInlineMarkup::ID:
      func(static_cast<replyInlineMarkup &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(ReportReason &obj, const T &func) {
  switch (obj.get_id()) {
    case inputReportReasonSpam::ID:
      func(static_cast<inputReportReasonSpam &>(obj));
      return true;
    case inputReportReasonViolence::ID:
      func(static_cast<inputReportReasonViolence &>(obj));
      return true;
    case inputReportReasonPornography::ID:
      func(static_cast<inputReportReasonPornography &>(obj));
      return true;
    case inputReportReasonChildAbuse::ID:
      func(static_cast<inputReportReasonChildAbuse &>(obj));
      return true;
    case inputReportReasonOther::ID:
      func(static_cast<inputReportReasonOther &>(obj));
      return true;
    case inputReportReasonCopyright::ID:
      func(static_cast<inputReportReasonCopyright &>(obj));
      return true;
    case inputReportReasonGeoIrrelevant::ID:
      func(static_cast<inputReportReasonGeoIrrelevant &>(obj));
      return true;
    case inputReportReasonFake::ID:
      func(static_cast<inputReportReasonFake &>(obj));
      return true;
    case inputReportReasonIllegalDrugs::ID:
      func(static_cast<inputReportReasonIllegalDrugs &>(obj));
      return true;
    case inputReportReasonPersonalDetails::ID:
      func(static_cast<inputReportReasonPersonalDetails &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(RequestPeerType &obj, const T &func) {
  switch (obj.get_id()) {
    case requestPeerTypeUser::ID:
      func(static_cast<requestPeerTypeUser &>(obj));
      return true;
    case requestPeerTypeChat::ID:
      func(static_cast<requestPeerTypeChat &>(obj));
      return true;
    case requestPeerTypeBroadcast::ID:
      func(static_cast<requestPeerTypeBroadcast &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(RichText &obj, const T &func) {
  switch (obj.get_id()) {
    case textEmpty::ID:
      func(static_cast<textEmpty &>(obj));
      return true;
    case textPlain::ID:
      func(static_cast<textPlain &>(obj));
      return true;
    case textBold::ID:
      func(static_cast<textBold &>(obj));
      return true;
    case textItalic::ID:
      func(static_cast<textItalic &>(obj));
      return true;
    case textUnderline::ID:
      func(static_cast<textUnderline &>(obj));
      return true;
    case textStrike::ID:
      func(static_cast<textStrike &>(obj));
      return true;
    case textFixed::ID:
      func(static_cast<textFixed &>(obj));
      return true;
    case textUrl::ID:
      func(static_cast<textUrl &>(obj));
      return true;
    case textEmail::ID:
      func(static_cast<textEmail &>(obj));
      return true;
    case textConcat::ID:
      func(static_cast<textConcat &>(obj));
      return true;
    case textSubscript::ID:
      func(static_cast<textSubscript &>(obj));
      return true;
    case textSuperscript::ID:
      func(static_cast<textSuperscript &>(obj));
      return true;
    case textMarked::ID:
      func(static_cast<textMarked &>(obj));
      return true;
    case textPhone::ID:
      func(static_cast<textPhone &>(obj));
      return true;
    case textImage::ID:
      func(static_cast<textImage &>(obj));
      return true;
    case textAnchor::ID:
      func(static_cast<textAnchor &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(SecureFile &obj, const T &func) {
  switch (obj.get_id()) {
    case secureFileEmpty::ID:
      func(static_cast<secureFileEmpty &>(obj));
      return true;
    case secureFile::ID:
      func(static_cast<secureFile &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(SecurePasswordKdfAlgo &obj, const T &func) {
  switch (obj.get_id()) {
    case securePasswordKdfAlgoUnknown::ID:
      func(static_cast<securePasswordKdfAlgoUnknown &>(obj));
      return true;
    case securePasswordKdfAlgoPBKDF2HMACSHA512iter100000::ID:
      func(static_cast<securePasswordKdfAlgoPBKDF2HMACSHA512iter100000 &>(obj));
      return true;
    case securePasswordKdfAlgoSHA512::ID:
      func(static_cast<securePasswordKdfAlgoSHA512 &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(SecurePlainData &obj, const T &func) {
  switch (obj.get_id()) {
    case securePlainPhone::ID:
      func(static_cast<securePlainPhone &>(obj));
      return true;
    case securePlainEmail::ID:
      func(static_cast<securePlainEmail &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(SecureRequiredType &obj, const T &func) {
  switch (obj.get_id()) {
    case secureRequiredType::ID:
      func(static_cast<secureRequiredType &>(obj));
      return true;
    case secureRequiredTypeOneOf::ID:
      func(static_cast<secureRequiredTypeOneOf &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(SecureValueError &obj, const T &func) {
  switch (obj.get_id()) {
    case secureValueErrorData::ID:
      func(static_cast<secureValueErrorData &>(obj));
      return true;
    case secureValueErrorFrontSide::ID:
      func(static_cast<secureValueErrorFrontSide &>(obj));
      return true;
    case secureValueErrorReverseSide::ID:
      func(static_cast<secureValueErrorReverseSide &>(obj));
      return true;
    case secureValueErrorSelfie::ID:
      func(static_cast<secureValueErrorSelfie &>(obj));
      return true;
    case secureValueErrorFile::ID:
      func(static_cast<secureValueErrorFile &>(obj));
      return true;
    case secureValueErrorFiles::ID:
      func(static_cast<secureValueErrorFiles &>(obj));
      return true;
    case secureValueError::ID:
      func(static_cast<secureValueError &>(obj));
      return true;
    case secureValueErrorTranslationFile::ID:
      func(static_cast<secureValueErrorTranslationFile &>(obj));
      return true;
    case secureValueErrorTranslationFiles::ID:
      func(static_cast<secureValueErrorTranslationFiles &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(SecureValueType &obj, const T &func) {
  switch (obj.get_id()) {
    case secureValueTypePersonalDetails::ID:
      func(static_cast<secureValueTypePersonalDetails &>(obj));
      return true;
    case secureValueTypePassport::ID:
      func(static_cast<secureValueTypePassport &>(obj));
      return true;
    case secureValueTypeDriverLicense::ID:
      func(static_cast<secureValueTypeDriverLicense &>(obj));
      return true;
    case secureValueTypeIdentityCard::ID:
      func(static_cast<secureValueTypeIdentityCard &>(obj));
      return true;
    case secureValueTypeInternalPassport::ID:
      func(static_cast<secureValueTypeInternalPassport &>(obj));
      return true;
    case secureValueTypeAddress::ID:
      func(static_cast<secureValueTypeAddress &>(obj));
      return true;
    case secureValueTypeUtilityBill::ID:
      func(static_cast<secureValueTypeUtilityBill &>(obj));
      return true;
    case secureValueTypeBankStatement::ID:
      func(static_cast<secureValueTypeBankStatement &>(obj));
      return true;
    case secureValueTypeRentalAgreement::ID:
      func(static_cast<secureValueTypeRentalAgreement &>(obj));
      return true;
    case secureValueTypePassportRegistration::ID:
      func(static_cast<secureValueTypePassportRegistration &>(obj));
      return true;
    case secureValueTypeTemporaryRegistration::ID:
      func(static_cast<secureValueTypeTemporaryRegistration &>(obj));
      return true;
    case secureValueTypePhone::ID:
      func(static_cast<secureValueTypePhone &>(obj));
      return true;
    case secureValueTypeEmail::ID:
      func(static_cast<secureValueTypeEmail &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(SendMessageAction &obj, const T &func) {
  switch (obj.get_id()) {
    case sendMessageTypingAction::ID:
      func(static_cast<sendMessageTypingAction &>(obj));
      return true;
    case sendMessageCancelAction::ID:
      func(static_cast<sendMessageCancelAction &>(obj));
      return true;
    case sendMessageRecordVideoAction::ID:
      func(static_cast<sendMessageRecordVideoAction &>(obj));
      return true;
    case sendMessageUploadVideoAction::ID:
      func(static_cast<sendMessageUploadVideoAction &>(obj));
      return true;
    case sendMessageRecordAudioAction::ID:
      func(static_cast<sendMessageRecordAudioAction &>(obj));
      return true;
    case sendMessageUploadAudioAction::ID:
      func(static_cast<sendMessageUploadAudioAction &>(obj));
      return true;
    case sendMessageUploadPhotoAction::ID:
      func(static_cast<sendMessageUploadPhotoAction &>(obj));
      return true;
    case sendMessageUploadDocumentAction::ID:
      func(static_cast<sendMessageUploadDocumentAction &>(obj));
      return true;
    case sendMessageGeoLocationAction::ID:
      func(static_cast<sendMessageGeoLocationAction &>(obj));
      return true;
    case sendMessageChooseContactAction::ID:
      func(static_cast<sendMessageChooseContactAction &>(obj));
      return true;
    case sendMessageGamePlayAction::ID:
      func(static_cast<sendMessageGamePlayAction &>(obj));
      return true;
    case sendMessageRecordRoundAction::ID:
      func(static_cast<sendMessageRecordRoundAction &>(obj));
      return true;
    case sendMessageUploadRoundAction::ID:
      func(static_cast<sendMessageUploadRoundAction &>(obj));
      return true;
    case speakingInGroupCallAction::ID:
      func(static_cast<speakingInGroupCallAction &>(obj));
      return true;
    case sendMessageHistoryImportAction::ID:
      func(static_cast<sendMessageHistoryImportAction &>(obj));
      return true;
    case sendMessageChooseStickerAction::ID:
      func(static_cast<sendMessageChooseStickerAction &>(obj));
      return true;
    case sendMessageEmojiInteraction::ID:
      func(static_cast<sendMessageEmojiInteraction &>(obj));
      return true;
    case sendMessageEmojiInteractionSeen::ID:
      func(static_cast<sendMessageEmojiInteractionSeen &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(StatsGraph &obj, const T &func) {
  switch (obj.get_id()) {
    case statsGraphAsync::ID:
      func(static_cast<statsGraphAsync &>(obj));
      return true;
    case statsGraphError::ID:
      func(static_cast<statsGraphError &>(obj));
      return true;
    case statsGraph::ID:
      func(static_cast<statsGraph &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(StickerSetCovered &obj, const T &func) {
  switch (obj.get_id()) {
    case stickerSetCovered::ID:
      func(static_cast<stickerSetCovered &>(obj));
      return true;
    case stickerSetMultiCovered::ID:
      func(static_cast<stickerSetMultiCovered &>(obj));
      return true;
    case stickerSetFullCovered::ID:
      func(static_cast<stickerSetFullCovered &>(obj));
      return true;
    case stickerSetNoCovered::ID:
      func(static_cast<stickerSetNoCovered &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(TopPeerCategory &obj, const T &func) {
  switch (obj.get_id()) {
    case topPeerCategoryBotsPM::ID:
      func(static_cast<topPeerCategoryBotsPM &>(obj));
      return true;
    case topPeerCategoryBotsInline::ID:
      func(static_cast<topPeerCategoryBotsInline &>(obj));
      return true;
    case topPeerCategoryCorrespondents::ID:
      func(static_cast<topPeerCategoryCorrespondents &>(obj));
      return true;
    case topPeerCategoryGroups::ID:
      func(static_cast<topPeerCategoryGroups &>(obj));
      return true;
    case topPeerCategoryChannels::ID:
      func(static_cast<topPeerCategoryChannels &>(obj));
      return true;
    case topPeerCategoryPhoneCalls::ID:
      func(static_cast<topPeerCategoryPhoneCalls &>(obj));
      return true;
    case topPeerCategoryForwardUsers::ID:
      func(static_cast<topPeerCategoryForwardUsers &>(obj));
      return true;
    case topPeerCategoryForwardChats::ID:
      func(static_cast<topPeerCategoryForwardChats &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(Update &obj, const T &func) {
  switch (obj.get_id()) {
    case updateNewMessage::ID:
      func(static_cast<updateNewMessage &>(obj));
      return true;
    case updateMessageID::ID:
      func(static_cast<updateMessageID &>(obj));
      return true;
    case updateDeleteMessages::ID:
      func(static_cast<updateDeleteMessages &>(obj));
      return true;
    case updateUserTyping::ID:
      func(static_cast<updateUserTyping &>(obj));
      return true;
    case updateChatUserTyping::ID:
      func(static_cast<updateChatUserTyping &>(obj));
      return true;
    case updateChatParticipants::ID:
      func(static_cast<updateChatParticipants &>(obj));
      return true;
    case updateUserStatus::ID:
      func(static_cast<updateUserStatus &>(obj));
      return true;
    case updateUserName::ID:
      func(static_cast<updateUserName &>(obj));
      return true;
    case updateNewEncryptedMessage::ID:
      func(static_cast<updateNewEncryptedMessage &>(obj));
      return true;
    case updateEncryptedChatTyping::ID:
      func(static_cast<updateEncryptedChatTyping &>(obj));
      return true;
    case updateEncryption::ID:
      func(static_cast<updateEncryption &>(obj));
      return true;
    case updateEncryptedMessagesRead::ID:
      func(static_cast<updateEncryptedMessagesRead &>(obj));
      return true;
    case updateChatParticipantAdd::ID:
      func(static_cast<updateChatParticipantAdd &>(obj));
      return true;
    case updateChatParticipantDelete::ID:
      func(static_cast<updateChatParticipantDelete &>(obj));
      return true;
    case updateDcOptions::ID:
      func(static_cast<updateDcOptions &>(obj));
      return true;
    case updateNotifySettings::ID:
      func(static_cast<updateNotifySettings &>(obj));
      return true;
    case updateServiceNotification::ID:
      func(static_cast<updateServiceNotification &>(obj));
      return true;
    case updatePrivacy::ID:
      func(static_cast<updatePrivacy &>(obj));
      return true;
    case updateUserPhone::ID:
      func(static_cast<updateUserPhone &>(obj));
      return true;
    case updateReadHistoryInbox::ID:
      func(static_cast<updateReadHistoryInbox &>(obj));
      return true;
    case updateReadHistoryOutbox::ID:
      func(static_cast<updateReadHistoryOutbox &>(obj));
      return true;
    case updateWebPage::ID:
      func(static_cast<updateWebPage &>(obj));
      return true;
    case updateReadMessagesContents::ID:
      func(static_cast<updateReadMessagesContents &>(obj));
      return true;
    case updateChannelTooLong::ID:
      func(static_cast<updateChannelTooLong &>(obj));
      return true;
    case updateChannel::ID:
      func(static_cast<updateChannel &>(obj));
      return true;
    case updateNewChannelMessage::ID:
      func(static_cast<updateNewChannelMessage &>(obj));
      return true;
    case updateReadChannelInbox::ID:
      func(static_cast<updateReadChannelInbox &>(obj));
      return true;
    case updateDeleteChannelMessages::ID:
      func(static_cast<updateDeleteChannelMessages &>(obj));
      return true;
    case updateChannelMessageViews::ID:
      func(static_cast<updateChannelMessageViews &>(obj));
      return true;
    case updateChatParticipantAdmin::ID:
      func(static_cast<updateChatParticipantAdmin &>(obj));
      return true;
    case updateNewStickerSet::ID:
      func(static_cast<updateNewStickerSet &>(obj));
      return true;
    case updateStickerSetsOrder::ID:
      func(static_cast<updateStickerSetsOrder &>(obj));
      return true;
    case updateStickerSets::ID:
      func(static_cast<updateStickerSets &>(obj));
      return true;
    case updateSavedGifs::ID:
      func(static_cast<updateSavedGifs &>(obj));
      return true;
    case updateBotInlineQuery::ID:
      func(static_cast<updateBotInlineQuery &>(obj));
      return true;
    case updateBotInlineSend::ID:
      func(static_cast<updateBotInlineSend &>(obj));
      return true;
    case updateEditChannelMessage::ID:
      func(static_cast<updateEditChannelMessage &>(obj));
      return true;
    case updateBotCallbackQuery::ID:
      func(static_cast<updateBotCallbackQuery &>(obj));
      return true;
    case updateEditMessage::ID:
      func(static_cast<updateEditMessage &>(obj));
      return true;
    case updateInlineBotCallbackQuery::ID:
      func(static_cast<updateInlineBotCallbackQuery &>(obj));
      return true;
    case updateReadChannelOutbox::ID:
      func(static_cast<updateReadChannelOutbox &>(obj));
      return true;
    case updateDraftMessage::ID:
      func(static_cast<updateDraftMessage &>(obj));
      return true;
    case updateReadFeaturedStickers::ID:
      func(static_cast<updateReadFeaturedStickers &>(obj));
      return true;
    case updateRecentStickers::ID:
      func(static_cast<updateRecentStickers &>(obj));
      return true;
    case updateConfig::ID:
      func(static_cast<updateConfig &>(obj));
      return true;
    case updatePtsChanged::ID:
      func(static_cast<updatePtsChanged &>(obj));
      return true;
    case updateChannelWebPage::ID:
      func(static_cast<updateChannelWebPage &>(obj));
      return true;
    case updateDialogPinned::ID:
      func(static_cast<updateDialogPinned &>(obj));
      return true;
    case updatePinnedDialogs::ID:
      func(static_cast<updatePinnedDialogs &>(obj));
      return true;
    case updateBotWebhookJSON::ID:
      func(static_cast<updateBotWebhookJSON &>(obj));
      return true;
    case updateBotWebhookJSONQuery::ID:
      func(static_cast<updateBotWebhookJSONQuery &>(obj));
      return true;
    case updateBotShippingQuery::ID:
      func(static_cast<updateBotShippingQuery &>(obj));
      return true;
    case updateBotPrecheckoutQuery::ID:
      func(static_cast<updateBotPrecheckoutQuery &>(obj));
      return true;
    case updatePhoneCall::ID:
      func(static_cast<updatePhoneCall &>(obj));
      return true;
    case updateLangPackTooLong::ID:
      func(static_cast<updateLangPackTooLong &>(obj));
      return true;
    case updateLangPack::ID:
      func(static_cast<updateLangPack &>(obj));
      return true;
    case updateFavedStickers::ID:
      func(static_cast<updateFavedStickers &>(obj));
      return true;
    case updateChannelReadMessagesContents::ID:
      func(static_cast<updateChannelReadMessagesContents &>(obj));
      return true;
    case updateContactsReset::ID:
      func(static_cast<updateContactsReset &>(obj));
      return true;
    case updateChannelAvailableMessages::ID:
      func(static_cast<updateChannelAvailableMessages &>(obj));
      return true;
    case updateDialogUnreadMark::ID:
      func(static_cast<updateDialogUnreadMark &>(obj));
      return true;
    case updateMessagePoll::ID:
      func(static_cast<updateMessagePoll &>(obj));
      return true;
    case updateChatDefaultBannedRights::ID:
      func(static_cast<updateChatDefaultBannedRights &>(obj));
      return true;
    case updateFolderPeers::ID:
      func(static_cast<updateFolderPeers &>(obj));
      return true;
    case updatePeerSettings::ID:
      func(static_cast<updatePeerSettings &>(obj));
      return true;
    case updatePeerLocated::ID:
      func(static_cast<updatePeerLocated &>(obj));
      return true;
    case updateNewScheduledMessage::ID:
      func(static_cast<updateNewScheduledMessage &>(obj));
      return true;
    case updateDeleteScheduledMessages::ID:
      func(static_cast<updateDeleteScheduledMessages &>(obj));
      return true;
    case updateTheme::ID:
      func(static_cast<updateTheme &>(obj));
      return true;
    case updateGeoLiveViewed::ID:
      func(static_cast<updateGeoLiveViewed &>(obj));
      return true;
    case updateLoginToken::ID:
      func(static_cast<updateLoginToken &>(obj));
      return true;
    case updateMessagePollVote::ID:
      func(static_cast<updateMessagePollVote &>(obj));
      return true;
    case updateDialogFilter::ID:
      func(static_cast<updateDialogFilter &>(obj));
      return true;
    case updateDialogFilterOrder::ID:
      func(static_cast<updateDialogFilterOrder &>(obj));
      return true;
    case updateDialogFilters::ID:
      func(static_cast<updateDialogFilters &>(obj));
      return true;
    case updatePhoneCallSignalingData::ID:
      func(static_cast<updatePhoneCallSignalingData &>(obj));
      return true;
    case updateChannelMessageForwards::ID:
      func(static_cast<updateChannelMessageForwards &>(obj));
      return true;
    case updateReadChannelDiscussionInbox::ID:
      func(static_cast<updateReadChannelDiscussionInbox &>(obj));
      return true;
    case updateReadChannelDiscussionOutbox::ID:
      func(static_cast<updateReadChannelDiscussionOutbox &>(obj));
      return true;
    case updatePeerBlocked::ID:
      func(static_cast<updatePeerBlocked &>(obj));
      return true;
    case updateChannelUserTyping::ID:
      func(static_cast<updateChannelUserTyping &>(obj));
      return true;
    case updatePinnedMessages::ID:
      func(static_cast<updatePinnedMessages &>(obj));
      return true;
    case updatePinnedChannelMessages::ID:
      func(static_cast<updatePinnedChannelMessages &>(obj));
      return true;
    case updateChat::ID:
      func(static_cast<updateChat &>(obj));
      return true;
    case updateGroupCallParticipants::ID:
      func(static_cast<updateGroupCallParticipants &>(obj));
      return true;
    case updateGroupCall::ID:
      func(static_cast<updateGroupCall &>(obj));
      return true;
    case updatePeerHistoryTTL::ID:
      func(static_cast<updatePeerHistoryTTL &>(obj));
      return true;
    case updateChatParticipant::ID:
      func(static_cast<updateChatParticipant &>(obj));
      return true;
    case updateChannelParticipant::ID:
      func(static_cast<updateChannelParticipant &>(obj));
      return true;
    case updateBotStopped::ID:
      func(static_cast<updateBotStopped &>(obj));
      return true;
    case updateGroupCallConnection::ID:
      func(static_cast<updateGroupCallConnection &>(obj));
      return true;
    case updateBotCommands::ID:
      func(static_cast<updateBotCommands &>(obj));
      return true;
    case updatePendingJoinRequests::ID:
      func(static_cast<updatePendingJoinRequests &>(obj));
      return true;
    case updateBotChatInviteRequester::ID:
      func(static_cast<updateBotChatInviteRequester &>(obj));
      return true;
    case updateMessageReactions::ID:
      func(static_cast<updateMessageReactions &>(obj));
      return true;
    case updateAttachMenuBots::ID:
      func(static_cast<updateAttachMenuBots &>(obj));
      return true;
    case updateWebViewResultSent::ID:
      func(static_cast<updateWebViewResultSent &>(obj));
      return true;
    case updateBotMenuButton::ID:
      func(static_cast<updateBotMenuButton &>(obj));
      return true;
    case updateSavedRingtones::ID:
      func(static_cast<updateSavedRingtones &>(obj));
      return true;
    case updateTranscribedAudio::ID:
      func(static_cast<updateTranscribedAudio &>(obj));
      return true;
    case updateReadFeaturedEmojiStickers::ID:
      func(static_cast<updateReadFeaturedEmojiStickers &>(obj));
      return true;
    case updateUserEmojiStatus::ID:
      func(static_cast<updateUserEmojiStatus &>(obj));
      return true;
    case updateRecentEmojiStatuses::ID:
      func(static_cast<updateRecentEmojiStatuses &>(obj));
      return true;
    case updateRecentReactions::ID:
      func(static_cast<updateRecentReactions &>(obj));
      return true;
    case updateMoveStickerSetToTop::ID:
      func(static_cast<updateMoveStickerSetToTop &>(obj));
      return true;
    case updateMessageExtendedMedia::ID:
      func(static_cast<updateMessageExtendedMedia &>(obj));
      return true;
    case updateChannelPinnedTopic::ID:
      func(static_cast<updateChannelPinnedTopic &>(obj));
      return true;
    case updateChannelPinnedTopics::ID:
      func(static_cast<updateChannelPinnedTopics &>(obj));
      return true;
    case updateUser::ID:
      func(static_cast<updateUser &>(obj));
      return true;
    case updateAutoSaveSettings::ID:
      func(static_cast<updateAutoSaveSettings &>(obj));
      return true;
    case updateGroupInvitePrivacyForbidden::ID:
      func(static_cast<updateGroupInvitePrivacyForbidden &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(Updates &obj, const T &func) {
  switch (obj.get_id()) {
    case updatesTooLong::ID:
      func(static_cast<updatesTooLong &>(obj));
      return true;
    case updateShortMessage::ID:
      func(static_cast<updateShortMessage &>(obj));
      return true;
    case updateShortChatMessage::ID:
      func(static_cast<updateShortChatMessage &>(obj));
      return true;
    case updateShort::ID:
      func(static_cast<updateShort &>(obj));
      return true;
    case updatesCombined::ID:
      func(static_cast<updatesCombined &>(obj));
      return true;
    case updates::ID:
      func(static_cast<updates &>(obj));
      return true;
    case updateShortSentMessage::ID:
      func(static_cast<updateShortSentMessage &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(UrlAuthResult &obj, const T &func) {
  switch (obj.get_id()) {
    case urlAuthResultRequest::ID:
      func(static_cast<urlAuthResultRequest &>(obj));
      return true;
    case urlAuthResultAccepted::ID:
      func(static_cast<urlAuthResultAccepted &>(obj));
      return true;
    case urlAuthResultDefault::ID:
      func(static_cast<urlAuthResultDefault &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(User &obj, const T &func) {
  switch (obj.get_id()) {
    case userEmpty::ID:
      func(static_cast<userEmpty &>(obj));
      return true;
    case user::ID:
      func(static_cast<user &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(UserProfilePhoto &obj, const T &func) {
  switch (obj.get_id()) {
    case userProfilePhotoEmpty::ID:
      func(static_cast<userProfilePhotoEmpty &>(obj));
      return true;
    case userProfilePhoto::ID:
      func(static_cast<userProfilePhoto &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(UserStatus &obj, const T &func) {
  switch (obj.get_id()) {
    case userStatusEmpty::ID:
      func(static_cast<userStatusEmpty &>(obj));
      return true;
    case userStatusOnline::ID:
      func(static_cast<userStatusOnline &>(obj));
      return true;
    case userStatusOffline::ID:
      func(static_cast<userStatusOffline &>(obj));
      return true;
    case userStatusRecently::ID:
      func(static_cast<userStatusRecently &>(obj));
      return true;
    case userStatusLastWeek::ID:
      func(static_cast<userStatusLastWeek &>(obj));
      return true;
    case userStatusLastMonth::ID:
      func(static_cast<userStatusLastMonth &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(VideoSize &obj, const T &func) {
  switch (obj.get_id()) {
    case videoSize::ID:
      func(static_cast<videoSize &>(obj));
      return true;
    case videoSizeEmojiMarkup::ID:
      func(static_cast<videoSizeEmojiMarkup &>(obj));
      return true;
    case videoSizeStickerMarkup::ID:
      func(static_cast<videoSizeStickerMarkup &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(WallPaper &obj, const T &func) {
  switch (obj.get_id()) {
    case wallPaper::ID:
      func(static_cast<wallPaper &>(obj));
      return true;
    case wallPaperNoFile::ID:
      func(static_cast<wallPaperNoFile &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(WebDocument &obj, const T &func) {
  switch (obj.get_id()) {
    case webDocument::ID:
      func(static_cast<webDocument &>(obj));
      return true;
    case webDocumentNoProxy::ID:
      func(static_cast<webDocumentNoProxy &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(WebPage &obj, const T &func) {
  switch (obj.get_id()) {
    case webPageEmpty::ID:
      func(static_cast<webPageEmpty &>(obj));
      return true;
    case webPagePending::ID:
      func(static_cast<webPagePending &>(obj));
      return true;
    case webPage::ID:
      func(static_cast<webPage &>(obj));
      return true;
    case webPageNotModified::ID:
      func(static_cast<webPageNotModified &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(account_EmailVerified &obj, const T &func) {
  switch (obj.get_id()) {
    case account_emailVerified::ID:
      func(static_cast<account_emailVerified &>(obj));
      return true;
    case account_emailVerifiedLogin::ID:
      func(static_cast<account_emailVerifiedLogin &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(account_EmojiStatuses &obj, const T &func) {
  switch (obj.get_id()) {
    case account_emojiStatusesNotModified::ID:
      func(static_cast<account_emojiStatusesNotModified &>(obj));
      return true;
    case account_emojiStatuses::ID:
      func(static_cast<account_emojiStatuses &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(account_ResetPasswordResult &obj, const T &func) {
  switch (obj.get_id()) {
    case account_resetPasswordFailedWait::ID:
      func(static_cast<account_resetPasswordFailedWait &>(obj));
      return true;
    case account_resetPasswordRequestedWait::ID:
      func(static_cast<account_resetPasswordRequestedWait &>(obj));
      return true;
    case account_resetPasswordOk::ID:
      func(static_cast<account_resetPasswordOk &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(account_SavedRingtone &obj, const T &func) {
  switch (obj.get_id()) {
    case account_savedRingtone::ID:
      func(static_cast<account_savedRingtone &>(obj));
      return true;
    case account_savedRingtoneConverted::ID:
      func(static_cast<account_savedRingtoneConverted &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(account_SavedRingtones &obj, const T &func) {
  switch (obj.get_id()) {
    case account_savedRingtonesNotModified::ID:
      func(static_cast<account_savedRingtonesNotModified &>(obj));
      return true;
    case account_savedRingtones::ID:
      func(static_cast<account_savedRingtones &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(account_Themes &obj, const T &func) {
  switch (obj.get_id()) {
    case account_themesNotModified::ID:
      func(static_cast<account_themesNotModified &>(obj));
      return true;
    case account_themes::ID:
      func(static_cast<account_themes &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(account_WallPapers &obj, const T &func) {
  switch (obj.get_id()) {
    case account_wallPapersNotModified::ID:
      func(static_cast<account_wallPapersNotModified &>(obj));
      return true;
    case account_wallPapers::ID:
      func(static_cast<account_wallPapers &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(auth_Authorization &obj, const T &func) {
  switch (obj.get_id()) {
    case auth_authorization::ID:
      func(static_cast<auth_authorization &>(obj));
      return true;
    case auth_authorizationSignUpRequired::ID:
      func(static_cast<auth_authorizationSignUpRequired &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(auth_CodeType &obj, const T &func) {
  switch (obj.get_id()) {
    case auth_codeTypeSms::ID:
      func(static_cast<auth_codeTypeSms &>(obj));
      return true;
    case auth_codeTypeCall::ID:
      func(static_cast<auth_codeTypeCall &>(obj));
      return true;
    case auth_codeTypeFlashCall::ID:
      func(static_cast<auth_codeTypeFlashCall &>(obj));
      return true;
    case auth_codeTypeMissedCall::ID:
      func(static_cast<auth_codeTypeMissedCall &>(obj));
      return true;
    case auth_codeTypeFragmentSms::ID:
      func(static_cast<auth_codeTypeFragmentSms &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(auth_LoginToken &obj, const T &func) {
  switch (obj.get_id()) {
    case auth_loginToken::ID:
      func(static_cast<auth_loginToken &>(obj));
      return true;
    case auth_loginTokenMigrateTo::ID:
      func(static_cast<auth_loginTokenMigrateTo &>(obj));
      return true;
    case auth_loginTokenSuccess::ID:
      func(static_cast<auth_loginTokenSuccess &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(auth_SentCode &obj, const T &func) {
  switch (obj.get_id()) {
    case auth_sentCode::ID:
      func(static_cast<auth_sentCode &>(obj));
      return true;
    case auth_sentCodeSuccess::ID:
      func(static_cast<auth_sentCodeSuccess &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(auth_SentCodeType &obj, const T &func) {
  switch (obj.get_id()) {
    case auth_sentCodeTypeApp::ID:
      func(static_cast<auth_sentCodeTypeApp &>(obj));
      return true;
    case auth_sentCodeTypeSms::ID:
      func(static_cast<auth_sentCodeTypeSms &>(obj));
      return true;
    case auth_sentCodeTypeCall::ID:
      func(static_cast<auth_sentCodeTypeCall &>(obj));
      return true;
    case auth_sentCodeTypeFlashCall::ID:
      func(static_cast<auth_sentCodeTypeFlashCall &>(obj));
      return true;
    case auth_sentCodeTypeMissedCall::ID:
      func(static_cast<auth_sentCodeTypeMissedCall &>(obj));
      return true;
    case auth_sentCodeTypeEmailCode::ID:
      func(static_cast<auth_sentCodeTypeEmailCode &>(obj));
      return true;
    case auth_sentCodeTypeSetUpEmailRequired::ID:
      func(static_cast<auth_sentCodeTypeSetUpEmailRequired &>(obj));
      return true;
    case auth_sentCodeTypeFragmentSms::ID:
      func(static_cast<auth_sentCodeTypeFragmentSms &>(obj));
      return true;
    case auth_sentCodeTypeFirebaseSms::ID:
      func(static_cast<auth_sentCodeTypeFirebaseSms &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(channels_ChannelParticipants &obj, const T &func) {
  switch (obj.get_id()) {
    case channels_channelParticipants::ID:
      func(static_cast<channels_channelParticipants &>(obj));
      return true;
    case channels_channelParticipantsNotModified::ID:
      func(static_cast<channels_channelParticipantsNotModified &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(chatlists_ChatlistInvite &obj, const T &func) {
  switch (obj.get_id()) {
    case chatlists_chatlistInviteAlready::ID:
      func(static_cast<chatlists_chatlistInviteAlready &>(obj));
      return true;
    case chatlists_chatlistInvite::ID:
      func(static_cast<chatlists_chatlistInvite &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(contacts_Blocked &obj, const T &func) {
  switch (obj.get_id()) {
    case contacts_blocked::ID:
      func(static_cast<contacts_blocked &>(obj));
      return true;
    case contacts_blockedSlice::ID:
      func(static_cast<contacts_blockedSlice &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(contacts_Contacts &obj, const T &func) {
  switch (obj.get_id()) {
    case contacts_contactsNotModified::ID:
      func(static_cast<contacts_contactsNotModified &>(obj));
      return true;
    case contacts_contacts::ID:
      func(static_cast<contacts_contacts &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(contacts_TopPeers &obj, const T &func) {
  switch (obj.get_id()) {
    case contacts_topPeersNotModified::ID:
      func(static_cast<contacts_topPeersNotModified &>(obj));
      return true;
    case contacts_topPeers::ID:
      func(static_cast<contacts_topPeers &>(obj));
      return true;
    case contacts_topPeersDisabled::ID:
      func(static_cast<contacts_topPeersDisabled &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(help_AppConfig &obj, const T &func) {
  switch (obj.get_id()) {
    case help_appConfigNotModified::ID:
      func(static_cast<help_appConfigNotModified &>(obj));
      return true;
    case help_appConfig::ID:
      func(static_cast<help_appConfig &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(help_AppUpdate &obj, const T &func) {
  switch (obj.get_id()) {
    case help_appUpdate::ID:
      func(static_cast<help_appUpdate &>(obj));
      return true;
    case help_noAppUpdate::ID:
      func(static_cast<help_noAppUpdate &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(help_CountriesList &obj, const T &func) {
  switch (obj.get_id()) {
    case help_countriesListNotModified::ID:
      func(static_cast<help_countriesListNotModified &>(obj));
      return true;
    case help_countriesList::ID:
      func(static_cast<help_countriesList &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(help_DeepLinkInfo &obj, const T &func) {
  switch (obj.get_id()) {
    case help_deepLinkInfoEmpty::ID:
      func(static_cast<help_deepLinkInfoEmpty &>(obj));
      return true;
    case help_deepLinkInfo::ID:
      func(static_cast<help_deepLinkInfo &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(help_PassportConfig &obj, const T &func) {
  switch (obj.get_id()) {
    case help_passportConfigNotModified::ID:
      func(static_cast<help_passportConfigNotModified &>(obj));
      return true;
    case help_passportConfig::ID:
      func(static_cast<help_passportConfig &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(help_PromoData &obj, const T &func) {
  switch (obj.get_id()) {
    case help_promoDataEmpty::ID:
      func(static_cast<help_promoDataEmpty &>(obj));
      return true;
    case help_promoData::ID:
      func(static_cast<help_promoData &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(help_TermsOfServiceUpdate &obj, const T &func) {
  switch (obj.get_id()) {
    case help_termsOfServiceUpdateEmpty::ID:
      func(static_cast<help_termsOfServiceUpdateEmpty &>(obj));
      return true;
    case help_termsOfServiceUpdate::ID:
      func(static_cast<help_termsOfServiceUpdate &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(help_UserInfo &obj, const T &func) {
  switch (obj.get_id()) {
    case help_userInfoEmpty::ID:
      func(static_cast<help_userInfoEmpty &>(obj));
      return true;
    case help_userInfo::ID:
      func(static_cast<help_userInfo &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(messages_AllStickers &obj, const T &func) {
  switch (obj.get_id()) {
    case messages_allStickersNotModified::ID:
      func(static_cast<messages_allStickersNotModified &>(obj));
      return true;
    case messages_allStickers::ID:
      func(static_cast<messages_allStickers &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(messages_AvailableReactions &obj, const T &func) {
  switch (obj.get_id()) {
    case messages_availableReactionsNotModified::ID:
      func(static_cast<messages_availableReactionsNotModified &>(obj));
      return true;
    case messages_availableReactions::ID:
      func(static_cast<messages_availableReactions &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(messages_Chats &obj, const T &func) {
  switch (obj.get_id()) {
    case messages_chats::ID:
      func(static_cast<messages_chats &>(obj));
      return true;
    case messages_chatsSlice::ID:
      func(static_cast<messages_chatsSlice &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(messages_DhConfig &obj, const T &func) {
  switch (obj.get_id()) {
    case messages_dhConfigNotModified::ID:
      func(static_cast<messages_dhConfigNotModified &>(obj));
      return true;
    case messages_dhConfig::ID:
      func(static_cast<messages_dhConfig &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(messages_Dialogs &obj, const T &func) {
  switch (obj.get_id()) {
    case messages_dialogs::ID:
      func(static_cast<messages_dialogs &>(obj));
      return true;
    case messages_dialogsSlice::ID:
      func(static_cast<messages_dialogsSlice &>(obj));
      return true;
    case messages_dialogsNotModified::ID:
      func(static_cast<messages_dialogsNotModified &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(messages_EmojiGroups &obj, const T &func) {
  switch (obj.get_id()) {
    case messages_emojiGroupsNotModified::ID:
      func(static_cast<messages_emojiGroupsNotModified &>(obj));
      return true;
    case messages_emojiGroups::ID:
      func(static_cast<messages_emojiGroups &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(messages_ExportedChatInvite &obj, const T &func) {
  switch (obj.get_id()) {
    case messages_exportedChatInvite::ID:
      func(static_cast<messages_exportedChatInvite &>(obj));
      return true;
    case messages_exportedChatInviteReplaced::ID:
      func(static_cast<messages_exportedChatInviteReplaced &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(messages_FavedStickers &obj, const T &func) {
  switch (obj.get_id()) {
    case messages_favedStickersNotModified::ID:
      func(static_cast<messages_favedStickersNotModified &>(obj));
      return true;
    case messages_favedStickers::ID:
      func(static_cast<messages_favedStickers &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(messages_FeaturedStickers &obj, const T &func) {
  switch (obj.get_id()) {
    case messages_featuredStickersNotModified::ID:
      func(static_cast<messages_featuredStickersNotModified &>(obj));
      return true;
    case messages_featuredStickers::ID:
      func(static_cast<messages_featuredStickers &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(messages_FoundStickerSets &obj, const T &func) {
  switch (obj.get_id()) {
    case messages_foundStickerSetsNotModified::ID:
      func(static_cast<messages_foundStickerSetsNotModified &>(obj));
      return true;
    case messages_foundStickerSets::ID:
      func(static_cast<messages_foundStickerSets &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(messages_Messages &obj, const T &func) {
  switch (obj.get_id()) {
    case messages_messages::ID:
      func(static_cast<messages_messages &>(obj));
      return true;
    case messages_messagesSlice::ID:
      func(static_cast<messages_messagesSlice &>(obj));
      return true;
    case messages_channelMessages::ID:
      func(static_cast<messages_channelMessages &>(obj));
      return true;
    case messages_messagesNotModified::ID:
      func(static_cast<messages_messagesNotModified &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(messages_Reactions &obj, const T &func) {
  switch (obj.get_id()) {
    case messages_reactionsNotModified::ID:
      func(static_cast<messages_reactionsNotModified &>(obj));
      return true;
    case messages_reactions::ID:
      func(static_cast<messages_reactions &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(messages_RecentStickers &obj, const T &func) {
  switch (obj.get_id()) {
    case messages_recentStickersNotModified::ID:
      func(static_cast<messages_recentStickersNotModified &>(obj));
      return true;
    case messages_recentStickers::ID:
      func(static_cast<messages_recentStickers &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(messages_SavedGifs &obj, const T &func) {
  switch (obj.get_id()) {
    case messages_savedGifsNotModified::ID:
      func(static_cast<messages_savedGifsNotModified &>(obj));
      return true;
    case messages_savedGifs::ID:
      func(static_cast<messages_savedGifs &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(messages_SentEncryptedMessage &obj, const T &func) {
  switch (obj.get_id()) {
    case messages_sentEncryptedMessage::ID:
      func(static_cast<messages_sentEncryptedMessage &>(obj));
      return true;
    case messages_sentEncryptedFile::ID:
      func(static_cast<messages_sentEncryptedFile &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(messages_SponsoredMessages &obj, const T &func) {
  switch (obj.get_id()) {
    case messages_sponsoredMessages::ID:
      func(static_cast<messages_sponsoredMessages &>(obj));
      return true;
    case messages_sponsoredMessagesEmpty::ID:
      func(static_cast<messages_sponsoredMessagesEmpty &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(messages_StickerSet &obj, const T &func) {
  switch (obj.get_id()) {
    case messages_stickerSet::ID:
      func(static_cast<messages_stickerSet &>(obj));
      return true;
    case messages_stickerSetNotModified::ID:
      func(static_cast<messages_stickerSetNotModified &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(messages_StickerSetInstallResult &obj, const T &func) {
  switch (obj.get_id()) {
    case messages_stickerSetInstallResultSuccess::ID:
      func(static_cast<messages_stickerSetInstallResultSuccess &>(obj));
      return true;
    case messages_stickerSetInstallResultArchive::ID:
      func(static_cast<messages_stickerSetInstallResultArchive &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(messages_Stickers &obj, const T &func) {
  switch (obj.get_id()) {
    case messages_stickersNotModified::ID:
      func(static_cast<messages_stickersNotModified &>(obj));
      return true;
    case messages_stickers::ID:
      func(static_cast<messages_stickers &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(payments_PaymentResult &obj, const T &func) {
  switch (obj.get_id()) {
    case payments_paymentResult::ID:
      func(static_cast<payments_paymentResult &>(obj));
      return true;
    case payments_paymentVerificationNeeded::ID:
      func(static_cast<payments_paymentVerificationNeeded &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(photos_Photos &obj, const T &func) {
  switch (obj.get_id()) {
    case photos_photos::ID:
      func(static_cast<photos_photos &>(obj));
      return true;
    case photos_photosSlice::ID:
      func(static_cast<photos_photosSlice &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(storage_FileType &obj, const T &func) {
  switch (obj.get_id()) {
    case storage_fileUnknown::ID:
      func(static_cast<storage_fileUnknown &>(obj));
      return true;
    case storage_filePartial::ID:
      func(static_cast<storage_filePartial &>(obj));
      return true;
    case storage_fileJpeg::ID:
      func(static_cast<storage_fileJpeg &>(obj));
      return true;
    case storage_fileGif::ID:
      func(static_cast<storage_fileGif &>(obj));
      return true;
    case storage_filePng::ID:
      func(static_cast<storage_filePng &>(obj));
      return true;
    case storage_filePdf::ID:
      func(static_cast<storage_filePdf &>(obj));
      return true;
    case storage_fileMp3::ID:
      func(static_cast<storage_fileMp3 &>(obj));
      return true;
    case storage_fileMov::ID:
      func(static_cast<storage_fileMov &>(obj));
      return true;
    case storage_fileMp4::ID:
      func(static_cast<storage_fileMp4 &>(obj));
      return true;
    case storage_fileWebp::ID:
      func(static_cast<storage_fileWebp &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(updates_ChannelDifference &obj, const T &func) {
  switch (obj.get_id()) {
    case updates_channelDifferenceEmpty::ID:
      func(static_cast<updates_channelDifferenceEmpty &>(obj));
      return true;
    case updates_channelDifferenceTooLong::ID:
      func(static_cast<updates_channelDifferenceTooLong &>(obj));
      return true;
    case updates_channelDifference::ID:
      func(static_cast<updates_channelDifference &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(updates_Difference &obj, const T &func) {
  switch (obj.get_id()) {
    case updates_differenceEmpty::ID:
      func(static_cast<updates_differenceEmpty &>(obj));
      return true;
    case updates_difference::ID:
      func(static_cast<updates_difference &>(obj));
      return true;
    case updates_differenceSlice::ID:
      func(static_cast<updates_differenceSlice &>(obj));
      return true;
    case updates_differenceTooLong::ID:
      func(static_cast<updates_differenceTooLong &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(upload_CdnFile &obj, const T &func) {
  switch (obj.get_id()) {
    case upload_cdnFileReuploadNeeded::ID:
      func(static_cast<upload_cdnFileReuploadNeeded &>(obj));
      return true;
    case upload_cdnFile::ID:
      func(static_cast<upload_cdnFile &>(obj));
      return true;
    default:
      return false;
  }
}

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(upload_File &obj, const T &func) {
  switch (obj.get_id()) {
    case upload_file::ID:
      func(static_cast<upload_file &>(obj));
      return true;
    case upload_fileCdnRedirect::ID:
      func(static_cast<upload_fileCdnRedirect &>(obj));
      return true;
    default:
      return false;
  }
}

}  // namespace telegram_api
}  // namespace td
