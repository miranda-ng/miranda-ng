/* tox.h
 *
 * The Tox public API.
 *
 *  Copyright (C) 2013 Tox project All Rights Reserved.
 *
 *  This file is part of Tox.
 *
 *  Tox is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Tox is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Tox.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TOX_H
#define TOX_H

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

#define TOX_MAX_NAME_LENGTH 128

/* Maximum length of single messages after which they should be split. */
#define TOX_MAX_MESSAGE_LENGTH 1368
#define TOX_MAX_STATUSMESSAGE_LENGTH 1007
#define TOX_MAX_FRIENDREQUEST_LENGTH 1016

#define TOX_PUBLIC_KEY_SIZE 32
/* TODO: remove */
#define TOX_CLIENT_ID_SIZE TOX_PUBLIC_KEY_SIZE

#define TOX_AVATAR_MAX_DATA_LENGTH 16384
#define TOX_HASH_LENGTH /*crypto_hash_sha256_BYTES*/ 32

#define TOX_FRIEND_ADDRESS_SIZE (TOX_PUBLIC_KEY_SIZE + sizeof(uint32_t) + sizeof(uint16_t))

#define TOX_ENABLE_IPV6_DEFAULT 1

#define TOX_ENC_SAVE_MAGIC_NUMBER "toxEsave"
#define TOX_ENC_SAVE_MAGIC_LENGTH 8

/* Errors for m_addfriend
 * FAERR - Friend Add Error
 */
enum {
    TOX_FAERR_TOOLONG = -1,
    TOX_FAERR_NOMESSAGE = -2,
    TOX_FAERR_OWNKEY = -3,
    TOX_FAERR_ALREADYSENT = -4,
    TOX_FAERR_UNKNOWN = -5,
    TOX_FAERR_BADCHECKSUM = -6,
    TOX_FAERR_SETNEWNOSPAM = -7,
    TOX_FAERR_NOMEM = -8
};

/* USERSTATUS -
 * Represents userstatuses someone can have.
 */
typedef enum {
    TOX_USERSTATUS_NONE,
    TOX_USERSTATUS_AWAY,
    TOX_USERSTATUS_BUSY,
    TOX_USERSTATUS_INVALID
}
TOX_USERSTATUS;


/* AVATAR_FORMAT -
 * Data formats for user avatar images
 */
typedef enum {
    TOX_AVATAR_FORMAT_NONE = 0,
    TOX_AVATAR_FORMAT_PNG
}
TOX_AVATAR_FORMAT;

#ifndef __TOX_DEFINED__
#define __TOX_DEFINED__
typedef struct Tox Tox;
#endif

/* NOTE: Strings in Tox are all UTF-8, (This means that there is no terminating NULL character.)
 *
 * The exact buffer you send will be received at the other end without modification.
 *
 * Do not treat Tox strings as C strings.
 */

/*  return TOX_FRIEND_ADDRESS_SIZE byte address to give to others.
 * format: [public_key (32 bytes)][nospam number (4 bytes)][checksum (2 bytes)]
 */
void tox_get_address(const Tox *tox, uint8_t *address);

/* Add a friend.
 * Set the data that will be sent along with friend request.
 * address is the address of the friend (returned by getaddress of the friend you wish to add) it must be TOX_FRIEND_ADDRESS_SIZE bytes. TODO: add checksum.
 * data is the data and length is the length (maximum length of data is TOX_MAX_FRIENDREQUEST_LENGTH).
 *
 *  return the friend number if success.
 *  return TOX_FAERR_TOOLONG if message length is too long.
 *  return TOX_FAERR_NOMESSAGE if no message (message length must be >= 1 byte).
 *  return TOX_FAERR_OWNKEY if user's own key.
 *  return TOX_FAERR_ALREADYSENT if friend request already sent or already a friend.
 *  return TOX_FAERR_UNKNOWN for unknown error.
 *  return TOX_FAERR_BADCHECKSUM if bad checksum in address.
 *  return TOX_FAERR_SETNEWNOSPAM if the friend was already there but the nospam was different.
 *  (the nospam for that friend was set to the new one).
 *  return TOX_FAERR_NOMEM if increasing the friend list size fails.
 */
int32_t tox_add_friend(Tox *tox, const uint8_t *address, const uint8_t *data, uint16_t length);


/* Add a friend without sending a friendrequest.
 *  return the friend number if success.
 *  return -1 if failure.
 */
int32_t tox_add_friend_norequest(Tox *tox, const uint8_t *public_key);

/*  return the friend number associated to that client id.
    return -1 if no such friend */
int32_t tox_get_friend_number(const Tox *tox, const uint8_t *public_key);

/* Copies the public key associated to that friend id into public_key buffer.
 * Make sure that public_key is of size TOX_PUBLIC_KEY_SIZE.
 *  return 0 if success.
 *  return -1 if failure.
 */
