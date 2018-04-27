#pragma once

struct CMTProto : public PROTO<CMTProto>
{
public:
	//////////////////////////////////////////////////////////////////////////////////////
	// Ctors

	CMTProto(const char *protoName, const wchar_t *userName);
	~CMTProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// Virtual functions

	INT_PTR GetCaps(int type, MCONTACT hContact = NULL) override;
	
	int SetStatus(int iNewStatus) override;
};
