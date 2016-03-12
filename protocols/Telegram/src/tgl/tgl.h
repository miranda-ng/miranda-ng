/*
    This file is part of tgl-library

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    Copyright Vitaly Valtman 2014-2015
*/
#ifndef __TGL_H__
#define __TGL_H__

#include "tgl-layout.h"
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include "config.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#define TGL_MAX_DC_NUM 100
#define TG_SERVER_1 "149.154.175.50"
#define TG_SERVER_2 "149.154.167.51"
#define TG_SERVER_3 "149.154.175.100"
#define TG_SERVER_4 "149.154.167.91"
#define TG_SERVER_5 "149.154.171.5"
#define TG_SERVER_IPV6_1 "2001:b28:f23d:f001::a"
#define TG_SERVER_IPV6_2 "2001:67c:4e8:f002::a"
#define TG_SERVER_IPV6_3 "2001:b28:f23d:f003::a"
#define TG_SERVER_IPV6_4 "2001:67c:4e8:f004::a"
#define TG_SERVER_IPV6_5 "2001:b28:f23f:f005::a"
#define TG_SERVER_DEFAULT 2

#define TG_SERVER_TEST_1 "149.154.175.10"
#define TG_SERVER_TEST_2 "149.154.167.40"
#define TG_SERVER_TEST_3 "149.154.175.117"
#define TG_SERVER_TEST_IPV6_1 "2001:b28:f23d:f001::e"
#define TG_SERVER_TEST_IPV6_2 "2001:67c:4e8:f002::e"
#define TG_SERVER_TEST_IPV6_3 "2001:b28:f23d:f003::e"
#define TG_SERVER_TEST_DEFAULT 2

// JUST RANDOM STRING
#define TGL_VERSION "2.0.3"

#define TGL_ENCRYPTED_LAYER 17
#define TGL_SCHEME_LAYER 31

struct connection;
struct mtproto_methods;
struct tgl_session;
struct tgl_dc;

#define TGL_UPDATE_CREATED 1
#define TGL_UPDATE_DELETED 2
#define TGL_UPDATE_PHONE 4
#define TGL_UPDATE_CONTACT 8
#define TGL_UPDATE_PHOTO 16
#define TGL_UPDATE_BLOCKED 32
#define TGL_UPDATE_REAL_NAME 64
#define TGL_UPDATE_NAME 128
#define TGL_UPDATE_REQUESTED 256
#define TGL_UPDATE_WORKING 512
#define TGL_UPDATE_FLAGS 1024
#define TGL_UPDATE_TITLE 2048
#define TGL_UPDATE_ADMIN 4096
#define TGL_UPDATE_MEMBERS 8192
#define TGL_UPDATE_ACCESS_HASH 16384
#define TGL_UPDATE_USERNAME (1 << 15)

#if defined(WIN32) || defined(_WIN32)
#   if !defined(__cplusplus) && ( !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L )
#      if __GNUC__ >= 2 || (defined(_MSC_VER) && _MSC_VER >= 1300)
#         define __func__ __FUNCTION__
#      else
#         define __func__ "<unknown>"
#      endif
#   endif
#   ifdef _MSC_VER
#      define __attribute__(x)
#   endif
#   define _PRINTF_INT64_ "I64"
#   define lrand48() rand()
#   define srand48(x) srand(x)
    inline const char* GetLastErrorStr(int err)
    {
	    static char errorText[256] = { 0 };
	    FormatMessageA(
		    FORMAT_MESSAGE_FROM_SYSTEM,
		    NULL,
		    err,
		    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		    (LPSTR)&errorText,
		    256,
		    NULL);
	    return (const char*)errorText;
    }

    inline const char* GetErrnoStr(int err)
    {
	    static char errnoStr[256] = { 0 };
	    strerror_s(errnoStr, 256, err);
	    return (const char*)errnoStr;
    }
#else
#   define _PRINTF_INT64_ "ll"
#endif

struct tgl_allocator {
  void *(*alloc)(size_t size);
  void *(*realloc)(void *ptr, size_t old_size, size_t size);
  void (*free)(void *ptr, int size);
  void (*check)(void);
  void (*exists)(void *ptr, int size);
};
extern struct tgl_allocator tgl_allocator_release;
extern struct tgl_allocator tgl_allocator_debug;
struct tgl_state;

