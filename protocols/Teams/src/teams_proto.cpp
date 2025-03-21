#include "stdafx.h"

CTeamsProto::CTeamsProto(const char *protoName, const wchar_t *userName) :
	PROTO<CTeamsProto>(protoName, userName)
{
}

CTeamsProto::~CTeamsProto()
{
}
