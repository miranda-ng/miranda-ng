#ifndef csocket_h__
#define csocket_h__

class CSocket
{
protected:
	SOCKET m_socket;

public:
	CSocket(SOCKET socket = INVALID_SOCKET): m_socket(socket) {}
	int Recv(char *buf, int count);
	int Send(char *buf, int count = -1);
	void Close();
};

#endif // csocket_h__