int tox_get_client_id(const Tox *tox, int32_t friendnumber, uint8_t *public_key);

/* Remove a friend.
 *
 *  return 0 if success.
 *  return -1 if failure.
 */
int tox_del_friend(Tox *tox, int32_t friendnumber);

/* Checks friend's connecting status.
 *
 *  return 1 if friend is connected to us (Online).
 *  return 0 if friend is not connected to us (Offline).
 *  return -1 on failure.
 */
int tox_get_friend_connection_status(const Tox *tox, int32_t friendnumber);

/* Checks if there exists a friend with given friendnumber.
 *
 *  return 1 if friend exists.
 *  return 0 if friend doesn't exist.
 */
int tox_friend_exists(const Tox *tox, int32_t friendnumber);

/* Send a text chat message to an online friend.
 *
 *  return the message id if packet was successfully put into the send queue.
 *  return 0 if it was not.
 *
 * maximum length of messages is TOX_MAX_MESSAGE_LENGTH, your client must split larger messages
 * or else sending them will not work. No the core will not split messages for you because that
 * requires me to parse UTF-8.
 *
 * You will want to retain the return value, it will be passed to your read_receipt callback
 * if one is received.
 */
uint32_t tox_send_message(Tox *tox, int32_t friendnumber, const uint8_t *message, uint32_t length);

/* Send an action to an online friend.
 *
 *  return the message id if packet was successfully put into the send queue.
 *  return 0 if it was not.
 *
 * maximum length of actions is TOX_MAX_MESSAGE_LENGTH, your client must split larger actions
 * or else sending them will not work. No the core will not split actions for you because that
 * requires me to parse UTF-8.
 *
 *  You will want to retain the return value, it will be passed to your read_receipt callback
 *  if one is received.
 */
uint32_t tox_send_action(Tox *tox, int32_t friendnumber, const uint8_t *action, uint32_t length);

/* Set our nickname.
 * name must be a string of maximum MAX_NAME_LENGTH length.
 * length must be at least 1 byte.
 * length is the length of name.
 *
 *  return 0 if success.
 *  return -1 if failure.
 */
int tox_set_name(Tox *tox, const uint8_t *name, uint16_t length);

/*
 * Get your nickname.
 * m - The messenger context to use.
 * name - needs to be a valid memory location with a size of at least MAX_NAME_LENGTH (128) bytes.
 *
 *  return length of name.
 *  return 0 on error.
 */
uint16_t tox_get_self_name(const Tox *tox, uint8_t *name);

/* Get name of friendnumber and put it in name.
 * name needs to be a valid memory location with a size of at least MAX_NAME_LENGTH (128) bytes.
 *
 *  return length of name if success.
 *  return -1 if failure.
 */
int tox_get_name(const Tox *tox, int32_t friendnumber, uint8_t *name);

/*  returns the length of name on success.
 *  returns -1 on failure.
 */
int tox_get_name_size(const Tox *tox, int32_t friendnumber);
int tox_get_self_name_size(const Tox *tox);

/* Set our user status.
 *
 * userstatus must be one of TOX_USERSTATUS values.
 * max length of the status is TOX_MAX_STATUSMESSAGE_LENGTH.
 *
 *  returns 0 on success.
 *  returns -1 on failure.
 */
int tox_set_status_message(Tox *tox, const uint8_t *status, uint16_t length);
int tox_set_user_status(Tox *tox, uint8_t userstatus);

/*  returns the length of status message on success.
 *  returns -1 on failure.
 */
int tox_get_status_message_size(const Tox *tox, int32_t friendnumber);
int tox_get_self_status_message_size(const Tox *tox);

/* Copy friendnumber's status message into buf, truncating if size is over maxlen.
 * Get the size you need to allocate from m_get_statusmessage_size.
 * The self variant will copy our own status message.
 *
 * returns the length of the copied data on success
 * retruns -1 on failure.
 */
int tox_get_status_message(const Tox *tox, int32_t friendnumber, uint8_t *buf, uint32_t maxlen);
int tox_get_self_status_message(const Tox *tox, uint8_t *buf, uint32_t maxlen);

/*  return one of TOX_USERSTATUS values.
 *  Values unknown to your application should be represented as TOX_USERSTATUS_NONE.
 *  As above, the self variant will return our own TOX_USERSTATUS.
 *  If friendnumber is invalid, this shall return TOX_USERSTATUS_INVALID.
 */
uint8_t tox_get_user_status(const Tox *tox, int32_t friendnumber);
uint8_t tox_get_self_user_status(const Tox *tox);

/* returns timestamp of last time friendnumber was seen online, or 0 if never seen.
 * returns -1 on error.
 */
uint64_t tox_get_last_online(const Tox *tox, int32_t friendnumber);