enum tgl_value_type {
    tgl_phone_number,           // user phone number
    tgl_code,                   // telegram login code, or 'call' for phone call request
    tgl_register_info,          // "Y/n" register?, first name, last name
    tgl_new_password,           // new pass, confirm new pass
    tgl_cur_and_new_password,   // curr pass, new pass, confirm new pass
    tgl_cur_password,           // current pass
    tgl_bot_hash
};

struct tgl_update_callback {
  void (*new_msg)(struct tgl_state *TLS, struct tgl_message *M);
  void (*marked_read)(struct tgl_state *TLS, int num, struct tgl_message *list[]);
  void (*logprintf)(const char *format, ...)  __attribute__ ((format (printf, 1, 2)));
  void (*get_values)(struct tgl_state *TLS, enum tgl_value_type type, const char *prompt, int num_values,
          void (*callback)(struct tgl_state *TLS, const char *string[], void *arg), void *arg);
  void (*logged_in)(struct tgl_state *TLS);
  void (*started)(struct tgl_state *TLS);
  void (*type_notification)(struct tgl_state *TLS, struct tgl_user *U, enum tgl_typing_status status);
  void (*type_in_chat_notification)(struct tgl_state *TLS, struct tgl_user *U, struct tgl_chat *C, enum tgl_typing_status status);
  void (*type_in_secret_chat_notification)(struct tgl_state *TLS, struct tgl_secret_chat *E);
  void (*status_notification)(struct tgl_state *TLS, struct tgl_user *U);
  void (*user_registered)(struct tgl_state *TLS, struct tgl_user *U);
  void (*user_activated)(struct tgl_state *TLS, struct tgl_user *U);
  void (*new_authorization)(struct tgl_state *TLS, const char *device, const char *location);
  void (*chat_update)(struct tgl_state *TLS, struct tgl_chat *C, unsigned flags);
  void (*user_update)(struct tgl_state *TLS, struct tgl_user *C, unsigned flags);
  void (*secret_chat_update)(struct tgl_state *TLS, struct tgl_secret_chat *C, unsigned flags);
  void (*msg_receive)(struct tgl_state *TLS, struct tgl_message *M);
  void (*our_id)(struct tgl_state *TLS, int id);
  void (*notification)(struct tgl_state *TLS, const char *type, const char *message);
  void (*user_status_update)(struct tgl_state *TLS, struct tgl_user *U);
  char *(*create_print_name) (struct tgl_state *TLS, tgl_peer_id_t id, const char *a1, const char *a2, const char *a3, const char *a4);
};

struct tgl_net_methods {
  int (*write_out) (struct connection *c, const void *data, int len);
  int (*read_in) (struct connection *c, void *data, int len);
  int (*read_in_lookup) (struct connection *c, void *data, int len);
  void (*flush_out) (struct connection *c);
  void (*incr_out_packet_num) (struct connection *c);
  void (*free) (struct connection *c);
  struct tgl_dc *(*get_dc) (struct connection *c);
  struct tgl_session *(*get_session) (struct connection *c);

  struct connection *(*create_connection) (struct tgl_state *TLS, const char *host, int port, struct tgl_session *session, struct tgl_dc *dc, struct mtproto_methods *methods);
};

struct mtproto_methods {
  int (*ready) (struct tgl_state *TLS, struct connection *c);
  int (*close) (struct tgl_state *TLS, struct connection *c);
  int (*execute) (struct tgl_state *TLS, struct connection *c, int op, int len);
};

struct tgl_timer;
struct tree_random_id;

struct tgl_timer_methods {
  struct tgl_timer *(*alloc) (struct tgl_state *TLS, void (*cb)(struct tgl_state *TLS, void *arg), void *arg);
  void (*insert) (struct tgl_timer *t, double timeout);
  void (*remove) (struct tgl_timer *t);
  void (*free) (struct tgl_timer *t);
};

#define E_ERROR 0
#define E_WARNING 1
#define E_NOTICE 2
#define E_DEBUG 6

#define TGL_LOCK_DIFF 1
#define TGL_LOCK_PASSWORD 2

#define TGL_MAX_RSA_KEYS_NUM 10
// Do not modify this structure, unless you know what you do

