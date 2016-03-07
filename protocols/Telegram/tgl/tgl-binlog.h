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

    Copyright Vitaly Valtman 2013-2015
*/
#ifndef __BINLOG_H__
#define __BINLOG_H__

//#include "structures.h"
#include "tgl.h"
#include "auto/auto-types.h"

#ifdef __cplusplus
extern "C" {
#endif

void bl_do_dc_option (struct tgl_state *TLS, int id, const char *name, int l1, const char *ip, int l2, int port);
void bl_do_dc_option_new (struct tgl_state *TLS, int flags, int id, const char *name, int l1, const char *ip, int l2, int port);
void bl_do_set_working_dc (struct tgl_state *TLS, int num);
void bl_do_dc_signed (struct tgl_state *TLS, int id);
void bl_do_set_our_id (struct tgl_state *TLS, int id);
void bl_do_set_dh_params (struct tgl_state *TLS, int root, unsigned char prime[], int version);

void bl_do_set_pts (struct tgl_state *TLS, int pts);
void bl_do_set_qts (struct tgl_state *TLS, int qts);
void bl_do_set_date (struct tgl_state *TLS, int date);
void bl_do_set_seq (struct tgl_state *TLS, int seq);

void bl_do_set_msg_id (struct tgl_state *TLS, struct tgl_message *M, int id);

void bl_do_user_delete (struct tgl_state *TLS, struct tgl_user *U);
void bl_do_encr_chat_delete (struct tgl_state *TLS, struct tgl_secret_chat *U);

void bl_do_chat_add_user (struct tgl_state *TLS, struct tgl_chat *C, int version, int user, int inviter, int date);
void bl_do_chat_del_user (struct tgl_state *TLS, struct tgl_chat *C, int version, int user);

void bl_do_create_message_new (struct tgl_state *TLS, long long id, int *from_id, int *to_type, int *to_id, int *fwd_from_id, int *fwd_date, int *date, const char *message, int message_len, struct tl_ds_message_media *media, struct tl_ds_message_action *action, int *reply_id, struct tl_ds_reply_markup *reply_markup, int flags);

void bl_do_create_message_encr_new (struct tgl_state *TLS, long long id, int *from_id, int *to_type, int *to_id, int *date, const char *message, int message_len, struct tl_ds_decrypted_message_media *media, struct tl_ds_decrypted_message_action *action, struct tl_ds_encrypted_file *file, int flags);
void bl_do_message_delete (struct tgl_state *TLS, struct tgl_message *M);

void bl_do_encr_chat_exchange_new (struct tgl_state *TLS, struct tgl_secret_chat *E, long long *exchange_id, const void *key, int *state);

void bl_do_msg_update (struct tgl_state *TLS, long long id);
void bl_do_reset_authorization (struct tgl_state *TLS);

void bl_do_user_new (struct tgl_state *TLS, int id, long long *access_hash, const char *first_name, int first_name_len, const char *last_name, int last_name_len, const char *phone, int phone_len, const char *username, int username_len, struct tl_ds_photo *photo, const char *real_first_name, int real_first_name_len, const char *real_last_name, int real_last_name_len, struct tl_ds_user_profile_photo *profile_photo, int *last_read_in, int *last_read_out, struct tl_ds_bot_info *bot_info, int flags);

void bl_do_set_auth_key (struct tgl_state *TLS, int num, unsigned char *buf);

void bl_do_chat_new (struct tgl_state *TLS, int id, const char *title, int title_len, int *user_num, int *date, int *version, struct tl_ds_vector *participants, struct tl_ds_chat_photo *chat_photo, struct tl_ds_photo *photo, int *admin, int *last_read_in, int *last_read_out, int flags);

void bl_do_encr_chat_new (struct tgl_state *TLS, int id, long long *access_hash, int *date, int *admin, int *user_id, void *key, void *g_key, void *first_key_id, int *state, int *ttl, int *layer, int *in_seq_no, int *last_in_seq_no, int *out_seq_no, long long *key_fingerprint, int flags);

#ifdef __cplusplus
}
#endif

#endif