/* Set our typing status for a friend.
 * You are responsible for turning it on or off.
 *
 * returns 0 on success.
 * returns -1 on failure.
 */
int tox_set_user_is_typing(Tox *tox, int32_t friendnumber, uint8_t is_typing);

/* Get the typing status of a friend.
 *
 * returns 0 if friend is not typing.
 * returns 1 if friend is typing.
 */
uint8_t tox_get_is_typing(const Tox *tox, int32_t friendnumber);

/* Return the number of friends in the instance m.
 * You should use this to determine how much memory to allocate
 * for copy_friendlist. */
uint32_t tox_count_friendlist(const Tox *tox);

/* Return the number of online friends in the instance m. */
uint32_t tox_get_num_online_friends(const Tox *tox);

/* Copy a list of valid friend IDs into the array out_list.
 * If out_list is NULL, returns 0.
 * Otherwise, returns the number of elements copied.
 * If the array was too small, the contents
 * of out_list will be truncated to list_size. */
uint32_t tox_get_friendlist(const Tox *tox, int32_t *out_list, uint32_t list_size);

/* Set the function that will be executed when a friend request is received.
 *  Function format is function(Tox *tox, const uint8_t * public_key, const uint8_t * data, uint16_t length, void *userdata)
 */
void tox_callback_friend_request(Tox *tox, void (*function)(Tox *tox, const uint8_t *, const uint8_t *, uint16_t,
                                 void *), void *userdata);

/* Set the function that will be executed when a message from a friend is received.
 *  Function format is: function(Tox *tox, int32_t friendnumber, const uint8_t * message, uint16_t length, void *userdata)
 */
void tox_callback_friend_message(Tox *tox, void (*function)(Tox *tox, int32_t, const uint8_t *, uint16_t, void *),
                                 void *userdata);

/* Set the function that will be executed when an action from a friend is received.
 *  Function format is: function(Tox *tox, int32_t friendnumber, const uint8_t * action, uint16_t length, void *userdata)
 */
void tox_callback_friend_action(Tox *tox, void (*function)(Tox *tox, int32_t, const uint8_t *, uint16_t, void *),
                                void *userdata);

/* Set the callback for name changes.
 *  function(Tox *tox, int32_t friendnumber, const uint8_t *newname, uint16_t length, void *userdata)
 *  You are not responsible for freeing newname
 */
void tox_callback_name_change(Tox *tox, void (*function)(Tox *tox, int32_t, const uint8_t *, uint16_t, void *),
                              void *userdata);

/* Set the callback for status message changes.
 *  function(Tox *tox, int32_t friendnumber, const uint8_t *newstatus, uint16_t length, void *userdata)
 *  You are not responsible for freeing newstatus.
 */
void tox_callback_status_message(Tox *tox, void (*function)(Tox *tox, int32_t, const uint8_t *, uint16_t, void *),
                                 void *userdata);

/* Set the callback for status type changes.
 *  function(Tox *tox, int32_t friendnumber, uint8_t TOX_USERSTATUS, void *userdata)
 */
void tox_callback_user_status(Tox *tox, void (*function)(Tox *tox, int32_t, uint8_t, void *), void *userdata);

/* Set the callback for typing changes.
 *  function (Tox *tox, int32_t friendnumber, uint8_t is_typing, void *userdata)
 */
void tox_callback_typing_change(Tox *tox, void (*function)(Tox *tox, int32_t, uint8_t, void *), void *userdata);

/* Set the callback for read receipts.
 *  function(Tox *tox, int32_t friendnumber, uint32_t receipt, void *userdata)
 *
 *  If you are keeping a record of returns from m_sendmessage;
 *  receipt might be one of those values, meaning the message
 *  has been received on the other side.
 *  Since core doesn't track ids for you, receipt may not correspond to any message.
 *  In that case, you should discard it.
 */
void tox_callback_read_receipt(Tox *tox, void (*function)(Tox *tox, int32_t, uint32_t, void *), void *userdata);

/* Set the callback for connection status changes.
 *  function(Tox *tox, int32_t friendnumber, uint8_t status, void *userdata)
 *
 *  Status:
 *    0 -- friend went offline after being previously online
 *    1 -- friend went online
 *
 *  NOTE: This callback is not called when adding friends, thus the "after
 *  being previously online" part. it's assumed that when adding friends,
 *  their connection status is offline.
 */
void tox_callback_connection_status(Tox *tox, void (*function)(Tox *tox, int32_t, uint8_t, void *), void *userdata);


/**********ADVANCED FUNCTIONS (If you don't know what they do you can safely ignore them.)  ************/

/* Functions to get/set the nospam part of the id.
 */
uint32_t tox_get_nospam(const Tox *tox);
void tox_set_nospam(Tox *tox, uint32_t nospam);

/* Copy the public and secret key from the Tox object.
   public_key and secret_key must be 32 bytes big.
   if the pointer is NULL, no data will be copied to it.*/