#pragma pack(push,4)
struct tgl_state {
  int our_id; // ID of logged in user
  int encr_root;
  unsigned char *encr_prime;
  void *encr_prime_bn;
  int encr_param_version;
  int pts;
  int qts;
  int date;
  int seq;
  int binlog_enabled;
  int test_mode;
  int verbosity;
  int unread_messages;
  int active_queries;
  int max_msg_id;
  int started;
  int disable_link_preview;

  long long locks;
  struct tgl_dc *DC_list[TGL_MAX_DC_NUM];
  struct tgl_dc *DC_working;
  int max_dc_num;
  int dc_working_num;
  int enable_pfs;
  int temp_key_expire_time;

  long long cur_uploading_bytes;
  long long cur_uploaded_bytes;
  long long cur_downloading_bytes;
  long long cur_downloaded_bytes;

  char *binlog_name;
  char *auth_file;
  char *downloads_directory;

  struct tgl_update_callback callback;
  struct tgl_net_methods *net_methods;
  void *ev_base;

  char *rsa_key_list[TGL_MAX_RSA_KEYS_NUM];
  void *rsa_key_loaded[TGL_MAX_RSA_KEYS_NUM];
  long long rsa_key_fingerprint[TGL_MAX_RSA_KEYS_NUM];
  int rsa_key_num;

  struct bignum_ctx *BN_ctx;

  struct tgl_allocator allocator;

  struct tree_peer *peer_tree;
  struct tree_peer_by_name *peer_by_name_tree;
  struct tree_message *message_tree;
  struct tree_message *message_unsent_tree;
  struct tree_photo *photo_tree;
  struct tree_document *document_tree;
  struct tree_webpage *webpage_tree;
  struct tree_encr_document *encr_document_tree;

  int users_allocated;
  int chats_allocated;
  int messages_allocated;
  int peer_num;
  int peer_size;
  int encr_chats_allocated;
  int geo_chats_allocated;

  tgl_peer_t **Peers;
  struct tgl_message message_list;

  int binlog_fd;

  struct tgl_timer_methods *timer_methods;

  struct tree_query *queries_tree;

  char *base_path;

  struct tree_user *online_updates;

  struct tgl_timer *online_updates_timer;

  int app_id;
  char *app_hash;

  void *ev_login;

  char *app_version;
  int ipv6_enabled;

  struct tree_random_id *random_id_tree;

  char *error;
  int error_code;

  int is_bot;
};
#pragma pack(pop)
//extern struct tgl_state tgl_state;

#ifdef __cplusplus
extern "C" {
#endif

void tgl_reopen_binlog_for_writing (struct tgl_state *TLS);
void tgl_replay_log (struct tgl_state *TLS);

int tgl_print_stat (struct tgl_state *TLS, char *s, int len);
tgl_peer_t *tgl_peer_get (struct tgl_state *TLS, tgl_peer_id_t id);
tgl_peer_t *tgl_peer_get_by_name (struct tgl_state *TLS, const char *s);

struct tgl_message *tgl_message_get (struct tgl_state *TLS, long long id);
void tgl_peer_iterator_ex (struct tgl_state *TLS, void (*it)(tgl_peer_t *P, void *extra), void *extra);

int tgl_complete_user_list (struct tgl_state *TLS, int index, const char *text, int len, char **R);
int tgl_complete_chat_list (struct tgl_state *TLS, int index, const char *text, int len, char **R);
int tgl_complete_encr_chat_list (struct tgl_state *TLS, int index, const char *text, int len, char **R);
int tgl_complete_peer_list (struct tgl_state *TLS, int index, const char *text, int len, char **R);
int tgl_secret_chat_for_user (struct tgl_state *TLS, tgl_peer_id_t user_id);
int tgl_do_send_bot_auth (struct tgl_state *TLS, const char *code, int code_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_user *Self), void *callback_extra);

#define TGL_PEER_USER 1
#define TGL_PEER_CHAT 2
#define TGL_PEER_GEO_CHAT 3
#define TGL_PEER_ENCR_CHAT 4
#define TGL_PEER_UNKNOWN 0

#define TGL_MK_USER(id) tgl_set_peer_id (TGL_PEER_USER,id)
#define TGL_MK_CHAT(id) tgl_set_peer_id (TGL_PEER_CHAT,id)
#define TGL_MK_GEO_CHAT(id) tgl_set_peer_id (TGL_PEER_GEO_CHAT,id)
#define TGL_MK_ENCR_CHAT(id) tgl_set_peer_id (TGL_PEER_ENCR_CHAT,id)

