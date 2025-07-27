#pragma once

/**
 * \file
 * Contains downcast_call methods for calling a function object on downcasted to
 * the most derived class TDLib API object.
 */
#include "e2e_api.h"

namespace td {
namespace e2e_api {

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(Object &obj, const T &func) {
  switch (obj.get_id()) {
    case ok::ID:
      func(static_cast<ok &>(obj));
      return true;
    case e2e_callPacket::ID:
      func(static_cast<e2e_callPacket &>(obj));
      return true;
    case e2e_callPacketLargeMsgId::ID:
      func(static_cast<e2e_callPacketLargeMsgId &>(obj));
      return true;
    case e2e_handshakePrivateAccept::ID:
      func(static_cast<e2e_handshakePrivateAccept &>(obj));
      return true;
    case e2e_handshakePrivateFinish::ID:
      func(static_cast<e2e_handshakePrivateFinish &>(obj));
      return true;
    case e2e_handshakeQR::ID:
      func(static_cast<e2e_handshakeQR &>(obj));
      return true;
    case e2e_handshakeEncryptedMessage::ID:
      func(static_cast<e2e_handshakeEncryptedMessage &>(obj));
      return true;
    case e2e_handshakeLoginExport::ID:
      func(static_cast<e2e_handshakeLoginExport &>(obj));
      return true;
    case e2e_keyContactByUserId::ID:
      func(static_cast<e2e_keyContactByUserId &>(obj));
      return true;
    case e2e_keyContactByPublicKey::ID:
      func(static_cast<e2e_keyContactByPublicKey &>(obj));
      return true;
    case e2e_personalUserId::ID:
      func(static_cast<e2e_personalUserId &>(obj));
      return true;
    case e2e_personalName::ID:
      func(static_cast<e2e_personalName &>(obj));
      return true;
    case e2e_personalPhoneNumber::ID:
      func(static_cast<e2e_personalPhoneNumber &>(obj));
      return true;
    case e2e_personalContactState::ID:
      func(static_cast<e2e_personalContactState &>(obj));
      return true;
    case e2e_personalEmojiNonces::ID:
      func(static_cast<e2e_personalEmojiNonces &>(obj));
      return true;
    case e2e_personalData::ID:
      func(static_cast<e2e_personalData &>(obj));
      return true;
    case e2e_personalOnClient::ID:
      func(static_cast<e2e_personalOnClient &>(obj));
      return true;
    case e2e_personalOnServer::ID:
      func(static_cast<e2e_personalOnServer &>(obj));
      return true;
    case e2e_valueContactByUserId::ID:
      func(static_cast<e2e_valueContactByUserId &>(obj));
      return true;
    case e2e_valueContactByPublicKey::ID:
      func(static_cast<e2e_valueContactByPublicKey &>(obj));
      return true;
    case e2e_chain_block::ID:
      func(static_cast<e2e_chain_block &>(obj));
      return true;
    case e2e_chain_changeNoop::ID:
      func(static_cast<e2e_chain_changeNoop &>(obj));
      return true;
    case e2e_chain_changeSetValue::ID:
      func(static_cast<e2e_chain_changeSetValue &>(obj));
      return true;
    case e2e_chain_changeSetGroupState::ID:
      func(static_cast<e2e_chain_changeSetGroupState &>(obj));
      return true;
    case e2e_chain_changeSetSharedKey::ID:
      func(static_cast<e2e_chain_changeSetSharedKey &>(obj));
      return true;
    case e2e_chain_groupBroadcastNonceCommit::ID:
      func(static_cast<e2e_chain_groupBroadcastNonceCommit &>(obj));
      return true;
    case e2e_chain_groupBroadcastNonceReveal::ID:
      func(static_cast<e2e_chain_groupBroadcastNonceReveal &>(obj));
      return true;
    case e2e_chain_groupParticipant::ID:
      func(static_cast<e2e_chain_groupParticipant &>(obj));
      return true;
    case e2e_chain_groupState::ID:
      func(static_cast<e2e_chain_groupState &>(obj));
      return true;
    case e2e_chain_sharedKey::ID:
      func(static_cast<e2e_chain_sharedKey &>(obj));
      return true;
    case e2e_chain_stateProof::ID:
      func(static_cast<e2e_chain_stateProof &>(obj));
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
    case e2e_nop::ID:
      func(static_cast<e2e_nop &>(obj));
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
bool downcast_call(e2e_HandshakePrivate &obj, const T &func) {
  switch (obj.get_id()) {
    case e2e_handshakePrivateAccept::ID:
      func(static_cast<e2e_handshakePrivateAccept &>(obj));
      return true;
    case e2e_handshakePrivateFinish::ID:
      func(static_cast<e2e_handshakePrivateFinish &>(obj));
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
bool downcast_call(e2e_HandshakePublic &obj, const T &func) {
  switch (obj.get_id()) {
    case e2e_handshakeQR::ID:
      func(static_cast<e2e_handshakeQR &>(obj));
      return true;
    case e2e_handshakeEncryptedMessage::ID:
      func(static_cast<e2e_handshakeEncryptedMessage &>(obj));
      return true;
    case e2e_handshakeLoginExport::ID:
      func(static_cast<e2e_handshakeLoginExport &>(obj));
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
bool downcast_call(e2e_Key &obj, const T &func) {
  switch (obj.get_id()) {
    case e2e_keyContactByUserId::ID:
      func(static_cast<e2e_keyContactByUserId &>(obj));
      return true;
    case e2e_keyContactByPublicKey::ID:
      func(static_cast<e2e_keyContactByPublicKey &>(obj));
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
bool downcast_call(e2e_Personal &obj, const T &func) {
  switch (obj.get_id()) {
    case e2e_personalUserId::ID:
      func(static_cast<e2e_personalUserId &>(obj));
      return true;
    case e2e_personalName::ID:
      func(static_cast<e2e_personalName &>(obj));
      return true;
    case e2e_personalPhoneNumber::ID:
      func(static_cast<e2e_personalPhoneNumber &>(obj));
      return true;
    case e2e_personalContactState::ID:
      func(static_cast<e2e_personalContactState &>(obj));
      return true;
    case e2e_personalEmojiNonces::ID:
      func(static_cast<e2e_personalEmojiNonces &>(obj));
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
bool downcast_call(e2e_Value &obj, const T &func) {
  switch (obj.get_id()) {
    case e2e_valueContactByUserId::ID:
      func(static_cast<e2e_valueContactByUserId &>(obj));
      return true;
    case e2e_valueContactByPublicKey::ID:
      func(static_cast<e2e_valueContactByPublicKey &>(obj));
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
bool downcast_call(e2e_chain_Change &obj, const T &func) {
  switch (obj.get_id()) {
    case e2e_chain_changeNoop::ID:
      func(static_cast<e2e_chain_changeNoop &>(obj));
      return true;
    case e2e_chain_changeSetValue::ID:
      func(static_cast<e2e_chain_changeSetValue &>(obj));
      return true;
    case e2e_chain_changeSetGroupState::ID:
      func(static_cast<e2e_chain_changeSetGroupState &>(obj));
      return true;
    case e2e_chain_changeSetSharedKey::ID:
      func(static_cast<e2e_chain_changeSetSharedKey &>(obj));
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
bool downcast_call(e2e_chain_GroupBroadcast &obj, const T &func) {
  switch (obj.get_id()) {
    case e2e_chain_groupBroadcastNonceCommit::ID:
      func(static_cast<e2e_chain_groupBroadcastNonceCommit &>(obj));
      return true;
    case e2e_chain_groupBroadcastNonceReveal::ID:
      func(static_cast<e2e_chain_groupBroadcastNonceReveal &>(obj));
      return true;
    default:
      return false;
  }
}

}  // namespace e2e_api
}  // namespace td