void tox_get_keys(Tox *tox, uint8_t *public_key, uint8_t *secret_key);

/* Maximum size of custom packets. */
#define TOX_MAX_CUSTOM_PACKET_SIZE 1373

/* Set handlers for custom lossy packets.
 * Set the function to be called when friend sends us a lossy packet starting with byte.
 * byte must be in the 200-254 range.
 *
 * NOTE: lossy packets behave like UDP packets meaning they might never reach the other side
 * or might arrive more than once (if someone is messing with the connection) or might arrive
 * in the wrong order.
 *
 * Unless latency is an issue, it is recommended that you use lossless packets instead.
 *
 * return -1 on failure.
 * return 0 on success.
 */
int tox_lossy_packet_registerhandler(Tox *tox, int32_t friendnumber, uint8_t byte,
                                     int (*packet_handler_callback)(Tox *tox, int32_t friendnumber, const uint8_t *data, uint32_t len, void *object),
                                     void *object);

/* Function to send custom lossy packets.
 * First byte of data must be in the range: 200-254.
 *
 * return -1 on failure.
 * return 0 on success.
 */
int tox_send_lossy_packet(const Tox *tox, int32_t friendnumber, const uint8_t *data, uint32_t length);

/* Set handlers for custom lossless packets.
 * Set the function to be called when friend sends us a lossless packet starting with byte.
 * byte must be in the 160-191 range.
 *
 * Lossless packets behave kind of like TCP (reliability, arrive in order.) but with packets instead of a stream.
 *
 * return -1 on failure.
 * return 0 on success.
 */
int tox_lossless_packet_registerhandler(Tox *tox, int32_t friendnumber, uint8_t byte,
                                        int (*packet_handler_callback)(Tox *tox, int32_t friendnumber, const uint8_t *data, uint32_t len, void *object),
                                        void *object);

/* Function to send custom lossless packets.
 * First byte of data must be in the range: 160-191.
 *
 * return -1 on failure.
 * return 0 on success.
 */
int tox_send_lossless_packet(const Tox *tox, int32_t friendnumber, const uint8_t *data, uint32_t length);

/**********GROUP CHAT FUNCTIONS: WARNING Group chats will be rewritten so this might change ************/

/* Group chat types for tox_callback_group_invite function.
 *
 * TOX_GROUPCHAT_TYPE_TEXT groupchats must be accepted with the tox_join_groupchat() function.
 * The function to accept TOX_GROUPCHAT_TYPE_AV is in toxav.
 */
enum {
    TOX_GROUPCHAT_TYPE_TEXT,
    TOX_GROUPCHAT_TYPE_AV
};

/* Set the callback for group invites.
 *
 *  Function(Tox *tox, int32_t friendnumber, uint8_t type, const uint8_t *data, uint16_t length, void *userdata)
 *
 * data of length is what needs to be passed to join_groupchat().
 *
 * for what type means see the enum right above this comment.
 */
void tox_callback_group_invite(Tox *tox, void (*function)(Tox *tox, int32_t, uint8_t, const uint8_t *, uint16_t,
                               void *), void *userdata);

/* Set the callback for group messages.
 *
 *  Function(Tox *tox, int groupnumber, int peernumber, const uint8_t * message, uint16_t length, void *userdata)
 */
void tox_callback_group_message(Tox *tox, void (*function)(Tox *tox, int, int, const uint8_t *, uint16_t, void *),
                                void *userdata);

/* Set the callback for group actions.
 *
 *  Function(Tox *tox, int groupnumber, int peernumber, const uint8_t * action, uint16_t length, void *userdata)
 */
void tox_callback_group_action(Tox *tox, void (*function)(Tox *tox, int, int, const uint8_t *, uint16_t, void *),
                               void *userdata);

/* Set callback function for title changes.
 *
 * Function(Tox *tox, int groupnumber, int peernumber, uint8_t * title, uint8_t length, void *userdata)
 * if peernumber == -1, then author is unknown (e.g. initial joining the group)
 */
void tox_callback_group_title(Tox *tox, void (*function)(Tox *tox, int, int, const uint8_t *, uint8_t,
                              void *), void *userdata);

/* Set callback function for peer name list changes.
 *
 * It gets called every time the name list changes(new peer/name, deleted peer)
 *  Function(Tox *tox, int groupnumber, int peernumber, TOX_CHAT_CHANGE change, void *userdata)
 */
typedef enum {
    TOX_CHAT_CHANGE_PEER_ADD,
    TOX_CHAT_CHANGE_PEER_DEL,
    TOX_CHAT_CHANGE_PEER_NAME,
} TOX_CHAT_CHANGE;

void tox_callback_group_namelist_change(Tox *tox, void (*function)(Tox *tox, int, int, uint8_t, void *),
                                        void *userdata);