void tgl_set_binlog_mode (struct tgl_state *TLS, int mode);
void tgl_set_binlog_path (struct tgl_state *TLS, const char *path);
void tgl_set_auth_file_path (struct tgl_state *TLS, const char *path);
void tgl_set_download_directory (struct tgl_state *TLS, const char *path);
void tgl_set_callback (struct tgl_state *TLS, struct tgl_update_callback *cb);
void tgl_set_rsa_key (struct tgl_state *TLS, const char *key);
void tgl_set_app_version (struct tgl_state *TLS, const char *app_version);

static inline int tgl_get_peer_type (tgl_peer_id_t id) {
  return id.type;
}

static inline int tgl_get_peer_id (tgl_peer_id_t id) {
  return id.id;
}

static inline tgl_peer_id_t tgl_set_peer_id (int type, int id) {
  tgl_peer_id_t ID;
  ID.id = id;
  ID.type = type;
  return ID;
}

static inline int tgl_cmp_peer_id (tgl_peer_id_t a, tgl_peer_id_t b) {
  return memcmp (&a, &b, sizeof (a));
}

void tgl_incr_verbosity (struct tgl_state *TLS);
void tgl_set_verbosity (struct tgl_state *TLS, int val);
void tgl_enable_pfs (struct tgl_state *TLS);
void tgl_set_test_mode (struct tgl_state *TLS);
void tgl_set_net_methods (struct tgl_state *TLS, struct tgl_net_methods *methods);
void tgl_set_timer_methods (struct tgl_state *TLS, struct tgl_timer_methods *methods);
void tgl_set_ev_base (struct tgl_state *TLS, void *ev_base);

int tgl_authorized_dc (struct tgl_state *TLS, struct tgl_dc *DC);
int tgl_signed_dc (struct tgl_state *TLS, struct tgl_dc *DC);

void tgl_init (struct tgl_state *TLS);
void tgl_dc_authorize (struct tgl_state *TLS, struct tgl_dc *DC);

void tgl_dc_iterator (struct tgl_state *TLS, void (*iterator)(struct tgl_dc *DC));
void tgl_dc_iterator_ex (struct tgl_state *TLS, void (*iterator)(struct tgl_dc *DC, void *extra), void *extra);

#define TGL_SEND_MSG_FLAG_DISABLE_PREVIEW 1
#define TGL_SEND_MSG_FLAG_ENABLE_PREVIEW 2

#define TGL_SEND_MSG_FLAG_DOCUMENT_IMAGE TGLDF_IMAGE
#define TGL_SEND_MSG_FLAG_DOCUMENT_STICKER TGLDF_STICKER
#define TGL_SEND_MSG_FLAG_DOCUMENT_ANIMATED TGLDF_ANIMATED
#define TGL_SEND_MSG_FLAG_DOCUMENT_AUDIO TGLDF_AUDIO
#define TGL_SEND_MSG_FLAG_DOCUMENT_VIDEO TGLDF_VIDEO
#define TGL_SEND_MSG_FLAG_DOCUMENT_AUTO 32
#define TGL_SEND_MSG_FLAG_DOCUMENT_PHOTO 64

#define TGL_SEND_MSG_FLAG_REPLY(x) (((unsigned long long)x) << 32)

typedef tgl_peer_id_t tgl_user_id_t;
typedef tgl_peer_id_t tgl_chat_id_t;
typedef tgl_peer_id_t tgl_secret_chat_id_t;
typedef tgl_peer_id_t tgl_user_or_chat_id_t;

void tgl_insert_empty_user (struct tgl_state *TLS, int id);
void tgl_insert_empty_chat (struct tgl_state *TLS, int id);


void tgl_free_all (struct tgl_state *TLS);
void tgl_register_app_id (struct tgl_state *TLS, int app_id, const char *app_hash);

void tgl_login (struct tgl_state *TLS);
void tgl_enable_ipv6 (struct tgl_state *TLS);
void tgl_enable_bot (struct tgl_state *TLS);

struct tgl_state *tgl_state_alloc (void);

void tgl_disable_link_preview (struct tgl_state *TLS);
void tgl_do_lookup_state (struct tgl_state *TLS);

