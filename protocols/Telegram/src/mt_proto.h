#pragma once

struct CMTProto : public PROTO<CMTProto>
{
	std::unique_ptr<td::ClientManager> client_manager_;

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