/* Creates a new groupchat and puts it in the chats array.
 *
 * return group number on success.
 * return -1 on failure.
 */
int tox_add_groupchat(Tox *tox);

/* Delete a groupchat from the chats array.
 *
 * return 0 on success.
 * return -1 if failure.
 */
int tox_del_groupchat(Tox *tox, int groupnumber);

/* Copy the name of peernumber who is in groupnumber to name.
 * name must be at least TOX_MAX_NAME_LENGTH long.
 *
 * return length of name if success
 * return -1 if failure
 */
int tox_group_peername(const Tox *tox, int groupnumber, int peernumber, uint8_t *name);

/* Copy the public key of peernumber who is in groupnumber to public_key.
 * public_key must be TOX_PUBLIC_KEY_SIZE long.
 *
 * returns 0 on success
 * returns -1 on failure
 */
int tox_group_peer_pubkey(const Tox *tox, int groupnumber, int peernumber, uint8_t *public_key);

/* invite friendnumber to groupnumber
 * return 0 on success
 * return -1 on failure
 */
int tox_invite_friend(Tox *tox, int32_t friendnumber, int groupnumber);

/* Join a group (you need to have been invited first.) using data of length obtained
 * in the group invite callback.
 *
 * returns group number on success
 * returns -1 on failure.
 */
int tox_join_groupchat(Tox *tox, int32_t friendnumber, const uint8_t *data, uint16_t length);

/* send a group message
 * return 0 on success
 * return -1 on failure
 */
int tox_group_message_send(Tox *tox, int groupnumber, const uint8_t *message, uint16_t length);

/* send a group action
 * return 0 on success
 * return -1 on failure
 */
int tox_group_action_send(Tox *tox, int groupnumber, const uint8_t *action, uint16_t length);

/* set the group's title, limited to MAX_NAME_LENGTH
 * return 0 on success
 * return -1 on failure
 */
int tox_group_set_title(Tox *tox, int groupnumber, const uint8_t *title, uint8_t length);

/* Get group title from groupnumber and put it in title.
 * title needs to be a valid memory location with a max_length size of at least MAX_NAME_LENGTH (128) bytes.
 *
 *  return length of copied title if success.
 *  return -1 if failure.
 */
int tox_group_get_title(Tox *tox, int groupnumber, uint8_t *title, uint32_t max_length);

/* Check if the current peernumber corresponds to ours.
 *
 * return 1 if the peernumber corresponds to ours.
 * return 0 on failure.
 */
unsigned int tox_group_peernumber_is_ours(const Tox *tox, int groupnumber, int peernumber);

/* Return the number of peers in the group chat on success.
 * return -1 on failure
 */
int tox_group_number_peers(const Tox *tox, int groupnumber);

/* List all the peers in the group chat.
 *
 * Copies the names of the peers to the name[length][TOX_MAX_NAME_LENGTH] array.
 *
 * Copies the lengths of the names to lengths[length]
 *
 * returns the number of peers on success.
 *
 * return -1 on failure.
 */
int tox_group_get_names(const Tox *tox, int groupnumber, uint8_t names[][TOX_MAX_NAME_LENGTH], uint16_t lengths[],
                        uint16_t length);

/* Return the number of chats in the instance m.
 * You should use this to determine how much memory to allocate
 * for copy_chatlist. */
uint32_t tox_count_chatlist(const Tox *tox);

/* Copy a list of valid chat IDs into the array out_list.
 * If out_list is NULL, returns 0.
 * Otherwise, returns the number of elements copied.
 * If the array was too small, the contents
 * of out_list will be truncated to list_size. */
uint32_t tox_get_chatlist(const Tox *tox, int32_t *out_list, uint32_t list_size);

/* return the type of groupchat (TOX_GROUPCHAT_TYPE_) that groupnumber is.
 *
 * return -1 on failure.
 * return type on success.
 */
int tox_group_get_type(const Tox *tox, int groupnumber);

/****************AVATAR FUNCTIONS*****************/

/* Set the callback function for avatar information.
 * This callback will be called when avatar information are received from friends. These events
 * can arrive at anytime, but are usually received uppon connection and in reply of avatar
 * information requests.
 *
 * Function format is:
 *  function(Tox *tox, int32_t friendnumber, uint8_t format, uint8_t *hash, void *userdata)
 *
 * where 'format' is the avatar image format (see TOX_AVATAR_FORMAT) and 'hash' is the hash of
 * the avatar data for caching purposes and it is exactly TOX_HASH_LENGTH long. If the image
 * format is NONE, the hash is zeroed.
 *
 */
void tox_callback_avatar_info(Tox *tox, void (*function)(Tox *tox, int32_t, uint8_t, uint8_t *, void *),
                              void *userdata);


