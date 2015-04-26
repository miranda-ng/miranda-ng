#include "stdafx.h"

/* COMMON A/V FUNCTIONS */

ToxAv *toxav_new(Tox *tox, int32_t max_calls)
{
	return CreateFunction<ToxAv*(*)(Tox*, int32_t)>(__FUNCTION__)(tox, max_calls);
}

void toxav_kill(ToxAv *av)
{
	CreateFunction<void(*)(ToxAv*)>(__FUNCTION__)(av);
}

uint32_t toxav_do_interval(ToxAv *av)
{
	return CreateFunction<uint32_t(*)(ToxAv*)>(__FUNCTION__)(av);
}

void toxav_do(ToxAv *av)
{
	CreateFunction<void(*)(ToxAv*)>(__FUNCTION__)(av);
}

void toxav_register_callstate_callback(ToxAv *av, ToxAVCallback cb, ToxAvCallbackID id, void *userdata)
{
	CreateFunction<void(*)(ToxAv*, ToxAVCallback, ToxAvCallbackID, void*)>(__FUNCTION__)(av, cb, id, userdata);
}

int toxav_call(ToxAv *av, int32_t *call_index, int friend_id, const ToxAvCSettings *csettings, 	int ringing_seconds)
{
	return CreateFunction<int(*)(ToxAv*, int32_t*, int, const ToxAvCSettings*, int)>(__FUNCTION__)(av, call_index, friend_id, csettings, ringing_seconds);
}

int toxav_hangup(ToxAv *av, int32_t call_index)
{
	return CreateFunction<int(*)(ToxAv*, int32_t)>(__FUNCTION__)(av, call_index);
}

int toxav_answer(ToxAv *av, int32_t call_index, const ToxAvCSettings *csettings)
{
	return CreateFunction<int(*)(ToxAv*, int32_t, const ToxAvCSettings*)>(__FUNCTION__)(av, call_index, csettings);
}

int toxav_reject(ToxAv *av, int32_t call_index, const char *reason)
{
	return CreateFunction<int(*)(ToxAv*, int32_t, const char*)>(__FUNCTION__)(av, call_index, reason);
}

int toxav_cancel(ToxAv *av, int32_t call_index, int peer_id, const char *reason)
{
	return CreateFunction<int(*)(ToxAv*, int32_t, int, const char*)>(__FUNCTION__)(av, call_index, peer_id, reason);
}

int toxav_change_settings(ToxAv *av, int32_t call_index, const ToxAvCSettings *csettings)
{
	return CreateFunction<int(*)(ToxAv*, int32_t, const ToxAvCSettings*)>(__FUNCTION__)(av, call_index, csettings);
}

int toxav_stop_call(ToxAv *av, int32_t call_index)
{
	return CreateFunction<int(*)(ToxAv*, int32_t)>(__FUNCTION__)(av, call_index);
}

int toxav_prepare_transmission(ToxAv *av, int32_t call_index, int support_video)
{
	return CreateFunction<int(*)(ToxAv*, int32_t, int)>(__FUNCTION__)(av, call_index, support_video);
}

int toxav_kill_transmission(ToxAv *av, int32_t call_index)
{
	return CreateFunction<int(*)(ToxAv*, int32_t)>(__FUNCTION__)(av, call_index);
}

int toxav_get_peer_csettings(ToxAv *av, int32_t call_index, int peer, ToxAvCSettings *dest)
{
	return CreateFunction<int(*)(ToxAv*, int32_t, int, ToxAvCSettings*)>(__FUNCTION__)(av, call_index, peer, dest);
}

int toxav_get_peer_id(ToxAv *av, int32_t call_index, int peer)
{
	return CreateFunction<int(*)(ToxAv*, int32_t, int)>(__FUNCTION__)(av, call_index, peer);
}

ToxAvCallState toxav_get_call_state(ToxAv *av, int32_t call_index)
{
	return CreateFunction<ToxAvCallState(*)(ToxAv*, int32_t)>(__FUNCTION__)(av, call_index);
}

int toxav_capability_supported(ToxAv *av, int32_t call_index, ToxAvCapabilities capability)
{
	return CreateFunction<int(*)(ToxAv*, int32_t, ToxAvCapabilities)>(__FUNCTION__)(av, call_index, capability);
}

Tox *toxav_get_tox(ToxAv *av)
{
	return CreateFunction<Tox*(*)(ToxAv*)>(__FUNCTION__)(av);
}

int toxav_get_active_count(ToxAv *av)
{
	return CreateFunction<int(*)(ToxAv*)>(__FUNCTION__)(av);
}

/* AUDIO FUNCTIONS */

void toxav_register_audio_callback(ToxAv *av, ToxAvAudioCallback cb, void *userdata)
{
	CreateFunction<void(*)(ToxAv*, ToxAvAudioCallback, void*)>(__FUNCTION__)(av, cb, userdata);
}