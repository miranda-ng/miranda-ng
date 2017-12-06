#include "stdafx.h"

/* GROUP CHAT FUNCTIONS: WARNING Group chats will be rewritten so this might change */

void tox_callback_group_invite(Tox *tox, void(*function)(Tox *tox, int32_t, uint8_t, const uint8_t *, uint16_t, void *), void *userdata)
{
	CreateFunction<int(*)(Tox*, void(*)(Tox*, int32_t, uint8_t, const uint8_t*, uint16_t, void*), void*)>(__FUNCTION__)(tox, function, userdata);
}

/*void tox_callback_group_message(Tox *tox, void(*function)(Tox *tox, int, int, const uint8_t *, uint16_t, void *), void *userdata)
{
}

void tox_callback_group_action(Tox *tox, void(*function)(Tox *tox, int, int, const uint8_t *, uint16_t, void *), void *userdata)
{
}

void tox_callback_group_title(Tox *tox, void(*function)(Tox *tox, int, int, const uint8_t *, uint8_t, void *), void *userdata)
{
}

void tox_callback_group_namelist_change(Tox *tox, void(*function)(Tox *tox, int, int, uint8_t, void *), void *userdata)
{
}*/

int tox_add_groupchat(Tox *tox)
{
	return CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
}

int tox_del_groupchat(Tox *tox, int groupnumber)
{
	return CreateFunction<int(*)(Tox*, int)>(__FUNCTION__)(tox, groupnumber);
}

/*int tox_group_peername(const Tox *tox, int groupnumber, int peernumber, uint8_t *name)
{
}

int tox_group_peer_pubkey(const Tox *tox, int groupnumber, int peernumber, uint8_t *pk)
{
}*/

int tox_invite_friend(Tox *tox, int32_t friendnumber, int groupnumber)
{
	return CreateFunction<int(*)(Tox*, int32_t, int)>(__FUNCTION__)(tox, friendnumber, groupnumber);
}

int tox_join_groupchat(Tox *tox, int32_t friendnumber, const uint8_t *data, uint16_t length)
{
	return CreateFunction<int(*)(Tox*, int32_t, const uint8_t*, uint32_t)>(__FUNCTION__)(tox, friendnumber, data, length);
}

/*int tox_group_message_send(Tox *tox, int groupnumber, const uint8_t *message, uint16_t length)
{
}

int tox_group_action_send(Tox *tox, int groupnumber, const uint8_t *action, uint16_t length)
{
}

int tox_group_set_title(Tox *tox, int groupnumber, const uint8_t *title, uint8_t length)
{
}*/

int tox_group_get_title(Tox *tox, int groupnumber, uint8_t *title, uint32_t max_length)
{
	return CreateFunction<int(*)(Tox*, int, uint8_t*, uint32_t)>(__FUNCTION__)(tox, groupnumber, title, max_length);
}

/*unsigned int tox_group_peernumber_is_ours(const Tox *tox, int groupnumber, int peernumber)
{
}

int tox_group_number_peers(const Tox *tox, int groupnumber)
{
}

int tox_group_get_names(const Tox *tox, int groupnumber, uint8_t names[][TOX_MAX_NAME_LENGTH], uint16_t lengths[], uint16_t length)
{
}*/

uint32_t tox_count_chatlist(const Tox *tox)
{
	return CreateFunction<int(*)(const Tox*)>(__FUNCTION__)(tox);
}

uint32_t tox_get_chatlist(const Tox *tox, int32_t *out_list, uint32_t list_size)
{
	return CreateFunction<int(*)(const Tox*, int32_t*, uint32_t)>(__FUNCTION__)(tox, out_list, list_size);
}

int tox_group_get_type(const Tox *tox, int groupnumber)
{
	return CreateFunction<int(*)(const Tox*, int)>(__FUNCTION__)(tox, groupnumber);
}