/* Set the callback function for avatar data.
 * This callback will be called when the complete avatar data was correctly received from a
 * friend. This only happens in reply of a avatar data request (see tox_request_avatar_data);
 *
 * Function format is:
 *  function(Tox *tox, int32_t friendnumber, uint8_t format, uint8_t *hash, uint8_t *data, uint32_t datalen, void *userdata)
 *
 * where 'format' is the avatar image format (see TOX_AVATAR_FORMAT); 'hash' is the
 * locally-calculated cryptographic hash of the avatar data and it is exactly
 * TOX_HASH_LENGTH long; 'data' is the avatar image data and 'datalen' is the length
 * of such data.
 *
 * If format is NONE, 'data' is NULL, 'datalen' is zero, and the hash is zeroed. The hash is
 * always validated locally with the function tox_hash and ensured to match the image data,
 * so this value can be safely used to compare with cached avatars.
 *
 * WARNING: users MUST treat all avatar image data received from another peer as untrusted and
 * potentially malicious. The library only ensures that the data which arrived is the same the
 * other user sent, and does not interpret or validate any image data.
 */
void tox_callback_avatar_data(Tox *tox, void (*function)(Tox *tox, int32_t, uint8_t, uint8_t *, uint8_t *, uint32_t,
                              void *), void *userdata);

/* Set the user avatar image data.
 * This should be made before connecting, so we will not announce that the user have no avatar
 * before setting and announcing a new one, forcing the peers to re-download it.
 *
 * Notice that the library treats the image as raw data and does not interpret it by any way.
 *
 * Arguments:
 *  format - Avatar image format or NONE for user with no avatar (see TOX_AVATAR_FORMAT);
 *  data - pointer to the avatar data (may be NULL it the format is NONE);
 *  length - length of image data. Must be <= TOX_AVATAR_MAX_DATA_LENGTH.
 *
 * returns 0 on success
 * returns -1 on failure.
 */
int tox_set_avatar(Tox *tox, uint8_t format, const uint8_t *data, uint32_t length);

/* Unsets the user avatar.

   returns 0 on success (currently always returns 0) */
int tox_unset_avatar(Tox *tox);

/* Get avatar data from the current user.
 * Copies the current user avatar data to the destination buffer and sets the image format
 * accordingly.
 *
 * If the avatar format is NONE, the buffer 'buf' isleft uninitialized, 'hash' is zeroed, and
 * 'length' is set to zero.
 *
 * If any of the pointers format, buf, length, and hash are NULL, that particular field will be ignored.
 *
 * Arguments:
 *   format - destination pointer to the avatar image format (see TOX_AVATAR_FORMAT);
 *   buf - destination buffer to the image data. Must have at least 'maxlen' bytes;
 *   length - destination pointer to the image data length;
 *   maxlen - length of the destination buffer 'buf';
 *   hash - destination pointer to the avatar hash (it must be exactly TOX_HASH_LENGTH bytes long).
 *
 * returns 0 on success;
 * returns -1 on failure.
 *
 */
int tox_get_self_avatar(const Tox *tox, uint8_t *format, uint8_t *buf, uint32_t *length, uint32_t maxlen,
                        uint8_t *hash);


/* Generates a cryptographic hash of the given data.
 * This function may be used by clients for any purpose, but is provided primarily for
 * validating cached avatars. This use is highly recommended to avoid unnecessary avatar
 * updates.
 * This function is a wrapper to internal message-digest functions.
 *
 * Arguments:
 *  hash - destination buffer for the hash data, it must be exactly TOX_HASH_LENGTH bytes long.
 *  data - data to be hashed;
 *  datalen - length of the data; for avatars, should be TOX_AVATAR_MAX_DATA_LENGTH
 *
 * returns 0 on success
 * returns -1 on failure.
 */
int tox_hash(uint8_t *hash, const uint8_t *data, const uint32_t datalen);

/* Request avatar information from a friend.
 * Asks a friend to provide their avatar information (image format and hash). The friend may
 * or may not answer this request and, if answered, the information will be provided through
 * the callback 'avatar_info'.
 *
 * returns 0 on success
 * returns -1 on failure.
 */
int tox_request_avatar_info(const Tox *tox, const int32_t friendnumber);


/* Send an unrequested avatar information to a friend.
 * Sends our avatar format and hash to a friend; he/she can use this information to validate
 * an avatar from the cache and may (or not) reply with an avatar data request.
 *
 * Notice: it is NOT necessary to send these notification after changing the avatar or
 * connecting. The library already does this.
 *
 * returns 0 on success
 * returns -1 on failure.
 */
int tox_send_avatar_info(Tox *tox, const int32_t friendnumber);


/* Request the avatar data from a friend.
 * Ask a friend to send their avatar data. The friend may or may not answer this request and,
 * if answered, the information will be provided in callback 'avatar_data'.
 *
 * returns 0 on sucess
 * returns -1 on failure.
 */
