#include "stdafx.h"

/* COMMON A/V FUNCTIONS */

ToxAV *toxav_new(Tox *tox, TOXAV_ERR_NEW *error)
{
	return CreateFunction<ToxAV*(*)(Tox*, TOXAV_ERR_NEW*)>(__FUNCTION__)(tox, error);
}

void toxav_kill(ToxAV *toxAV)
{
	CreateFunction<void(*)(ToxAV*)>(__FUNCTION__)(toxAV);
}

Tox *toxav_get_tox(const ToxAV *toxAV)
{
	return CreateFunction<Tox*(*)(const ToxAV*)>(__FUNCTION__)(toxAV);
}

uint32_t toxav_iteration_interval(ToxAV *toxAV)
{
	return CreateFunction<uint32_t(*)(ToxAV*)>(__FUNCTION__)(toxAV);
}

void toxav_iterate(ToxAV *toxAV)
{
	CreateFunction<void(*)(ToxAV*)>(__FUNCTION__)(toxAV);
}

bool toxav_call(ToxAV *toxAV, uint32_t friend_number, uint32_t audio_bit_rate, uint32_t video_bit_rate, TOXAV_ERR_CALL *error)
{
	return CreateFunction<bool(*)(ToxAV*, int32_t, uint32_t, uint32_t, TOXAV_ERR_CALL*)>(__FUNCTION__)(toxAV, friend_number, audio_bit_rate, video_bit_rate, error);
}

void toxav_callback_call(ToxAV *toxAV, toxav_call_cb *callback, void *user_data)
{
	CreateFunction<void(*)(ToxAV*, toxav_call_cb, void*)>(__FUNCTION__)(toxAV, callback, user_data);
}

bool toxav_answer(ToxAV *toxAV, uint32_t friend_number, uint32_t audio_bit_rate, uint32_t video_bit_rate, TOXAV_ERR_ANSWER *error)
{
	return CreateFunction<bool(*)(ToxAV*, int32_t, uint32_t, uint32_t, TOXAV_ERR_ANSWER*)>(__FUNCTION__)(toxAV, friend_number, audio_bit_rate, video_bit_rate, error);
}

void toxav_callback_call_state(ToxAV *toxAV, toxav_call_state_cb *callback, void *user_data)
{
	CreateFunction<void(*)(ToxAV*, toxav_call_state_cb, void*)>(__FUNCTION__)(toxAV, callback, user_data);
}

bool toxav_call_control(ToxAV *toxAV, uint32_t friend_number, TOXAV_CALL_CONTROL control, TOXAV_ERR_CALL_CONTROL *error)
{
	return CreateFunction<bool(*)(ToxAV*, uint32_t, TOXAV_CALL_CONTROL, TOXAV_ERR_CALL_CONTROL*)>(__FUNCTION__)(toxAV, friend_number, control, error);
}

bool toxav_bit_rate_set(ToxAV *toxAV, uint32_t friend_number, int32_t audio_bit_rate, int32_t video_bit_rate, TOXAV_ERR_BIT_RATE_SET *error)
{
	return CreateFunction<bool(*)(ToxAV*, int32_t, uint32_t, uint32_t, TOXAV_ERR_BIT_RATE_SET*)>(__FUNCTION__)(toxAV, friend_number, audio_bit_rate, video_bit_rate, error);
}

void toxav_callback_bit_rate_status(ToxAV *toxAV, toxav_bit_rate_status_cb *callback, void *user_data)
{
	CreateFunction<void(*)(ToxAV*, toxav_bit_rate_status_cb, void*)>(__FUNCTION__)(toxAV, callback, user_data);
}

bool toxav_audio_send_frame(ToxAV *toxAV, uint32_t friend_number, const int16_t *pcm, size_t sample_count, uint8_t channels, uint32_t sampling_rate, TOXAV_ERR_SEND_FRAME *error)
{
	return CreateFunction<bool(*)(ToxAV*, int32_t, const int16_t*, size_t, uint8_t, uint32_t, TOXAV_ERR_SEND_FRAME*)>(__FUNCTION__)(toxAV, friend_number, pcm, sample_count, channels, sampling_rate, error);
}

bool toxav_video_send_frame(ToxAV *toxAV, uint32_t friend_number, uint16_t width, uint16_t height, const uint8_t *y, const uint8_t *u, const uint8_t *v, TOXAV_ERR_SEND_FRAME *error)
{
	return CreateFunction<bool(*)(ToxAV*, int32_t, int16_t, uint16_t, const uint8_t*, const uint8_t*, const uint8_t*, TOXAV_ERR_SEND_FRAME*)>(__FUNCTION__)(toxAV, friend_number, width, height, y, u, v, error);
}

void toxav_callback_audio_receive_frame(ToxAV *toxAV, toxav_audio_receive_frame_cb *callback, void *user_data)
{
	CreateFunction<void(*)(ToxAV*, toxav_audio_receive_frame_cb, void*)>(__FUNCTION__)(toxAV, callback, user_data);
}

void toxav_callback_video_receive_frame(ToxAV *toxAV, toxav_video_receive_frame_cb *callback, void *user_data)
{
	CreateFunction<void(*)(ToxAV*, toxav_video_receive_frame_cb, void*)>(__FUNCTION__)(toxAV, callback, user_data);
}

int toxav_add_av_groupchat(Tox *tox, void(*audio_callback)(void *, int, int, const int16_t *, unsigned int, uint8_t, unsigned int, void *), void *userdata)
{
	return CreateFunction<int(*)(Tox*, void(*)(void*, int, int, const int16_t*, unsigned int, uint8_t, unsigned int, void*), void*)>(__FUNCTION__)(tox, audio_callback, userdata);
}

int toxav_join_av_groupchat(Tox *tox, int32_t friendnumber, const uint8_t *data, uint16_t length, void(*audio_callback)(void *, int, int, const int16_t *, unsigned int, uint8_t, unsigned int, void *), void *userdata)
{
	return CreateFunction<int(*)(Tox*, int32_t, const uint8_t*, uint16_t, void(*)(void*, int, int, const int16_t*, unsigned int, uint8_t, unsigned int, void*), void*)>(__FUNCTION__)(tox, friendnumber, data, length, audio_callback, userdata);
}

int toxav_group_send_audio(Tox *tox, int groupnumber, const int16_t *pcm, unsigned int samples, uint8_t channels, unsigned int sample_rate)
{
	return CreateFunction<int(*)(Tox*, int, const int16_t*, unsigned int, uint8_t, unsigned int)>(__FUNCTION__)(tox, groupnumber, pcm, samples, channels, sample_rate);
}