#include "stdafx.h"

CMTProto::CMTProto(const char* protoName, const wchar_t* userName) :
	PROTO<CMTProto>(protoName, userName),
	client_manager_(std::make_unique<td::ClientManager>())
{
}

CMTProto::~CMTProto()
{
}

INT_PTR CMTProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)"Phone";
	}
	return 0;
}

int CMTProto::SetStatus(int iNewStatus)
{
	return 0;
}