int tox_request_avatar_data(const Tox *tox, const int32_t friendnumber);

/****************FILE SENDING FUNCTIONS*****************/
/* NOTE: This how to will be updated.
 *
 * HOW TO SEND FILES CORRECTLY:
 * 1. Use tox_new_file_sender(...) to create a new file sender.
 * 2. Wait for the callback set with tox_callback_file_control(...) to be called with receive_send == 1 and control_type == TOX_FILECONTROL_ACCEPT
 * 3. Send the data with tox_file_send_data(...) with chunk size tox_file_data_size(...)
 * 4. When sending is done, send a tox_file_send_control(...) with send_receive = 0 and message_id = TOX_FILECONTROL_FINISHED
 * 5. when the callback set with tox_callback_file_control(...) is called with receive_send == 1 and control_type == TOX_FILECONTROL_FINISHED
 * the other person has received the file correctly.
 *
 * HOW TO RECEIVE FILES CORRECTLY:
 * 1. wait for the callback set with tox_callback_file_send_request(...)
 * 2. accept or refuse the connection with tox_file_send_control(...) with send_receive = 1 and message_id = TOX_FILECONTROL_ACCEPT or TOX_FILECONTROL_KILL
 * 3. save all the data received with the callback set with tox_callback_file_data(...) to a file.
 * 4. when the callback set with tox_callback_file_control(...) is called with receive_send == 0 and control_type == TOX_FILECONTROL_FINISHED
 * the file is done transferring.
 * 5. send a tox_file_send_control(...) with send_receive = 1 and message_id = TOX_FILECONTROL_FINISHED to confirm that we did receive the file.
 *
 * tox_file_data_remaining(...) can be used to know how many bytes are left to send/receive.
 *
 * If the connection breaks during file sending (The other person goes offline without pausing the sending and then comes back)
 * the receiver must send a control packet with send_receive == 1 message_id = TOX_FILECONTROL_RESUME_BROKEN and the data being
 * a uint64_t (in host byte order) containing the number of bytes received.
 *
 * If the sender receives this packet, he must send a control packet with send_receive == 0 and control_type == TOX_FILECONTROL_ACCEPT
 * then he must start sending file data from the position (data , uint64_t in host byte order) received in the TOX_FILECONTROL_RESUME_BROKEN packet.
 *
 * To pause a file transfer send a control packet with control_type == TOX_FILECONTROL_PAUSE.
 * To unpause a file transfer send a control packet with control_type == TOX_FILECONTROL_ACCEPT.
 *
 * If you receive a control packet with receive_send == 1 and control_type == TOX_FILECONTROL_PAUSE, you must stop sending filenumber until the other
 * person sends a control packet with send_receive == 0 and control_type == TOX_FILECONTROL_ACCEPT with the filenumber being a paused filenumber.
 *
 * If you receive a control packet with receive_send == 0 and control_type == TOX_FILECONTROL_PAUSE, it means the sender of filenumber has paused the
 * transfer and will resume it later with a control packet with send_receive == 1 and control_type == TOX_FILECONTROL_ACCEPT for that file number.
 *
 * More to come...
 */

enum {
    TOX_FILECONTROL_ACCEPT,
    TOX_FILECONTROL_PAUSE,
    TOX_FILECONTROL_KILL,
    TOX_FILECONTROL_FINISHED,
    TOX_FILECONTROL_RESUME_BROKEN
};
/* Set the callback for file send requests.
 *
 *  Function(Tox *tox, int32_t friendnumber, uint8_t filenumber, uint64_t filesize, const uint8_t *filename, uint16_t filename_length, void *userdata)
 */
void tox_callback_file_send_request(Tox *tox, void (*function)(Tox *m, int32_t, uint8_t, uint64_t, const uint8_t *,
                                    uint16_t, void *), void *userdata);

/* Set the callback for file control requests.
 *
 *  receive_send is 1 if the message is for a slot on which we are currently sending a file and 0 if the message
 *  is for a slot on which we are receiving the file
 *
 *  Function(Tox *tox, int32_t friendnumber, uint8_t receive_send, uint8_t filenumber, uint8_t control_type, const uint8_t *data, uint16_t length, void *userdata)
 *
 */
void tox_callback_file_control(Tox *tox, void (*function)(Tox *m, int32_t, uint8_t, uint8_t, uint8_t, const uint8_t *,
                               uint16_t, void *), void *userdata);

/* Set the callback for file data.
 *
 *  Function(Tox *tox, int32_t friendnumber, uint8_t filenumber, const uint8_t *data, uint16_t length, void *userdata)
 *
 */
void tox_callback_file_data(Tox *tox, void (*function)(Tox *m, int32_t, uint8_t, const uint8_t *, uint16_t length,
                            void *), void *userdata);


