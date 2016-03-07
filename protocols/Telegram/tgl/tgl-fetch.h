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
#ifndef __TGL_FETCH_H__
#define __TGL_FETCH_H__
#include "tgl.h"
#include "auto/auto-types.h"

struct tgl_user *tglf_fetch_alloc_user_new (struct tgl_state *TLS, struct tl_ds_user *DS_U);
struct tgl_user *tglf_fetch_alloc_user_full_new (struct tgl_state *TLS, struct tl_ds_user_full *DS_U);
struct tgl_chat *tglf_fetch_alloc_chat_new (struct tgl_state *TLS, struct tl_ds_chat *DS_C);
struct tgl_chat *tglf_fetch_alloc_chat_full_new (struct tgl_state *TLS, struct tl_ds_messages_chat_full *DS_MCF);
struct tgl_secret_chat *tglf_fetch_alloc_encrypted_chat_new (struct tgl_state *TLS, struct tl_ds_encrypted_chat *DS_EC);
struct tgl_message *tglf_fetch_alloc_message_new (struct tgl_state *TLS, struct tl_ds_message *DS_M);
struct tgl_message *tglf_fetch_alloc_message_short (struct tgl_state *TLS);
struct tgl_message *tglf_fetch_alloc_message_short_chat (struct tgl_state *TLS);
struct tgl_message *tglf_fetch_alloc_encrypted_message_new (struct tgl_state *TLS, struct tl_ds_encrypted_message *DS_EM);
tgl_peer_id_t tglf_fetch_peer_id_new (struct tgl_state *TLS, struct tl_ds_peer *DS_P);
long long tglf_fetch_user_photo_new (struct tgl_state *TLS, struct tgl_user *U, struct tl_ds_user_profile_photo *DS_UPP);

void tglf_fetch_message_media_new (struct tgl_state *TLS, struct tgl_message_media *M, struct tl_ds_message_media *DS_MM);
void tglf_fetch_message_action_new (struct tgl_state *TLS, struct tgl_message_action *M, struct tl_ds_message_action *DS_MA);
void tglf_fetch_chat_full (struct tgl_state *TLS, struct tgl_chat *C);

void tglf_fetch_encrypted_message_file_new (struct tgl_state *TLS, struct tgl_message_media *M, struct tl_ds_encrypted_file *DS_EF);
void tglf_fetch_message_media_encrypted_new (struct tgl_state *TLS, struct tgl_message_media *M, struct tl_ds_decrypted_message_media *DS_DMM);
void tglf_fetch_message_action_encrypted_new (struct tgl_state *TLS, struct tgl_message_action *M, struct tl_ds_decrypted_message_action *DS_DMA);
int tglf_fetch_user_status_new (struct tgl_state *TLS, struct tgl_user_status *S, struct tgl_user *U, struct tl_ds_user_status *DS_US);
enum tgl_typing_status tglf_fetch_typing_new (struct tl_ds_send_message_action *DS_SMA);
void tglf_fetch_chat_participants_new (struct tgl_state *TLS, struct tgl_chat *C, struct tl_ds_chat_participants *DS_CP);
void tglf_fetch_int_array (int *dst, struct tl_ds_vector *src, int len);
void tglf_fetch_int_tuple (int *dst, int **src, int len);
int tglf_fetch_file_location_new (struct tgl_state *TLS, struct tgl_file_location *loc, struct tl_ds_file_location *DS_FL);

void tglf_fetch_message_short_new (struct tgl_state *TLS, struct tgl_message *M, struct tl_ds_updates *DS_U);
void tglf_fetch_message_short_chat_new (struct tgl_state *TLS, struct tgl_message *M, struct tl_ds_updates *DS_U);

struct tgl_message *tglf_fetch_alloc_message_short_new (struct tgl_state *TLS, struct tl_ds_updates *DS_U);
struct tgl_message *tglf_fetch_alloc_message_short_chat_new (struct tgl_state *TLS, struct tl_ds_updates *DS_U);
struct tgl_photo *tglf_fetch_alloc_photo_new (struct tgl_state *TLS, struct tl_ds_photo *DS_P);
struct tgl_bot_info *tglf_fetch_alloc_bot_info (struct tgl_state *TLS, struct tl_ds_bot_info *DS_BI);
struct tgl_message_reply_markup *tglf_fetch_alloc_reply_markup (struct tgl_state *TLS, struct tgl_message *M, struct tl_ds_reply_markup *DS_RM);
#endif
