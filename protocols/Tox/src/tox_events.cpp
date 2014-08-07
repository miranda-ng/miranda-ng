#include "common.h"

void CToxProto::OnFriendRequest(Tox *tox, const uint8_t *userId, const uint8_t *message, const uint16_t messageSize, void *arg)
{
}

void CToxProto::OnFriendMessage(Tox *tox, const int friendId, const uint8_t *message, const uint16_t messageSize, void *arg)
{
}

void CToxProto::OnFriendNameChange(Tox *tox, const int friendId, const uint8_t *name, const uint16_t nameSize, void *arg)
{
}

void CToxProto::OnStatusMessageChanged(Tox *tox, const int friendId, const uint8_t* message, const uint16_t messageSize, void *arg)
{
}

void CToxProto::OnUserStatusChanged(Tox *tox, int32_t friendnumber, uint8_t TOX_USERSTATUS, void *userdata)
{
}

void CToxProto::OnConnectionStatusChanged(Tox *tox, const int friendId, const uint8_t status, void *arg)
{
}

void CToxProto::OnAction(Tox *tox, const int friendId, const uint8_t *message, const uint16_t messageSize, void *arg)
{
}