/* {{{ WORK WITH ACCOUNT */
// sets account password
// user will be requested to type his current password and new password (twice)
void tgl_do_set_password (struct tgl_state *TLS, const char *hint, int hint_len, void (*callback)(struct tgl_state *TLS, void *extra, int success), void *extra);
/* }}} */

/* {{{ SENDING MESSAGES */

struct tl_ds_reply_markup;

// send plain text message to peer id
// flags is combination of TGL_SEND_MSG_FLAG_*
// reply markup can be NULL
void tgl_do_send_message (struct tgl_state *TLS, tgl_peer_id_t id, const char *text, int text_len, unsigned long long flags, struct tl_ds_reply_markup *reply_markup, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra);

// sends plain text reply on message *reply_id*
// message *reply_id* should be cached
void tgl_do_reply_message (struct tgl_state *TLS, int reply_id, const char *text, int text_len, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra);

// sends contents of text file *file_name* as plain text message
void tgl_do_send_text (struct tgl_state *TLS, tgl_peer_id_t id, const char *file_name, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra);
void tgl_do_reply_text (struct tgl_state *TLS, int reply_id, const char *file_name, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra);

// send media from file *file_name* to peer *to_id*
// if reply > 0 this message is sent as reply to message *reply*
// *caption* is used only for photos
void tgl_do_send_document (struct tgl_state *TLS, tgl_peer_id_t to_id, const char *file_name, const char *caption, int caption_len, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra);
void tgl_do_reply_document (struct tgl_state *TLS, int reply_id, const char *file_name, const char *caption, int caption_len, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra);

// forward message *msg_id* to peer *id*
// message can not be encrypted and peer can not be secret chat
void tgl_do_forward_message (struct tgl_state *TLS, tgl_user_or_chat_id_t id, int msg_id, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra);

// forward messages *ids* to peer *id*
// messages can not be encrypted and peer can not be secret chat
void tgl_do_forward_messages (struct tgl_state *TLS, tgl_user_or_chat_id_t id, int size, const int ids[], unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, int count, struct tgl_message *ML[]), void *callback_extra);

// sends contact to another user.
// This contact may be or may not be telegram user
void tgl_do_send_contact (struct tgl_state *TLS, tgl_peer_id_t id, const char *phone, int phone_len, const char *first_name, int first_name_len, const char *last_name, int last_name_len, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra);

// reply on message *reply_id* with contact
void tgl_do_reply_contact (struct tgl_state *TLS, int reply_id, const char *phone, int phone_len, const char *first_name, int first_name_len, const char *last_name, int last_name_len, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra);

// sends media from message *msg_id* to another dialog
// a bit different from forwarding message with media
// secret message media can be forwarded to secret chats
// and non-secret - to non-secret chats and users
void tgl_do_forward_media (struct tgl_state *TLS, tgl_peer_id_t id, int msg_id, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra);

// sends location to chat *id*
void tgl_do_send_location (struct tgl_state *TLS, tgl_peer_id_t id, double latitude, double longitude, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra);

// replies on message *reply_id* with location
void tgl_do_reply_location (struct tgl_state *TLS, int reply_id, double latitude, double longitude, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra);

// sends broadcast (i.e. message to several users at once)
// flags are same as in tgl_do_send_message
void tgl_do_send_broadcast (struct tgl_state *TLS, int num, tgl_user_id_t id[], const char *text, int text_len, unsigned long long flags, void (*callback)(struct tgl_state *TLS, void *extra, int success, int num, struct tgl_message *ML[]), void *callback_extra);

/* }}} */

/* {{{ EDITING SELF PROFILE */
// sets self profile photo
// server will cut central square from this photo
void tgl_do_set_profile_photo (struct tgl_state *TLS, const char *file_name, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra);

// rename self account
void tgl_do_set_profile_name (struct tgl_state *TLS, const char *first_name, int first_name_len, const char *last_name, int last_name_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_user *U), void *callback_extra);

// sets self username
void tgl_do_set_username (struct tgl_state *TLS, const char *username, int username_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_user *U), void *callback_extra);

// updates online/offline status
void tgl_do_update_status (struct tgl_state *TLS, int online, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra);