/* Send a file send request.
 * Maximum filename length is 255 bytes.
 *  return file number on success
 *  return -1 on failure
 */
int tox_new_file_sender(Tox *tox, int32_t friendnumber, uint64_t filesize, const uint8_t *filename,
                        uint16_t filename_length);

/* Send a file control request.
 *
 * send_receive is 0 if we want the control packet to target a file we are currently sending,
 * 1 if it targets a file we are currently receiving.
 *
 *  return 0 on success
 *  return -1 on failure
 */
int tox_file_send_control(Tox *tox, int32_t friendnumber, uint8_t send_receive, uint8_t filenumber, uint8_t message_id,
                          const uint8_t *data, uint16_t length);

/* Send file data.
 *
 *  return 0 on success
 *  return -1 on failure
 */
int tox_file_send_data(Tox *tox, int32_t friendnumber, uint8_t filenumber, const uint8_t *data, uint16_t length);

/* Returns the recommended/maximum size of the filedata you send with tox_file_send_data()
 *
 *  return size on success
 *  return -1 on failure (currently will never return -1)
 */
int tox_file_data_size(const Tox *tox, int32_t friendnumber);

/* Give the number of bytes left to be sent/received.
 *
 *  send_receive is 0 if we want the sending files, 1 if we want the receiving.
 *
 *  return number of bytes remaining to be sent/received on success
 *  return 0 on failure
 */
uint64_t tox_file_data_remaining(const Tox *tox, int32_t friendnumber, uint8_t filenumber, uint8_t send_receive);

/***************END OF FILE SENDING FUNCTIONS******************/

/*
 * Use this function to bootstrap the client.
 */

/* Resolves address into an IP address. If successful, sends a "get nodes"
 *   request to the given node with ip, port (in host byte order).
 *   and public_key to setup connections
 *
 * address can be a hostname or an IP address (IPv4 or IPv6).
 *
 *  returns 1 if the address could be converted into an IP address
 *  returns 0 otherwise
 */
int tox_bootstrap_from_address(Tox *tox, const char *address, uint16_t port, const uint8_t *public_key);

/* Like tox_bootstrap_from_address but for TCP relays only.
 *
 * return 0 on failure.
 * return 1 on success.
 */
int tox_add_tcp_relay(Tox *tox, const char *address, uint16_t port, const uint8_t *public_key);

/*  return 0 if we are not connected to the DHT.
 *  return 1 if we are.
 */
int tox_isconnected(const Tox *tox);

typedef enum {
    TOX_PROXY_NONE,
    TOX_PROXY_SOCKS5,
    TOX_PROXY_HTTP
} TOX_PROXY_TYPE;

typedef struct {
    /*
    *  The type of UDP socket created depends on ipv6enabled:
    *  If set to 0 (zero), creates an IPv4 socket which subsequently only allows
    *    IPv4 communication
    *  If set to anything else (default), creates an IPv6 socket which allows both IPv4 AND
    *    IPv6 communication
    */
    uint8_t ipv6enabled;

    /* Set to 1 to disable udp support. (default: 0)
       This will force Tox to use TCP only which may slow things down.
       Disabling udp support is necessary when using proxies or Tor.*/
    uint8_t udp_disabled;
    uint8_t proxy_type; /* a value from TOX_PROXY_TYPE */
    char proxy_address[256]; /* Proxy ip or domain in NULL terminated string format. */
    uint16_t proxy_port; /* Proxy port in host byte order. */
} Tox_Options;

/*
 *  Run this function at startup.
 *
 * Options are some options that can be passed to the Tox instance (see above struct).
 *
 * If options is NULL, tox_new() will use default settings.
 *
 * Initializes a tox structure
 *  return allocated instance of tox on success.
 *  return NULL on failure.
 */
Tox *tox_new(Tox_Options *options);

/* Run this before closing shop.
 * Free all datastructures. */
void tox_kill(Tox *tox);

/* Return the time in milliseconds before tox_do() should be called again
 * for optimal performance.
 *
 * returns time (in ms) before the next tox_do() needs to be run on success.
 */
uint32_t tox_do_interval(Tox *tox);

/* The main loop that needs to be run in intervals of tox_do_interval() ms. */
void tox_do(Tox *tox);

/* SAVING AND LOADING FUNCTIONS: */

/*  return size of messenger data (for saving). */
uint32_t tox_size(const Tox *tox);

/* Save the messenger in data (must be allocated memory of size Messenger_size()). */
void tox_save(const Tox *tox, uint8_t *data);

/* Load the messenger from data of size length.
 * NOTE: The Tox save format isn't stable yet meaning this function sometimes
 * returns -1 when loading older saves. This however does not mean nothing was
 * loaded from the save.
 *
 *  returns 0 on success
 *  returns -1 on failure
 *  returns +1 on finding encrypted save data
 */
int tox_load(Tox *tox, const uint8_t *data, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif
