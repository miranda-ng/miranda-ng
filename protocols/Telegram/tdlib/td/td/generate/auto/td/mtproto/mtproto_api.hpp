#pragma once

/**
 * \file
 * Contains downcast_call methods for calling a function object on downcasted to
 * the most derived class TDLib API object.
 */
#include "mtproto_api.h"

namespace td {
namespace mtproto_api {

/**
 * Calls the specified function object with the given object downcasted to its most derived type.
 * \param[in] obj Object to pass as an argument to the function object.
 * \param[in] func Function object to which the object will be passed.
 * \returns Whether function object call has happened. Should always return true for correct parameters.
 */
template <class T>
bool downcast_call(Object &obj, const T &func) {
  switch (obj.get_id()) {
    case bad_msg_notification::ID:
      func(static_cast<bad_msg_notification &>(obj));
      return true;
    case bad_server_salt::ID:
      func(static_cast<bad_server_salt &>(obj));
      return true;
    case bind_auth_key_inner::ID:
      func(static_cast<bind_auth_key_inner &>(obj));
      return true;
    case client_DH_inner_data::ID:
      func(static_cast<client_DH_inner_data &>(obj));
      return true;
    case destroy_auth_key_ok::ID:
      func(static_cast<destroy_auth_key_ok &>(obj));
      return true;
    case destroy_auth_key_none::ID:
      func(static_cast<destroy_auth_key_none &>(obj));
      return true;
    case destroy_auth_key_fail::ID:
      func(static_cast<destroy_auth_key_fail &>(obj));
      return true;
    case future_salt::ID:
      func(static_cast<future_salt &>(obj));
      return true;
    case future_salts::ID:
      func(static_cast<future_salts &>(obj));
      return true;
    case gzip_packed::ID:
      func(static_cast<gzip_packed &>(obj));
      return true;
    case dummyHttpWait::ID:
      func(static_cast<dummyHttpWait &>(obj));
      return true;
    case msg_detailed_info::ID:
      func(static_cast<msg_detailed_info &>(obj));
      return true;
    case msg_new_detailed_info::ID:
      func(static_cast<msg_new_detailed_info &>(obj));
      return true;
    case msg_resend_req::ID:
      func(static_cast<msg_resend_req &>(obj));
      return true;
    case msgs_ack::ID:
      func(static_cast<msgs_ack &>(obj));
      return true;
    case msgs_all_info::ID:
      func(static_cast<msgs_all_info &>(obj));
      return true;
    case msgs_state_info::ID:
      func(static_cast<msgs_state_info &>(obj));
      return true;
    case msgs_state_req::ID:
      func(static_cast<msgs_state_req &>(obj));
      return true;
    case new_session_created::ID:
      func(static_cast<new_session_created &>(obj));
      return true;
    case p_q_inner_data_dc::ID:
      func(static_cast<p_q_inner_data_dc &>(obj));
      return true;
    case p_q_inner_data_temp_dc::ID:
      func(static_cast<p_q_inner_data_temp_dc &>(obj));
      return true;
    case pong::ID:
      func(static_cast<pong &>(obj));
      return true;
    case rsa_public_key::ID:
      func(static_cast<rsa_public_key &>(obj));
      return true;
    case resPQ::ID:
      func(static_cast<resPQ &>(obj));
      return true;
    case rpc_answer_unknown::ID:
      func(static_cast<rpc_answer_unknown &>(obj));
      return true;
    case rpc_answer_dropped_running::ID:
      func(static_cast<rpc_answer_dropped_running &>(obj));
      return true;
    case rpc_answer_dropped::ID:
      func(static_cast<rpc_answer_dropped &>(obj));
      return true;
    case rpc_error::ID:
      func(static_cast<rpc_error &>(obj));
      return true;
    case server_DH_params_ok::ID:
      func(static_cast<server_DH_params_ok &>(obj));
      return true;
    case server_DH_inner_data::ID:
      func(static_cast<server_DH_inner_data &>(obj));
      return true;
    case dh_gen_ok::ID:
      func(static_cast<dh_gen_ok &>(obj));
      return true;
    case dh_gen_retry::ID:
      func(static_cast<dh_gen_retry &>(obj));
      return true;
    case dh_gen_fail::ID:
      func(static_cast<dh_gen_fail &>(obj));
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
    case destroy_auth_key::ID:
      func(static_cast<destroy_auth_key &>(obj));
      return true;
    case get_future_salts::ID:
      func(static_cast<get_future_salts &>(obj));
      return true;
    case http_wait::ID:
      func(static_cast<http_wait &>(obj));
      return true;
    case ping_delay_disconnect::ID:
      func(static_cast<ping_delay_disconnect &>(obj));
      return true;
    case req_DH_params::ID:
      func(static_cast<req_DH_params &>(obj));
      return true;
    case req_pq_multi::ID:
      func(static_cast<req_pq_multi &>(obj));
      return true;
    case rpc_drop_answer::ID:
      func(static_cast<rpc_drop_answer &>(obj));
      return true;
    case set_client_DH_params::ID:
      func(static_cast<set_client_DH_params &>(obj));
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
bool downcast_call(BadMsgNotification &obj, const T &func) {
  switch (obj.get_id()) {
    case bad_msg_notification::ID:
      func(static_cast<bad_msg_notification &>(obj));
      return true;
    case bad_server_salt::ID:
      func(static_cast<bad_server_salt &>(obj));
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
bool downcast_call(DestroyAuthKeyRes &obj, const T &func) {
  switch (obj.get_id()) {
    case destroy_auth_key_ok::ID:
      func(static_cast<destroy_auth_key_ok &>(obj));
      return true;
    case destroy_auth_key_none::ID:
      func(static_cast<destroy_auth_key_none &>(obj));
      return true;
    case destroy_auth_key_fail::ID:
      func(static_cast<destroy_auth_key_fail &>(obj));
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
bool downcast_call(MsgDetailedInfo &obj, const T &func) {
  switch (obj.get_id()) {
    case msg_detailed_info::ID:
      func(static_cast<msg_detailed_info &>(obj));
      return true;
    case msg_new_detailed_info::ID:
      func(static_cast<msg_new_detailed_info &>(obj));
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
bool downcast_call(P_Q_inner_data &obj, const T &func) {
  switch (obj.get_id()) {
    case p_q_inner_data_dc::ID:
      func(static_cast<p_q_inner_data_dc &>(obj));
      return true;
    case p_q_inner_data_temp_dc::ID:
      func(static_cast<p_q_inner_data_temp_dc &>(obj));
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
bool downcast_call(RpcDropAnswer &obj, const T &func) {
  switch (obj.get_id()) {
    case rpc_answer_unknown::ID:
      func(static_cast<rpc_answer_unknown &>(obj));
      return true;
    case rpc_answer_dropped_running::ID:
      func(static_cast<rpc_answer_dropped_running &>(obj));
      return true;
    case rpc_answer_dropped::ID:
      func(static_cast<rpc_answer_dropped &>(obj));
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
bool downcast_call(Set_client_DH_params_answer &obj, const T &func) {
  switch (obj.get_id()) {
    case dh_gen_ok::ID:
      func(static_cast<dh_gen_ok &>(obj));
      return true;
    case dh_gen_retry::ID:
      func(static_cast<dh_gen_retry &>(obj));
      return true;
    case dh_gen_fail::ID:
      func(static_cast<dh_gen_fail &>(obj));
      return true;
    default:
      return false;
  }
}

}  // namespace mtproto_api
}  // namespace td