// exports card. This card can be later be used by another user to add you to dialog list.
void tgl_do_export_card (struct tgl_state *TLS, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, int size, int *card), void *callback_extra);
/* }}} */

/* {{{ WORKING WITH GROUP CHATS */

// sets chat photo
// server will cut central square from this photo
void tgl_do_set_chat_photo (struct tgl_state *TLS, tgl_chat_id_t chat_id, const char *file_name, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra);

// sets chat title
void tgl_do_rename_chat (struct tgl_state *TLS, tgl_chat_id_t id, const char *new_title, int new_title_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra);

// requests full info about chat *id*.
// if *offline_mode* is set no actual query is sent
void tgl_do_get_chat_info (struct tgl_state *TLS, tgl_chat_id_t id, int offline_mode, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_chat *C), void *callback_extra);

// adds user *id* to chat *chat_id*
// sends *limit* last messages from this chat to user
void tgl_do_add_user_to_chat (struct tgl_state *TLS, tgl_chat_id_t chat_id, tgl_user_id_t id, int limit, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra);

// deleted user *id* from chat *chat_id*
// you can do it if you are admin (=creator) of chat or if you invited this user or if it is yourself
void tgl_do_del_user_from_chat (struct tgl_state *TLS, tgl_chat_id_t chat_id, tgl_user_id_t id, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra);

// creates group chat with users ids
// there should be at least one user other then you in chat
void tgl_do_create_group_chat (struct tgl_state *TLS, int users_num, tgl_user_id_t ids[], const char *chat_topic, int chat_topic_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra);

// receives invitation link to this chat
// only chat admin can create one
// prevoius link invalidated, if existed
void tgl_do_export_chat_link (struct tgl_state *TLS, tgl_peer_id_t id, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *link), void *callback_extra);

// joins to secret chat by link (or hash of this link)
void tgl_do_import_chat_link (struct tgl_state *TLS, const char *link, int link_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra);

/* }}} */

/* {{{ WORKING WITH USERS */

// requests full info about user *id*.
// if *offline_mode* is set no actual query is sent
void tgl_do_get_user_info (struct tgl_state *TLS, tgl_user_id_t id, int offline_mode, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_user *U), void *callback_extra);

// adds contact to contact list by phone number
// user will be named  *first_name* *last_name* in contact list
// force should be set to 0
void tgl_do_add_contact (struct tgl_state *TLS, const char *phone, int phone_len, const char *first_name, int first_name_len, const char *last_name, int last_name_len, int force, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, int size, struct tgl_user *users[]), void *callback_extra);

// deletes user *id* from contact list
void tgl_do_del_contact (struct tgl_state *TLS, tgl_peer_id_t id, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra);

// imports card exported by another user
void tgl_do_import_card (struct tgl_state *TLS, int size, int *card, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_user *U), void *callback_extra);

// blocks user
void tgl_do_block_user (struct tgl_state *TLS, tgl_peer_id_t id, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra);

// unblocks blocked user
void tgl_do_unblock_user (struct tgl_state *TLS, tgl_peer_id_t id, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra);
/* }}} */

/* {{{ WORKING WITH SECRET CHATS */

// requests creation of secret chat with user *user_id*
//void tgl_do_create_encr_chat_request (struct tgl_state *TLS, int user_id, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_secret_chat *E), void *callback_extra);

// accepts secret chat request
// method can fail if another device will be first to accept it
void tgl_do_accept_encr_chat_request (struct tgl_state *TLS, struct tgl_secret_chat *E, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_secret_chat *E), void *callback_extra);

// sets ttl of secret chat
void tgl_do_set_encr_chat_ttl (struct tgl_state *TLS, struct tgl_secret_chat *E, int ttl, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra);

// returns secret chat fingerprint
int tgl_do_visualize_key (struct tgl_state *TLS, tgl_secret_chat_id_t id, unsigned char buf[16]);

// requests creation of secret chat with user id
void tgl_do_create_secret_chat (struct tgl_state *TLS, tgl_user_id_t id, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_secret_chat *E), void *callback_extra);
/* }}} */

/* {{{ WORKING WITH DIALOG LIST */

// receives all dialogs (except secret chats) from offset=*offset* with limit=*limit*
// dialogs are sorted by last message received
// if limit is > 100 there is a (small) chance of one dialog received twice
void tgl_do_get_dialog_list (struct tgl_state *TLS, int limit, int offset, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, int size, tgl_peer_id_t peers[], int last_msg_id[], int unread_count[]), void *callback_extra);

// resolves username
void tgl_do_contact_search (struct tgl_state *TLS, const char *name, int name_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_user *U), void *callback_extra);

// requests contact list
void tgl_do_update_contact_list (struct tgl_state *TLS, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, int size, struct tgl_user *contacts[]), void *callback_extra);

/* }}} */

/* {{{ WORKING WITH ONE DIALOG */

// marks all inbound messages from peer id as read
void tgl_do_mark_read (struct tgl_state *TLS, tgl_peer_id_t id, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra);

// requests last *limit* from offset *offset* (offset = 0 means most recent) messages from dialog with peer id
// if offline_mode=1 then no actual query is sent
// only locally cached messages returned
// also marks messages from this chat as read
void tgl_do_get_history (struct tgl_state *TLS, tgl_peer_id_t id, int offset, int limit, int offline_mode, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, int size, struct tgl_message *list[]), void *callback_extra);

// sends typing event to chat
// set status=tgl_typing_typing for default typing event
void tgl_do_send_typing (struct tgl_state *TLS, tgl_peer_id_t id, enum tgl_typing_status status, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra);

/* }}} */

/* {{{ WORKING WITH MEDIA */

// loads photo/document/document_thumb to downloads directory
// if file is presented it is not redownloaded (but if it is shortened tail will be downloaded)
// returns file name in callback
void tgl_do_load_photo (struct tgl_state *TLS, struct tgl_photo *photo, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *file_name), void *callback_extra);
void tgl_do_load_encr_document (struct tgl_state *TLS, struct tgl_encr_document *V, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *file_name), void *callback_extra);
void tgl_do_load_document (struct tgl_state *TLS, struct tgl_document *V, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *file_name), void *callback_extra);
void tgl_do_load_document_thumb (struct tgl_state *TLS, struct tgl_document *video, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *file_name), void *callback_extra);
void tgl_do_load_video (struct tgl_state *TLS, struct tgl_document *V, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *filename), void *callback_extra);
void tgl_do_load_audio (struct tgl_state *TLS, struct tgl_document *V, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *filename), void *callback_extra);


// loads file by location. Use only for small files!
void tgl_do_load_file_location (struct tgl_state *TLS, struct tgl_file_location *FL, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *file_name), void *callback_extra);

/* }}} */


/* {{{ ANOTHER MESSAGES FUNCTIONS */
// search messages with ids *from* .. *to* in dialog id
// id type of id is UNKNOWN uses global search (in all dialogs) instead
// if *from* or *to* is means *from*=0 and *to*=+INF
// return up to *limit* entries from offset=*offset*
void tgl_do_msg_search (struct tgl_state *TLS, tgl_user_or_chat_id_t id, int from, int to, int limit, int offset, const char *query, int query_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, int size, struct tgl_message *list[]), void *callback_extra);

// deletes message *id*
void tgl_do_delete_msg (struct tgl_state *TLS, long long msg_id, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra);

// gets message by *id*
void tgl_do_get_message (struct tgl_state *TLS, long long id, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, struct tgl_message *M), void *callback_extra);

/* }}} */


/* {{{ EXTENDED QUERIES USE WITH CAUTION */
// sends query with extended text syntax
// use only for debug or when you known what are you doing
// since answer is not interpretated by library in any way
void tgl_do_send_extf (struct tgl_state *TLS, const char *data, int data_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success, const char *data), void *callback_extra);
int tglf_extf_autocomplete (struct tgl_state *TLS, const char *text, int text_len, int index, char **R, char *data, int data_len);
struct paramed_type *tglf_extf_store (struct tgl_state *TLS, const char *data, int data_len);
char *tglf_extf_fetch (struct tgl_state *TLS, struct paramed_type *T);
/* }}} */

/* {{{ BOT */
void tgl_do_start_bot (struct tgl_state *TLS, tgl_peer_id_t bot, tgl_peer_id_t chat, const char *str, int str_len, void (*callback)(struct tgl_state *TLS, void *callback_extra, int success), void *callback_extra);
/* }}} */

#ifdef __cplusplus
}
#endif

#endif
